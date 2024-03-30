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
#include "NIKE3DProject.h"

//-----------------------------------------------------------------------------
// Implements a class to import NIKE files
//
class NIKE3DImport : public FSFileImport
{
	enum {MAXLINE = 256};

public:
	NIKE3DImport(FSProject& prj);

	bool Load(const char* szfile);

protected:
	void UpdateFEModel(FSModel& fem);
	void UpdateMesh(FSMesh& mesh);
	int FindFace(int n[4], int noff = 0);

private:
	char* read_line(FILE* fp, char* szline, int n, bool bskiptxt = true);

	bool ReadControlDeck     (NIKE3DProject& prj);
	bool ReadMaterialDeck    (NIKE3DProject& prj);
	bool ReadNodes           (NIKE3DProject& prj);
	bool ReadBricks          (NIKE3DProject& prj);
	bool ReadShells          (NIKE3DProject& prj);
	bool ReadRigidFacets     (NIKE3DProject& prj);
	bool ReadDiscreteElements(NIKE3DProject& prj);
	bool ReadInterfaces      (NIKE3DProject& prj);
	bool ReadLoadCurves      (NIKE3DProject& prj);
	bool ReadNodalForces     (NIKE3DProject& prj);
	bool ReadPressureFacets  (NIKE3DProject& prj);
	bool ReadDisplacements   (NIKE3DProject& prj);
	bool ReadBodyForce       (NIKE3DProject& prj);
	bool ReadVelocities      (NIKE3DProject& prj);

private:
	void build_model      (NIKE3DProject& prj);
	void build_control    (NIKE3DProject& prj);
	void build_mesh       (NIKE3DProject& prj);
	void build_constraints(NIKE3DProject& prj);
	void build_materials  (NIKE3DProject& prj);
	void build_rigidfacets(NIKE3DProject& prj);
	void build_discrete   (NIKE3DProject& prj);
	void build_interfaces (NIKE3DProject& prj);
	void build_loadcurves (NIKE3DProject& prj);

private:
	GObject*	m_po;
	FSModel*	m_fem;

	int		m_nmat;	// nr of materials
	int		m_nmplc;	// nr of must point load curve

	std::vector<int>	m_iFace;
	std::vector<int*>	m_pFace;
	std::vector<int>	m_nFace;

	std::vector<LoadCurve>		m_LC;
	std::vector<GMaterial*>		m_pMat;
};
