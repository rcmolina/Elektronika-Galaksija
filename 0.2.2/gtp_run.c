#include <stdio.h>

#include "gtp.h"
#include "gtp_run.h"
#include "modulation.h"

int gtp_block_run(struct gtp_block *dest, int n)
{
	int r;

	printf("--- block %d (%u bytes) ---\n", n, dest->len);

	switch(dest->id) {
		case GTP_BLOCK_STANDARD:
			r=gtp_block_run_standard(dest);
			break;
		case GTP_BLOCK_TURBO:
			r=gtp_block_run_unimplemented(dest);
			break;
		case GTP_BLOCK_NAME:
			r=gtp_block_run_name(dest);
			break;
		default:
			r=gtp_block_run_unknown(dest);
			break;
	}

	if(r) {
		return -1;
	} else {
		return 0;
	}
}

int gtp_block_run_unimplemented(struct gtp_block *dest)
{
	printf("Handling of block ID %02x is not implemented. Sorry.\n",
								dest->id);
	return 0;
}

int gtp_block_run_unknown(struct gtp_block *dest)
{
	printf("Block ID %02x is unknown (corrupted file or "
			"outdated version of gtp2wav).\n", dest->id);
	return 0;
}

int gtp_block_run_name(struct gtp_block *dest)
{
	return(gtp_block_name_info(dest));
}

int gtp_block_run_standard(struct gtp_block *dest)
{
	if(dest->data[0]!=0xa5) {
		printf("Corrupted standard block (wrong magic byte).\n");
		return -1;
	}

	if(dest->len<6) {
		printf("Corrupted standard block (too short).\n");
		return -1;
	}

	if(dest->data[1]==0xff) {
		printf("STANDARD DATA BLOCK : Galaksija Plus - header\n");
	} else {
		printf("STANDARD DATA BLOCK : Galaksija\n");
		gtp_block_standard_info(dest);
	}

	if(mod_interblock_pause()) return -1;
	if(mod_sync()) return -1;
	if(mod_interbyte_pause()) return -1; // este es el que tarda
		printf("wait please..\n");	
	if(mod_block(dest->data, dest->len)) return -1;
	return 0;
}

