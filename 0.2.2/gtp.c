#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "gtp.h"
#include "endian.h"

struct gtp_block *gtp_block_new()
{
	struct gtp_block *dest;

	dest=calloc(1, sizeof(*dest));
	if(dest==NULL) {
		fprintf(stderr, "Can't allocate memory\n");
		return NULL;
	}

	return dest;
}

void gtp_block_free(struct gtp_block *dest)
{
	if(dest->data!=NULL) free(dest->data);
	free(dest);
}

/* returns: -1 on error, 0 on success */
int gtp_block_write(struct gtp_block *dest, int fd)
{
	unsigned char buff[5];
	int r;

	buff[0]=dest->id;

	h2le_int(dest->len, &buff[1]);

	r=write(fd, &buff, 5);
	if(r!=5) {
		perror("gtp");
		return -1;
	}

	r=write(fd, dest->data, dest->len);
	if(r!=dest->len) {
		perror("gtp");
		return -1;
	}

	return 0;
}

/* returns: -1 on error, 0 on success, 1 on EOF */
int gtp_block_read(struct gtp_block *dest, int fd)
{
	unsigned char buff[5];
	int r;

	r=read(fd, &buff, 5);

	if(r==0) {
		/* end of file */
		return 1;
	} else if(r<0) {
		perror("gtp2wav");
		return -1;
	} else if(r!=5) {
		fprintf(stderr, "Incomplete block header at the end of file. "
				  "Ignoring.\n");
		return 1;
	}

	dest->id = buff[0];
	dest->len = le2h_int(&buff[1]);

	/* printf("%u %u\n", dest->id, dest->len); */

	dest->data = calloc(dest->len, sizeof(*dest->data));
	if(dest->data==NULL) {
		fprintf(stderr, "Can't allocate memory\n");
		return -1;
	}

	r=read(fd, dest->data, dest->len);

	if(r<0) {
		perror("gtp2wav");
		return -1;
	} else if(r!=dest->len) {
		fprintf(stderr, "Incomplete block at the end of file. "
				  "Ignoring.\n");
		return 1;
	}

	return 0;
}

int gtp_header_read(int fd)
{
	return 0;
}

int gtp_block_name_info(struct gtp_block *dest)
{
	if(dest->data[dest->len-1]!=0) {
		printf("Corrupted name block. Not NULL terminated.\n");
		return -1;
	}
	printf("          Name: %s\n", dest->data);
	return 0;
}

struct gtp_block *gtp_block_name_new(char *name)
{
	struct gtp_block *dest;

	dest=gtp_block_new();
	if(dest==NULL) {
		fprintf(stderr, "Can't allocate memory\n");
		return NULL;
	};

	dest->id=GTP_BLOCK_NAME;
	dest->len=strlen(name)+1;
	dest->data=(unsigned char *) strdup(name);

	return dest;
}

int gtp_block_standard_info(struct gtp_block *dest)
{
	unsigned int blocklen, datalen, start, end;
	int n;
	unsigned char checksum;
	unsigned char realchecksum;

	start=le2h_short(&dest->data[1]);
	end=le2h_short(&dest->data[3]);

	printf("    Start addr: 0x%04x\n", start);
	printf("      End addr: 0x%04x\n", end);

	blocklen = end - start;
	datalen = dest->len - 6;

	if(datalen > blocklen) {
		printf("%u bytes of junk after checksum\n", 
							datalen - blocklen);
		datalen=blocklen;
	} else if(datalen < blocklen) {
		printf("missing %u bytes in data block\n", 
							blocklen - datalen);
	}

	realchecksum = dest->data[datalen + 5];

	checksum=0;
	for(n=0;n<datalen+5;n++) {
		checksum=checksum + dest->data[n];
	}

	checksum=0xff - checksum;

	if(checksum != realchecksum) {
		printf("checksum error\n");
	}

	return 0;
}

struct gtp_block *gtp_block_standard_new(int start, int datalen, 
					 unsigned char *data)
{
	unsigned int blocklen, end;
	int n;
	struct gtp_block *dest;
	unsigned char cksum;

	dest=gtp_block_new();
	if(dest==NULL) {
		fprintf(stderr, "Can't allocate memory\n");
		return NULL;
	};

	blocklen=datalen+6;

	dest->id=GTP_BLOCK_STANDARD;
	dest->len=blocklen;
	dest->data=calloc(blocklen, sizeof(*dest->data));

	if(dest->data==NULL) {
		free(dest);
		fprintf(stderr, "Can't allocate memory\n");
		return NULL;
	}

	end=start+datalen;

	dest->data[0]=0xa5;
	h2le_short(start, &dest->data[1]);
	h2le_short(end, &dest->data[3]);

	memcpy(&dest->data[5], data, datalen);

	cksum=0;
	for(n=0;n<blocklen-1;n++) {
		cksum+=dest->data[n];
	}
	cksum=0xff-cksum;

	dest->data[blocklen-1]=cksum;

	return dest;
}
