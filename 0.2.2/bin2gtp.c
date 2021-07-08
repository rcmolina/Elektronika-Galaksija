#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include "gtp.h"
#include "endian.h"
#include "basic.h"

char *a_outputfile=NULL;
char *a_name=NULL;
char *a_basicfile=NULL;

char basicdef[] = "\001\000A=USR(&2C3A)\015";
int basicdeflen=15;

char *basic;
int basiclen;

void print_syntax()
{
	printf("BIN2GTP, Encapsulate Z80 machine code into Galaksija Tape Format\n");
	printf("Copyright (c) 2007 Tomaz Solc\n\n");
	printf("SYNTAX: bin2gtp [ options ] -o out-file.gtp in-file.bin\n\n");
	printf("Available options:	-n NAME		Name for the data block on the tape.\n"
	       "                                        Default is the name of the binary.\n"
	       "                        -b FILE         Append BASIC program stored in file\n"
	       "                                        FILE. Uses a built-in stub by default\n"
	       "                        -b none         Do not append any BASIC.\n"
	       "\n"
	       "Bug reports to tomaz.solc@tablix.org\n"
	       );
	exit(0);
}

int main(int argc, char **argv)
{
	int c;

	int fd,r;
	struct stat st;
	struct gtp_block *b;

	unsigned char *data;
	int datalen, binlen;

        while ((c=getopt(argc, argv, "ho:n:b:"))!=-1) {
                switch (c) {
                        case 'h': print_syntax();
				  return 0;
				  break;
			case 'o': a_outputfile=strdup(optarg);
				  break;
			case 'n': a_name=strdup(optarg);
				  break;
			case 'b': a_basicfile=strdup(optarg);
				  break;
                }
        }

        if (!(optind<argc)) {
                printf("Missing input file name. Try -h for help\n");
		return 1;
        }

	if(a_outputfile==NULL) {
                printf("Missing output file name. Try -h for help\n");
		return 1;
	}

	if(a_name==NULL) {
		a_name=strdup(argv[optind]);
	}

	if(stat(argv[optind], &st)) {
		perror("bin2gtp");
		return 1;
	}
	binlen=st.st_size;

	if(a_basicfile==NULL) {
		basic=basicdef;
		basiclen=basicdeflen;
	} else if(!strcmp(a_basicfile, "none")) {
		basic=basicdef;
		basiclen=0;
	} else {
		r=basic_read(a_basicfile, &basic, &basiclen);
		if(r) return 1;
	}

	datalen=4+binlen+basiclen;
	data=malloc(datalen);
	if(data==NULL) {
		fprintf(stderr, "Can't allocate memory\n");
		return 1;
	}

	/* basic start addr */
	h2le_short(0x2c3a+binlen, &data[0]);	
	/* basic end addr */
	h2le_short(0x2c3a+binlen+basiclen, &data[2]);

	fd=open(argv[optind], O_RDONLY+O_BINARY);
	if(fd<0) {
		perror("bin2gtp");
		return 1;
	}
	/* binary file */
	r=read(fd, &data[4], binlen);
	if(r!=binlen) {
		fprintf(stderr, "Expected %d bytes in %s, read %d bytes\n",
			binlen, argv[optind], r);
		return 1;
	}
	close(fd);

	/* basic */
	memcpy(&data[4+binlen], basic, basiclen); 

	//fd=creat(a_outputfile, 0644);
	fd=open(a_outputfile, O_CREAT+O_TRUNC+O_WRONLY+O_BINARY);
	if(fd<0) {
		perror("bin2gtp");
		return 1;
	}

	/* **** GTP Header **** */

	/* *** Name block *** */

	b=gtp_block_name_new(a_name);
	gtp_block_write(b, fd);
	gtp_block_free(b);

	/* *** Data block *** */

	b=gtp_block_standard_new(0x2c36, datalen, data);
	gtp_block_write(b, fd);
	gtp_block_free(b);

	close(fd);

	free(data);

	return 0;
}

