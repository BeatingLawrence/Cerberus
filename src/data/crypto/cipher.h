#ifndef CERBERUS_DATA_CRYPTO_CIPHER_H
#define CERBERUS_DATA_CRYPTO_CIPHER_H

#include <openssl/ssl3.h>

namespace cerberus
{
    namespace data
    {
        class ByteBuffer;
        namespace crypto
        {

            class Cipher
            {
                private:
                    EVP_MD_CTX*         m_md_ctx;       //message-digest context
                    EVP_CIPHER_CTX*     m_cipher_ctx;   //cipher context
                    const EVP_MD*       m_sha256;       //message-digest tool
                    const EVP_CIPHER*   m_aes256;       //cipher tool

                public:
                    Cipher();

                    ~Cipher();

                    //Computes the SHA256 digest of the input buffer and puts the result in
                    //the output buffer which is automatically allocated (32bytes).
                    void computeDigest_SHA256(const ByteBuffer& input, ByteBuffer& digest);

                    //Crypts the input buffer and puts the result in output buffer which is automatically allocated (with padding).
                    void encryptData_AES256(const ByteBuffer& input, const ByteBuffer& key, ByteBuffer& crypted);

                    //Decrypts the input buffer and puts the result in output buffer which is automatically allocated.
                    void decryptData_AES256(const ByteBuffer& input, const ByteBuffer& key, ByteBuffer& decrypted);
            };
        }
    }
}

#endif // CERBERUS_DATA_CRYPTO_CIPHER_H
