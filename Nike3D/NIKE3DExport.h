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
#include <MeshIO/FSFileExport.h>
#include "NIKE3DProject.h"

//-----------------------------------------------------------------------------
// This class takes an FSProject, converts it to a Nike3d project and
// stores it to a file.
//
class NIKE3DExport : public FSFileExport
{
public:
	NIKE3DExport(FSProject& prj);
	virtual ~NIKE3DExport();

	bool Write(const char* szfile);

protected:
	bool ExportControl      (NIKE3DProject& prj);
	bool ExportMaterials    (NIKE3DProject& prj);
	bool ExportNodes        (NIKE3DProject& prj);
	bool ExportHexElements  (NIKE3DProject& prj);
	bool ExportShellElements(NIKE3DProject& prj);
	bool ExportRigid        (NIKE3DProject& prj);
	bool ExportDiscrete		(NIKE3DProject& prj);
	bool ExportSliding      (NIKE3DProject& prj);
	bool ExportLoadCurve    (NIKE3DProject& prj);
	bool ExportNodalLoads   (NIKE3DProject& prj);
	bool ExportPressure     (NIKE3DProject& prj);
	bool ExportDisplacement (NIKE3DProject& prj);
	bool ExportBodyForce    (NIKE3DProject& prj);
	bool ExportVelocities   (NIKE3DProject& prj);

private:
	FILE*	m_fp;		//!< the file pointer
	bool	m_bcomm;	//!< add comments or not?
};
