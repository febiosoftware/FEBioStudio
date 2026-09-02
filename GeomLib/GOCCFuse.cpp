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
#include "GOCCFuse.h"

#ifdef HAS_OCC
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#endif

GOCCFuse::GOCCFuse() : GOCCModifier("Fuse")
{
}

GOCCObject* GOCCFuse::Apply(GOCCObject* po)
{
#ifdef HAS_OCC
	Reset();
	if (po == nullptr) return nullptr;

	TopoDS_Shape shape = po->GetShape();

	std::vector<TopoDS_Shape> solids;
	for (TopExp_Explorer it(shape, TopAbs_SOLID); it.More(); it.Next())
	{
		solids.push_back(TopoDS::Solid(it.Current()));
	}

	if (solids.empty()) return nullptr;

	TopoDS_Shape result = solids[0];

	for (size_t i = 1; i < solids.size(); ++i)
	{
		BRepAlgoAPI_Fuse fuse(result, solids[i]);
		fuse.Build();

		if (!fuse.IsDone() || fuse.HasErrors()) return nullptr;

		result = fuse.Shape();
	}

	ShapeUpgrade_UnifySameDomain unify(result, Standard_True, Standard_True, Standard_True);
	unify.Build();
	result = unify.Shape();

	GOCCObject* occ = new GOCCObject;
	occ->SetShape(result);
	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}
