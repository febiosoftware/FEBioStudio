#pragma once
#include "FEFileExport.h"

namespace Post {

	//-----------------------------------------------------------------------------
class FEVTKExport : public FEFileExport
{
public:
    FEVTKExport(void);
    ~FEVTKExport(void);
    
    bool Save(FEModel& fem, const char* szfile) override;

	void ExportAllStates(bool b);

private:
	bool WriteState(const char* szname, FEState* ps);
	bool FillNodeDataArray(vector<float>& val, FEMeshData& data);
	bool FillElementNodeDataArray(vector<float>& val, FEMeshData& meshData);
	bool FillElemDataArray(vector<float>& val, FEMeshData& data, FEPart& part);
    
private:
	void WriteHeader(FEState* ps);
	void WritePoints(FEState* ps);
	void WriteCells (FEState* ps);
	void WritePointData(FEState* ps);
	void WriteCellData(FEState* ps);

private:
	bool	m_bwriteAllStates;	// write all states

private:
	FILE*	m_fp;
};
}
