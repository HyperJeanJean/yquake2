/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// cl_newfx.c -- MORE entity effects parsing and management

#include "header/client.h"

extern cparticle_t	*active_particles, *free_particles;
extern cparticle_t	particles[MAX_PARTICLES];
extern int			cl_numparticles;
extern cvar_t		*vid_ref;

extern void MakeNormalVectors (vec3_t forward, vec3_t right, vec3_t up);

/*
======
vectoangles2 - this is duplicated in the game DLL, but I need it here.
======
*/
void vectoangles2 (vec3_t value1, vec3_t angles)
{
	float	forward;
	float	yaw, pitch;
	
	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
	// PMM - fixed to correct for pitch of 0
		if (value1[0])
			yaw = ((float)atan2(value1[1], value1[0]) * 180 / M_PI);
		else if (value1[1] > 0)
			yaw = 90;
		else
			yaw = 270;

		if (yaw < 0)
			yaw += 360;

		forward = (float)sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = ((float)atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

//=============
//=============
void CL_Flashlight (int ent, vec3_t pos)
{
	cdlight_t	*dl;

	dl = CL_AllocDlight (ent);
	VectorCopy (pos,  dl->origin);
	dl->radius = 400;
	dl->minlight = 250;
	dl->die = cl.time + 100;
	dl->color[0] = 1;
	dl->color[1] = 1;
	dl->color[2] = 1;
}

/*
======
CL_ColorFlash - flash of light
======
*/
void CL_ColorFlash (vec3_t pos, int ent, float intensity, float r, float g, float b)
{
	cdlight_t	*dl;

	if((vidref_val == VIDREF_SOFT) && ((r < 0) || (g<0) || (b<0)))
	{
		intensity = -intensity;
		r = -r;
		g = -g;
		b = -b;
	}

	dl = CL_AllocDlight (ent);
	VectorCopy (pos,  dl->origin);
	dl->radius = intensity;
	dl->minlight = 250;
	dl->die = cl.time + 100;
	dl->color[0] = r;
	dl->color[1] = g;
	dl->color[2] = b;
}

/*
======
CL_DebugTrail
======
*/
void CL_DebugTrail (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	cparticle_t	*p;
	float		dec;
	vec3_t		right, up;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	MakeNormalVectors (vec, right, up);

	dec = 3;
	VectorScale (vec, dec, vec);
	VectorCopy (start, move);

	while (len > 0)
	{
		len -= dec;
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = (float)cl.time;
		VectorClear (p->accel);
		VectorClear (p->vel);
		p->alpha = 1.0;
		p->alphavel = -0.1f;
		p->color = 0x74 + (rand()&7);
		VectorCopy (move, p->org);
		VectorAdd (move, vec, move);
	}

}

/*
===============
CL_SmokeTrail
===============
*/
void CL_SmokeTrail (vec3_t start, vec3_t end, int colorStart, int colorRun, int spacing)
{
	vec3_t		move;
	vec3_t		vec;
	float		len, time;
	int			j;
	cparticle_t	*p;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	VectorScale (vec, spacing, vec);

	time = (float)cl.time;

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= spacing;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = time;

		p->alpha = 1.0;
		p->alphavel = -1.0f / (1+frand()*0.5f);
		p->color = colorStart + (float)(rand() % colorRun);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = move[j] + crand()*3;
			p->accel[j] = 0;
		}
		p->vel[2] = 20 + crand()*5;

		VectorAdd (move, vec, move);
	}
}

void CL_ForceWall (vec3_t start, vec3_t end, int color8)
{
	vec3_t		move;
	vec3_t		vec;
	int			j;
	cparticle_t	*p;

	float		len, time;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	VectorScale (vec, 4, vec);

	time = (float)cl.time;

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= 4;

		if (!free_particles)
			return;
		
		if (frand() > 0.3)
		{
		  p = free_particles;
		  free_particles = p->next;
		  p->next = active_particles;
		  active_particles = p;
		  VectorClear (p->accel);
		  
		  p->time = time;
		  
		  p->alpha = 1.0;
		  p->alphavel =  -1.0f / (3.0+frand()*0.5f);
		  p->color = color8;
		  for (j=0 ; j<3 ; j++)
		    {
		      p->org[j] = move[j] + crand()*3;
		      p->accel[j] = 0;
		    }
		  p->vel[0] = 0;
		  p->vel[1] = 0;
		  p->vel[2] = -40 - (crand()*10);
		}

		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_GenericParticleEffect
===============
*/
void CL_GenericParticleEffect (vec3_t org, vec3_t dir, int color, int count, int numcolors, int dirspread, float alphavel)
{
	int			i, j;
	cparticle_t	*p;
	float		d;
	float		time;
	time = (float)cl.time;

	for (i=0 ; i<count ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = time;
		if (numcolors > 1)
		  p->color = color + (rand() & numcolors);
		else
		  p->color = color;
		d = (float)(rand() & dirspread);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand()&7)-4) + d*dir[j];
			p->vel[j] = crand()*20;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -1.0f / (0.5f + frand()*alphavel);
	}
}

/*
===============
CL_BubbleTrail2 (lets you control the # of bubbles by setting the distance between the spawns)

===============
*/
void CL_BubbleTrail2 (vec3_t start, vec3_t end, int dist)
{
	vec3_t		move;
	vec3_t		vec;
	float		len, time;
	int			i;
	int j;
	cparticle_t	*p;

	time = (float)cl.time;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	VectorScale (vec, dist, vec);

	for (i=0 ; i<len ; i+=dist)
	{
		if (!free_particles)
		  return;
		
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		VectorClear (p->accel);
		p->time = time;

		p->alpha = 1.0;
		p->alphavel = -1.0f / (1+frand()*0.1f);
		p->color = 4 + (rand()&7);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = move[j] + crand()*2;
			p->vel[j] = crand()*10;
		}
		p->org[2] -= 4;
		p->vel[2] += 20;
		VectorAdd (move, vec, move);
	}
}

void CL_Heatbeam (vec3_t start, vec3_t forward)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			j;
	cparticle_t	*p;
	vec3_t		right, up;
	float		i;
	float		c, s;
	vec3_t		dir;
	float		ltime;
	float		step = 32.0, rstep;
	float		start_pt;
	float		rot;
	float		variance;
	float		time;
	vec3_t		end;

	VectorMA (start, 4096, forward, end);

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	// FIXME - pmm - these might end up using old values?
	VectorCopy (cl.v_right, right);
	VectorCopy (cl.v_up, up);
	if (vidref_val == VIDREF_GL)
	{ // GL mode
		VectorMA (move, -0.5, right, move);
		VectorMA (move, -0.5, up, move);
	}
	// otherwise assume SOFT

	time = (float)cl.time;

    ltime = (float) cl.time/1000.0f;
	start_pt = (float)fmod(ltime*96.0f,step);
	VectorMA (move, start_pt, vec, move);

	VectorScale (vec, step, vec);

	rstep = M_PI/10.0f;
	for (i=start_pt ; i<len ; i+=step)
	{
		if (i>step*5) // don't bother after the 5th ring
			break;

		for (rot = 0; rot < M_PI*2; rot += rstep)
		{

			if (!free_particles)
				return;

			p = free_particles;
			free_particles = p->next;
			p->next = active_particles;
			active_particles = p;
			
			p->time = time;
			VectorClear (p->accel);
			variance = 0.5;
			c = (float)cos(rot)*variance;
			s = (float)sin(rot)*variance;
			
			// trim it so it looks like it's starting at the origin
			if (i < 10)
			{
				VectorScale (right, c*(i/10.0f), dir);
				VectorMA (dir, s*(i/10.0f), up, dir);
			}
			else
			{
				VectorScale (right, c, dir);
				VectorMA (dir, s, up, dir);
			}
		
			p->alpha = 0.5;
			p->alphavel = -1000.0;
			p->color = 223 - (rand()&7);
			for (j=0 ; j<3 ; j++)
			{
				p->org[j] = move[j] + dir[j]*3;
				p->vel[j] = 0;
			}
		}
		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_ParticleSteamEffect

Puffs with velocity along direction, with some randomness thrown in
===============
*/
void CL_ParticleSteamEffect (vec3_t org, vec3_t dir, int color, int count, int magnitude)
{
	int			i, j;
	cparticle_t	*p;
	float		d, time;
	vec3_t		r, u;

	time = (float)cl.time;
	MakeNormalVectors (dir, r, u);

	for (i=0 ; i<count ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = time;
		p->color = color + (rand()&7);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + magnitude*0.1f*crand();
		}
		VectorScale (dir, magnitude, p->vel);
		d = crand()*magnitude/3;
		VectorMA (p->vel, d, r, p->vel);
		d = crand()*magnitude/3;
		VectorMA (p->vel, d, u, p->vel);

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY/2;
		p->alpha = 1.0;

		p->alphavel = -1.0f / (0.5f + frand()*0.3f);
	}
}

void CL_ParticleSteamEffect2 (cl_sustain_t *self)
{
	int			i, j;
	cparticle_t	*p;
	float		d;
	vec3_t		r, u;
	vec3_t		dir;

	VectorCopy (self->dir, dir);
	MakeNormalVectors (dir, r, u);

	for (i=0 ; i<self->count ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = cl.time;
		p->color = self->color + (rand()&7);

		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = self->org[j] + self->magnitude*0.1*crand();
		}
		VectorScale (dir, self->magnitude, p->vel);
		d = crand()*self->magnitude/3;
		VectorMA (p->vel, d, r, p->vel);
		d = crand()*self->magnitude/3;
		VectorMA (p->vel, d, u, p->vel);

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY/2;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (0.5 + frand()*0.3);
	}
	self->nextthink += self->thinkinterval;
}

/*
===============
CL_TrackerTrail
===============
*/
void CL_TrackerTrail (vec3_t start, vec3_t end, int particleColor)
{
	vec3_t		move;
	vec3_t		vec;
	vec3_t		forward,right,up,angle_dir;
	float		len;
	int			j;
	cparticle_t	*p;
	int			dec;
	float		dist;
	float		time;

	time = (float)cl.time;
	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	VectorCopy(vec, forward);
	vectoangles2 (forward, angle_dir);
	AngleVectors (angle_dir, forward, right, up);

	dec = 3;
	VectorScale (vec, 3, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = time;

		p->alpha = 1.0;
		p->alphavel = -2.0;
		p->color = particleColor;
		dist = DotProduct(move, forward);
		VectorMA(move, 8 * cos(dist), up, p->org);
		for (j=0 ; j<3 ; j++)
		{
			p->vel[j] = 0;
			p->accel[j] = 0;
		}
		p->vel[2] = 5;

		VectorAdd (move, vec, move);
	}
}

void CL_Tracker_Shell(vec3_t origin)
{
	vec3_t			dir;
	int				i;
	cparticle_t		*p;
	float			time;

	time = (float)cl.time;

	for(i=0;i<300;i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = time;

		p->alpha = 1.0;
		p->alphavel = INSTANT_PARTICLE;
		p->color = 0;
		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
	
		VectorMA(origin, 40, dir, p->org);
	}
}

void CL_MonsterPlasma_Shell(vec3_t origin)
{
	vec3_t			dir;
	int				i;
	cparticle_t		*p;
	float			time;

	time = (float)cl.time;

	for(i=0;i<40;i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = time;

		p->alpha = 1.0;
		p->alphavel = INSTANT_PARTICLE;
		p->color = 0xe0;
		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
	
		VectorMA(origin, 10, dir, p->org);
	}
}

void CL_Widowbeamout (cl_sustain_t *self)
{
	vec3_t			dir;
	int				i;
	cparticle_t		*p;
	static int colortable[4] = {2*8,13*8,21*8,18*8};
	float			ratio;
	float			time;

	ratio = 1.0f - (((float)self->endtime - (float)cl.time)/2100.0f);
	time = (float)cl.time;

	for(i=0;i<300;i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = time;

		p->alpha = 1.0;
		p->alphavel = INSTANT_PARTICLE;
		p->color = colortable[rand()&3];
		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
	
		VectorMA(self->org, (45.0 * ratio), dir, p->org);
	}
}

void CL_Nukeblast (cl_sustain_t *self)
{
	vec3_t			dir;
	int				i;
	cparticle_t		*p;
	static int colortable[4] = {110, 112, 114, 116};
	float			ratio;
	float			time;

	ratio = 1.0f - (((float)self->endtime - (float)cl.time)/1000.0f);
	time = (float)cl.time;

	for(i=0;i<700;i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = time;

		p->alpha = 1.0;
		p->alphavel = INSTANT_PARTICLE;
		p->color = colortable[rand()&3];
		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
	
		VectorMA(self->org, (200.0 * ratio), dir, p->org);
	}
}

void CL_WidowSplash (vec3_t org)
{
	static int colortable[4] = {2*8,13*8,21*8,18*8};
	int			i;
	cparticle_t	*p;
	vec3_t		dir;
	float		time;

	time = (float)cl.time;

	for (i=0 ; i<256 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = time;
		p->color = colortable[rand()&3];
		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
		VectorMA(org, 45.0, dir, p->org);
		VectorMA(vec3_origin, 40.0, dir, p->vel);

		p->accel[0] = p->accel[1] = 0;
		p->alpha = 1.0;

		p->alphavel = -0.8f / (0.5f + frand()*0.3f);
	}

}

void CL_Tracker_Explode(vec3_t	origin)
{
	vec3_t			dir, backdir;
	int				i;
	cparticle_t		*p;
	float			time;

	time = (float)cl.time;

	for(i=0;i<300;i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = time;

		p->alpha = 1.0;
		p->alphavel = -1.0;
		p->color = 0;
		dir[0] = crand();
		dir[1] = crand();
		dir[2] = crand();
		VectorNormalize(dir);
		VectorScale(dir, -1, backdir);
	
		VectorMA(origin, 64, dir, p->org);
		VectorScale(backdir, 64, p->vel);
	}
	
}

/*
===============
CL_TagTrail

===============
*/
void CL_TagTrail (vec3_t start, vec3_t end, int color)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			j;
	cparticle_t	*p;
	int			dec;
	float		time;

	time = (float)cl.time;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	while (len >= 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = time;

		p->alpha = 1.0;
		p->alphavel = -1.0f / (0.8f+frand()*0.2f);
		p->color = color;
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = move[j] + crand()*16;
			p->vel[j] = crand()*5;
			p->accel[j] = 0;
		}

		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_ColorExplosionParticles
===============
*/
void CL_ColorExplosionParticles (vec3_t org, int color, int run)
{
	int			i;
	int j;
	cparticle_t	*p;
	float		time;

	time = (float)cl.time;

	for (i=0 ; i<128 ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = time;
		p->color = color + (rand() % run);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand()%32)-16);
			p->vel[j] = (rand()%256)-128;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -0.4f / (0.6f + frand()*0.2f);
	}
}

/*
===============
CL_ParticleSmokeEffect - like the steam effect, but unaffected by gravity
===============
*/
void CL_ParticleSmokeEffect (vec3_t org, vec3_t dir, int color, int count, int magnitude)
{
	int			i, j;
	cparticle_t	*p;
	float		d;
	vec3_t		r, u;
	float		time;

	time = (float)cl.time;

	MakeNormalVectors (dir, r, u);

	for (i=0 ; i<count ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = time;
		p->color = color + (rand()&7);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + magnitude*0.1f*crand();
		}
		VectorScale (dir, magnitude, p->vel);
		d = crand()*magnitude/3;
		VectorMA (p->vel, d, r, p->vel);
		d = crand()*magnitude/3;
		VectorMA (p->vel, d, u, p->vel);

		p->accel[0] = p->accel[1] = p->accel[2] = 0;
		p->alpha = 1.0;

		p->alphavel = -1.0f / (0.5f + frand()*0.3f);
	}
}

/*
===============
CL_BlasterParticles2

Wall impact puffs (Green)
===============
*/
void CL_BlasterParticles2 (vec3_t org, vec3_t dir, unsigned int color)
{
	int			i, j;
	cparticle_t	*p;
	float		d;
	int			count;
	float		time;
	
	time = (float)cl.time;

	count = 40;
	for (i=0 ; i<count ; i++)
	{
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = time;
		p->color = color + (rand()&7);
		d = (float)(rand()&15);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand()&7)-4) + d*dir[j];
			p->vel[j] = dir[j] * 30 + crand()*40;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -1.0f / (0.5f + frand()*0.3f);
	}
}

/*
===============
CL_BlasterTrail2

Green!
===============
*/
void CL_BlasterTrail2 (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			j;
	cparticle_t	*p;
	int			dec;
	float		time;

	time = (float)cl.time;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		
		p->time = time;

		p->alpha = 1.0;
		p->alphavel = -1.0f / (float)(0.3f+frand()*0.2f);
		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = move[j] + crand();
			p->vel[j] = crand()*5;
			p->accel[j] = 0;
		}

		VectorAdd (move, vec, move);
	}
}
