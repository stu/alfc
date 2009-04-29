#ifndef LZSS_H_
#define LZSS_H_
#ifdef __cplusplus
extern "C"{
#endif

extern uint32_t lzss_encode(uint8_t *out_buff, uint32_t max_out, uint8_t *in_buff, uint32_t max_in);
extern uint32_t lzss_decode(uint8_t *out_buff, uint32_t max_out, uint8_t *in_buff, uint32_t max_in);

#ifdef __cplusplus
}
#endif
#endif /* LZSS_H_ */
