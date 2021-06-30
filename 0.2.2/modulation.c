#include <stdlib.h>

#include <sndfile.h>

#include "modulation.h"

static unsigned int mod_sf;
static unsigned int mod_bits;

static int mod_buffer_len = 0;
static double *mod_buffer = NULL;

static unsigned int samp(double ms) 
{
	unsigned int a;
	double b;

	b=((double) mod_sf) * ms / 1000.0;

	a=((unsigned int) (b + 0.5));

	return a;
}

static int mod_impulse(double value, double ms)
{
	unsigned int n,i;
	sf_count_t r;

	n=samp(ms);

	mod_buffer=realloc(mod_buffer, (mod_buffer_len+n)*sizeof(*mod_buffer));
	if(mod_buffer==NULL) {
		fprintf(stderr, "Can't allocate memory\n");
		return -1;
	}

	r=0;
	for(i=0;i<n;i++) {
		mod_buffer[mod_buffer_len+i]=value;
	}

	mod_buffer_len=mod_buffer_len+n;

	return 0;
}

int mod_init(int samplerate, int bits)
{
	mod_sf=samplerate;
	if(bits!=16 && bits!=8) {
		fprintf(stderr, "Only 8 and 16 bit sample size supported\n");
		return -1;
	}
	mod_bits=bits;
	return 0;
}

int mod_close()
{
	if(mod_buffer!=NULL) free(mod_buffer);

	mod_buffer=NULL;
	mod_buffer_len=0;

	return 0;
}

int mod_write(const char *file)
{
	SNDFILE *f;
	SF_INFO info;
	sf_count_t r;

	info.frames = mod_buffer_len;
	info.samplerate = mod_sf;
	info.channels = 1;

	if(mod_bits==16) {
		info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	} else {
		info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_U8;
	}

	f=sf_open(file, SFM_WRITE, &info);
	if(f==NULL) {
		fprintf(stderr, "Can't open WAV file\n");
		return -1;
	}

	r=sf_writef_double(f, mod_buffer, mod_buffer_len);
	if(r!=mod_buffer_len) {
		fprintf(stderr, "Can't write WAV file\n");
		sf_close(f);
		return -1;
	}

	sf_close(f);

	return 0;
}

int mod_interbyte_pause()
{
	//printf("pause: %.1lf ms\n", INTERBYTE_PAUSE);
	return mod_impulse(0.0, INTERBYTE_PAUSE);
}

int mod_interblock_pause()
{
	//printf("pause: %.1lf ms\n", INTERBLOCK_PAUSE);
	return mod_impulse(0.0, INTERBLOCK_PAUSE);
}

int mod_sync()
{
	int n;

	for(n=0;n<SYNCBYTES;n++) {
		if(n!=0) {
			if(mod_interbyte_pause()) return -1;
		}
		if(mod_byte(0)) return -1;
	}

	return 0;
}

int mod_block(unsigned char *buff, unsigned int len)
{
	int n;
	for(n=0;n<len;n++) {
		if(n!=0) {
			if(mod_interbyte_pause()) return -1;
		}
		if(mod_byte(buff[n])) return -1;
	}

	return 0;
}

int mod_byte(unsigned char b)
{
	int n;

	//printf("byte: %02x\n", b);

	for(n=0;n<8;n++) {
		if(b & 0x01) {
			if(mod_1()) return -1;
		} else {
			if(mod_0()) return -1;
		}
		b=b>>1;
	}

	return 0;
}

int mod_1()
{
	int n,i;

	n=PERIOD_BASE/PERIOD_1;

	for(i=0;i<n;i++) {
		if(mod_impulse(-1.0, PULSE_WIDTH)) return -1;
		if(mod_impulse(1.0, PULSE_WIDTH)) return -1;
		if(mod_impulse(0.0, PERIOD_1-2*PULSE_WIDTH)) return -1;
	}

	return 0;
}

int mod_0()
{
	int n,i;

	n=PERIOD_BASE/PERIOD_0;

	for(i=0;i<n;i++) {
		if(mod_impulse(-1.0, PULSE_WIDTH)) return -1;
		if(mod_impulse(1.0, PULSE_WIDTH)) return -1;
		if(mod_impulse(0.0, PERIOD_0-2*PULSE_WIDTH)) return -1;
	}

	return 0;
}
