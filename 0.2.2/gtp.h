#define 	GTP_BLOCK_STANDARD	0x00
#define		GTP_BLOCK_TURBO		0x01
#define		GTP_BLOCK_NAME		0x10

struct gtp_block {
	unsigned char id;
	unsigned int len;
	unsigned char *data;
};

struct gtp_block *gtp_block_new();
void gtp_block_free(struct gtp_block *dest);

int gtp_header_read(int fd);

int gtp_block_read(struct gtp_block *dest, int fd);
int gtp_block_write(struct gtp_block *dest, int fd);

int gtp_block_name_info(struct gtp_block *dest);
struct gtp_block *gtp_block_name_new(char *name);

int gtp_block_standard_info(struct gtp_block *dest);
struct gtp_block *gtp_block_standard_new(int start, int datalen, 
					 unsigned char *data);
