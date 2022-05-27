#include "cipher.h"
#include <openssl/evp.h>
#include "../bytebuffer.h"

using namespace cerberus::data::crypto;

//=============================================================================
Cipher::Cipher()
{
    m_md_ctx = EVP_MD_CTX_new();
    m_cipher_ctx = EVP_CIPHER_CTX_new();
    m_sha256 = EVP_sha256();
    m_aes256 = EVP_aes_256_cbc();
}
//=============================================================================
Cipher::~Cipher()
{
    EVP_MD_CTX_free(m_md_ctx);
    EVP_CIPHER_CTX_free(m_cipher_ctx);
}
//=============================================================================
void Cipher::computeDigest_SHA256(const ByteBuffer& input, ByteBuffer& digest)
{
    unsigned int outlen = 0;
    EVP_DigestInit_ex(m_md_ctx, m_sha256, NULL);
    EVP_DigestUpdate(m_md_ctx, input.data(), input.size());
    digest.resize(32u);
    EVP_DigestFinal_ex(m_md_ctx, digest.data(), &outlen);
}
//=============================================================================
void Cipher::encryptData_AES256(const ByteBuffer& input, const ByteBuffer& key, ByteBuffer& crypted)
{
    EVP_EncryptInit_ex(m_cipher_ctx, m_aes256, NULL, key.data(), NULL);
    crypted.resize(((input.size() / 16) + 1) * 16);
    int wrote = 0;
    unsigned char* out = crypted.data();
    EVP_EncryptUpdate(m_cipher_ctx, &out[0], &wrote, input.data(), input.size());
    int wrote2;
    EVP_EncryptFinal_ex(m_cipher_ctx, &out[wrote], &wrote2);
}
//=============================================================================
void Cipher::decryptData_AES256(const ByteBuffer& input, const ByteBuffer& key, ByteBuffer& decrypted)
{
    EVP_DecryptInit_ex(m_cipher_ctx, m_aes256, NULL, key.data(), NULL);
    decrypted.resize(input.size());
    int wrote = 0;
    unsigned char* out = decrypted.data();
    EVP_DecryptUpdate(m_cipher_ctx, &out[0], &wrote, input.data(), input.size());
    int wrote2;
    EVP_DecryptFinal_ex(m_cipher_ctx, &out[wrote], &wrote2);
    decrypted.resize(wrote + wrote2);
}
//=============================================================================
