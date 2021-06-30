AM_CFLAGS = -Wall -g

bin_PROGRAMS = bin2gtp chargendump

if HAS_SNDFILE
bin_PROGRAMS += gtp2wav
endif

if HAS_NETPBM
bin_PROGRAMS += pgm2scr
endif

gtp2wav_SOURCES = 	gtp2wav.c \
			endian.c \
			endian.h \
			gtp.c \
			gtp.h \
			modulation.c \
			modulation.h \
			gtp_run.c \
			gtp_run.h

gtp2wav_LDADD = @SNDFILE_LIBS@
#gtp2wav_CPPFLAGS = @SNDFILE_CFLAGS@

bin2gtp_SOURCES =	bin2gtp.c \
			gtp.c \
			gtp.h \
			endian.c \
			endian.h \
			basic.c \
			basic.h

chargendump_SOURCES =	chargendump.c

pgm2scr_SOURCES =	pgm2scr.c

pgm2scr_LDADD = -lnetpbm
