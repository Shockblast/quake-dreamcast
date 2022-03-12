/*
Copyright (C) 1996-1997 Id Software, Inc.

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
// Z_zone.c

//#include "quakedef.h"
#include <stdio.h> /* for NULL */

typedef unsigned char byte;
typedef struct cache_user_s
{
	void	*data;
} cache_user_t;
typedef enum {false, true}	qboolean;

static	byte	*hunk_base;
static	int		hunk_size;

//static	int		hunk_low_used;
//static	int		hunk_high_used;
#define	hunk_low_used 0
#define	hunk_high_used 0


#define G2_LOCK(OLD) \
	do { \
		if (!irq_inside_int()) \
			OLD = irq_disable(); \
		/* suspend any G2 DMA here... */ \
		while((*(volatile long *)0xa05f688c) & 0x20) \
			; \
	} while(0)

#define G2_UNLOCK(OLD) \
	do { \
		/* resume any G2 DMA here... */ \
		if (!irq_inside_int()) \
			irq_restore(OLD); \
	} while(0)


/*
===============================================================================

CACHE MEMORY

===============================================================================
*/

typedef struct cache_system_s
{
	int						size;		// including this header
	cache_user_t			*user;
//	char					name[16];
	struct cache_system_s	*prev, *next;
	struct cache_system_s	*lru_prev, *lru_next;	// for LRU flushing	
} cache_system_t;

static cache_system_t *Cache_TryAlloc (int size, qboolean nobottom);

static cache_system_t	cache_head;

static void Cache_UnlinkLRU (cache_system_t *cs)
{
	if (!cs->lru_next || !cs->lru_prev)
		Sys_Error ("Cache_UnlinkLRU: NULL link");

	cs->lru_next->lru_prev = cs->lru_prev;
	cs->lru_prev->lru_next = cs->lru_next;
	
	cs->lru_prev = cs->lru_next = NULL;

	/* write 4 */
}

static void Cache_MakeLRU (cache_system_t *cs)
{
	if (cs->lru_next || cs->lru_prev)
		Sys_Error ("Cache_MakeLRU: active link");

	cache_head.lru_next->lru_prev = cs;
	cs->lru_next = cache_head.lru_next;
	cs->lru_prev = &cache_head;
	cache_head.lru_next = cs;

	/* write 3 */
}

#include <assert.h>
/*
============
Cache_TryAlloc

Looks for a free block of memory between the high and low hunk marks
Size should already include the header and padding
============
*/
static cache_system_t *Cache_TryAlloc (int size, qboolean nobottom)
{
	cache_system_t	*cs, *new;

// is the cache completely empty?

	if (!nobottom && cache_head.prev == &cache_head)
	{
		if (hunk_size - hunk_high_used - hunk_low_used < size)
			Sys_Error ("Cache_TryAlloc: %i is greater then free hunk", size);

	//	printf("first:\n");

		new = (cache_system_t *) (hunk_base + hunk_low_used);
	//	memset (new, 0, sizeof(*new));
		new->size = size;

		cache_head.prev = cache_head.next = new;
		new->prev = new->next = &cache_head;
		
		//Cache_MakeLRU (new);
		/*if (cs->lru_next || cs->lru_prev)
			Sys_Error ("Cache_MakeLRU: active link"); */

		cache_head.lru_next->lru_prev = new;
		new->lru_next = cache_head.lru_next;
		new->lru_prev = &cache_head;
		cache_head.lru_next = new;

		return new;
	}
	
// search from the bottom up for space

	new = (cache_system_t *) (hunk_base + hunk_low_used);
	cs = cache_head.next;
	
	do
	{
		if (!nobottom || cs != cache_head.next)
		{
			if ( (byte *)cs - (byte *)new >= size)
			{	// found space
			//	printf("mid:\n");
				goto found;
			}
		}

	// continue looking		
		new = (cache_system_t *)((byte *)cs + cs->size);
	//	printf("new:%p %p %d\n",new,cs,cs->size);
		cs = cs->next;

	} while (cs != &cache_head);
	
// try to allocate one at the very end
	assert(cs==&cache_head);
	if ( hunk_base + hunk_size - hunk_high_used - (byte *)new < size) return NULL; // couldn't allocate
	//printf("last\n");

found:
	assert((byte*)new+size <= hunk_base + hunk_size);
	{
	//	memset (new, 0, sizeof(*new));
		new->size = size;
		
		new->next = cs;
		new->prev = cs->prev;
		cs->prev->next = new;
		cs->prev = new;
		
		// Cache_MakeLRU (new);
		/* if (cs->lru_next || cs->lru_prev)
			Sys_Error ("Cache_MakeLRU: active link"); */

		cache_head.lru_next->lru_prev = new;
		new->lru_next = cache_head.lru_next;
		new->lru_prev = &cache_head;
		cache_head.lru_next = new;

		return new;
	}
}

#if 0
/*
============
Cache_Flush

Throw everything out, so new data will be demand cached
============
*/
void Cache_Flush (void)
{
	while (cache_head.next != &cache_head)
		Cache_Free ( cache_head.next->user );	// reclaim the space
}


/*
============
Cache_Print

============
*/
void Cache_Print (void)
{
	cache_system_t	*cd;

	for (cd = cache_head.next ; cd != &cache_head ; cd = cd->next)
	{
		Con_Printf ("%8i : %s\n", cd->size, cd->name);
	}
}

/*
============
Cache_Report

============
*/
void Cache_Report (void)
{
	Con_DPrintf ("%4.1f megabyte data cache\n", (hunk_size - hunk_high_used - hunk_low_used) / (float)(1024*1024) );
}

/*
============
Cache_Compact

============
*/
void Cache_Compact (void)
{
}
#endif

/*
============
Cache_Init

============
*/
static void Cache_Init (void)
{
	cache_head.next = cache_head.prev = &cache_head;
	cache_head.lru_next = cache_head.lru_prev = &cache_head;

//	Cmd_AddCommand ("flush", Cache_Flush);
}

/*
==============
Cache_Free

Frees the memory and removes it from the LRU list
==============
*/
static void Cache_Free (cache_user_t *c)
{
	cache_system_t	*cs;

	if (!c->data)
		Sys_Error ("Cache_Free: not allocated");

	cs = ((cache_system_t *)c->data) - 1;

	cs->prev->next = cs->next;
	cs->next->prev = cs->prev;
	cs->next = cs->prev = NULL;

	c->data = NULL;

	Cache_UnlinkLRU (cs);
}



/*
==============
Cache_Check
==============
*/
void *SNDCache_Check (cache_user_t *c)
{
	cache_system_t	*cs;
	int old;

	if (!c->data)
		return NULL;

	cs = ((cache_system_t *)c->data) - 1;

// move to head of LRU
	G2_LOCK(old);

	/* Cache_UnlinkLRU (cs); */
	if (!cs->lru_next || !cs->lru_prev)
		Sys_Error ("Cache_UnlinkLRU: NULL link");

	cs->lru_next->lru_prev = cs->lru_prev;
	cs->lru_prev->lru_next = cs->lru_next;
	
	/* cs->lru_prev = cs->lru_next = NULL; */

	/* Cache_MakeLRU (cs); */
	/* if (cs->lru_next || cs->lru_prev)
		Sys_Error ("Cache_MakeLRU: active link"); */

	cache_head.lru_next->lru_prev = cs;
	cs->lru_next = cache_head.lru_next;
	cs->lru_prev = &cache_head;
	cache_head.lru_next = cs;


	G2_UNLOCK(old);

	g2_fifo_wait();
	
	return c->data;
}


/*
==============
Cache_Alloc
==============
*/
void *SNDCache_Alloc (cache_user_t *c, int size, char *name)
{
	cache_system_t	*cs;
	int old;

	if (c->data)
		Sys_Error ("Cache_Alloc: allready allocated");
	
	if (size <= 0)
		Sys_Error ("Cache_Alloc: size %i", size);

	size = (size + sizeof(cache_system_t) + 3) & ~3;

	printf("sndcache_alloc:%d\n",size);

// find memory for it	
	while (1)
	{
		G2_LOCK(old);
		cs = Cache_TryAlloc (size, false);
		if (cs)
		{
		//	strncpy (cs->name, name, sizeof(cs->name)-1);
			c->data = (void *)(cs+1);
			cs->user = c;
			G2_UNLOCK(old);
			g2_fifo_wait();
			return c->data;
		}
		G2_UNLOCK(old);
		g2_fifo_wait();
		
	
	// free the least recently used cahedat
		if (cache_head.lru_prev == &cache_head)
			Sys_Error ("Cache_Alloc: out of memory");
													// not enough memory at all
		G2_LOCK(old);
		Cache_Free ( cache_head.lru_prev->user );
		G2_UNLOCK(old);
		g2_fifo_wait();
	}
}

//============================================================================


/*
========================
Memory_Init
========================
*/
void SNDCache_Init (void *buf, int size)
{
	int p;
	int zonesize;

	hunk_base = buf;
	hunk_size = size;
//	hunk_low_used = 0;
//	hunk_high_used = 0;
	
	Cache_Init ();
#if 0
	zonesize = DYNAMIC_SIZE;
	p = COM_CheckParm ("-zone");
	if (p)
	{
		if (p < com_argc-1)
			zonesize = Q_atoi (com_argv[p+1]) * 1024;
		else
			Sys_Error ("Memory_Init: you must specify a size in KB after -zone");
	}
	mainzone = Hunk_AllocName (zonesize, "zone" );
	Z_ClearZone (mainzone, zonesize);
#endif
}

