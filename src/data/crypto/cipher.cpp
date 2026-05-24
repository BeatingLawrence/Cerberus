#include "cipher.h"

#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <openssl/ssl3.h>

#include "../../exception/exception.h"
#include "../bytebuffer.h"

using namespace crb;

//=============================================================================
Cipher::Cipher()
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L

    m_sha256 = EVP_MD_fetch(NULL, "SHA256", NULL);  // fetch message-digest implementation

    m_cipher_ctx = EVP_CIPHER_CTX_new();                         // create cipher context
    m_aes256     = EVP_CIPHER_fetch(NULL, "AES-256-CBC", NULL);  // fetch cipher implementation

#else

    m_sha256 = (EVP_MD *)EVP_sha256();  // OpenSSL 1.1.1: built-in digest

    m_cipher_ctx = EVP_CIPHER_CTX_new();             // create cipher context
    m_aes256     = (EVP_CIPHER *)EVP_aes_256_cbc();  // OpenSSL 1.1.1: built-in cipher

#endif

    if (m_sha256 && m_cipher_ctx && m_aes256) return;

    throw cSystemExc("Cipher context initialization failure");
}
//=============================================================================
Cipher::~Cipher()
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L

    EVP_MD_free(m_sha256);  // free message-digest implementation

    EVP_CIPHER_free(m_aes256);  // free cipher implementation

#endif

    EVP_CIPHER_CTX_free(m_cipher_ctx);  // free cipher context
}
//=============================================================================
HASH256 Cipher::computeDigest_SHA256(const ByteBuffer &input)
{
    unsigned char hash[EVP_MAX_MD_SIZE] = {};

    if (!EVP_Digest(input.data(), input.size(), hash, NULL, (const EVP_MD *)m_sha256, NULL)) return HASH256();

    return HASH256(hash, 32);  // truncate 32 bytes
}
//=============================================================================
OpResData<ByteBuffer> Cipher::encryptData_AES256(const ByteBuffer &input, const KEY256 &key)
{
    ByteBuffer ret;
    ret.resize(((input.size() / 16) + 1) * 16);

    if (!EVP_CIPHER_CTX_reset(m_cipher_ctx)) return OR_Failure;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L

    if (!EVP_EncryptInit_ex2(

            m_cipher_ctx, m_aes256, key.p(), NULL, NULL))
        return OR_Failure;

#else

    if (!EVP_EncryptInit_ex(

            m_cipher_ctx, (const EVP_CIPHER *)m_aes256, NULL, key.p(), NULL))
        return OR_Failure;

#endif

    int written = 0;
    if (!EVP_EncryptUpdate(m_cipher_ctx, ret.data(), &written, input.data(), input.size())) return OR_Failure;

    int final = 0;
    if (!EVP_EncryptFinal_ex(m_cipher_ctx, ret.data(written), &final)) return OR_Failure;

    return ret;
}
//=============================================================================
OpResData<ByteBuffer> Cipher::decryptData_AES256(const ByteBuffer &input, const KEY256 &key)
{
    ByteBuffer ret;
    ret.resize(input.size());

    if (!EVP_CIPHER_CTX_reset(m_cipher_ctx)) return OR_Failure;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L

    if (!EVP_DecryptInit_ex2(

            m_cipher_ctx, m_aes256, key.p(), NULL, NULL))
        return OR_Failure;

#else

    if (!EVP_DecryptInit_ex(

            m_cipher_ctx, (const EVP_CIPHER *)m_aes256, NULL, key.p(), NULL))
        return OR_Failure;

#endif

    int written = 0;
    if (!EVP_DecryptUpdate(m_cipher_ctx, ret.data(), &written, input.data(), input.size())) return OR_Failure;

    int final = 0;
    if (!EVP_DecryptFinal_ex(m_cipher_ctx, ret.data(written), &final)) return OR_Failure;

    ret.resize(written + final);

    return ret;
}
//=============================================================================
