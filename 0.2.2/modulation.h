/* All numbers in miliseconds */
#define PULSE_WIDTH		0.6

#define PERIOD_BASE		3.0
#define PERIOD_1		(PERIOD_BASE/2)
#define PERIOD_0		(PERIOD_BASE)

#define INTERBYTE_PAUSE		4.5
#define INTERBLOCK_PAUSE 	2000.0

#define SYNCBYTES		100

int mod_init(int samplerate, int bits);
int mod_close();
int mod_write(const char *file);

int mod_interbyte_pause();
int mod_interblock_pause();

int mod_sync();

int mod_block(unsigned char *buff, unsigned int len);
int mod_byte(unsigned char b);

int mod_1();
int mod_0();
