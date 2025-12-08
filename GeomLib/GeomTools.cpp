/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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
#include "GeomTools.h"
using namespace std;

GObject* GeomTools::CloneObject(GObject* po)
{
	if (po == nullptr) return nullptr;

	// clone counter
	static int n = 1;

	// clone the object
	GObject* pco = po->Clone();
	if (pco == 0) return nullptr;

	pco->CopyTransform(po);
	pco->SetMaterial(po->GetMaterial());

	// set a new name
	char sz[256];
	sprintf(sz, "Clone%02d", n++);
	pco->SetName(sz);

	// copy material assignments
	assert(pco->Parts() == po->Parts());
	if (pco->Parts() == po->Parts())
	{
		int NP = po->Parts();
		for (int i = 0; i < NP; ++i)
		{
			GPart* srcPart = po->Part(i);
			GPart* dstPart = pco->Part(i);

			dstPart->SetMaterialID(srcPart->GetMaterialID());
		}
	}

	return pco;
}

vector<GObject*> GeomTools::CloneGrid(GObject* po, int x0, int x1, int y0, int y1, int z0, int z1, double dx, double dy, double dz)
{
	// clone counter
	static int n = 1;

	// sanity checks
	if (x1 < x0) { int tmp = x0; x0 = x1; x1 = tmp; }
	if (y1 < y0) { int tmp = y0; y0 = y1; y1 = tmp; }
	if (z1 < z0) { int tmp = z0; z0 = z1; z1 = tmp; }

	// list of cloned objects
	vector<GObject*> newObjects;

	for (int i = x0; i <= x1; ++i)
		for (int j = y0; j <= y1; ++j)
			for (int k = z0; k <= z1; ++k)
				if ((i != 0) || (j != 0) || (k != 0))
				{
					// clone the object
					GObject* pco = po->Clone();
					if (pco == 0) return newObjects;

					// set a new name
					char sz[256];
					sprintf(sz, "GridClone%02d", n++);
					pco->SetName(sz);

					// copy material assignments
					assert(pco->Parts() == po->Parts());
					if (pco->Parts() == po->Parts())
					{
						int NP = po->Parts();
						for (int i = 0; i < NP; ++i)
						{
							GPart* srcPart = po->Part(i);
							GPart* dstPart = pco->Part(i);

							dstPart->SetMaterialID(srcPart->GetMaterialID());
						}
					}

					// apply transform
					pco->GetTransform().Translate(vec3d(i * dx, j * dy, k * dz));

					newObjects.push_back(pco);
				}

	return newObjects;
}

vector<GObject*> GeomTools::CloneRevolve(GObject* po, int count, double range, double spiral, const vec3d& center, const vec3d& axis, bool rotateClones)
{
	// clone counter
	static int n = 1;

	vector<GObject*> newObjects;

	// make sure there is work to do
	if (count <= 0) return newObjects;

	// get the source object's position
	vec3d r = po->GetTransform().GetPosition();

	vec3d normAxis = axis.Normalized();

	// create clones
	for (int i = 0; i < count; ++i)
	{
		double w = DEG2RAD * ((i + 1) * range / (count + 1));

		double d = (i + 1) * spiral / count;

		quatd Q = quatd(w, axis);

		// clone the object
		GObject* pco = po->Clone();
		if (pco == 0) return newObjects;

		// set a new name
		char sz[256];
		sprintf(sz, "RevolveClone%02d", n++);
		pco->SetName(sz);

		// calculate new positions
		if (rotateClones) pco->GetTransform().Rotate(Q, center);
		else
		{
			vec3d pos = center + Q * (r - center);
			pco->GetTransform().SetPosition(pos);
		}
		pco->GetTransform().Translate(normAxis * d);

		// add the object
		newObjects.push_back(pco);
	}

	// all done
	return newObjects;
}
