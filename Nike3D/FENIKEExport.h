#pragma once
#include <MeshTools/FEFileExport.h>
#include "FENikeProject.h"

//-----------------------------------------------------------------------------
// This class takes an FEProject, converts it to a Nike project and
// stores it to a file.
//
class FENIKEExport : public FEFileExport
{
public:
	FENIKEExport(FEProject& prj);
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
