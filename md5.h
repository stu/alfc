#ifndef MD5_H
#define MD5_H
#ifdef __cplusplus
extern "C"{
#endif

typedef struct udtMD5State
{
	uint32_t state[4];
	uint32_t length;
	uint8_t buffer[64];
} uMD5State;

extern void md5_init(uMD5State *state);
extern void md5_update (uMD5State *state, const uint8_t *input, int inputlen);
extern void md5_final(uMD5State *state);

#ifdef __cplusplus
}
#endif
#endif // MD5_H
