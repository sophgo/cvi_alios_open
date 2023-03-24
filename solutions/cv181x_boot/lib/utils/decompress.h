#ifndef __DECOMPRESS_H__
#define __DECOMPRESS_H__
#define DECOMP_ALLOC_SIZE (1 << 20)
#define DECOMP_ALLOC_ADDR 0x81800000 
#define DECOMP_BUF_ADDR (DECOMP_ALLOC_ADDR + DECOMP_ALLOC_SIZE)

enum COMPRESS_TYPE { COMP_NONE, COMP_LZMA, COMP_LZ4 };

int decompress(void *dst, size_t *dst_size, const void *src, size_t src_size, enum COMPRESS_TYPE type);

#endif /* __DECOMPRESS_H__ */
