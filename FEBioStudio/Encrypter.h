/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

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
    CEncrypter& operator=(CEncrypter const&) { return *this; }
	virtual ~CEncrypter() {}

	static CEncrypter* instance;
	static AES_KEY enc_key, dec_key;
	static unsigned char aes_key[KEYLENGTH/8];
	static unsigned char aes_iv[AES_BLOCK_SIZE];
};

#endif
