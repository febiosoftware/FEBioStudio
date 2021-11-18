/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include "ParamBlock.h"
#include <FEBioStudio/ClassDescriptor.h>
#include <string>

//-----------------------------------------------------------------------------
// Base class for most classes used by FEBio Studio
// 
class FSObject : public ParamContainer
{
public:
	FSObject(FSObject* parent = nullptr);
	virtual ~FSObject(void);

	void SetName(const std::string& name);
	const std::string& GetName() const;

	void SetInfo(const std::string& info);
	const std::string& GetInfo() const;

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// update the object's data
	virtual bool Update(bool b = true);

	// update parameters
	// return true if parameter list was modified
	virtual bool UpdateData(bool bsave = true);

	const char* GetTypeString() const { return m_sztypeStr; }

public:
	void SetParent(FSObject* parent);
	FSObject* GetParent();
	const FSObject* GetParent() const;
	virtual size_t RemoveChild(FSObject* po);
	virtual void InsertChild(size_t pos, FSObject* po);

protected:
	void SetTypeString(const char* sz) { m_sztypeStr = sz; }

private:
	std::string		m_name;
	std::string		m_info;
	FSObject*		m_parent;

private:
	const char* m_sztypeStr;
};

