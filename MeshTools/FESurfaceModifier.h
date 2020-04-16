#pragma once
#include <FSCore/FSObject.h>

class FESurfaceMesh;
class FEGroup;

//-------------------------------------------------------------------
// Class for modifying surface meshes
class FESurfaceModifier : public FSObject
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

protected:
	void PartitionSelectedFaces(FESurfaceMesh* mesh);

private:
	int	m_partition;
};

//-----------------------------------------------------------------------------
class FESurfaceAutoPartition : public FESurfaceModifier
{
public:
	FESurfaceAutoPartition();
	FESurfaceMesh* Apply(FESurfaceMesh* pm);
};
