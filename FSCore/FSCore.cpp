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
#include "stdafx.h"
#include "FSCore.h"
#include <string.h>

std::string FSCore::beautify_string(const char* sz)
{
	std::string s;
	if ((sz == nullptr) || (*sz == 0)) return s;

	if (strncmp(sz, "FE", 2) == 0) sz += 2;

	int n = 0;
	char cp = 0;
	while (*sz)
	{
		char c = *sz;
		if ((n == 0) && islower(c))
		{
			c = toupper(c);
		}
		else if ((n > 0) && (isupper(c) && islower(cp)))
		{
			s += ' ';
		}
		else if (c == '_') c = ' ';

		s += c;
		sz++;
		n++;
		cp = c;
	}

	return s;
}
