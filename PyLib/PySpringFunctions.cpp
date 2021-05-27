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

#include "PySpringFunctions.h"

#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/ModelDocument.h>
#include <MeshTools/GModel.h>
#include <MeshLib/MeshTools.h>
#include <GeomLib/GMeshObject.h>
#include "PyExceptions.h"


GDiscreteSpringSet* SpringSet_init(const char* name, char* type)
{
    auto wnd = PRV::getMainWindow();
    auto doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());

    if(!doc)
    {
        throw pyNoModelDocExcept();
    }

    auto gmodel = doc->GetGModel();

    auto set = new GDiscreteSpringSet(gmodel);

    if(strcmp(type, "Linear") == 0)
    {
        set->SetMaterial(new FELinearSpringMaterial);
    }
    else if(strcmp(type, "Nonlinear") == 0)
    {
        set->SetMaterial(new FENonLinearSpringMaterial);
    }
    else if(strcmp(type, "Hill") == 0)
    {
        set->SetMaterial(new FEHillContractileMaterial);
    }
    else
    {
        delete set;
        return nullptr;
    }

    set->SetName(name);

    gmodel->AddDiscreteObject(set);

    return set;
}

int FindOrMakeNode(vec3d r, double tol)
{
    auto wnd = PRV::getMainWindow();
    auto doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());
    if(!doc)
    {
        throw pyNoModelDocExcept();
    }

    auto po = dynamic_cast<GMeshObject*>(doc->GetActiveObject());
    if(!po)
    {
        throw pyGenericExcept("There is no currently selected object.");
    }

	// find closest node
	int imin = -1;
	double l2min = 0.0;
	FEMesh* m = po->GetFEMesh();
	int N = m->Nodes();
	imin = -1;
	for (int i = 0; i < N; ++i)
	{
		FENode& ni = m->Node(i);
		if (ni.IsExterior())
		{
			vec3d ri = m->LocalToGlobal(ni.r);

			double l2 = (r - ri).SqrLength();
			if ((imin == -1) || (l2 < l2min))
			{
				imin = i;
				l2min = l2;
			}
		}
	}
	if ((imin!=-1) && (l2min < tol*tol))
	{
		return po->MakeGNode(imin);
	}

    return po->AddNode(r);
}

void IntersectWithObject(vec3d& r0, vec3d& r1, double tol)
{
    auto wnd = PRV::getMainWindow();
    auto doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());
    if(!doc)
    {
        throw pyNoModelDocExcept();
    }

    auto po = dynamic_cast<GMeshObject*>(doc->GetActiveObject());
    if(!po)
    {
        throw pyGenericExcept("There is no currently selected object.");
    }

	FEMesh* mesh = po->GetFEMesh();

	vec3d n = r1 - r0; n.Normalize();

	vector<vec3d> intersections = FindAllIntersections(*mesh, r0, n);
	for (int i=0; i<intersections.size(); ++i)
	{
		vec3d q = intersections[i];

		// does the projection lie within tolerance
		double d = (q - r0).Length();
		if (d < tol)
		{
			// does it decrease the distance
			if ((r1 - q).Length() < (r1 - r0).Length())
			{
				r0 = q;
			}
		}
	}

	intersections = FindAllIntersections(*mesh, r1, -n);
	for (int i = 0; i<intersections.size(); ++i)
	{
		vec3d q = intersections[i];

		// does the projection lie within tolerance
		double d = (q - r1).Length();
		if (d < tol)
		{
			// does it decrease the distance
			if ((q - r0).Length() < (r1 - r0).Length())
			{
				r1 = q;
			}
		}
	}
}