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
#include <FSCore/FSThreadedTask.h>
#include "FEBioOpt.h"
#include <QString>

class CModelDocument;

class CFEBioStudy : public FSThreadedTask
{
public:
	CFEBioStudy(CModelDocument* doc);

	CModelDocument* GetDocument() { return m_doc; }

	virtual bool Run() = 0;

	virtual QString GetOutputFileName() const { return QString(); }

private:
	CModelDocument* m_doc;
};

class COptimizationStudy : public CFEBioStudy
{
public:
	COptimizationStudy(CModelDocument* doc);

	void SetOptions(FEBioOpt ops) { m_ops = ops; }
	FEBioOpt& Options() { return m_ops; }

	bool Run() override;

	QString GetOutputFileName() const override { return m_logFileName; }

private:
	FEBioOpt m_ops;
	QString m_logFileName;
};
