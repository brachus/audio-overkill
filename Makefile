#
# Audio Overkill Make File (recycled from VGMPlay Makefile)


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
CCPP = g++

#PREFIX = /usr/local
#MANPREFIX = $(PREFIX)/share/man


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


CFLAGS :=  -O3  $(CFLAGS) -g

CFLAGS += -Wno-unused-variable -Wno-unused-value -Wno-unused-but-set-variable
CFLAGS += -Wno-pointer-to-int-cast

CPPFLAGS := -O3 -fpermissive -Wno-unused-variable -Wno-unused-value -g
CPPFLAGS += -Wno-unused-but-set-variable -Wno-overflow -Wno-deprecated

LDFLAGS := -lm -lz -lglib-2.0 `pkg-config --cflags --libs gtk+-3.0` $(LDFLAGS) 

LDFLAGS += -I/usr/include/glib-2.0\
        -I/usr/lib/x86_64-linux-gnu/glib-2.0/include\
        -I/usr/include/SDL2 -lSDL2 -lSDL2_ttf -lrt -lpthread -pthread\
        

# add Library Path, if defined
ifdef LD_LIBRARY_PATH
LDFLAGS += -L $(LD_LIBRARY_PATH)
endif

SRC = .
OBJ = obj




VGMOBJS = \
	$(OBJ)/vgm/VGMPlay.o \
	$(OBJ)/vgm/VGMPlay_AddFmts.o \
	$(OBJ)/vgm/Stream.o \
	$(OBJ)/vgm/ChipMapper.o\
	$(OBJ)/vgm/plugin.o

SIDOBJS = \
	$(OBJ)/sid/plugin.o \
	$(OBJ)/sid/sidengine.o \
	$(OBJ)/sid/soundcard.o
	
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

PSFOBJS = \
	$(OBJ)/psf/plugin.o\
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

GMEOBJS = \
	$(OBJ)/gme/plugin.o\
	$(OBJ)/gme/Ay_Apu.o\
	$(OBJ)/gme/Ay_Cpu.o\
	$(OBJ)/gme/Ay_Emu.o\
	$(OBJ)/gme/Blip_Buffer.o\
	$(OBJ)/gme/Classic_Emu.o\
	$(OBJ)/gme/Data_Reader.o\
	$(OBJ)/gme/Dual_Resampler.o\
	$(OBJ)/gme/Effects_Buffer.o\
	$(OBJ)/gme/Fir_Resampler.o\
	$(OBJ)/gme/Gb_Apu.o\
	$(OBJ)/gme/Gb_Cpu.o\
	$(OBJ)/gme/Gb_Oscs.o\
	$(OBJ)/gme/Gbs_Emu.o\
	$(OBJ)/gme/gme.o\
	$(OBJ)/gme/Gme_File.o\
	$(OBJ)/gme/Gym_Emu.o\
	$(OBJ)/gme/Hes_Apu.o\
	$(OBJ)/gme/Hes_Cpu.o\
	$(OBJ)/gme/Hes_Emu.o\
	$(OBJ)/gme/Kss_Cpu.o\
	$(OBJ)/gme/Kss_Emu.o\
	$(OBJ)/gme/Kss_Scc_Apu.o\
	$(OBJ)/gme/M3u_Playlist.o\
	$(OBJ)/gme/Multi_Buffer.o\
	$(OBJ)/gme/Music_Emu.o\
	$(OBJ)/gme/Nes_Apu.o\
	$(OBJ)/gme/Nes_Cpu.o\
	$(OBJ)/gme/Nes_Fme7_Apu.o\
	$(OBJ)/gme/Nes_Namco_Apu.o\
	$(OBJ)/gme/Nes_Oscs.o\
	$(OBJ)/gme/Nes_Vrc6_Apu.o\
	$(OBJ)/gme/Nsfe_Emu.o\
	$(OBJ)/gme/Nsf_Emu.o\
	$(OBJ)/gme/Sap_Apu.o\
	$(OBJ)/gme/Sap_Cpu.o\
	$(OBJ)/gme/Sap_Emu.o\
	$(OBJ)/gme/Sms_Apu.o\
	$(OBJ)/gme/Snes_Spc.o\
	$(OBJ)/gme/Spc_Cpu.o\
	$(OBJ)/gme/Spc_Dsp.o\
	$(OBJ)/gme/Spc_Emu.o\
	$(OBJ)/gme/Vgm_Emu.o\
	$(OBJ)/gme/Vgm_Emu_Impl.o\
	$(OBJ)/gme/Ym2413_Emu.o\
	$(OBJ)/gme/Ym2612_Emu.o

GSFOBJS = \
	$(OBJ)/gsf/plugin.o \
	$(OBJ)/gsf/gsf.o \
	$(OBJ)/gsf/VBA/bios.o \
	$(OBJ)/gsf/VBA/GBA.o \
	$(OBJ)/gsf/VBA/Globals.o \
	$(OBJ)/gsf/VBA/snd_interp.o \
	$(OBJ)/gsf/VBA/Sound.o \
	$(OBJ)/gsf/VBA/unzip.o \
	$(OBJ)/gsf/VBA/Util.o \
	$(OBJ)/gsf/VBA/getopt1.o \
	$(OBJ)/gsf/VBA/memgzio.o \
	$(OBJ)/gsf/VBA/psftag.o

USFOBJS = \
	$(OBJ)/usf/audio_hle_main.o \
	$(OBJ)/usf/audio_ucode1.o \
	$(OBJ)/usf/audio_ucode2.o \
	$(OBJ)/usf/audio_ucode3.o \
	$(OBJ)/usf/audio_ucode3mp3.o \
	$(OBJ)/usf/cpu.o \
	$(OBJ)/usf/dma.o \
	$(OBJ)/usf/exception.o \
	$(OBJ)/usf/interpreter_cpu.o \
	$(OBJ)/usf/interpreter_ops.o \
	$(OBJ)/usf/main.o \
	$(OBJ)/usf/memory.o \
	$(OBJ)/usf/pif.o \
	$(OBJ)/usf/psftag.o \
	$(OBJ)/usf/recompiler_cpu.o \
	$(OBJ)/usf/recompiler_fpu_ops.o \
	$(OBJ)/usf/recompiler_ops.o \
	$(OBJ)/usf/registers.o \
	$(OBJ)/usf/rsp.o \
	$(OBJ)/usf/rsp_interpreter_cpu.o \
	$(OBJ)/usf/rsp_mmx.o \
	$(OBJ)/usf/rsp_recompiler_analysis.o \
	$(OBJ)/usf/rsp_recompiler_cpu.o \
	$(OBJ)/usf/rsp_recompiler_ops.o \
	$(OBJ)/usf/rsp_sse.o \
	$(OBJ)/usf/rsp_x86.o \
	$(OBJ)/usf/tlb.o \
	$(OBJ)/usf/usf.o \
	$(OBJ)/usf/usfplugin.o \
	$(OBJ)/usf/x86.o \
	$(OBJ)/usf/x86_fpu.o

DSFOBJS = $(OBJ)/dsf/aica.o \
	$(OBJ)/dsf/aicadsp.o \
	$(OBJ)/dsf/arm7.o \
	$(OBJ)/dsf/arm7i.o \
	$(OBJ)/dsf/dc_hw.o \
	$(OBJ)/dsf/dsf_plugin.o \
	$(OBJ)/dsf/eng_dsf.o

DUHOBJS = $(OBJ)/dumb/src/plugin.o \
	$(OBJ)/dumb/src/core/makeduh.o \
	$(OBJ)/dumb/src/core/duhtag.o \
	$(OBJ)/dumb/src/core/register.o \
	$(OBJ)/dumb/src/core/readduh.o \
	$(OBJ)/dumb/src/core/unload.o \
	$(OBJ)/dumb/src/core/rendduh.o \
	$(OBJ)/dumb/src/core/rawsig.o \
	$(OBJ)/dumb/src/core/duhlen.o \
	$(OBJ)/dumb/src/core/atexit.o \
	$(OBJ)/dumb/src/core/rendsig.o \
	$(OBJ)/dumb/src/core/dumbfile.o \
	$(OBJ)/dumb/src/core/loadduh.o \
	$(OBJ)/dumb/src/helpers/resample.o \
	$(OBJ)/dumb/src/helpers/sampbuf.o \
	$(OBJ)/dumb/src/helpers/silence.o \
	$(OBJ)/dumb/src/helpers/clickrem.o \
	$(OBJ)/dumb/src/helpers/memfile.o \
	$(OBJ)/dumb/src/helpers/stdfile.o \
	$(OBJ)/dumb/src/it/loadmod2.o \
	$(OBJ)/dumb/src/it/loads3m2.o \
	$(OBJ)/dumb/src/it/itread2.o \
	$(OBJ)/dumb/src/it/itload.o \
	$(OBJ)/dumb/src/it/readmod2.o \
	$(OBJ)/dumb/src/it/readxm2.o \
	$(OBJ)/dumb/src/it/itmisc.o \
	$(OBJ)/dumb/src/it/xmeffect.o \
	$(OBJ)/dumb/src/it/itload2.o \
	$(OBJ)/dumb/src/it/itrender.o \
	$(OBJ)/dumb/src/it/readxm.o \
	$(OBJ)/dumb/src/it/loads3m.o \
	$(OBJ)/dumb/src/it/loadxm2.o \
	$(OBJ)/dumb/src/it/readmod.o \
	$(OBJ)/dumb/src/it/loadxm.o \
	$(OBJ)/dumb/src/it/loadmod.o \
	$(OBJ)/dumb/src/it/itread.o \
	$(OBJ)/dumb/src/it/itorder.o \
	$(OBJ)/dumb/src/it/itunload.o \
	$(OBJ)/dumb/src/it/reads3m2.o \
	$(OBJ)/dumb/src/it/reads3m.o
	

# DSF engine
#OBJS += eng_dsf/eng_dsf.o eng_dsf/dc_hw.o eng_dsf/aica.o eng_dsf/aicadsp.o eng_dsf/arm7.o eng_dsf/arm7i.o

# SSF engine
#OBJS += eng_ssf/m68kcpu.o eng_ssf/m68kopac.o eng_ssf/m68kopdm.o eng_ssf/m68kopnz.o eng_ssf/m68kops.o
#OBJS += eng_ssf/scsp.o eng_ssf/scspdsp.o eng_ssf/sat_hw.o eng_ssf/eng_ssf.o

# QSF engine
#OBJS += eng_qsf/eng_qsf.o eng_qsf/kabuki.o eng_qsf/qsound.o eng_qsf/z80.o eng_qsf/z80dasm.o

# PSF engine
#OBJS += eng_psf/eng_psf.o eng_psf/psx.o eng_psf/psx_hw.o eng_psf/peops/spu.o

# PSF2 extentions
#OBJS += eng_psf/eng_psf2.o eng_psf/peops2/spu.o eng_psf/peops2/dma.o eng_psf/peops2/registers.o

# SPU engine (requires PSF engine)
#OBJS += eng_psf/eng_spu.o


LIBRESAMPLEOBJS = \
	$(OBJ)/libresample/filterkit.o \
	$(OBJ)/libresample/resample.o \
	$(OBJ)/libresample/resamplesubs.o


MAINOBJS=\
	$(OBJ)/ao.o\
	$(OBJ)/corlett.o\
	$(OBJ)/corlett_newer.o\
	$(OBJ)/utils.o\
	$(OBJ)/conf.o\
	$(OBJ)/filelist.o\
	$(OBJ)/main.o


all: audiooverkill

audiooverkill: $(EMUOBJS) $(VGMOBJS) $(SIDOBJS) $(GMEOBJS) $(LIBRESAMPLEOBJS) $(GSFOBJS) $(USFOBJS) $(DSFOBJS) $(PSFOBJS) $(DUHOBJS) $(MAINOBJS)
	@echo Linking audio overkill ...
	@$(CCPP) $(LDFLAGS) $(EMUOBJS) $(VGMOBJS) $(SIDOBJS) $(GMEOBJS) $(LIBRESAMPLEOBJS) $(GSFOBJS) $(USFOBJS) $(DSFOBJS) $(PSFOBJS) $(DUHOBJS) $(MAINOBJS)  -o audio_overkill
	@echo Done.


# compile the vgm chip-emulator c-files
$(EMUOBJ)/%.o:	$(SRC)/vgm/chips/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS)  $(LDFLAGS) $(EMUFLAGS) -c $< -o $@

# compile the main vgm c-files
$(OBJ)/vgm/%.o:	$(SRC)/vgm/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS)  $(LDFLAGS) -c $< -o $@

# compile the main sid c-files
$(OBJ)/sid/%.o:	$(SRC)/sid/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS)  $(LDFLAGS) -c $< -o $@

# compile blargg's gme library files (c++)
$(OBJ)/gme/%.o:	$(SRC)/gme/%.cpp
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CCPP) $(CPPFLAGS) -c $< -o $@

# compile gsf code (c++)
$(OBJ)/gsf/%.o:	$(SRC)/gsf/%.cpp
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CCPP) $(CPPFLAGS) -DLINUX -c $< -o $@
$(OBJ)/gsf/VBA/%.o:	$(SRC)/gsf/VBA/%.cpp
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CCPP) $(CPPFLAGS) -DLINUX -c $< -o $@
$(OBJ)/gsf/VBA/%.o:	$(SRC)/gsf/VBA/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -DLINUX -c $< -o $@


# libresample
$(OBJ)/libresample/%.o:	$(SRC)/gsf/libresample-0.1.3/src/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS)  -c $< -o $@


# compile usf code (c++)
$(OBJ)/usf/%.o:	$(SRC)/usf/%.cc
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CCPP) $(CPPFLAGS) $(LDFLAGS)  -DLINUX -c $< -o $@


# compile dsf code
$(OBJ)/dsf/%.o:	$(SRC)/dsf/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LDFLAGS) -DLINUX -c $< -o $@
	
# compile psf code
$(OBJ)/psf/%.o:	$(SRC)/psf/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@
	
$(OBJ)/psf/peops/%.o:	$(SRC)/psf/peops/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

$(OBJ)/psf/peops2/%.o:	$(SRC)/psf/peops2/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

# compile DUMB
$(OBJ)/dumb/src/%.o:	$(SRC)/dumb/src/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LDFLAGS) -Idumb/include -c $< -o $@

$(OBJ)/dumb/src/helpers/%.o:	$(SRC)/dumb/src/helpers/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LDFLAGS) -Idumb/include -c $< -o $@

$(OBJ)/dumb/src/core/%.o:	$(SRC)/dumb/src/core/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LDFLAGS) -Idumb/include -c $< -o $@
	
$(OBJ)/dumb/src/it/%.o:	$(SRC)/dumb/src/it/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LDFLAGS) -Idumb/include -c $< -o $@


# compile main code
$(OBJ)/%.o:	$(SRC)/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

	

clean:
	@echo Deleting object files ...
	@rm -rf $(OBJ)
	@echo Deleting executable file\(s\) ...
	@rm -f audio_overkill 
	@echo Done.
