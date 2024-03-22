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
#include "FEFileExport.h"
#include "FEPostModel.h"
namespace Post {

//-----------------------------------------------------------------------------
class FEVTKExport : public FEFileExport
{
public:
    FEVTKExport(void);
    ~FEVTKExport(void);
    
    bool Save(FEPostModel& fem, const char* szfile) override;

	void ExportAllStates(bool b);
	void ExportSelectedElementsOnly(bool b);
	void WriteSeriesFile(bool b);

private:
	bool WriteState(const char* szname, FEState* ps);
	bool FillNodeDataArray(std::vector<float>& val, FEMeshData& data);
	bool FillElementNodeDataArray(std::vector<float>& val, FEMeshData& meshData);
	bool FillElemDataArray(std::vector<float>& val, FEMeshData& data);
    
private:
	void WriteHeader(FEState* ps);
	void WritePoints(FEState* ps);
	void WriteCells (FEState* ps);
	void WritePointData(FEState* ps);
	void WriteCellData(FEState* ps);

private:
	void WriteVTKSeriesFile(const char* szfile, std::vector<std::pair<std::string, float> >& series);

private:
	bool UpdateData(bool bsave) override;

private:
	bool	m_bwriteAllStates;	// write all states
	bool	m_bselElemsOnly;	// only output selected elements
	bool	m_bwriteSeriesFile;	// write the vtk.series file (only for writeAllStates)
	bool	m_bwritePartIDs;	// write the element part IDs as cell data

private:
	FILE*	m_fp;
	int		m_nodes;
	int		m_elems;
};
}
