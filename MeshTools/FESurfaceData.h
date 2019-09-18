#pragma once
#include "FEMeshData.h"
#include "FEGroup.h"
#include <vector>

class FESurfaceData : public FEMeshData
{
public:
	FESurfaceData(FEMesh* mesh = nullptr);
	FESurfaceData(const FESurfaceData& data);
	~FESurfaceData();
	void operator = (const FESurfaceData& data);
	double& operator [] (int index);

	void Create(FEMesh* mesh, FESurface* surface, FEMeshData::DATA_TYPE dataType);

	vector<double>* getData();

	FESurface* getSurface() {return m_surface;}

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	vector<double>	m_data;
	FESurface* m_surface;
};
