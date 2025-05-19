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

#include "VTUImport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
#include <VTKLib/VTUFileReader.h>
#include <VTKLib/PVTUFileReader.h>
#include <VTKLib/VTPFileReader.h>
#include <VTKLib/VTKTools.h>

bool BuildMeshFromVTKModel(FSProject& prj, const VTK::vtkModel& vtk);

VTUimport::VTUimport(FSProject& prj) : FSFileImport(prj) 
{ 

}

bool VTUimport::Load(const char* szfile)
{
	VTK::VTUFileReader vtu;

	if (vtu.Load(szfile) == false)
	{
		setErrorString(vtu.GetErrorString());
		return false;
	}

	const VTK::vtkModel& vtk = vtu.GetVTKModel();
	return BuildMeshFromVTKModel(GetProject(), vtk);
}

VTPimport::VTPimport(FSProject& prj) : FSFileImport(prj) { }

bool VTPimport::Load(const char* szfile)
{
	VTK::VTPFileReader vtp;

	if (vtp.Load(szfile) == false)
	{
		setErrorString(vtp.GetErrorString());
		return false;
	}

	const VTK::vtkModel& vtk = vtp.GetVTKModel();
	return BuildMeshFromVTKModel(GetProject(), vtk);
}

bool BuildMeshFromVTKModel(FSProject& prj, const VTK::vtkModel& vtk)
{
	FSModel& fem = prj.GetFSModel();

	for (int n = 0; n < vtk.Pieces(); ++n)
	{
		const VTK::vtkPiece& piece = vtk.Piece(n);

		FSMesh* pm = VTKTools::BuildFEMesh(piece);
		if (pm == nullptr) return false;
	
		GMeshObject* po = new GMeshObject(pm);
		po->Update();

		char szname[256];
		sprintf(szname, "vtkObject%d", n + 1);
		po->SetName(szname);
		fem.GetModel().AddObject(po);
	}

	return true;
}

PVTUimport::PVTUimport(FSProject& prj) : FSFileImport(prj)
{

}

bool PVTUimport::Load(const char* szfile)
{
	VTK::PVTUFileReader pvtu;

	if (pvtu.Load(szfile) == false)
	{
		setErrorString(pvtu.GetErrorString());
		return false;
	}

	const VTK::vtkModel& vtk = pvtu.GetVTKModel();
	return BuildMeshFromVTKModel(GetProject(), vtk);
}
