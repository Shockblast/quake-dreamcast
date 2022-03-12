CC=sh-elf-gcc -ml -m4-single-only
KOS_BASE=/prog/kos-1.1.7

LWIPDIR = $(KOS_BASE)/addons/lwip/src
LWIPARCH = kos
LWIP_INCS = -I$(LWIPDIR)/include -I$(LWIPDIR)/arch/$(LWIPARCH)/include \
	-I$(LWIPDIR)/include/ipv4

KOS_INCS=-I$(KOS_BASE)/libc/include -I$(KOS_BASE)/kernel/arch/dreamcast/include -I$(KOS_BASE)/include 

BASE_CFLAGS=-Dstricmp=strcasecmp $(KOS_INCS) $(LWIP_INCS) -DDC
RELEASE_CFLAGS=$(BASE_CFLAGS) -g -O6 -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations
CFLAGS=$(RELEASE_CFLAGS) -Wall -DUSE_ZLIB -DGLQUAKE

all : glquake.elf

COMMON_OBJS =  chase.o cl_demo.o cl_input.o cl_main.o cl_parse.o cl_tent.o \
	cmd.o common.o console.o crc.o cvar.o \
	host.o host_cmd.o keys.o mathlib.o menu.o \
	pr_cmds.o pr_edict.o pr_exec.o r_part.o sbar.o \
	sv_main.o sv_move.o sv_phys.o sv_user.o \
	view.o wad.o world.o zone.o \
	net_loop.o net_main.o  net_vcr.o 

SND_OBJS =	snd_dma.o snd_mem.o snd_mix.o


NET_OBJS = net_bsd.o net_dgrm.o net_udp.o


NET_NULL_OBJS = net_none.o

GL_OBJS = gl_draw.o gl_mesh.o gl_model.o gl_refrag.o gl_rlight.o gl_rmain.o gl_rmisc.o \
	gl_rsurf.o gl_screen.o gl_test.o gl_warp.o

SOFT_OBJS = d_edge.o d_fill.o d_init.o d_modech.o d_part.o d_polyse.o d_scan.o \
	d_sky.o d_sprite.o d_surf.o d_vars.o d_zpoint.o draw.o model.o \
	r_aclip.o r_alias.o r_bsp.o r_draw.o r_edge.o \
	r_efrag.o r_light.o r_main.o r_misc.o r_sky.o r_sprite.o \
	r_surf.o r_vars.o   nonintel.o screen.o 

SDL_OBJS = cd_sdl.o snd_sdl.o sys_sdl.o vid_sdl.o
NULL_OBJS = cd_null.o in_null.o net_none.o snd_dmy.o sys_null.o vid_null.o

DC_OBJS = cd_dc.o in_dc.o sys_dc.o snddma_dc.o snd_mix_dc.o snd_dma.o snd_mem.o \
	dc/aica.o dc/fake_cdda.o vmuheader.o dc/dcmenu.o

X86_OBJS	= d_copy.o snd_mixa.o sys_dosa.o  dosasm.o d_draw.o \
d_draw16.o d_parta.o d_polysa.o d_scana.o d_spr8.o d_varsa.o math.o \
r_aclipa.o r_aliasa.o r_drawa.o r_edgea.o surf16.o surf8.o worlda.o r_varsa.o 


#OBJS = $(COMMON_OBJS) $(SOFT_OBJS) $(DC_OBJS) $(NET_OBJS) dc/netdb.o vid_dc.o
OBJS = $(COMMON_OBJS) $(SOFT_OBJS) $(DC_OBJS) $(NET_NULL_OBJS) vid_dc.o
GLOBJS = $(COMMON_OBJS) $(GL_OBJS) $(DC_OBJS) $(NET_NULL_OBJS) gl_viddc.o

LDFLAGS = -nostartfiles -Wl,-Ttext=0x8c010000 dc/startup.o dc/syscalls.o

quake.elf : $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o quake.elf $(OBJS) -lm -lz -L/prog/kos-1.1.7/lib -lkallisti -llwip4 -lc -lm

glquake.elf : $(GLOBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o glquake.elf $(GLOBJS) -lm -lz -L/prog/kos-1.1.7/lib -lgl -ldcutils -lkallisti -lc -lm

dcmenu.elf : dc/dcmenu.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o dcmenu.elf dc/dcmenu.o -lm -L/prog/kos-1.1.7/lib -lkallisti -llwip4
	
#	for optimize problem
screen.o : screen.c
	$(CC) $(CFLAGS) -c -O1 $<
gl_screen.o : gl_screen.c
	$(CC) $(CFLAGS) -c -O1 $<
