#pragma once
#include "FEObject.h"

class FESurfaceMesh;
class FEGroup;

//-------------------------------------------------------------------
// Class for modifying surface meshes
class FESurfaceModifier : public FEObject
{
public:
	FESurfaceModifier(const std::string& name = "");
	virtual ~FESurfaceModifier();

	virtual FESurfaceMesh* Apply(FESurfaceMesh* pm) { return 0; }
	virtual FESurfaceMesh* Apply(FESurfaceMesh* pm, FEGroup* pg) { return Apply(pm); }

	static bool SetError(const char* szerr, ...);

	static std::string GetErrorString();

protected:
	static std::string	m_error;
};

//-----------------------------------------------------------------------------
class FESurfacePartitionSelection : public FESurfaceModifier
{
public:
	FESurfacePartitionSelection();
	FESurfaceMesh* Apply(FESurfaceMesh* pm, FEGroup* pg);

	void assignToPartition(int n);

private:
	int	m_partition;
};
