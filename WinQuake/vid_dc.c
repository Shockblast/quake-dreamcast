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
// vid_null.c -- null video driver to aid porting efforts

#include "quakedef.h"
#include "d_local.h"
#include <dc/sq.h>

viddef_t	vid;				// global video state

#define	BASEWIDTH	320
#define	BASEHEIGHT	240


static int		VID_highhunkmark;

//byte	vid_buffer[BASEWIDTH*BASEHEIGHT];
//short	zbuffer[BASEWIDTH*BASEHEIGHT];
//byte	surfcache[256*1024];

unsigned short	d_8to16table[256];
//unsigned	d_8to24table[256];
static unsigned short palette[256];


#define	RGB565(r,g,b)	( (((r)>>3)<<11) |(((g)>>2)<<5)|((b)>>3) )

void	VID_SetPalette (unsigned char *pal)
{
	int i;
	for(i=0;i<256;i++) {
		palette[i] = RGB565(pal[0],pal[1],pal[2]);
		pal+=3;
	}
}

void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette(palette);
}

#define PM_RGB555	0
#define PM_RGB565	1
#define PM_RGB888	3
#define DM_320x240	1

void	VID_Init (unsigned char *palette)
{
	int bsize, zsize, tsize;
	byte	*vid_surfcache;

	vid.maxwarpwidth = vid.width = vid.conwidth = BASEWIDTH;
	vid.maxwarpheight = vid.height = vid.conheight = BASEHEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
//	vid.buffer = vid.conbuffer = vid_buffer;
	vid.rowbytes = vid.conrowbytes = BASEWIDTH;
	
	if (d_pzbuffer) {
		D_FlushCaches();
		Hunk_FreeToHighMark (VID_highhunkmark);
		d_pzbuffer = NULL;
		vid_surfcache = NULL;
	}

	bsize = vid.rowbytes * vid.height;
	tsize = D_SurfaceCacheForRes (vid.width, vid.height);
	zsize = vid.width * vid.height * sizeof(*d_pzbuffer);

	VID_highhunkmark = Hunk_HighMark ();

	d_pzbuffer = Hunk_HighAllocName (bsize+tsize+zsize, "video");

	vid_surfcache = ((byte *)d_pzbuffer) + zsize;

	vid.conbuffer = vid.buffer = (pixel_t *)(((byte *)d_pzbuffer) + zsize + tsize);

	D_InitCaches (vid_surfcache, tsize);

//	d_pzbuffer = zbuffer;
//	D_InitCaches (surfcache, sizeof(surfcache));

	vid_set_mode(DM_320x240, PM_RGB565);
	VID_SetPalette(palette);
}

void	VID_Shutdown (void)
{
}

static void sq_update(unsigned short *dest,unsigned char *s,unsigned short *pal,int n)
{
	unsigned int *d = (unsigned int *)(void *)
		(0xe0000000 | (((unsigned long)dest) & 0x03ffffe0));
    
	/* Set store queue memory area as desired */
	QACR0 = ((((unsigned int)dest)>>26)<<2)&0x1c;
	QACR1 = ((((unsigned int)dest)>>26)<<2)&0x1c;
    
	/* fill/write queues as many times necessary */
	n>>=5;
	while(n--) {
		int v;
#if 1
		v = pal[*s++];
		v|= pal[*s++]<<16;
		d[0] = v;
		v = pal[*s++];
		v|= pal[*s++]<<16;
		d[1] = v;
		v = pal[*s++];
		v|= pal[*s++]<<16;
		d[2] = v;
		v = pal[*s++];
		v|= pal[*s++]<<16;
		d[3] = v;
		v = pal[*s++];
		v|= pal[*s++]<<16;
		d[4] = v;
		v = pal[*s++];
		v|= pal[*s++]<<16;
		d[5] = v;
		v = pal[*s++];
		v|= pal[*s++]<<16;
		d[6] = v;
		v = pal[*s++];
		v|= pal[*s++]<<16;
		d[7] = v;
#else
		v = *s++;
		d[0] = pal[v&255] | (pal[v>>8]<<16);
		v = *s++;
		d[1] = pal[v&255] | (pal[v>>8]<<16);
		v = *s++;
		d[2] = pal[v&255] | (pal[v>>8]<<16);
		v = *s++;
		d[3] = pal[v&255] | (pal[v>>8]<<16);
		v = *s++;
		d[4] = pal[v&255] | (pal[v>>8]<<16);
		v = *s++;
		d[5] = pal[v&255] | (pal[v>>8]<<16);
		v = *s++;
		d[6] = pal[v&255] | (pal[v>>8]<<16);
		v = *s++;
		d[7] = pal[v&255] | (pal[v>>8]<<16);
#endif
		asm("pref @%0" : : "r" (d));
		d += 8;
	}
	/* Wait for both store queues to complete */
	d = (unsigned int *)0xe0000000;
	d[0] = d[8] = 0;
}

extern unsigned short *vram_s;

void	VID_Update (vrect_t *rects)
{
	int i;
#if 1
	sq_update(vram_s,vid.buffer,palette,BASEWIDTH*BASEHEIGHT*2);
#else
	unsigned char *in = vid.buffer;
	unsigned short *out = vram_s;
	for(i=0;i<BASEWIDTH*BASEHEIGHT;i++) {
		out[i] = palette[in[i]];
	}
#endif
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
	int i,j;
	unsigned short *d = vram_s + BASEWIDTH*y + x;
	for(i=0;i<height;i++) {
		for(j=0;j<width;j++) {
			d[j] = palette[*pbitmap++];
		}
		d+=BASEWIDTH;
	}
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
	int i,j;
	unsigned short *d = vram_s + BASEWIDTH*y + x;
	unsigned char *s = vid.buffer + vid.width*y + x;
	for(i=0;i<height;i++) {
		for(j=0;j<width;j++) {
			d[j] = palette[s[j]];
		}
		d+=BASEWIDTH;
		s+=vid.width;
	}
}


