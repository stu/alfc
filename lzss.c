/* LZSS encoder-decoder	 (c) Haruhiko Okumura */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#define EI 11  /* typically 10..13 */
#define EJ	4  /* typically 4..5 */
#define P	1  /* If match length <= P then output one character */
#define N (1 << EI)	 /* buffer size */
#define F ((1 << EJ) + P)  /* lookahead buffer size */

#define LZSS_EOF 	0xFFFF

int bit_buffer = 0, bit_mask = 128;
uint32_t codecount = 0, textcount = 0;
uint8_t buffer[N * 2];

uint32_t inbuff_length;
uint32_t inbuff_idx;

uint32_t outbuff_idx;
uint32_t outbuff_length;

uint8_t *inbuff;
uint8_t *outbuff;

int buf, mask = 0;

static void error(void)
{
	printf("Output error (outbuff_idx=%i, outbuff_length=%i, inbuff_idx=%i, inbuff_length=%i)\n", outbuff_idx, outbuff_length, inbuff_idx, inbuff_length);
	exit(1);
}

static void putbit1(void)
{
	bit_buffer |= bit_mask;
	if ((bit_mask >>= 1) == 0)
	{
		if (outbuff_idx >= outbuff_length)
			error();
		outbuff[outbuff_idx++] = bit_buffer;

		bit_buffer = 0;
		bit_mask = 128;
		codecount++;
	}
}

static void putbit0(void)
{
	if ((bit_mask >>= 1) == 0)
	{
		if (outbuff_idx >= outbuff_length)
			error();
		outbuff[outbuff_idx++] = bit_buffer;

		bit_buffer = 0;
		bit_mask = 128;
		codecount++;
	}
}

static void flush_bit_buffer(void)
{
	if (bit_mask != 128)
	{
		if (outbuff_idx >= outbuff_length)
			error();
		outbuff[outbuff_idx++] = bit_buffer;

		codecount++;
	}
}

static void output1(int c)
{
	int mask;

	putbit1();
	mask = 256;
	while (mask >>= 1)
	{
		if (c & mask)
			putbit1();
		else
			putbit0();
	}
}

static void output2(int x, int y)
{
	int mask;

	putbit0();
	mask = N;
	while (mask >>= 1)
	{
		if (x & mask)
			putbit1();
		else
			putbit0();
	}
	mask = (1 << EJ);
	while (mask >>= 1)
	{
		if (y & mask)
			putbit1();
		else
			putbit0();
	}
}

uint32_t lzss_encode(uint8_t *out_buff, uint32_t max_out, uint8_t *in_buff, uint32_t max_in)
{
	int i, j, f1, x, y, r, s, bufferend, c;

	outbuff_idx = 0;
	inbuff_idx = 0;

	inbuff_length = max_in;
	outbuff_length = max_out;

	outbuff = out_buff;
	inbuff = in_buff;

	bit_buffer = 0;
	bit_mask = 128;
	codecount = 0;
	textcount = 0;

	buf = 0;
	mask = 0;

	for (i = 0; i < N - F; i++)
		buffer[i] = ' ';

	for (i = N - F; i < N * 2; i++)
	{
		if (inbuff_idx >= inbuff_length)
			break;
		c = inbuff[inbuff_idx++];

		buffer[i] = c;
		textcount++;
	}

	bufferend = i;
	r = N - F;
	s = 0;

	while (r < bufferend)
	{
		f1 = (F <= bufferend - r) ? F : bufferend - r;

		x = 0;
		y = 1;
		c = buffer[r];

		for (i = r - 1; i >= s; i--)
		{
			if (buffer[i] == c)
			{
				for (j = 1; j < f1; j++)
					if (buffer[i + j] != buffer[r + j])
						break;

				if (j > y)
				{
					x = i;
					y = j;
				}
			}
		}

		if (y <= P)
			output1(c);
		else
			output2(x & (N - 1), y - 2);

		r += y;
		s += y;

		if (r >= N * 2 - F)
		{
			for (i = 0; i < N; i++)
				buffer[i] = buffer[i + N];

			bufferend -= N;
			r -= N;
			s -= N;

			while (bufferend < N * 2)
			{
				if (inbuff_idx >= inbuff_length)
					break;
				c = inbuff[inbuff_idx++];

				buffer[bufferend++] = c;
				textcount++;
			}
		}
	}

	flush_bit_buffer();
	//printf("lzss in (%i) out (%i) (%i%%)\n", inbuff_length, outbuff_idx, (outbuff_idx * 100) / inbuff_length);
	assert(codecount == outbuff_idx);

	return outbuff_idx;
}

static uint16_t getbit(int n) /* get n bits */
{
	int i, x;

	if (inbuff_idx >= inbuff_length)
		return LZSS_EOF;

	x = 0;
	for (i = 0; i < n; i++)
	{
		if (mask == 0)
		{
			if (inbuff_idx >= inbuff_length)
				return LZSS_EOF;

			buf = inbuff[inbuff_idx++];

			mask = 128;
		}
		x <<= 1;
		if (buf & mask)
			x++;
		mask >>= 1;
	}

	return x;
}

uint32_t lzss_decode(uint8_t *out_buff, uint32_t max_out, uint8_t *in_buff, uint32_t max_in)
{
	int i, j, k, r, c;

	outbuff_idx = 0;
	inbuff_idx = 0;

	inbuff_length = max_in;
	outbuff_length = max_out;

	outbuff = out_buff;
	inbuff = in_buff;

	bit_buffer = 0;
	bit_mask = 128;
	codecount = 0;
	textcount = 0;

	buf = 0;
	mask = 0;

	for (i = 0; i < N - F; i++)
		buffer[i] = ' ';

	r = N - F;
	while ((c = getbit(1)) != LZSS_EOF)
	{
		if (c)
		{
			if ((c = getbit(8)) == LZSS_EOF)
				break;

			if (outbuff_idx >= outbuff_length)
				error();
			outbuff[outbuff_idx++] = c;

			buffer[r++] = c;
			r &= (N - 1);
		}
		else
		{
			if ((i = getbit(EI)) == LZSS_EOF)
				break;
			if ((j = getbit(EJ)) == LZSS_EOF)
				break;
			for (k = 0; k <= j + 1; k++)
			{
				c = buffer[(i + k) & (N - 1)];

				if (outbuff_idx >= outbuff_length)
					error();
				outbuff[outbuff_idx++] = c;

				buffer[r++] = c;
				r &= (N - 1);
			}
		}
	}

	//printf("Output (outbuff_idx=%i, outbuff_length=%i, inbuff_idx=%i, inbuff_length=%i)\n", outbuff_idx, outbuff_length, inbuff_idx, inbuff_length);
	return outbuff_idx;
}

