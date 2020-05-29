#include "Encrypter.h"
#ifdef HAS_SSH
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>

CEncrypter* CEncrypter::instance = nullptr;
AES_KEY CEncrypter::enc_key = AES_KEY();
AES_KEY CEncrypter::dec_key = AES_KEY();
unsigned char CEncrypter::aes_key[KEYLENGTH/8] = "";
unsigned char CEncrypter::aes_iv[AES_BLOCK_SIZE] = "";
//unsigned char CEncrypter::iv_dec[AES_BLOCK_SIZE] = "";
//unsigned char CEncrypter::iv_enc[AES_BLOCK_SIZE] = "";


CEncrypter::CEncrypter()
{
	// generate aes key
	memset(aes_key, 0, KEYLENGTH/8);
	RAND_bytes(aes_key, KEYLENGTH/8);

	// init vector
	RAND_bytes(aes_iv, AES_BLOCK_SIZE);

	// set encryption key
	AES_set_encrypt_key(aes_key, KEYLENGTH, &enc_key);

	// set decryption key
	AES_set_decrypt_key(aes_key, KEYLENGTH, &dec_key);
}


CEncrypter* CEncrypter::Instance()
{
	if(!instance)
	{
		instance = new CEncrypter();
	}

	return instance;
}

std::vector<unsigned char> CEncrypter::Encrypt(std::string in)
{
	std::vector<unsigned char> out(((in.length() + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE);

	unsigned char iv_enc[AES_BLOCK_SIZE];
	memcpy(iv_enc, aes_iv, AES_BLOCK_SIZE);
	AES_cbc_encrypt((unsigned char*)in.c_str(), &out[0], in.length(), &enc_key, iv_enc, AES_ENCRYPT);

	return out;
}

std::string CEncrypter::Decrypt(std::vector<unsigned char> in, size_t length)
{
	const size_t encslength = ((length + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;

	std::vector<unsigned char> uout(encslength);
	std::vector<char> out(encslength);

	unsigned char iv_dec[AES_BLOCK_SIZE];
	memcpy(iv_dec, aes_iv, AES_BLOCK_SIZE);
	AES_cbc_encrypt(&in[0], &uout[0], encslength, &dec_key, iv_dec, AES_DECRYPT);

	for(int i = 0; i < encslength; i++)
	{
		out[i] = uout[i];
	}

	return std::string(&out[0]);
}
#endif // HAS_SSH
