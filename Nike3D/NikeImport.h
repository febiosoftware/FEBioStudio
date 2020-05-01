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
