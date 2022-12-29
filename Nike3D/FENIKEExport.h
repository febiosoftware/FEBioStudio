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
#include <MeshIO/FEFileExport.h>
#include "FENikeProject.h"

//-----------------------------------------------------------------------------
// This class takes an FSProject, converts it to a Nike project and
// stores it to a file.
//
class FENIKEExport : public FEFileExport
{
public:
	FENIKEExport(FSProject& prj);
	virtual ~FENIKEExport();

	bool Write(const char* szfile);

protected:
	bool ExportControl      (FENikeProject& prj);
	bool ExportMaterials    (FENikeProject& prj);
	bool ExportNodes        (FENikeProject& prj);
	bool ExportHexElements  (FENikeProject& prj);
	bool ExportShellElements(FENikeProject& prj);
	bool ExportRigid        (FENikeProject& prj);
	bool ExportDiscrete		(FENikeProject& prj);
	bool ExportSliding      (FENikeProject& prj);
	bool ExportLoadCurve    (FENikeProject& prj);
	bool ExportNodalLoads   (FENikeProject& prj);
	bool ExportPressure     (FENikeProject& prj);
	bool ExportDisplacement (FENikeProject& prj);
	bool ExportBodyForce    (FENikeProject& prj);
	bool ExportVelocities   (FENikeProject& prj);

private:
	FILE*	m_fp;		//!< the file pointer
	bool	m_bcomm;	//!< add comments or not?
};
