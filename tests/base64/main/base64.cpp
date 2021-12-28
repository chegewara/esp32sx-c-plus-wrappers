#include <stdio.h>

#include "base64-comp.h"

#define STRING_TEXT "test string to encode to base64 string"
#define BASE64_TEXT "dGhpcyBpcyBiYXNlNjQgZW5jb2RlZCB0ZXh0"

extern "C" void app_main(void)
{
    int plain_txt_len = sizeof(STRING_TEXT) - 1; // dont count trailing \0
    int enc_len = Base64::encodedSize(STRING_TEXT, plain_txt_len);
    char* enc_txt_buf = (char*)malloc(enc_len + 1);
    printf("encode status/len: %d\n", Base64::encode(STRING_TEXT, plain_txt_len, enc_txt_buf, enc_len));
    printf("encoded string: %s\n", enc_txt_buf);
    int base64_len = sizeof(BASE64_TEXT) - 1; // dont count trailing \0
    int dec_len = Base64::decodedSize(BASE64_TEXT, base64_len);
    char* dec_txt_buf = (char*)malloc(dec_len + 1);
    printf("decode status/len: %d\n", Base64::decode(BASE64_TEXT, base64_len, dec_txt_buf, dec_len));
    printf("decoded string: %.*s\n", dec_len, dec_txt_buf);
}
