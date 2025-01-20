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
#include <GLLib/GLScene.h>
#include <PostGL/GLModel.h>
#include <MeshLib/FSNodeFaceList.h>
#include <PostGL/PostObject.h>
#include "../FEBioStudio/GLPostScene.h"
#include <QMutex>

class FEBioMonitorDoc;

namespace Post {
	class FEMeshData;
}

// from FEBio
class FEModel;
class FEPlotData;

class CGLMonitorScene : public CGLPostScene
{
public:
	CGLMonitorScene(FEBioMonitorDoc* doc);
	~CGLMonitorScene();

	void InitScene(FEModel* fem);
	void UpdateMeshState(FEModel* fem);
	void AddState();
	void UpdateStateData();
	void UpdateScene();

	void Render(GLRenderEngine& engine, GLContext& rc) override;

	void RenderTags(GLContext& rc);

	// get the bounding box of the entire scene
	BOX GetBoundingBox() override;

	// get the bounding box of the current selection
	BOX GetSelectionBox() override;

	// get the post model
	Post::CGLModel* GetGLModel() { return m_glm; }

	Post::FEPostModel* GetFSModel() { return m_postModel; }

	bool AddDataField(const std::string& fieldName);

	CPostObject* GetPostObject() { return m_glm->GetPostObject(); }

private:
	void Clear();

	void BuildMesh();
	void BuildGLModel();
	void UpdateModelData();
	void UpdateDataField(FEPlotData* dataField, Post::FEMeshData& meshData);
	void UpdateNodalData(FEPlotData* dataField, Post::FEMeshData& meshData);
	void UpdateDomainData(FEPlotData* dataField, Post::FEMeshData& meshData);
	void UpdateSurfaceData(FEPlotData* dataField, Post::FEMeshData& meshData);

private:
	FEBioMonitorDoc* m_fmdoc;
	Post::FEPostModel* m_postModel;
	Post::CGLModel* m_glm;
	FSNodeFaceList	m_NFT;
	FEModel* m_fem;
	QMutex	m_mutex;
	std::vector<FEPlotData*>	m_dataFields;
};
