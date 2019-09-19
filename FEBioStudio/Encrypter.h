#pragma once

#ifdef HAS_SSH
#include <string>
#include <vector>
#include <openssl/aes.h>

#define KEYLENGTH 128

class CEncrypter
{
public:
	static CEncrypter* Instance();

	std::vector<unsigned char> Encrypt(std::string in);
	std::string Decrypt(std::vector<unsigned char> in, size_t length);


private:
	CEncrypter();
	CEncrypter(CEncrypter const&) {}
	CEncrypter& operator=(CEncrypter const&) {}
	virtual ~CEncrypter() {}

	static CEncrypter* instance;
	static AES_KEY enc_key, dec_key;
	static unsigned char aes_key[KEYLENGTH/8];
	static unsigned char aes_iv[AES_BLOCK_SIZE];
};

#endif
