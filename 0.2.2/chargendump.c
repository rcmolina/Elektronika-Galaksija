#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CHARNUM		128
#define ROMSIZE		(16*CHARNUM)

#define ROWS		8

int a_invert = 0;

void syntax()
{
	printf("Galaksija character generator ROM dump\n");
	printf("Copyright (c) 2007 Tomaz Solc\n\n");
	printf("SYNTAX: chardump [options] chrgen.bin\n\n");
	printf("Options:  -i       Simulate new board (shift register wired\n");
	printf("                   in reverse bit order)\n");
	printf("          -a NUM   Dump character with ASCII code NUM to stdout.\n");
	printf("          -n NUM   Dump character number NUM to stdout.\n");
	printf("          -p       Dump all characters in PBM format to stdout.\n");
}

char reverse(char x) {
	char h;
	int i;

	h=0;
	for(i=0;i<8;i++) {
		h=(h<<1)+(x&0x01); 
		x=x>>1; 
	}

	return h;
}

void ascii_line(char b)
{
	int n;

	if(a_invert) {
		b=reverse(b);
	}

	for(n=0;n<8;n++) {
		if(b&0x01) {
			printf(".");
		} else {
			printf("#");
		}
		b=b>>1;
	}
	printf("\n");
}

void ascii_dump(int fd, int charcode)
{
	int n,off;
	char line;

	for(n=0;n<16;n++) {
		off=(n<<7)+charcode;
		lseek(fd, off, SEEK_SET);
		read(fd, &line, 1);
		printf("0x%02X | ", n);
		ascii_line(line);
	}
}

void pbm_dump(int fd)
{
	int charcode;

	int x,y,n;
	char line;
	int columns = (CHARNUM / ROWS + ((CHARNUM%ROWS)?1:0)); 

	int width = (8 + 8) * ROWS - 8;
	int height = (16 + 8) * columns - 8;
	int off;

	printf("P4 %d %d\n", width, height);

	for(y=0;y<height;y++) {
		n=y%(16+8);

		for(x=0;x<width/8;x++) {
			if(n<16 && x%2==0) {
				charcode=x/2+y/24*ROWS;
				off=(n<<7)+charcode;
				lseek(fd, off, SEEK_SET);
				read(fd, &line, 1);

				if(!a_invert) { 
					line=reverse(line);
				}
				printf("%c", line);
			} else {
				printf("%c", 0xFF);
			}
		}
	}
}

/* This function converts an ASCII character code (i.e. value stored in
 * Galaksija's video RAM) to the serial number of the character in 
 * character ROM. 
 *
 * This conversion is determined by the hardwired connection between CPU's
 * data bus and character ROM's address pins. 
 */
int ascii_to_num(int ascii)
{
	int low, high;

	low=ascii&0x3f;

	high=(ascii&0x80)>>1;

	return(low|high);
}

#define MODE_ASCII	0
#define MODE_PBM	1
#define MODE_NUM	2

int main(int argc, char **argv)
{
	int charcode;
	int cmd=-1;
	int fd;
	int c, r;

        while ((c=getopt(argc, argv, "hia:n:p"))!=-1) {
                switch (c) {
                        case 'h':
                        case '?': syntax();
                                  return 0;
                        case 'i': a_invert=1;
                                  break;
			case 'a': r=sscanf(optarg, "%d", &charcode);
				  if(r!=1 || charcode<0 || charcode>255) {
				  	printf("Invalid ASCII code '%s'\n",
									optarg);
					return 1;
				  }
			          cmd=MODE_ASCII;
				  break;
			case 'n': r=sscanf(optarg, "%d", &charcode);
				  if(r!=1 || charcode<0 || charcode>127) {
				  	printf("Invalid character code '%s'\n",
									optarg);
					return 1;
				  }
			          cmd=MODE_NUM;
				  break;
			case 'p': cmd=MODE_PBM;
				  break;
                }
        }

	if(optind>argc-1) {
		syntax();
		return 0;
	}

	fd=open(argv[optind], O_RDONLY);
	if(fd<0) {
		fprintf(stderr, "open failed!\n");
		perror("chardump");
		return 1;
	}

	switch(cmd) {
		case MODE_ASCII:
			charcode=ascii_to_num(charcode);
			ascii_dump(fd,charcode);
			break;
		case MODE_NUM:
			ascii_dump(fd,charcode);
			break;
		case MODE_PBM:
			pbm_dump(fd);
			break;
		default:
			syntax();
			break;
	}

	close(fd);

	return 0;
}
