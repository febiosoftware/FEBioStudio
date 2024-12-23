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
#include "PropertyList.h"
#include <QtCore/QString>
#include <vector>

class FSBase;
class FSObject;

class CObjectProps : public CPropertyList
{
public:
	CObjectProps(FSBase* po, bool beautifyStrings = true);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

	virtual FSBase* GetFEObject() { return m_po; }

	int Params() const { return (int) m_params.size(); }

protected:
	virtual void BuildParamList(FSBase* po, bool showNonPersistent = false);

	void AddParameter(Param& p);
	void AddParameterList(FSBase* po);
	QVariant GetPropertyValue(Param& p);
	void SetPropertyValue(Param& p, const QVariant& v);

	virtual QStringList GetEnumValues(const char* ch);

protected:
	FSBase*			m_po;
	std::vector<Param*>	m_params;
	bool m_beautify;
};
