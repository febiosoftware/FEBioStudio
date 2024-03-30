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
#include <MeshIO/FSFileImport.h>
#include <FEMLib/FSProject.h>

//-----------------------------------------------------------------------------
//! This class imports an 8-bit 3D RAW image file and converts it to a FE model
//! by making each voxel a hex-element. Different gray scales are assigned to 
//! different partitions.
class RAWToMeshImport : public FSFileImport
{
public:
	RAWToMeshImport(FSProject& prj);
	~RAWToMeshImport();

	//! Set the image dimensions
	void SetImageDimensions(int nx, int ny, int nz);

	//! Set box position and size
	void SetBoxSize(double x0, double y0, double z0, double w, double h, double d);

	//! Load the image data
	bool Load(const char* szfile);

	bool UpdateData(bool bsave) override;

protected:
	int	m_ntype;
	int	m_nx, m_ny, m_nz;		//!< image dimensions
	double	m_x0, m_y0, m_z0;	//!< position of box
	double	m_w, m_h, m_d;		//!< size of box
	FSModel*	m_pfem;
};
