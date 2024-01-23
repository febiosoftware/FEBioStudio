#pragma once
#include "FEModelComponent.h"
#include <MeshLib/IHasItemList.h>

class FEItemListBuilder;

enum MeshDataGeneratorType
{
	FE_FEBIO_NODEDATA_GENERATOR = 1,
	FE_FEBIO_EDGEDATA_GENERATOR = 2,
	FE_FEBIO_FACEDATA_GENERATOR = 3,
	FE_FEBIO_ELEMDATA_GENERATOR = 4,
};

// TODO: serialization!!
class FSMeshDataGenerator : public FSModelComponent, public FSHasOneItemList
{
public:
	FSMeshDataGenerator(FSModel* fem, int ntype);

	int Type() const;

private:
	int	m_ntype;
};

//===========================================================================
class FSNodeDataGenerator : public FSMeshDataGenerator
{
public:
	FSNodeDataGenerator(FSModel* fem, int ntype);

public:
	int GetID() const;
	void SetID(int nid);

	static void ResetCounter();
	static void SetCounter(int n);
	static int GetCounter();

private:
	int			m_nUID;		// unique ID
	static	int	m_nref;
};

class FEBioNodeDataGenerator : public FSNodeDataGenerator
{
public:
	FEBioNodeDataGenerator(FSModel* fem = nullptr);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;
};

//===========================================================================
class FSEdgeDataGenerator : public FSMeshDataGenerator
{
public:
	FSEdgeDataGenerator(FSModel* fem, int ntype);

public:
	int GetID() const;
	void SetID(int nid);

	static void ResetCounter();
	static void SetCounter(int n);
	static int GetCounter();

private:
	int			m_nUID;		// unique ID
	static	int	m_nref;
};

class FEBioEdgeDataGenerator : public FSEdgeDataGenerator
{
public:
	FEBioEdgeDataGenerator(FSModel* fem = nullptr);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;
};

//===========================================================================
class FSFaceDataGenerator : public FSMeshDataGenerator
{
public:
	FSFaceDataGenerator(FSModel* fem, int ntype);

public:
	int GetID() const;
	void SetID(int nid);

	static void ResetCounter();
	static void SetCounter(int n);
	static int GetCounter();

private:
	int			m_nUID;		// unique ID
	static	int	m_nref;
};

class FEBioFaceDataGenerator : public FSFaceDataGenerator
{
public:
	FEBioFaceDataGenerator(FSModel* fem = nullptr);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;
};

//===========================================================================
class FSElemDataGenerator : public FSMeshDataGenerator
{
public:
	FSElemDataGenerator(FSModel* fem, int ntype);

public:
	int GetID() const;
	void SetID(int nid);

	static void ResetCounter();
	static void SetCounter(int n);
	static int GetCounter();

private:
	int			m_nUID;		// unique ID
	static	int	m_nref;
};

class FEBioElemDataGenerator : public FSElemDataGenerator
{
public:
	FEBioElemDataGenerator(FSModel* fem = nullptr);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;
};
