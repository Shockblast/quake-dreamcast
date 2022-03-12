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
// sys_null.h -- null system driver to aid porting efforts

#include "quakedef.h"
#include "errno.h"
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

qboolean			isDedicated;

/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_FileOpenRead (char *path, int *hndl)
{
	int h;
	h = fs_open(path,O_RDONLY);
	if (h==0) h=-1;

	*hndl = h;
	if (h==-1) {
		return -1;
	} else {

		return fs_total(h);
	}
}

int Sys_FileOpenWrite (char *path)
{
	int h;
	h = fs_open(path,O_WRONLY);
	if (h==0) h=-1;

	if (h==-1)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));
	
	return h;
}

void Sys_FileClose (int handle)
{
	fs_close (handle);
}

void Sys_FileSeek (int handle, int position)
{
	int ret;
	ret = fs_seek (handle, position, SEEK_SET);
//	printf("Sys_FileSeek(%d,%d)=%d\n",handle,position,ret);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	assert(dest!=NULL);
	return fs_read (handle,dest,count);
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return fs_write (handle,data, count);
}

int     Sys_FileTime (char *path)
{
	int    h;
	
	h = fs_open(path, O_RDONLY);
	if (h!=0)
	{
		fs_close(h);
		return 1;
	}
	
	return -1;
}

void Sys_mkdir (char *path)
{
}


/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}


void Sys_Error (char *error, ...)
{
	va_list         argptr;

	printf ("Sys_Error: ");   
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list         argptr;
	
	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr);
}

void Sys_Quit (void)
{
	exit (0);
}

#include <sys/time.h>

double Sys_FloatTime (void)
{
    struct timeval tp;
    struct timezone tzp; 
    static int      secbase; 
    
    gettimeofday(&tp, &tzp);  

    if (!secbase)
    {
        secbase = tp.tv_sec;
        return tp.tv_usec/1000000.0;
    }

    return (tp.tv_sec - secbase) + tp.tv_usec/1000000.0;
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void Sys_Sleep (void)
{
}

/*
test()
{
    struct timeval tp;
    struct timezone tzp; 
    int i;
	for(i=0;i<1000;i++) {
	    gettimeofday(&tp, &tzp);
	    printf("src = %d msec=%d\n",tp.tv_sec,tp.tv_usec);
	}
}
*/

#if 0
void Sys_SendKeyEvents (void)
{
}
#endif

void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}

//=============================================================================

#define	_arch_dreamcast
#include <kos.h>
#include <lwip/lwip.h>

/* KOS_INIT_FLAGS(INIT_DEFAULT | INIT_NET); */
extern char* menu(int *argc,char **argv,char **basedir);

int main (int argc, char **argv)
{
	extern int vcrFile;
	extern int recording;
	static quakeparms_t    parms;
	double		time, oldtime, newtime;

	const static char *basedirs[]={
	"/cd/QUAKE", /* installed  */
	"/cd/QUAKE_SW", /* shareware */
	"/cd/data", /* official CD-ROM */
	"/pc/quake", /* debug */
	"/pc/quake_sw", /* debug */
	NULL
	};

	char *basedir;
#if 0
	int i;
	
	for(i=0;(basedir = basedirs[i])!=NULL;i++) {
		int fd  = fs_open(basedir,O_DIR);
		if (fd!=0) {
			fs_close(fd);
			break;
		}
	}
	if (basedir==NULL)
		Sys_Error("can't find quake dir");
#else
	{
	static char *args[10] = {"quake",NULL};
	argc = 1;
	argv = args;
	basedir = menu(&argc,argv,basedirs);
	}
#endif
#if 0
	struct ip_addr ipaddr, gw, netmask;

	/* Change these for your network */
	IP4_ADDR(&ipaddr, 192,168,0,4);
	IP4_ADDR(&netmask, 255,255,255,0);
	IP4_ADDR(&gw, 192,168,0,1);
	lwip_init_static(&ipaddr, &netmask, &gw);
#endif

	parms.memsize = 10*1024*1024;

	parms.membase = malloc (parms.memsize);
	parms.basedir = basedir;

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	Host_Init (&parms);

#if 1 /* same as dos */
	oldtime = Sys_FloatTime ();
	while (1)
	{
		newtime = Sys_FloatTime ();
		time = newtime - oldtime;

		if (cls.state == ca_dedicated && (time<sys_ticrate.value))
			continue;

		Host_Frame (time);

		oldtime = newtime;
	}
#else /* same as liunx */
    oldtime = Sys_FloatTime () - 0.1;
    while (1)
    {
// find time spent rendering last frame
        newtime = Sys_FloatTime ();
        time = newtime - oldtime;

        if (cls.state == ca_dedicated)
        {   // play vcrfiles at max speed
            if (time < sys_ticrate.value && (vcrFile == -1 || recording) )
            {
				usleep(1);
                continue;       // not time to run a server only tic yet
            }
            time = sys_ticrate.value;
        }

        if (time > sys_ticrate.value*2)
            oldtime = newtime;
        else
            oldtime += time;

        Host_Frame (time);
    }
#endif
}

char *get_savedir(void)
{
	static char savedir[]="/vmu/a1";

	int port,unit;
	uint8 addr;
	addr = maple_first_vmu();
	if (addr==0) return "dmy";
	maple_raddr(addr,&port,&unit);
	savedir[5] = 'a' + port;
	savedir[6] = '0' + unit;

	return savedir;
}
