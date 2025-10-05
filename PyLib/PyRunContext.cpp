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
#include "PyRunContext.h"
#include <FEBioStudio/GLDocument.h>
#include <FEBioStudio/FEBioStudio.h>
#include <assert.h>

PyRunContext* PyRunContext::This = nullptr;

PyRunContext* PyRunContext::Instance()
{
	if (This == nullptr) This = new PyRunContext;
	return This;
}

PyRunContext::PyRunContext()
{
	doc = nullptr;
	obj = nullptr;
	This = this;
}

void PyRunContext::SetDocument(CDocument* doc)
{
	PyRunContext* pc = Instance(); assert(pc);
	pc->doc = doc;

	CGLDocument* gldoc = dynamic_cast<CGLDocument*>(doc);
	if (gldoc) pc->obj = gldoc->GetActiveObject();
}

CDocument* PyRunContext::GetDocument()
{
	PyRunContext* pc = Instance(); assert(pc);
	return pc->doc;
}

GObject* PyRunContext::GetActiveObject()
{
	PyRunContext* pc = Instance(); assert(pc);
	return pc->obj;
}

void PyRunContext::Init()
{
	CDocument* doc = FBS::getActiveDocument();
	SetDocument(doc);
}
