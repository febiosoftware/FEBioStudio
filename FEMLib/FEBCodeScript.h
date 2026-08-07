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
#pragma once
#include <string>
#include <FSCore/FSObject.h>
#include <FECore/FEScriptedBehavior.h>

class FEBCodeScript : public FSObject
{
public:
	FEBCodeScript(const std::string& name, const std::string& code);

	void SetScriptContext(const ScriptContext& c) { context = c; }
	ScriptContext GetScriptContext() const { return context; }

	void SetID(int n) { id = n; }
	int GetID() const { return id; }

	void SetCode(const std::string& s) { code = s; }
	std::string GetCode() const { return code; }

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

public:
	// reference counting for keeping track of how many components are using this script
	void ResetRefCount() { m_refs = 0; }
	void IncRef() { m_refs++; }
	void DecRef() { if (m_refs > 0) m_refs--; }
	int GetRefCount() const { return m_refs; }

private:
	int id; // unique ID for the script, assigned by the model when the script is added to the model
	std::string code;
	ScriptContext context;

	int m_refs = 0;
};
