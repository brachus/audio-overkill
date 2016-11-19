#
# Audio Overkill Make File


# disable Hardware OPL Support
DISABLE_HWOPL_SUPPORT = 1

# Uncomment if you want to use libao instead of OSS for sound streaming under Linux
#USE_LIBAO = 1

# Uncomment for debug builds
#DEBUG = 1

# Uncomment for swprintf fix on older MinGW versions
#OLD_SWPRINTF = 1

# Byte Order
#   0 = undefined (use endian-safe code, might be slightly slower)
#   1 = Little Endian (Intel)
#   2 = Big Endian (Motorola)
BYTE_ORDER = 1

EMUFLAGS := -DENABLE_ALL_CORES

CC = gcc
#PREFIX = /usr/local
#MANPREFIX = $(PREFIX)/share/man

# -- VGMPlay specific Compile Flags --
#MAINFLAGS := -DCONSOLE_MODE -DADDITIONAL_FORMATS -DSET_CONSOLE_TITLE

#EMUFLAGS := -DENABLE_ALL_CORES

## -- Byte Order Optimizations --
#ifeq ($(BYTE_ORDER), 1)
## Intel Byte Order
#MAINFLAGS += -DVGM_LITTLE_ENDIAN
#EMUFLAGS += -DVGM_LITTLE_ENDIAN
#else
#ifeq ($(BYTE_ORDER), 2)
## Motorola Byte Order
#MAINFLAGS += -DVGM_BIG_ENDIAN
#EMUFLAGS += -DVGM_BIG_ENDIAN
#else
## undefined byte order
#endif
#endif

#ifdef DISABLE_HWOPL_SUPPORT
#MAINFLAGS += -DDISABLE_HW_SUPPORT
#endif

#gcc main.c ao.c conf.c psf/plugin.c psf/audstrings.c psf/corlett.c\
#       psf/psx.c \
#       psf/psx_hw.c \
#       psf/eng_psf.c \
#       psf/eng_psf2.c \
#       psf/peops/spu.c \
#       psf/peops2/dma.c \
#       psf/peops2/registers.c \
#       psf/peops2/spu.c\
#       \
#       -Wno-unused-variable -Wno-unused-value -Wno-unused-but-set-variable\
#        -g0\
#	    -lglib-2.0 -I/usr/include/glib-2.0\
#        -I/usr/lib/x86_64-linux-gnu/glib-2.0/include\
#        -I/usr/include/SDL2 \
#        -lz -lm -Wno-sign-compare -I../.. -Ipsf/ -Ispu/\
#        -I. -Ivgm/chips/ -lrt -lpthread -pthread -ggdb3   -O2 -o audio_overkill


CFLAGS := -O3  $(CFLAGS) 

CFLAGS += -Wno-unused-variable -Wno-unused-value -Wno-unused-but-set-variable

LDFLAGS := -lm -lz -lglib-2.0 $(LDFLAGS)

LDFLAGS += -I/usr/include/glib-2.0\
        -I/usr/lib/x86_64-linux-gnu/glib-2.0/include\
        -I/usr/include/SDL2 -lSDL2 -lSDL2_ttf -lrt -lpthread -pthread -ggdb3

# add Library Path, if defined
ifdef LD_LIBRARY_PATH
LDFLAGS += -L $(LD_LIBRARY_PATH)
endif

SRC = .
OBJ = obj

PSFSRCS = \
	  $(SRC)/psf/plugin.c\
	  $(SRC)/psf/audstrings.c\
	  $(SRC)/psf/corlett.c\
      $(SRC)/psf/psx.c \
      $(SRC)/psf/psx_hw.c \
      $(SRC)/psf/eng_psf.c \
      $(SRC)/psf/eng_psf2.c \
      $(SRC)/psf/peops/spu.c \
      $(SRC)/psf/peops2/dma.c \
      $(SRC)/psf/peops2/registers.c \
      $(SRC)/psf/peops2/spu.c\


#OBJDIRS = \
#	$(OBJ) \
#	$(EMUOBJ)

VGMMAINOBJS = \
	$(OBJ)/vgm/VGMPlay.o \
	$(OBJ)/vgm/VGMPlay_AddFmts.o \
	$(OBJ)/vgm/Stream.o \
	$(OBJ)/vgm/ChipMapper.o\
	$(OBJ)/vgm/plugin.o
	
EMUOBJ = $(OBJ)/chips

EMUOBJS = \
	$(EMUOBJ)/262intf.o \
	$(EMUOBJ)/2151intf.o \
	$(EMUOBJ)/2203intf.o \
	$(EMUOBJ)/2413intf.o \
	$(EMUOBJ)/2608intf.o \
	$(EMUOBJ)/2610intf.o \
	$(EMUOBJ)/2612intf.o \
	$(EMUOBJ)/3526intf.o \
	$(EMUOBJ)/3812intf.o \
	$(EMUOBJ)/8950intf.o \
	$(EMUOBJ)/adlibemu_opl2.o \
	$(EMUOBJ)/adlibemu_opl3.o \
	$(EMUOBJ)/ay8910.o \
	$(EMUOBJ)/ay_intf.o \
	$(EMUOBJ)/c140.o \
	$(EMUOBJ)/c352.o \
	$(EMUOBJ)/c6280.o \
	$(EMUOBJ)/c6280intf.o \
	$(EMUOBJ)/dac_control.o \
	$(EMUOBJ)/es5503.o \
	$(EMUOBJ)/es5506.o \
	$(EMUOBJ)/emu2149.o \
	$(EMUOBJ)/emu2413.o \
	$(EMUOBJ)/fm2612.o \
	$(EMUOBJ)/fm.o \
	$(EMUOBJ)/fmopl.o \
	$(EMUOBJ)/gb.o \
	$(EMUOBJ)/iremga20.o \
	$(EMUOBJ)/k051649.o \
	$(EMUOBJ)/k053260.o \
	$(EMUOBJ)/k054539.o \
	$(EMUOBJ)/multipcm.o \
	$(EMUOBJ)/nes_apu.o \
	$(EMUOBJ)/nes_intf.o \
	$(EMUOBJ)/np_nes_apu.o \
	$(EMUOBJ)/np_nes_dmc.o \
	$(EMUOBJ)/np_nes_fds.o \
	$(EMUOBJ)/okim6258.o \
	$(EMUOBJ)/okim6295.o \
	$(EMUOBJ)/Ootake_PSG.o \
	$(EMUOBJ)/panning.o \
	$(EMUOBJ)/pokey.o \
	$(EMUOBJ)/pwm.o \
	$(EMUOBJ)/qsound.o \
	$(EMUOBJ)/rf5c68.o \
	$(EMUOBJ)/saa1099.o \
	$(EMUOBJ)/segapcm.o \
	$(EMUOBJ)/scd_pcm.o \
	$(EMUOBJ)/scsp.o \
	$(EMUOBJ)/scspdsp.o \
	$(EMUOBJ)/sn76489.o \
	$(EMUOBJ)/sn76496.o \
	$(EMUOBJ)/sn764intf.o \
	$(EMUOBJ)/upd7759.o \
	$(EMUOBJ)/vsu.o \
	$(EMUOBJ)/ws_audio.o \
	$(EMUOBJ)/x1_010.o \
	$(EMUOBJ)/ym2151.o \
	$(EMUOBJ)/ym2413.o \
	$(EMUOBJ)/ym2612.o \
	$(EMUOBJ)/ymdeltat.o \
	$(EMUOBJ)/ymf262.o \
	$(EMUOBJ)/ymf271.o \
	$(EMUOBJ)/ymf278b.o \
	$(EMUOBJ)/ymz280b.o \
	$(EMUOBJ)/ay8910_opl.o \
	$(EMUOBJ)/sn76496_opl.o \
	$(EMUOBJ)/ym2413hd.o \
	$(EMUOBJ)/ym2413_opl.o
#VGMPLAY_OBJS = \
#	$(OBJ)/VGMPlayUI.o
#VGM2PCM_OBJS = \
#	$(OBJ)/vgm2pcm.o
#VGM2WAV_OBJS = \
#	$(OBJ)/vgm2wav.o
#EXTRA_OBJS = $(VGMPLAY_OBJS) $(VGM2PCM_OBJS) $(VGM2WAV_OBJS)


PSFOBJS = \
	$(OBJ)/psf/plugin.o\
	$(OBJ)/psf/audstrings.o\
	$(OBJ)/psf/corlett.o\
	$(OBJ)/psf/psx.o\
	$(OBJ)/psf/psx_hw.o\
	$(OBJ)/psf/eng_psf.o\
	$(OBJ)/psf/eng_psf2.o\
	$(OBJ)/psf/peops/spu.o\
	$(OBJ)/psf/peops/adsr.o\
	$(OBJ)/psf/peops/reverb.o\
	$(OBJ)/psf/peops2/dma.o\
	$(OBJ)/psf/peops2/registers.o\
	$(OBJ)/psf/peops2/spu.o\
	$(OBJ)/psf/peops2/xa.o\
	$(OBJ)/psf/peops2/adsr.o\
	$(OBJ)/psf/peops2/reverb.o

MAINOBJS=\
	$(OBJ)/ao.o\
	$(OBJ)/conf.o\
	$(OBJ)/filelist.o\
	$(OBJ)/main.o


#all:	vgmplay vgm2pcm vgm2wav

#vgmplay:	$(EMUOBJS) $(MAINOBJS) $(VGMPLAY_OBJS)
#	@echo Linking vgmplay ...
#	@$(CC) $(VGMPLAY_OBJS) $(MAINOBJS) $(EMUOBJS) $(LDFLAGS) -o vgmplay
#	@echo Done.

#vgm2pcm:	$(EMUOBJS) $(MAINOBJS) $(VGM2PCM_OBJS)
#	@echo Linking vgm2pcm ...
#	@$(CC) $(VGM2PCM_OBJS) $(MAINOBJS) $(EMUOBJS) $(LDFLAGS) -o vgm2pcm
#	@echo Done.

#vgm2wav:	$(EMUOBJS) $(MAINOBJS) $(VGM2WAV_OBJS)
#	@echo Linking vgm2wav ...
#	@$(CC) $(VGM2WAV_OBJS) $(MAINOBJS) $(EMUOBJS) $(LDFLAGS) -o vgm2wav
#	@echo Done.

all: audiooverkill

audiooverkill: $(EMUOBJS) $(VGMMAINOBJS) psf.o main.o
	@echo Linking audio overkill ...
	@$(CC) $(LDFLAGS) $(EMUOBJS) $(VGMMAINOBJS) $(PSFOBJS) $(MAINOBJS)  -o audio_overkill
	@echo Done.
	
audiooverkill_onlypsf: psf.o main.o
	@echo Linking audio overkill \( PSF support only. \) ...
	@$(CC) $(LDFLAGS) $(PSFOBJS) $(MAINOBJS) -o audio_overkill
	@echo Done.

 #$(OBJ)/psf/peops/dma.o\

# compile the chip-emulator c-files
$(EMUOBJ)/%.o:	$(SRC)/vgm/chips/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS)  $(LDFLAGS) $(EMUFLAGS) -c $< -o $@

# compile the main vgm c-files
$(OBJ)/vgm/%.o:	$(SRC)/vgm/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS)  $(LDFLAGS) -c $< -o $@

# compile psf
psf.o:
	@echo Compiling psf objects ...
	@mkdir -p $(OBJ)/psf/
	@mkdir -p $(OBJ)/psf/peops/
	@mkdir -p $(OBJ)/psf/peops2/
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/plugin.c -o $(OBJ)/psf/plugin.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/audstrings.c -o $(OBJ)/psf/audstrings.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/corlett.c -o $(OBJ)/psf/corlett.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/psx.c -o $(OBJ)/psf/psx.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/psx_hw.c -o $(OBJ)/psf/psx_hw.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/eng_psf.c -o $(OBJ)/psf/eng_psf.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/eng_psf2.c -o $(OBJ)/psf/eng_psf2.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/peops/spu.c -o $(OBJ)/psf/peops/spu.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/peops/adsr.c -o $(OBJ)/psf/peops/adsr.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/peops/reverb.c -o $(OBJ)/psf/peops/reverb.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/peops2/dma.c -o $(OBJ)/psf/peops2/dma.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/peops2/registers.c -o $(OBJ)/psf/peops2/registers.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/peops2/spu.c -o $(OBJ)/psf/peops2/spu.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/peops2/xa.c -o $(OBJ)/psf/peops2/xa.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/peops2/adsr.c -o $(OBJ)/psf/peops2/adsr.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/peops2/reverb.c -o $(OBJ)/psf/peops2/reverb.o

#@$(CC) $(CFLAGS) $(LDFLAGS) -c psf/peops/dma.c -o $(OBJ)/psf/peops/dma.o

# compile main
main.o:
	@echo Compiling main objects ...
	@mkdir -p $(OBJ)
	@$(CC) $(CFLAGS)  $(LDFLAGS) -c main.c -o $(OBJ)/main.o
	@$(CC) $(CFLAGS)  $(LDFLAGS) -c ao.c -o $(OBJ)/ao.o
	@$(CC) $(CFLAGS)  $(LDFLAGS) -c filelist.c -o $(OBJ)/filelist.o
	@$(CC) $(CFLAGS)  $(LDFLAGS) -c conf.c -o $(OBJ)/conf.o

clean:
	@echo Deleting object files ...
	@rm -rf $(OBJ)
	@echo Deleting executable file\(s\) ...
	@rm -f audio_overkill 
	@echo Done.

# Thanks to ZekeSulastin and nextvolume for the install and uninstall routines.
#install:	vgmplay
#	install -m 755 vgmplay $(DESTDIR)$(PREFIX)/bin/vgmplay
#	install -m 644 vgmplay.1 $(DESTDIR)$(MANPREFIX)/man1/vgmplay.1
#	mkdir -m 755 -p $(DESTDIR)$(PREFIX)/share/vgmplay
#	install -m 644 VGMPlay.ini $(DESTDIR)$(PREFIX)/share/vgmplay/vgmplay.ini
#	-install -m 644 yrw801.rom $(DESTDIR)$(PREFIX)/share/vgmplay/yrw801.rom


## Install the "vgm-player" wrapper
#play_inst:	install
#	install -m 755 vgm-player $(DESTDIR)$(PREFIX)/bin/vgm-player

#uninstall:
#	rm $(DESTDIR)$(PREFIX)/bin/vgmplay
#	rm $(DESTDIR)$(PREFIX)/bin/vgm-player
#	rm $(DESTDIR)$(MANPREFIX)/man1/vgmplay.1
#	rm -rf $(DESTDIR)$(PREFIX)/share/vgmplay

#.PHONY: all clean install uninstall
