#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <pgm.h>

#include "config.h"

int a_asm=0;

int f_height, f_width;
gray f_maxval;
gray **f_image;

void print_syntax()
{
	printf("PGM2SCR, Convert a portable graymap to Galaksija framebuffer\n");
	printf("Copyright (c) 2007 Tomaz Solc\n\n");
	printf("SYNTAX: pgm2scr [ options ] in-file.pgm\n\n");
	printf("Available options:	-a	Output assembler directives"
	       "\n"
	       "Bug reports to tomaz.solc@tablix.org\n"
	       );
	exit(0);
}

int convert_char(int x, int y)
{
	int n,m;

	int result=0;
	int mask=1;

	for(m=0;m<3;m++) {
		for(n=0;n<2;n++) {
			if(f_image[y+m][x+n]!=0){
				/* belo */
				result=result|mask;	
			}
			mask=mask<<1;
		}
	}

	result=result|0xc0;

	return result;
}

void convert_bin()
{
	int x,y;
	int xs,ys;
	int n=0;

	xs=f_width/2;
	ys=f_height/3;
	for(y=0;y<ys;y++) {
		for(x=0;x<xs;x++) {
			printf("%c", convert_char(x*2,y*3));
			n++;
		}
	}
	fprintf(stderr, "Written %d bytes\n", n);
}

void convert_asm()
{
	int x,y;
	int xs,ys;
	int n=0;

	xs=f_width/2;
	ys=f_height/3;
	for(y=0;y<ys;y++) {
		printf("; line %d\n", y);
		for(x=0;x<xs;x++) {
			printf("db 0x%x\n", convert_char(x*2,y*3));
			n++;
		}
	}

	printf("; %d bytes\n", n);
	fprintf(stderr, "Written %d bytes\n", n);
}

int main(int argc, char **argv)
{
	int c;

	FILE *f;

        while ((c=getopt(argc, argv, "aho:"))!=-1) {
                switch (c) {
                        case 'h': print_syntax();
				  return 0;
				  break;
			case 'a': a_asm=1;
				  break;
                }
        }

        if (!(optind<argc)) {
                printf("Missing input file name. Try -h for help\n");
		return 1;
        }

	f=fopen(argv[optind],"r");

	if(f==NULL) {
		perror("pgm2scr");
		return 1;
	}

	f_image=pgm_readpgm(f, &f_width, &f_height, &f_maxval);
	if(f_image==NULL) {
                printf("Can't read image.\n");
		return 1;
	}

	fclose(f);

	if(f_width%2!=0) {
		fprintf(stderr, "Width not multiple of 2. Image will be "
				"cropped.\n");
	}
	if(f_height%3!=0) {
		fprintf(stderr, "Height not multiple of 3. Image will be "
				"cropped.\n");
	}
	
	if(a_asm) {
		printf("; Made with pgm2scr " VERSION "\n");
		printf("; %s (%d x %d pixels, %d x %d chars)\n", argv[optind], 
							f_width, f_height,
							f_width/2, f_height/3);
		printf(";\n");
		convert_asm();
	} else {
		convert_bin();
	}

	pgm_freearray(f_image, f_height);
	
	return 0;
}
