#pragma once

#include <MeshTools/FEFileExport.h>

//-----------------------------------------------------------------------------
struct VTKEXPORT
{
	bool	bshellthick;	// shell thickness
	bool	bscalar_data;   //user scalar data
};


class FEVTKExport : public FEFileExport
{
public:
	FEVTKExport(FEProject& prj);
	~FEVTKExport(void);

	bool Write(const char* szfile) override;
	void SetOptions(VTKEXPORT o) { m_ops = o; }
protected:
	VTKEXPORT m_ops;
};


