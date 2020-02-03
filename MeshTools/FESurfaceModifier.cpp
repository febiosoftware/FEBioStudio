#include "stdafx.h"
#include "FESurfaceModifier.h"
#include <MeshLib/FESurfaceMesh.h>
#include <MeshTools/FEGroup.h>
#include <stdarg.h>

std::string	FESurfaceModifier::m_error;

FESurfaceModifier::FESurfaceModifier(const std::string& name)
{ 
	SetName(name);
}

FESurfaceModifier::~FESurfaceModifier() {}

bool FESurfaceModifier::SetError(const char* szerr, ...)
{
	// get a pointer to the argument list
	va_list	args;

	// copy to string
	char sz[256] = { 0 };
	va_start(args, szerr);
	vsprintf(sz, szerr, args);
	va_end(args);

	m_error = std::string(sz);
	return false;
}

std::string FESurfaceModifier::GetErrorString()
{
	return m_error;
}

//=============================================================================

FESurfacePartitionSelection::FESurfacePartitionSelection()
{
	m_partition = -1;
}

void FESurfacePartitionSelection::assignToPartition(int n)
{
	m_partition = n;
}


FESurfaceMesh* FESurfacePartitionSelection::Apply(FESurfaceMesh* pm, FEGroup* pg)
{
	if (dynamic_cast<FEEdgeSet*>(pg))
	{
		FESurfaceMesh* newMesh = new FESurfaceMesh(*pm);
		newMesh->PartitionEdgeSelection(m_partition);
		return newMesh;
	}
	else if (dynamic_cast<FENodeSet*>(pg))
	{
		FESurfaceMesh* newMesh = new FESurfaceMesh(*pm);
		newMesh->PartitionNodeSelection();
		return newMesh;
	}

	return 0;
}

//=============================================================================
FESurfaceAutoPartition::FESurfaceAutoPartition() : FESurfaceModifier("Auto Partition")
{
	AddDoubleParam(30.0, "Crease angle:", "Crease angle (degrees):");
}

//-----------------------------------------------------------------------------
FESurfaceMesh* FESurfaceAutoPartition::Apply(FESurfaceMesh* pm)
{
	double w = GetFloatValue(0);
	FESurfaceMesh* newMesh = new FESurfaceMesh(*pm);
	newMesh->AutoPartition(w);
	return newMesh;
}
