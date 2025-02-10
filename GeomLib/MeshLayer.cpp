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

#include "MeshLayer.h"
#include <assert.h>
#include "GObject.h"
#include <MeshLib/FSMesh.h>
#include "GModel.h"
#include <MeshTools/FEMesher.h>
#include <MeshTools/FETetGenMesher.h>
//using namespace std;

//-----------------------------------------------------------------------------
class MeshLayer : CSerializable
{
	struct Mesh
	{
		GObject*	po;			// the object
		FEMesher*	mesher;		// the class the generates the mesh
		FSMesh*		mesh;		// the mesh for this object

		Mesh()
		{
			mesher = nullptr;
			mesh = nullptr;
		}
	};

public:
	MeshLayer(MeshLayerManager* mgr);
	~MeshLayer();
	MeshLayer(const std::string& name, MeshLayerManager* mgr);

	void setName(const std::string& name);

	const std::string& name() const;

	void AddObject(GObject* po);

	void InsertObject(int npos, GObject* po);

	int Meshes() const;

	void SetMeshData(int n, FEMesher* mesher, FSMesh* mesh);

	void InsertMeshData(int n, GObject* po, FEMesher* mesher, FSMesh* mesh);

	GObject* GetObject(int n);

	FEMesher* GetFEMesher(int n);

	FSMesh* GetFEMesh(int n);

	int FindObject(GObject* po);

	void RemoveObject(int n, bool deleteMeshData);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

protected:
	void DeleteMeshList();

private:
	MeshLayerManager*	m_mgr;
	std::string			m_name;	// layer name
	std::vector<Mesh>	m_meshList;
};

MeshLayer::MeshLayer(MeshLayerManager* mgr) : m_mgr(mgr) 
{
	GModel* gm = m_mgr->GetGModel();
	int objCount = gm->Objects();
	m_meshList.resize(objCount);
	for (int i = 0; i < objCount; ++i)
	{
		m_meshList[i].po = gm->Object(i);
	}
}

MeshLayer::~MeshLayer()
{
	DeleteMeshList();
}

void MeshLayer::DeleteMeshList()
{
	for (int i = 0; i < (int)m_meshList.size(); ++i)
	{
		Mesh& m = m_meshList[i];
		if (m.mesh) delete m.mesh;
		if (m.mesher) delete m.mesher;
	}
	m_meshList.clear();
}

MeshLayer::MeshLayer(const std::string& name, MeshLayerManager* mgr) : m_name(name), m_mgr(mgr)
{
	GModel* gm = m_mgr->GetGModel();
	int objCount = gm->Objects();
	m_meshList.resize(objCount);
	for (int i = 0; i < objCount; ++i)
	{
		m_meshList[i].po = gm->Object(i);
	}
}

void MeshLayer::setName(const std::string& name)
{
	m_name = name;
}

const std::string& MeshLayer::name() const
{
	return m_name;
}

void MeshLayer::AddObject(GObject* po)
{
	Mesh m;
	m.po = po;
	m.mesh = nullptr;
	m.mesher = nullptr;
	m_meshList.push_back(m);
}

void MeshLayer::InsertObject(int npos, GObject* po)
{
	Mesh m;
	m.po = po;
	m.mesh = nullptr;
	m.mesher = nullptr;
	m_meshList.insert(m_meshList.begin() + npos, m);
}

GObject* MeshLayer::GetObject(int n)
{
	return m_meshList[n].po;
}

FEMesher* MeshLayer::GetFEMesher(int n)
{
	return m_meshList[n].mesher;
}

FSMesh* MeshLayer::GetFEMesh(int n)
{
	return m_meshList[n].mesh;
}

int MeshLayer::Meshes() const
{
	return (int)m_meshList.size();
}

void MeshLayer::SetMeshData(int n, FEMesher* mesher, FSMesh* mesh)
{
	Mesh& m = m_meshList[n];
	m.mesher = mesher;
	m.mesh = mesh;
}

void MeshLayer::InsertMeshData(int n, GObject* po, FEMesher* mesher, FSMesh* mesh)
{
	Mesh m;
	m.po = po;
	m.mesh = mesh;
	m.mesher = mesher;
	m_meshList.insert(m_meshList.begin() + n, m);
}

int MeshLayer::FindObject(GObject* po)
{
	for (int i = 0; i < m_meshList.size(); ++i)
	{
		Mesh& m = m_meshList[i];
		if (m.po == po) return i;
	}
	assert(false);
	return -1;
}

void MeshLayer::RemoveObject(int n, bool deleteMeshData)
{
	m_meshList.erase(m_meshList.begin() + n);
}

void MeshLayer::Save(OArchive& ar)
{
	ar.WriteChunk(CID_MESH_LAYER_NAME, m_name);
	for (int i = 0; i < m_meshList.size(); ++i)
	{
		ar.WriteChunk(CID_MESH_LAYER_MESH_INDEX, i);
		Mesh& m = m_meshList[i];
		if (m.mesher)
		{
			ar.BeginChunk(CID_OBJ_FEMESHER);
			{
				int ntype = 0;
				if (dynamic_cast<FETetGenMesher*>(m.mesher)) ntype = 1;
				ar.BeginChunk(ntype);
				{
					m.mesher->Save(ar);
				}
				ar.EndChunk();
			}
			ar.EndChunk();
		}

		if (m.mesh)
		{
			ar.BeginChunk(CID_MESH);
			{
				m.mesh->Save(ar);
			}
			ar.EndChunk();
		}		
	}
}

void MeshLayer::Load(IArchive& ar)
{
	int index = -1;
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == CID_MESH_LAYER_NAME) ar.read(m_name);
		else if (nid == CID_MESH_LAYER_MESH_INDEX)
		{
			ar.read(index);
			assert((index >= 0) && (index < m_meshList.size()));
		}
		else if (nid == CID_OBJ_FEMESHER)
		{
			if (ar.OpenChunk() != IArchive::IO_OK) throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
			else
			{
				Mesh& m = m_meshList[index];
				int ntype = ar.GetChunkID();
				assert(m.mesher == nullptr);
				m.mesher = FEMesher::Create(m.po, ntype);
				if (m.mesher == nullptr)
				{
					throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
				}
				m.mesher->Load(ar);
			}
			ar.CloseChunk();
			if (ar.OpenChunk() != IArchive::IO_END) throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
		}
		else if (nid == CID_MESH)
		{
			Mesh& m = m_meshList[index];
			assert(m.mesh == nullptr);
			m.mesh = new FSMesh();
			m.mesh->SetGObject(m.po);
			m.mesh->Load(ar);
		}
		ar.CloseChunk();
	}
}

//=============================================================================
class ObjectMeshList
{
public:
	int					m_index;	// index into mesh list array
	GObject*			m_po;		// the object whose list this is
	std::vector<FEMesher*>	m_mesher;	// list of FE meshers
	std::vector<FSMesh*  >	m_mesh;		// list of FE meshes

public:
	ObjectMeshList() { m_po = nullptr; m_index = -1; }

	~ObjectMeshList()
	{
		for (int i = 0; i < m_mesher.size(); ++i) delete m_mesher[i];
		for (int i = 0; i < m_mesh.size(); ++i) delete m_mesh[i];
	}
};

//=============================================================================
MeshLayerManager::MeshLayerManager(GModel* m) : m_gm(m)
{
	m_activeLayer = -1;
}

MeshLayerManager::~MeshLayerManager()
{
	DeleteAllLayers();
}

GModel* MeshLayerManager::GetGModel()
{
	return m_gm;
}

void MeshLayerManager::DeleteAllLayers()
{
	for (int i = 0; i < Layers(); ++i) delete m_layerList[i];
	m_layerList.clear();
	m_activeLayer = -1;
}

int MeshLayerManager::GetActiveLayer() { return m_activeLayer; }

int MeshLayerManager::Layers() { return (int)m_layerList.size(); }

bool MeshLayerManager::AddLayer(const std::string& layerName)
{
	// make sure the name does not exist yet.
	for (int i = 0; i < m_layerList.size(); ++i)
	{
		const std::string& si = m_layerList[i]->name();
		if (si.compare(layerName) == 0) return false;
	}

	// create a new mesh layer
	MeshLayer* meshLayer = new MeshLayer(layerName, this);

	// if this is not the first layer, we allocate default meshers for all objects
	if (m_layerList.empty() == false)
	{
		int nobjs = m_gm->Objects();
		for (int i = 0; i < nobjs; ++i)
		{
			GObject* po = m_gm->Object(i);
			assert(po == meshLayer->GetObject(i));
			assert(meshLayer->GetFEMesher(i) == nullptr);
			assert(meshLayer->GetFEMesh(i) == nullptr);
			meshLayer->SetMeshData(i, po->CreateDefaultMesher(), nullptr);
		}
	}

	// add it to the list
	m_layerList.push_back(meshLayer);

	// if this is the first layer, make it the active layer
	if (m_layerList.size() == 1) m_activeLayer = 0;

	return true;
}

void MeshLayerManager::DeleteLayer(int n)
{
	assert((n >= 0) && (n < Layers()));
	MeshLayer* meshLayer = m_layerList[n];
	m_layerList.erase(m_layerList.begin() + n);
	delete meshLayer;
}

const std::string& MeshLayerManager::GetLayerName(int i)
{
	return m_layerList[i]->name();
}

void MeshLayerManager::SetActiveLayer(int n)
{
	assert((n >= 0) && (n < Layers()));
	if (n == m_activeLayer) return;

	// grab the currently active layer
	MeshLayer* currLayer = m_layerList[m_activeLayer];

	// copy all the meshers and meshes to this layer
	GModel* gm = m_gm;
	int objs = gm->Objects();
	assert(objs == currLayer->Meshes());
	for (int i = 0; i < objs; ++i)
	{
		GObject* po = gm->Object(i);
		assert(po == currLayer->GetObject(i));
		assert(currLayer->GetFEMesher(i) == nullptr);
		assert(currLayer->GetFEMesh(i) == nullptr);

		currLayer->SetMeshData(i, po->GetFEMesher(), po->GetFEMesh());
	}

	// copy all meshes and meshers from the new active layer
	m_activeLayer = n;
	currLayer = m_layerList[n];
	assert(objs == currLayer->Meshes());
	for (int i = 0; i < objs; ++i)
	{
		GObject* po = gm->Object(i);
		assert(po == currLayer->GetObject(i));

		po->SetFEMesher(currLayer->GetFEMesher(i));
		po->SetFEMesh(currLayer->GetFEMesh(i));

		// clear the mesh data in this layer
		currLayer->SetMeshData(i, nullptr, nullptr);
	}
}

int MeshLayerManager::FindMeshLayer(const std::string& s)
{
	for (int i = 0; i < Layers(); ++i)
	{
		const std::string& si = m_layerList[i]->name();
		if (si.compare(s) == 0)
		{
			return i;
		}
	}
	return -1;
}

void MeshLayerManager::Save(OArchive& ar)
{
	int activeLayer = GetActiveLayer();
	ar.WriteChunk(CID_MESH_LAYER_ACTIVE, activeLayer);

	int layers = Layers();
	for (int i = 0; i < layers; ++i)
	{
		ar.BeginChunk(CID_MESH_LAYER);
		{
			MeshLayer* layer_i = m_layerList[i];
			layer_i->Save(ar);
		}
		ar.EndChunk();
	}
}

void MeshLayerManager::Load(IArchive& ar)
{
	DeleteAllLayers();
	int objects = m_gm->Objects();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == CID_MESH_LAYER_ACTIVE) ar.read(m_activeLayer);
		else if (nid == CID_MESH_LAYER)
		{
			MeshLayer* meshLayer = new MeshLayer(this);
			meshLayer->Load(ar);
			m_layerList.push_back(meshLayer);
		}
		ar.CloseChunk();
	}
}

void MeshLayerManager::AddObject(GObject* po)
{
	// add the object to all the layers
	for (int i = 0; i < Layers(); ++i)
	{
		m_layerList[i]->AddObject(po);
	}

	// it is assumed that the object was added to the end of the object list
	int n = m_gm->Objects() - 1;

	// initialize meshers for this object on all layers (except the active one)
	for (int i = 0; i < Layers(); ++i)
	{
		MeshLayer* layer = m_layerList[i];
		assert(layer->GetObject(n) == po);
		assert(layer->GetFEMesher(n) == nullptr);
		assert(layer->GetFEMesh(n) == nullptr);

		if (i != m_activeLayer)
		{
			FEMesher* mesher = po->CreateDefaultMesher();
			FSMesh* mesh = nullptr;

			// if the object has no mesher, we are going to copy the current mesh
			if (mesher == nullptr)
			{
				FSMesh* oldMesh = po->GetFEMesh();
				if (oldMesh) mesh = new FSMesh(*oldMesh);
			}

			// add the object (and mesh data) to the layer
			layer->SetMeshData(n, mesher, mesh);
		}
	}
}

void MeshLayerManager::InsertObject(int npos, GObject* po)
{
	// add the object to all the layers
	for (int i = 0; i < Layers(); ++i)
	{
		m_layerList[i]->InsertObject(npos, po);
	}

	// initialize meshers for this object on all layers (except the active one)
	for (int i = 0; i < Layers(); ++i)
	{
		MeshLayer* layer = m_layerList[i];
		assert(layer->GetObject(npos) == po);
		assert(layer->GetFEMesher(npos) == nullptr);
		assert(layer->GetFEMesh(npos) == nullptr);

		if (i != m_activeLayer)
		{
			FEMesher* mesher = po->CreateDefaultMesher();
			FSMesh* mesh = nullptr;

			// if the object has no mesher, we are going to copy the current mesh
			if (mesher == nullptr)
			{
				FSMesh* oldMesh = po->GetFEMesh();
				if (oldMesh) mesh = new FSMesh(*oldMesh);
			}

			// add the object (and mesh data) to the layer
			layer->SetMeshData(npos, mesher, mesh);
		}
	}
}

void MeshLayerManager::RemoveObject(GObject* po, bool deleteMeshList)
{
	if (m_layerList.empty()) return;

	MeshLayer* meshLayer = m_layerList[0];
	int n = meshLayer->FindObject(po); assert(n >= 0);

	// remove the object from all the layers
	for (int i = 0; i < Layers(); ++i)
	{
		meshLayer = m_layerList[i];
		assert(meshLayer->GetObject(n) == po);
		meshLayer->RemoveObject(n, deleteMeshList);
	}
}

ObjectMeshList* MeshLayerManager::GetObjectMeshList(GObject* po)
{
	if (m_layerList.empty()) return nullptr;

	MeshLayer* meshLayer = m_layerList[0];
	int n = meshLayer->FindObject(po); assert(n >= 0);
	assert(m_gm->Object(n) == po);

	ObjectMeshList* oml = new ObjectMeshList;
	oml->m_index = n;
	oml->m_po = po;

	for (int i = 0; i < Layers(); ++i)
	{
		meshLayer = m_layerList[i];
		assert(meshLayer->GetObject(n) == po);
		oml->m_mesher.push_back(meshLayer->GetFEMesher(n));
		oml->m_mesh.push_back(meshLayer->GetFEMesh(n));
	}

	return oml;
}

void MeshLayerManager::InsertObjectMeshList(ObjectMeshList* oml)
{
	assert(oml->m_mesh.size() == Layers());
	assert(oml->m_mesher.size() == Layers());

	m_gm->InsertObject(oml->m_po, oml->m_index, false);

	for (int i = 0; i < Layers(); ++i)
	{
		MeshLayer* meshLayer = m_layerList[i];
		meshLayer->InsertMeshData(oml->m_index, oml->m_po, oml->m_mesher[i], oml->m_mesh[i]);
	}

	// clear the mesh and mesher lists, so they won't get deleted when oml is deleted
	oml->m_mesh.clear();
	oml->m_mesher.clear();
	delete oml;
}

MeshLayer* MeshLayerManager::RemoveMeshLayer(int index)
{
	assert(index > 0);				// make sure we are not deleting the first layer
	assert(index != m_activeLayer);	// make sure this layer is not the active one
	MeshLayer* layer = m_layerList[index];
	m_layerList.erase(m_layerList.begin() + index);
	return layer;
}

void MeshLayerManager::InsertMeshLayer(int index, MeshLayer* layer)
{
	m_layerList.insert(m_layerList.begin() + index, layer);
}

void MeshLayerManager::Destroy(MeshLayer* layer)
{
	delete layer;
}

void MeshLayerManager::Destroy(ObjectMeshList* oml)
{
	delete oml;
}

const FEMesher* MeshLayerManager::GetFEMesher(int layer, int obj)
{
	return m_layerList[layer]->GetFEMesher(obj);
}

const FSMesh* MeshLayerManager::GetFEMesh(int layer, int obj)
{
	return m_layerList[layer]->GetFEMesh(obj);
}

int MeshLayerManager::FEMeshes(int layer)
{
	return m_layerList[layer]->Meshes();
}
