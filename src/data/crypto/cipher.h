#ifndef CERBERUS_DATA_CRYPTO_CIPHER_H
#define CERBERUS_DATA_CRYPTO_CIPHER_H

#include "../../Cerberus_global.h"
#include "../../types.h"

typedef struct evp_md_ctx_st EVP_MD_CTX;
typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
typedef struct evp_md_st EVP_MD;
typedef struct evp_cipher_st EVP_CIPHER;

namespace cerberus
{
    class ByteBuffer;

    class CERBERUS_EXPORT Cipher
    {
       private:
        EVP_MD* m_sha256;  // message-digest tool

        EVP_CIPHER_CTX* m_cipher_ctx;  // cipher context
        EVP_CIPHER* m_aes256;          // cipher tool

       public:
        Cipher();

        Cipher(const Cipher& other) = delete;

        ~Cipher();

        // Compute the SHA256 digest of the input buffer
        HASH256 computeDigest_SHA256(const ByteBuffer& input);

        // Encrypt the buffer and return the result
        OpResData<ByteBuffer> encryptData_AES256(const ByteBuffer& buf, const KEY256& key);

        // Decrypt the buffer and return the result
        OpResData<ByteBuffer> decryptData_AES256(const ByteBuffer& buf, const KEY256& key);
    };
}  // namespace cerberus

#endif  // CERBERUS_DATA_CRYPTO_CIPHER_H
