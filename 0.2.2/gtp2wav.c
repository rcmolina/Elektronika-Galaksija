#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <sndfile.h>

#include "gtp.h"
#include "gtp_run.h"
#include "modulation.h"

char *a_outputfile=NULL;
int a_force=0;
int a_sf=11025;
int a_bits=8;

void print_syntax()
{
	printf("GTP2WAV, Convert Galaksija Tape files to WAV audio\n");
	printf("Copyright (c) 2007 Tomaz Solc\n\n");
	printf("SYNTAX: gtp2wav [ options ] -o out-file.wav in-file.gtp\n\n");
	printf("Available options:	-r rate		Desired sample rate [11025 Hz]\n"
	       "			-s size		Sample size [8 bits]\n"
	       "			-f		Try to continue even if errors were\n"
	       "					found in the GTP file [off]\n"
	       "\n"
	       "Bug reports to tomaz.solc@tablix.org\n"
	       );
	exit(0);
}

int main(int argc, char **argv)
{
	int c;

	int fd,n,r;
	struct gtp_block *blk;

        while ((c=getopt(argc, argv, "ho:fr:s:"))!=-1) {
                switch (c) {
                        case 'h': print_syntax();
				  return 0;
				  break;
			case 'o': a_outputfile=strdup(optarg);
				  break;
		        case 'f': a_force=1;
				  break;
			case 'r': r=sscanf(optarg, "%d", &a_sf);
			          if(r!=1) {
				  	fprintf(stderr, 
						"Invalid sample rate '%s'\n", 
								optarg);
					return 1;
				  }
				  break;
			case 's': r=sscanf(optarg, "%d", &a_bits);
			          if(r!=1) {
				  	fprintf(stderr, 
						"Invalid sample size '%s'\n", 
								optarg);
					return 1;
				  }
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
	fd=open(argv[optind], O_RDONLY+O_BINARY);
	if(fd<0) {
		perror("gtp2wav");
		return 1;
	}
	if(mod_init(a_sf, a_bits)) {
		return 1;
	}
	n=1;
	while(1) {
		blk=gtp_block_new();
		r=gtp_block_read(blk, fd);
		if(r<0) {
			if(!a_force) {
				break;
			}
		}
		if(r>0) {
			break;
		}
		//printf("block_run_%d\n",n);
		gtp_block_run(blk, n);
		//printf("block_free_%d\n",n);
		gtp_block_free(blk);
		n++;
		//printf("n=%d\n",n); 	
	}			
	if(mod_write(a_outputfile)) {
		return 1;
	}
	if(mod_close()) {
		return 1;
	}

	close(fd);

	if(r<0) {
		return 1;
	} else {
		return 0;
	}
}

