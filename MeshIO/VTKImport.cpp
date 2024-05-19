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

#include "VTKImport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
#include <VTKLib/VTKLegacyFileReader.h>
#include <VTKLib/VTKTools.h>

#ifdef LINUX
#include <wctype.h>
#endif
using namespace std;

VTKimport::VTKimport(FSProject& prj) : FSFileImport(prj)
{
}

VTKimport::~VTKimport(void)
{
}

bool VTKimport::Load(const char* szfile)
{
	VTK::vtkLegacyFileReader vtkReader;
	if (vtkReader.Load(szfile) == false)
		return errf("Failed opening file %s.", szfile);
	SetFileName(szfile);
	const VTK::vtkModel& vtk = vtkReader.GetVTKModel();
	return BuildMesh(vtk);
}

bool VTKimport::BuildMesh(const VTK::vtkModel& vtk)
{
	FSModel& fem = m_prj.GetFSModel();

	// get the number of nodes and elements
	const VTK::vtkPiece& piece = vtk.Piece(0);

	FSMesh* pm = VTKTools::BuildFEMesh(piece);

	GMeshObject* po = new GMeshObject(pm);
	po->Update();

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
