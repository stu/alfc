/* md5.c - an implementation of the MD5 algorithm and MD5 crypt */

#include <stdint.h>
#include <string.h>
#include "md5.h"


#define cpu_to_le32(x) (x)
#define le32_to_cpu(x) cpu_to_le32(x)

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x >> (32 - (n)))))

static uint32_t initstate[4] =
{
  0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476
};

static char s1[4] = {  7, 12, 17, 22 };
static char s2[4] = {  5,  9, 14, 20 };
static char s3[4] = {  4, 11, 16, 23 };
static char s4[4] = {  6, 10, 15, 21 };

static uint32_t T[64] =
{
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
  0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
  0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
  0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
  0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
  0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
  0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
  0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
  0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
  0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
  0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
  0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
  0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
  0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
  0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
  0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

static void md5_transform (uMD5State *state, const uint8_t block[64])
{
  int i, j;
  uint32_t a,b,c,d,tmp;
  const uint32_t *x = (uint32_t *) block;

  a = state->state[0];
  b = state->state[1];
  c = state->state[2];
  d = state->state[3];

  /* Round 1 */
  for (i = 0; i < 16; i++)
    {
      tmp = a + F (b, c, d) + le32_to_cpu (x[i]) + T[i];
      tmp = ROTATE_LEFT (tmp, s1[i & 3]);
      tmp += b;
      a = d; d = c; c = b; b = tmp;
    }
  /* Round 2 */
  for (i = 0, j = 1; i < 16; i++, j += 5)
    {
      tmp = a + G (b, c, d) + le32_to_cpu (x[j & 15]) + T[i+16];
      tmp = ROTATE_LEFT (tmp, s2[i & 3]);
      tmp += b;
      a = d; d = c; c = b; b = tmp;
    }
  /* Round 3 */
  for (i = 0, j = 5; i < 16; i++, j += 3)
    {
      tmp = a + H (b, c, d) + le32_to_cpu (x[j & 15]) + T[i+32];
      tmp = ROTATE_LEFT (tmp, s3[i & 3]);
      tmp += b;
      a = d; d = c; c = b; b = tmp;
    }
  /* Round 4 */
  for (i = 0, j = 0; i < 16; i++, j += 7)
    {
      tmp = a + I (b, c, d) + le32_to_cpu (x[j & 15]) + T[i+48];
      tmp = ROTATE_LEFT (tmp, s4[i & 3]);
      tmp += b;
      a = d; d = c; c = b; b = tmp;
    }

  state->state[0] += a;
  state->state[1] += b;
  state->state[2] += c;
  state->state[3] += d;
}

void md5_init(uMD5State *state)
{
  memcpy ((char *) state->state, (char *) initstate, sizeof (initstate));
  state->length = 0;
}

void md5_update (uMD5State *state, const uint8_t *input, int inputlen)
{
  int buflen = state->length & 63;
  state->length += inputlen;
  if (buflen + inputlen < 64)
    {
      memcpy (state->buffer + buflen, input, inputlen);
      buflen += inputlen;
      return;
    }

  memcpy (state->buffer + buflen, input, 64 - buflen);
  md5_transform (state, state->buffer);
  input += 64 - buflen;
  inputlen -= 64 - buflen;
  while (inputlen >= 64)
    {
      md5_transform (state, input);
      input += 64;
      inputlen -= 64;
    }
  memcpy (state->buffer, input, inputlen);
  buflen = inputlen;
}

void md5_final(uMD5State *state)
{
  int i, buflen = state->length & 63;

  state->buffer[buflen++] = 0x80;
  memset (state->buffer+buflen, 0, 64 - buflen);
  if (buflen > 56)
    {
      md5_transform (state, state->buffer);
      memset (state->buffer, 0, 64);
      buflen = 0;
    }

  *(uint32_t *) (state->buffer + 56) = cpu_to_le32 (8 * state->length);
  *(uint32_t *) (state->buffer + 60) = 0;
  md5_transform (state, state->buffer);

  for (i = 0; i < 4; i++)
    state->state[i] = cpu_to_le32 (state->state[i]);
}

