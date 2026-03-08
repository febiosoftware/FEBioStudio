/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2026 University of Utah, The Trustees of Columbia University in
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
#include "FEBCodeScript.h"

FEBCodeScript::FEBCodeScript(const std::string& name, const std::string& code) : FSObject(nullptr), code(code)
{
	id = -1;
	SetName(name);
	SetTypeString("febcode");
}

void FEBCodeScript::Save(OArchive& ar)
{
	std::string name = GetName();
	ar.WriteChunk(CID_SCRIPT_ID  , id);
	ar.WriteChunk(CID_SCRIPT_NAME, name);
	ar.WriteChunk(CID_SCRIPT_CODE, code);
}

void FEBCodeScript::Load(IArchive& ar)
{
	std::string name;
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();
		if      (ntype == CID_SCRIPT_ID  ) ar.read(id);
		else if (ntype == CID_SCRIPT_NAME) ar.read(name);
		else if (ntype == CID_SCRIPT_CODE) ar.read(code);
		else throw ReadError("unknown CID in FEBCodeScript::Load");
		ar.CloseChunk();
	}
	assert(id != -1);
	SetName(name);
}
