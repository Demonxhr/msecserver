#include "base64.hpp"
#include <iostream>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <cstring>
using namespace std;

Base64::Base64()
{
}

Base64::~Base64()
{
}

std::string Base64::Base64Encode(const std::string &input)
{
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, input.c_str(), static_cast<int>(input.length()));
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);

    return std::string(bufferPtr->data, bufferPtr->length);
}

std::string Base64::Base64Decode(const std::string &input)
{
    BIO *bio, *b64;
    char buffer[4096];
    int len;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(input.c_str(), static_cast<int>(input.length()));
    bio = BIO_push(b64, bio);

    len = BIO_read(bio, buffer, sizeof(buffer));
    BIO_free_all(bio);

    return std::string(buffer, len);
}
