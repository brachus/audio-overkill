

gcc main.c ao.c conf.c psf/plugin.c psf/audstrings.c psf/corlett.c\
       psf/psx.c \
       psf/psx_hw.c \
       psf/eng_psf.c \
       psf/eng_psf2.c \
       psf/peops/spu.c \
       psf/peops2/dma.c \
       psf/peops2/registers.c \
       psf/peops2/spu.c\
       \
       -Wno-unused-variable -Wno-unused-value -Wno-unused-but-set-variable\
        -g0\
	    -lglib-2.0 -I/usr/include/glib-2.0\
        -I/usr/lib/x86_64-linux-gnu/glib-2.0/include\
        -I/usr/include/SDL2 -lSDL2 -lSDL2_ttf\
        -lz -lm -Wno-sign-compare -I../.. -Ipsf/ -Ispu/\
        -I. -Ivgm/chips/ -lrt -lpthread -pthread -ggdb3   -O2 -o audio_overkill
