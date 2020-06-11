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

#pragma once
#include <MeshIO/FileReader.h>
#include "FENikeProject.h"

//-----------------------------------------------------------------------------
// Implements a class to import NIKE files
//
class FENIKEImport : public FEFileImport
{
	enum {MAXLINE = 256};

public:
	FENIKEImport(FEProject& prj);

	bool Load(const char* szfile);

protected:
	void UpdateFEModel(FEModel& fem);
	void UpdateMesh(FEMesh& mesh);
	int FindFace(int n[4], int noff = 0);

private:
	char* read_line(FILE* fp, char* szline, int n, bool bskiptxt = true);

	bool ReadControlDeck     (FENikeProject& prj);
	bool ReadMaterialDeck    (FENikeProject& prj);
	bool ReadNodes           (FENikeProject& prj);
	bool ReadBricks          (FENikeProject& prj);
	bool ReadShells          (FENikeProject& prj);
	bool ReadRigidFacets     (FENikeProject& prj);
	bool ReadDiscreteElements(FENikeProject& prj);
	bool ReadInterfaces      (FENikeProject& prj);
	bool ReadLoadCurves      (FENikeProject& prj);
	bool ReadNodalForces     (FENikeProject& prj);
	bool ReadPressureFacets  (FENikeProject& prj);
	bool ReadDisplacements   (FENikeProject& prj);
	bool ReadBodyForce       (FENikeProject& prj);
	bool ReadVelocities      (FENikeProject& prj);

private:
	void build_model      (FENikeProject& prj);
	void build_control    (FENikeProject& prj);
	void build_mesh       (FENikeProject& prj);
	void build_constraints(FENikeProject& prj);
	void build_materials  (FENikeProject& prj);
	void build_rigidfacets(FENikeProject& prj);
	void build_discrete   (FENikeProject& prj);
	void build_interfaces (FENikeProject& prj);
	void build_loadcurves (FENikeProject& prj);

private:
	GObject*	m_po;
	FEModel*	m_fem;

	int		m_nmat;	// nr of materials
	int		m_nmplc;	// nr of must point load curve

	vector<int>		m_iFace;
	vector<int*>	m_pFace;
	vector<int>		m_nFace;

	vector<FELoadCurve>		m_LC;
	vector<GMaterial*>		m_pMat;
};
