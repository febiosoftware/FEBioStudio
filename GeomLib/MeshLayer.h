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

#pragma once
#include <string>
#include <vector>
#include <FSCore/Serializable.h>

//-----------------------------------------------------------------------------
class GModel;
class GObject;
class FEMesher;
class FSMesh;

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
	const FSMesh* GetFEMesh(int layer, int obj);
	int FEMeshes(int layer);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	int m_activeLayer;	// the active mesh layer
	std::vector<MeshLayer*>	m_layerList;
	GModel*		m_gm;
};
