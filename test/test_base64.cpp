#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <iostream>
#include <string>

// Base64编码
std::string base64_encode(const std::string &input) {
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

// Base64解码
std::string base64_decode(const std::string &input) {
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

int main() {
    std::string originalString = "Hello, Base64!";
    
    // Base64编码
    std::string encodedString = base64_encode(originalString);
    std::cout << "Encoded: " << encodedString << std::endl;

    // Base64解码
    std::string decodedString = base64_decode(encodedString);
    std::cout << "Decoded: " << decodedString << std::endl;

    return 0;
}