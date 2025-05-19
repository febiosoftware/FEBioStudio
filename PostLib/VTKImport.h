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
#include "FEFileReader.h"
#include <VTKLib/VTKModel.h>
#include <vector>

namespace Post {

class FEState;

class VTKFileImport : public FEFileReader
{
public:
	VTKFileImport(FEPostModel* fem);
	~VTKFileImport(void);

	bool Load(const char* szfile) override;

protected:
	virtual bool LoadVTKModel(const char* szfilename, VTK::vtkModel& vtk) = 0;

private:
	bool BuildMesh(const VTK::vtkPiece& piece);
	bool UpdateModel(const VTK::vtkPiece& piece);
	bool BuildState(double time, const VTK::vtkPiece& vtk);
	bool ProcessSeries(const char* szfile);

protected:
	FEState*	m_ps;
	bool		m_bmapNodes = false;
	std::vector<int> m_nodeMap;
	double	m_currentTime;
	int		m_fileCount;
	bool	m_processSeries = true;
};

class VTKImport : public VTKFileImport
{
public:
	VTKImport(FEPostModel* fem) : VTKFileImport(fem) {}

private:
	bool LoadVTKModel(const char* szfilename, VTK::vtkModel& vtk) override;
};

class VTUImport : public VTKFileImport
{
public:
	VTUImport(FEPostModel* fem) : VTKFileImport(fem) {}

private:
	bool LoadVTKModel(const char* szfilename, VTK::vtkModel& vtk) override;
};

class VTMImport : public VTKFileImport
{
public:
	VTMImport(FEPostModel* fem);

	bool LoadVTKModel(const char* szfilename, VTK::vtkModel& vtk) override;
};

class PVTUImport : public VTKFileImport
{
public:
	PVTUImport(FEPostModel* fem) : VTKFileImport(fem) { m_bmapNodes = true; }

private:
	bool LoadVTKModel(const char* szfilename, VTK::vtkModel& vtk) override;
};

}
