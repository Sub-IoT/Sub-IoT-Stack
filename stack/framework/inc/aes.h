#ifndef _AES_H_
#define _AES_H_

#include <types.h>


#define AES_BLOCK_SIZE 16

// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES128 encryption in CBC-mode of operation and handles 0-padding.
// ECB enables the basic ECB 16-byte block algorithm. Both can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
  #define CBC 0 // We don't need this mode in the latest DASH7 specification
#endif

#ifndef ECB
  #define ECB 1
#endif

#ifndef CTR
  #define CTR 1
#endif

#if defined(ECB) && ECB

// The two functions AES128_ECB_xxcrypt() do most of the work, and they expect inputs of 128 bit length.
void AES128_ECB_encrypt(uint8_t *input, const uint8_t *key, uint8_t *output);
void AES128_ECB_decrypt(uint8_t *input, const uint8_t *key, uint8_t *output);

#endif // #if defined(ECB) && ECB


#if defined(CBC) && CBC

void AES128_CBC_encrypt_buffer(uint8_t *output, uint8_t *input, uint32_t length, const uint8_t *key, const uint8_t *iv);
void AES128_CBC_decrypt_buffer(uint8_t *output, uint8_t *input, uint32_t length, const uint8_t *key, const uint8_t *iv);

#endif // #if defined(CBC) && CBC

#if defined(CTR) && CTR
void AES128_CTR_encrypt(uint8_t *output, uint8_t *input, uint32_t length, const uint8_t *key, uint8_t* ctr_blk);
// Decryption is exactly the same operation as encryption

#endif // #if defined(CTR) && CTR

error_t AES128_CBC_MAC( uint8_t *auth, uint8_t *input, uint32_t length,
                        const uint8_t *key, const uint8_t *iv, uint32_t iv_len,
                        const uint8_t *add, uint32_t add_len, uint8_t auth_len );

error_t AES128_CCM_encrypt( uint8_t *output, uint8_t *input, uint32_t length,
                            const uint8_t *key, const uint8_t *iv, uint32_t iv_len,
                            const uint8_t *add, uint32_t add_len, uint8_t auth_len );

error_t AES128_CCM_decrypt( uint8_t *output, uint8_t *input, uint32_t length,
                        const uint8_t *key, const uint8_t *iv, uint32_t iv_len,
                        const uint8_t *add, uint32_t add_len, const uint8_t *auth,
                        uint8_t auth_len );

#endif //_AES_H_
