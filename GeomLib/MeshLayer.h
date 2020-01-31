#pragma once
#include <string>
#include <vector>
#include <FSCore/Serializable.h>

//-----------------------------------------------------------------------------
class GModel;
class GObject;
class FEMesher;
class FEMesh;

//-----------------------------------------------------------------------------
class MeshLayer;
class ObjectMeshList;

//-----------------------------------------------------------------------------
// Class for managing layers of a GModel
class MeshLayerManager : CSerializable
{
public:
	MeshLayerManager(GModel* m);
	~MeshLayerManager();
	void DeleteAllLayers();
	int GetActiveLayer();
	int Layers();
	bool AddLayer(const std::string& layerName);
	void DeleteLayer(int n);
	const std::string& GetLayerName(int i);
	void SetActiveLayer(int n);
	int FindMeshLayer(const std::string& s);

	GModel* GetGModel();

	void AddObject(GObject* po);

	void RemoveObject(GObject* po, bool deleteMeshList);

	void InsertObject(int pos, GObject* po);

	ObjectMeshList* GetObjectMeshList(GObject* po);
	void InsertObjectMeshList(ObjectMeshList* oml);

	MeshLayer* RemoveMeshLayer(int index);
	void InsertMeshLayer(int index, MeshLayer* layer);

	static void Destroy(MeshLayer* layer);
	static void Destroy(ObjectMeshList* oml);

public:
	const FEMesher* GetFEMesher(int layer, int obj);
	const FEMesh* GetFEMesh(int layer, int obj);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	int m_activeLayer;	// the active mesh layer
	std::vector<MeshLayer*>	m_layerList;
	GModel*		m_gm;
};
