/**
 * FROM: ECC-Encryptor/ECC/Base64.h at master ¡¤ disymayufei/ECC-Encryptor
 * URL: https://github.com/disymayufei/ECC-Encryptor/blob/master/ECC/Base64.h
*/
#pragma once
#ifndef AKINET_CRYPTO_BASE64_H
#define AKINET_CRYPTO_BASE64_H
class Base64 {
private:
	std::string _base64_table;
	static const char base64_pad = '=';
public:
	Base64();
	std::string Encode(const unsigned char* str, int bytes);
	std::string Decode(const std::string str);
};
#endif // !AKINET_CRYPTO_BASE64_H

