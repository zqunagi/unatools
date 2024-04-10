// md5.h

#ifndef MD5_H
#define MD5_H

#include <stdint.h>
#include <stddef.h>

#define MD5_BLOCK_SIZE 64
#define MD5_DIGEST_SIZE 16

typedef struct
{
  uint32_t count[2];
  uint32_t state[4];
  unsigned char buffer[MD5_BLOCK_SIZE];
} MD5_CTX;

void md5_init(MD5_CTX *ctx);
void md5_update(MD5_CTX *ctx, const unsigned char *data, size_t len);
void md5_final(unsigned char *digest, MD5_CTX *ctx);

#endif /* MD5_H */