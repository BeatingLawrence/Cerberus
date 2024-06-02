#include <cerberus.h>
#include <data/crypto/cipher.h>
#include <gtest/gtest.h>

#define CRYPTO_KEY "thisisthekey112233"

using namespace cerberus;

TEST(cryptoTest, sha256)
{
    Cipher c;
    auto hash = c.computeDigest_SHA256("Hello, world!");

    ByteBuffer bb(hash.p(), 32);
    logInfo("hash of \"Hello, world!\" is \n%s", bb.toBinaryDump().c_str());
}

TEST(cryptoTest, aes256_enc_dec)
{
    Cipher c;
    ByteBuffer clear;

    for (int i = 0; i < 50; i++) clear += "Hello, world!\n";

    auto enc = c.encryptData_AES256(clear, CRYPTO_KEY);

    ASSERT_TRUE(enc.ok());

    logInfo("encrypted: %s", enc.value.toHex().c_str());

    auto dec = c.decryptData_AES256(enc.value, CRYPTO_KEY);

    ASSERT_TRUE(dec.ok());

    logInfo("decrypted: %s", dec.value.toString().c_str());

    ASSERT_TRUE(clear.isEqual(dec.value));
}
