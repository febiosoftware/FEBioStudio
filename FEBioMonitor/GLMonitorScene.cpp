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
#include "GLMonitorScene.h"
#include "FEBioMonitorDoc.h"
#include <FECore/FEModel.h>
#include <FECore/FEMesh.h>
#include <FECore/FEDomain.h>
#include <FECore/FESurface.h>
#include <FECore/FEMaterial.h>
#include <FECore/FEPlotDataStore.h>
#include <FECore/FEPlotData.h>
#include <PostLib/Palette.h>
#include <PostLib/FEMeshData_T.h>
#include <PostLib/FEDataField.h>
#include <GLLib/GLContext.h>
#include <PostLib/FEState.h>
#include <QtCore/QFileInfo>
#include <sstream>

CGLMonitorScene::CGLMonitorScene(FEBioMonitorDoc* doc) : CGLPostScene(doc), m_fmdoc(doc)
{
	m_fem = nullptr;
	m_postModel = new Post::FEPostModel;
	m_glm = new Post::CGLModel(m_postModel);
	m_glm->SetSubDivisions(1);
}

CGLMonitorScene::~CGLMonitorScene()
{
	Clear();
	delete m_glm;
}

void CGLMonitorScene::Clear()
{
	m_glm->SetFEModel(nullptr);
	delete m_postModel;
	m_postModel = nullptr;
	for (FEPlotData* p : m_dataFields) delete p;
	m_dataFields.clear();
}

void CGLMonitorScene::Render(GLRenderEngine& engine, CGLContext& rc)
{
	QMutexLocker lock(&m_mutex);

/*
	// TODO: This was moved to CGLView and will be handled differently 
	int nfield = m_glm->GetColorMap()->GetEvalField();
	std::string dataFieldName = m_postModel->GetDataManager()->getDataString(nfield, Post::Data_Tensor_Type::TENSOR_SCALAR);


	// Update GLWidget string table for post rendering
	QString febFile = QString::fromStdString(m_doc->GetFEBioInputFile());
	QFileInfo fi(febFile);
	QString filename = fi.fileName();
	GLWidget::addToStringTable("$(filename)", filename.toStdString());
	GLWidget::addToStringTable("$(datafield)", dataFieldName);
	//	GLWidget::addToStringTable("$(units)", m_doc->GetFieldUnits());
	GLWidget::addToStringTable("$(time)", m_postModel->CurrentTime());
*/
	CGLPostScene::Render(engine, rc);
}

void CGLMonitorScene::RenderTags(CGLContext& rc)
{
	ClearTags();

	GLViewSettings& view = rc.m_settings;

	GObject* po = m_fmdoc->GetActiveObject();
	if (po == nullptr) return;

	FSMesh* pm = po->GetFEMesh();

	// create the tag array.
	// We add a tag for each selected item
	GLTAG tag;

	// clear the node tags
	pm->TagAllNodes(0);

	int mode = m_fmdoc->GetItemMode();

	GLColor extcol(255, 255, 0);
	GLColor intcol(255, 0, 0);

	// process elements
	if (view.m_ntagInfo > TagInfoOption::NO_TAG_INFO)
	{
		if ((mode == ITEM_ELEM) && pm)
		{
			ForAllSelectedElements(*pm, [&](FEElement_& el) {
				GLTAG tag;
				tag.r = pm->LocalToGlobal(pm->ElementCenter(el));
				tag.c = extcol;
				int nid = el.GetID();
				snprintf(tag.sztag, sizeof tag.sztag, "E%d", nid);
				AddTag(tag);

				int ne = el.Nodes();
				for (int j = 0; j < ne; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
				});
		}

		// process faces
		if (mode == ITEM_FACE)
		{
			int NF = pm->Faces();
			for (int i = 0; i < NF; ++i)
			{
				FSFace& f = pm->Face(i);
				if (f.IsSelected())
				{
					tag.r = pm->LocalToGlobal(pm->FaceCenter(f));
					tag.c = (f.IsExternal() ? extcol : intcol);
					int nid = f.GetID();
					if (nid < 0) nid = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "F%d", nid);
					AddTag(tag);

					int nf = f.Nodes();
					for (int j = 0; j < nf; ++j) pm->Node(f.n[j]).m_ntag = 1;
				}
			}
		}

		// process edges
		if (mode == ITEM_EDGE)
		{
			int NC = pm->Edges();
			for (int i = 0; i < NC; i++)
			{
				FSEdge& edge = pm->Edge(i);
				if (edge.IsSelected())
				{
					tag.r = pm->LocalToGlobal(pm->EdgeCenter(edge));
					tag.c = extcol;
					int nid = edge.GetID();
					if (nid < 0) nid = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "L%d", nid);
					AddTag(tag);

					int ne = edge.Nodes();
					for (int j = 0; j < ne; ++j) pm->Node(edge.n[j]).m_ntag = 1;
				}
			}
		}

		// process nodes
		if (mode == ITEM_NODE)
		{
			ForAllSelectedNodes(*pm, [&](FSNode& node) {
				GLTAG tag;
				tag.r = pm->LocalToGlobal(node.r);
				tag.c = (node.IsExterior() ? extcol : intcol);
				int nid = node.GetID();
				snprintf(tag.sztag, sizeof tag.sztag, "N%d", nid);
				AddTag(tag);
				});
		}

		// add additional nodes
		if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES)
		{
			ForAllTaggedNodes(*pm, 1, [&](FSNode& node) {
				GLTAG tag;
				tag.r = pm->LocalToGlobal(node.r);
				tag.c = (node.IsExterior() ? extcol : intcol);
				int n = node.GetID();
				snprintf(tag.sztag, sizeof tag.sztag, "N%d", n);
				AddTag(tag);
				});
		}
	}
}

void CGLMonitorScene::InitScene(FEModel* fem)
{
	m_fem = fem;

	Clear();
	m_glm->SetFEModel(m_postModel = new Post::FEPostModel);

	BuildMesh();
	BuildGLModel();
	UpdateStateData();
	BOX box = GetBoundingBox();
	if (box.IsValid())
	{
		double f = box.GetMaxExtent();
		if (f == 0) f = 1;
		CGLCamera& cam = GetCamera();
		cam.SetTarget(box.Center());
		cam.SetTargetDistance(2.0 * f);
		cam.Update(true);
	}
}

void CGLMonitorScene::BuildMesh()
{
	QMutexLocker lock(&m_mutex);
	FEModel* fem = m_fem;
	if (fem == nullptr) return;

	FEMesh& febioMesh = fem->GetMesh();

	// count all nodes
	int NN = febioMesh.Nodes();

	// count all elements
	int ND = febioMesh.Domains();
	int NE = 0;
	for (int i = 0; i < ND; ++i) NE += febioMesh.Domain(i).Elements();

	// find the element type
	int ntype = febioMesh.Domain(0).ElementRef(0).Shape();
	bool blinear = true;	// all linear elements flag
	for (int i = 0; i < ND; ++i)
	{
		int domType = febioMesh.Domain(i).ElementRef(0).Shape();
		if (domType != ntype) ntype = -1;
		if ((domType != ET_TRUSS2) &&
			(domType != ET_TRI3) &&
			(domType != ET_QUAD4) &&
			(domType != ET_TET4) &&
			(domType != ET_PENTA6) &&
			(domType != ET_HEX8) &&
			(domType != ET_PYRA5)) blinear = false;
	}

	Post::FEPostMesh* pmesh = new Post::FEPostMesh;
	pmesh->Create(NN, NE);

	// read the element connectivity
	int ne = 0;
	for (int i = 0; i < ND; i++)
	{
		FEDomain& D = febioMesh.Domain(i);
		for (int j = 0; j < D.Elements(); ++j)
		{
			FEElement& E = D.ElementRef(j);
			FSElement& el = pmesh->Element(ne++);
			el.m_MatID = E.GetMatID();
			el.m_gid = i;
			el.SetID(E.GetID());

			FEElementType etype;
			switch (E.Shape())
			{
			case ET_HEX8: etype = FE_HEX8; break;
			case ET_PENTA6: etype = FE_PENTA6; break;
			case ET_PENTA15: etype = FE_PENTA15; break;
			case ET_TET4: etype = FE_TET4; break;
			case ET_TET5: etype = FE_TET5; break;
			case ET_QUAD4: etype = FE_QUAD4; break;
			case ET_TRI3: etype = FE_TRI3; break;
			case ET_TRUSS2: etype = FE_BEAM2; break;
			case ET_HEX20: etype = FE_HEX20; break;
			case ET_HEX27: etype = FE_HEX27; break;
			case ET_TET10: etype = FE_TET10; break;
			case ET_TET15: etype = FE_TET15; break;
			case ET_TET20: etype = FE_TET20; break;
			case ET_TRI6: etype = FE_TRI6; break;
			case ET_QUAD8: etype = FE_QUAD8; break;
			case ET_QUAD9: etype = FE_QUAD9; break;
			case ET_PYRA5: etype = FE_PYRA5; break;
			case ET_PYRA13: etype = FE_PYRA13; break;
			case ET_LINE2: etype = FE_BEAM2; break;
			case ET_LINE3: etype = FE_BEAM3; break;
			case ET_DISCRETE: etype = FE_BEAM2; break;
			default:
				assert(false);
			}
			el.SetType(etype);
			int ne = el.Nodes();
			for (int k = 0; k < ne; ++k) el.m_node[k] = E.m_node[k];
		}
	}

	// initialize material properties
	const Post::CPalette& pal = Post::CPaletteManager::CurrentPalette();
	int NCOL = pal.Colors();
	m_postModel->ClearMaterials();
	int nmat = fem->Materials();
	for (int i = 0; i < nmat; i++)
	{
		FEMaterial* pmat = fem->GetMaterial(i);

		GLColor c = pal.Color(i % NCOL);

		Post::Material m;
		m.diffuse = c;
		m.ambient = c;
		m.specular = GLColor(128, 128, 128);
		m.emission = GLColor(0, 0, 0);
		m.shininess = 0.5f;
		m.transparency = 1.f;

		string matName = pmat->GetName();
		m.SetName(matName.c_str());
		m_postModel->AddMaterial(m);
	}

	NN = febioMesh.Nodes();
	for (int i = 0; i < NN; i++)
	{
		FSNode& n = pmesh->Node(i);
		FENode& N = febioMesh.Node(i);
		n.m_nid = N.GetID();
		n.r = N.m_r0;
	}

	for (int i = 0; i < NN; ++i) pmesh->Node(i).Disable();
	for (int i = 0; i < NE; ++i)
	{
		FEElement_& el = pmesh->ElementRef(i);
		if (el.IsEnabled())
		{
			int n = el.Nodes();
			for (int j = 0; j < n; ++j) pmesh->Node(el.m_node[j]).Enable();
		}
	}

	// Update the mesh
	// This will also build the faces
	pmesh->BuildMesh();
	m_glm->GetPostObject()->SetFEMesh(pmesh);
	m_glm->GetPostObject()->Update(true);

	// store the current mesh
	m_postModel->AddMesh(pmesh);

	m_postModel->UpdateBoundingBox();
	m_NFT.Build(pmesh);
}

Post::ModelDataField* BuildModelDataField(FEPlotData* ps, Post::FEPostModel* fem)
{
	Post::ModelDataField* pdf = nullptr;
	Var_Type dataType = ps->DataType();
	Region_Type regionType = ps->RegionType();
	Storage_Fmt storageFmt = ps->StorageFormat();

	if      (regionType == FE_REGION_NODE)
	{
		switch (dataType)
		{
		case PLT_FLOAT  : pdf = new Post::FEDataField_T<Post::FENodeData<float  > >(fem, Post::EXPORT_DATA); break;
		case PLT_VEC3F  : pdf = new Post::FEDataField_T<Post::FENodeData<vec3f  > >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FS : pdf = new Post::FEDataField_T<Post::FENodeData<mat3fs > >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FD : pdf = new Post::FEDataField_T<Post::FENodeData<mat3fd > >(fem, Post::EXPORT_DATA); break;
		case PLT_TENS4FS: pdf = new Post::FEDataField_T<Post::FENodeData<tens4fs> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3F  : pdf = new Post::FEDataField_T<Post::FENodeData<mat3f  > >(fem, Post::EXPORT_DATA); break;
		case PLT_ARRAY:
		{
			Post::FEArrayDataField* data = new Post::FEArrayDataField(fem, NODE_DATA, DATA_ITEM, Post::EXPORT_DATA);
			data->SetArraySize(ps->GetArraysize());
			data->SetArrayNames(ps->GetArrayNames());
			pdf = data;
		}
		break;
		default:
			assert(false);
			break;
		}
	}
	else if ((regionType == FE_REGION_DOMAIN) && (storageFmt == FMT_ITEM))
	{
		switch (dataType)
		{
		case PLT_FLOAT  : pdf = new Post::FEDataField_T<Post::FEElementData<float  , DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_VEC3F  : pdf = new Post::FEDataField_T<Post::FEElementData<vec3f  , DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FS : pdf = new Post::FEDataField_T<Post::FEElementData<mat3fs , DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FD : pdf = new Post::FEDataField_T<Post::FEElementData<mat3fd , DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_TENS4FS: pdf = new Post::FEDataField_T<Post::FEElementData<tens4fs, DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3F  : pdf = new Post::FEDataField_T<Post::FEElementData<mat3f  , DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		default:
			assert(false);
			break;
		}
	}
	else if ((regionType == FE_REGION_DOMAIN) && (storageFmt == FMT_MULT))
	{
		switch (dataType)
		{
		case PLT_FLOAT  : pdf = new Post::FEDataField_T<Post::FEElementData<float  , DATA_MULT> >(fem, Post::EXPORT_DATA); break;
		case PLT_VEC3F  : pdf = new Post::FEDataField_T<Post::FEElementData<vec3f  , DATA_MULT> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FS : pdf = new Post::FEDataField_T<Post::FEElementData<mat3fs , DATA_MULT> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FD : pdf = new Post::FEDataField_T<Post::FEElementData<mat3fd , DATA_MULT> >(fem, Post::EXPORT_DATA); break;
		case PLT_TENS4FS: pdf = new Post::FEDataField_T<Post::FEElementData<tens4fs, DATA_MULT> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3F  : pdf = new Post::FEDataField_T<Post::FEElementData<mat3f  , DATA_MULT> >(fem, Post::EXPORT_DATA); break;
		default:
			assert(false);
			break;
		}
	}
	else if ((regionType == FE_REGION_SURFACE) && (storageFmt == FMT_ITEM))
	{
		switch (dataType)
		{
		case PLT_FLOAT  : pdf = new Post::FEDataField_T<Post::FEFaceData<float  , DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_VEC3F  : pdf = new Post::FEDataField_T<Post::FEFaceData<vec3f  , DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FS : pdf = new Post::FEDataField_T<Post::FEFaceData<mat3fs , DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FD : pdf = new Post::FEDataField_T<Post::FEFaceData<mat3fd , DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_TENS4FS: pdf = new Post::FEDataField_T<Post::FEFaceData<tens4fs, DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3F  : pdf = new Post::FEDataField_T<Post::FEFaceData<mat3f  , DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		default:
			assert(false);
			break;
		}
	}

	return pdf;
}

bool CGLMonitorScene::AddDataField(const std::string& fieldName)
{
	// try to allocate the FEBio plot field
	FECoreKernel& febio = FECoreKernel::GetInstance();

	FEPlotFieldDescriptor PD(fieldName);
	if (!PD.isValid()) return false;

	FEPlotData* ps = fecore_new<FEPlotData>(PD.fieldName.c_str(), m_fem);
	if (ps == nullptr) return false;

	if (PD.HasFilter())
	{
		if (PD.IsNumberFilter()) ps->SetFilter(PD.numFilter);
		else if (PD.IsStringFilter()) ps->SetFilter(PD.strFilter.c_str());
	}

	Post::ModelDataField* pdf = BuildModelDataField(ps, m_postModel);
	if (pdf)
	{
		pdf->SetName(PD.alias);
		m_postModel->AddDataField(pdf);
		m_dataFields.push_back(ps);
		if (m_postModel->GetStates())
		{
			Post::FEState* state = m_postModel->GetState(0);
			int n = state->m_Data.size();
			Post::FEMeshData& meshData = state->m_Data[n - 1];
			UpdateDataField(ps, meshData);
		}
	}
	else
	{
		delete ps;
	}
	return (pdf != nullptr);
}

void CGLMonitorScene::BuildGLModel()
{
	QMutexLocker lock(&m_mutex);
	Post::FEPostModel& fem = *m_postModel;
	Post::FEDataManager* DM = m_postModel->GetDataManager();

	// add the primary variables
	std::set<std::string> varNames;
	DOFS& dofs = m_fem->GetDOFS();
	for (int i = 0; i < dofs.Variables(); ++i)
	{
		string varName = dofs.GetVariableName(i);
		stringstream ss;
		ss << "field[\'" + varName << "\']=" << varName;
		varNames.insert(ss.str());
	}

	// add all plot variables
	FEPlotDataStore& dataStore = m_fem->GetPlotDataStore();
	for (int i = 0; i < dataStore.PlotVariables(); ++i)
	{
		FEPlotVariable& var = dataStore.GetPlotVariable(i);
		std::string name = var.Name();
		varNames.insert(name);
	}

	// add all mesh data
	FEMesh& mesh = m_fem->GetMesh();
	for (int i = 0; i < mesh.DataMaps(); ++i)
	{
		string mapName = mesh.GetDataMap(i)->GetName();
		stringstream ss;
		ss << "mesh_data[\'" + mapName << "\']=" << mapName;
		varNames.insert(ss.str());
	}

	// add all the plot variables
	for (auto name : varNames) AddDataField(name);

	m_postModel->AddState(new Post::FEState(0.f, m_postModel, m_postModel->GetFEMesh(0)));
	m_glm->UpdateEdge();
	m_glm->Update(true);
}

void CGLMonitorScene::UpdateMeshState(FEModel* fem)
{
	QMutexLocker lock(&m_mutex);
	FEMesh& mesh = fem->GetMesh();

	Post::FEState* state = m_postModel->CurrentState(); assert(state);
	if (state == nullptr) return;

	Post::FEPostMesh& postMesh = *m_postModel->GetFEMesh(0);
	assert(mesh.Nodes() == postMesh.Nodes());
	assert(mesh.Elements() == postMesh.Elements());

	int ne = 0;
	int ND = mesh.Domains();
	for (int i = 0; i < ND; i++)
	{
		FEDomain& D = mesh.Domain(i);
		for (int j = 0; j < D.Elements(); ++j, ne++)
		{
			FEElement& el = D.ElementRef(j);
			if (!el.isActive())
			{
				unsigned int mask = ~(Post::StatusFlags::VISIBLE);
				state->m_ELEM[ne].m_state &= mask;
			}
		}
	}
	m_glm->Update(true);
}

void CGLMonitorScene::AddState()
{
	m_postModel->AddState(new Post::FEState(m_fmdoc->GetTimeValue(), m_postModel, m_postModel->GetFEMesh(0)));
	m_postModel->SetCurrentTimeIndex(m_postModel->GetStates() - 1);
}

void CGLMonitorScene::UpdateStateData()
{
	QMutexLocker lock(&m_mutex);

	if (m_fem == nullptr) return;

	Post::FEState* ps = m_postModel->CurrentState();
	ps->m_time = m_fmdoc->GetTimeValue();
	m_postModel->SetCurrentTimeIndex(ps->m_id);

	FEMesh& febioMesh = m_fem->GetMesh();
	for (int i = 0; i < febioMesh.Nodes(); ++i)
	{
		FENode& feNode = febioMesh.Node(i);
		ps->m_NODE[i].m_rt = to_vec3f(feNode.m_rt);
	}

	UpdateModelData();

	UpdateScene();
}

void CGLMonitorScene::UpdateScene()
{
	Post::FEState* ps = m_postModel->CurrentState();
	FSMesh* pm = ps->GetFEMesh();
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		ps->GetFEMesh()->Node(i).r = to_vec3d(ps->m_NODE[i].m_rt);
	}
	m_postModel->UpdateMeshState(m_postModel->CurrentTimeIndex());
	ps->GetFEMesh()->UpdateNormals();

	m_postModel->UpdateBoundingBox();
	m_glm->Update(true);
}

void CGLMonitorScene::UpdateDataField(FEPlotData* dataField, Post::FEMeshData& meshData)
{
	switch (dataField->RegionType())
	{
	case Region_Type::FE_REGION_NODE   : UpdateNodalData(dataField, meshData); break;
	case Region_Type::FE_REGION_DOMAIN : UpdateDomainData(dataField, meshData); break;
	case Region_Type::FE_REGION_SURFACE: UpdateSurfaceData(dataField, meshData); break;
	default:
		assert(false);
	}
}

void CGLMonitorScene::UpdateModelData()
{
	Post::FEState* ps = m_postModel->CurrentState();
	for (int n=0; n<m_dataFields.size(); ++n)
	{
		FEPlotData* pd = m_dataFields[n];
		Post::FEMeshData& meshData = ps->m_Data[n];
		UpdateDataField(pd, meshData);
	}
}

void CGLMonitorScene::UpdateNodalData(FEPlotData* pd, Post::FEMeshData& meshData)
{
	Region_Type rgn = pd->RegionType();
	Var_Type dataType = pd->DataType();
	Storage_Fmt storageFmt = pd->StorageFormat();
	int ndata = pd->VarSize(pd->DataType());
	int N = m_fem->GetMesh().Nodes();
	FEDataStream a; a.reserve(ndata * N);
	if (pd->Save(m_fem->GetMesh(), a))
	{
		// pad mismatches
		assert(a.size() == N * ndata);
		if (a.size() != N * ndata) a.resize(N * ndata, 0.f);

		if      (dataType == Var_Type::PLT_FLOAT)
		{
			Post::FENodeData<float>& d = dynamic_cast<Post::FENodeData<float>&>(meshData);
			for (int i = 0; i < N; ++i) d[i] = a[i];
		}
		else if (dataType == Var_Type::PLT_VEC3F)
		{
			Post::FENodeData<vec3f>& d = dynamic_cast<Post::FENodeData<vec3f>&>(meshData);
			for (int i = 0; i < N; ++i) d[i] = a.get<vec3f>(i);
		}
		else if (dataType == Var_Type::PLT_MAT3FS)
		{
			Post::FENodeData<mat3fs>& d = dynamic_cast<Post::FENodeData<mat3fs>&>(meshData);
			for (int i = 0; i < N; ++i) d[i] = a.get<mat3fs>(i);
		}
		else if (dataType == Var_Type::PLT_ARRAY)
		{
			Post::FENodeArrayData& d = dynamic_cast<Post::FENodeArrayData&>(meshData);
			d.setData(a.data());
		}
		else assert(false);
	}
}

void CGLMonitorScene::UpdateDomainData(FEPlotData* pd, Post::FEMeshData& meshData)
{
	Var_Type dataType = pd->DataType();

	FEMesh& m = m_fem->GetMesh();
	int ND = m.Domains();
	vector<int> item = pd->GetItemList();
	if (item.empty())
	{
		for (int i = 0; i < ND; ++i) item.push_back(i);
	}

	// get the domain name (if any)
	string domName;
	const char* szdom = pd->GetDomainName();
	if (szdom) domName = szdom;

	// loop over all domains in the item list
	int elementCounter = 0;
	for (int i = 0; i < ND; ++i)
	{
		// get the domain
		FEDomain& D = m.Domain(item[i]);
		int NE = D.Elements();

		if (domName.empty() || (D.GetName() == domName))
		{
			// calculate the size of the data vector
			int nsize = pd->VarSize(pd->DataType());
			switch (pd->StorageFormat())
			{
			case FMT_NODE: nsize *= D.Nodes(); break;
			case FMT_ITEM: nsize *= D.Elements(); break;
			case FMT_MULT:
			{
				// since all elements have the same type within a domain
				// we just grab the number of nodes of the first element 
				// to figure out how much storage we need
				FEElement& e = D.ElementRef(0);
				int n = e.Nodes();
				nsize *= n * D.Elements();
			}
			break;
			case FMT_REGION:
				// one value for this domain so nsize remains unchanged
				break;
			default:
				assert(false);
			}
			assert(nsize > 0);

			// fill data vector and save
			FEDataStream a;
			a.reserve(nsize);
			if (pd->Save(D, a))
			{
				assert(a.size() == nsize);
				if (pd->StorageFormat() == Storage_Fmt::FMT_ITEM)
				{
					if (dataType == Var_Type::PLT_FLOAT)
					{
						Post::FEElementData<float, DATA_ITEM>& d = dynamic_cast<Post::FEElementData<float, DATA_ITEM>&>(meshData);
						for (int i = 0; i < NE; ++i, ++elementCounter)
						{
							if (d.active(elementCounter))
							{
								d.set(elementCounter, a[i]);
							}
							else
							{
								d.add(elementCounter, a[i]);
							}
						}
					}
					else if (dataType == Var_Type::PLT_VEC3F)
					{
						Post::FEElementData<vec3f, DATA_ITEM>& d = dynamic_cast<Post::FEElementData<vec3f, DATA_ITEM>&>(meshData);
						for (int i = 0; i < NE; ++i, ++elementCounter)
						{
							vec3f v = a.get<vec3f>(i);
							if (d.active(elementCounter))
							{
								d.set(elementCounter, v);
							}
							else
							{
								d.add(elementCounter, v);
							}
						}
					}
					else if (dataType == Var_Type::PLT_MAT3FS)
					{
						Post::FEElementData<mat3fs, DATA_ITEM>& d = dynamic_cast<Post::FEElementData<mat3fs, DATA_ITEM>&>(meshData);
						for (int i = 0; i < NE; ++i, ++elementCounter)
						{
							mat3fs m = a.get<mat3fs>(i);
							if (d.active(elementCounter))
							{
								d.set(elementCounter, m);
							}
							else
							{
								d.add(elementCounter, m);
							}
						}
					}
					else assert(false);
				}
				else if (pd->StorageFormat() == Storage_Fmt::FMT_MULT)
				{
					FEElement& e = D.ElementRef(0);
					int ne = e.Nodes();

					if (dataType == Var_Type::PLT_FLOAT)
					{
						Post::FEElementData<float, DATA_MULT>& d = dynamic_cast<Post::FEElementData<float, DATA_MULT>&>(meshData);
						for (int i = 0; i < NE; ++i, ++elementCounter)
						{
							if (d.active(elementCounter))
							{
								d.set(elementCounter, &a[i*ne]);
							}
							else
							{
								d.add(elementCounter, ne, &a[i*ne]);
							}
						}
					}
				}
				else assert(false);
			}
		}
		else elementCounter += NE;
	}
}

template <class T>
void mapSurfaceData_ITEM(Post::FEMeshData& meshData, FESurface& surf, FSNodeFaceList& NFT, FEDataStream& a)
{
	Post::FEFaceData<T, DATA_ITEM>& d = dynamic_cast<Post::FEFaceData<T, DATA_ITEM>&>(meshData);
	int NF = surf.Elements();
	for (int i = 0; i < NF; ++i)
	{
		FESurfaceElement& el = surf.Element(i);
		int m = NFT.FindFace(el.m_node[0], &el.m_node[0], el.Nodes());
		if (m >= 0) d.set(m, a.get<T>(i));
	}
}

void CGLMonitorScene::UpdateSurfaceData(FEPlotData* pd, Post::FEMeshData& meshData)
{
	Var_Type dataType = pd->DataType();
	Storage_Fmt storageFmt = pd->StorageFormat();

	// get the domain name (if any)
	string domName;
	const char* szdom = pd->GetDomainName();
	if (szdom) domName = szdom;

	// loop over all surfaces
	FEMesh& m = m_fem->GetMesh();
	for (int i = 0; i <m.Surfaces(); ++i)
	{
		FESurface& surf = m.Surface(i);
		int NF = surf.Elements();
		int maxNodes = FSFace::MAX_NODES;
		if (domName.empty() || (domName == surf.GetName()))
		{
			// Determine data size.
			// Note that for the FMT_MULT case we are 
			// assuming 9 data entries per facet
			// regardless of the nr of nodes a facet really has
			// this is because for surfaces, all elements are not
			// necessarily of the same type
			// TODO: Fix the assumption of the FMT_MULT
			int datasize = pd->VarSize(pd->DataType());
			int nsize = datasize;
			switch (pd->StorageFormat())
			{
			case FMT_NODE: nsize *= surf.Nodes(); break;
			case FMT_ITEM: nsize *= surf.Elements(); break;
			case FMT_MULT: nsize *= maxNodes * surf.Elements(); break;
			case FMT_REGION:
				// one value per surface so nsize remains unchanged
				break;
			default:
				assert(false);
			}

			// save data
			FEDataStream a; a.reserve(nsize);
			if (pd->Save(surf, a))
			{
				// in FEBio 3.0, the data streams are assumed to have no padding, but for now we still need to pad 
				// the data stream before we write it to the file
				if (a.size() != nsize)
				{
					// this is only needed for FMT_MULT storage
					assert(pd->StorageFormat() == FMT_MULT);

					// add padding
					const int M = maxNodes;
					int m = 0;
					FEDataStream b; b.assign(nsize, 0.f);
					for (int n = 0; n < surf.Elements(); ++n)
					{
						FESurfaceElement& el = surf.Element(n);
						int ne = el.Nodes();
						for (int j = 0; j < ne; ++j)
						{
							for (int k = 0; k < datasize; ++k) b[n * M * datasize + j * datasize + k] = a[m++];
						}
					}
					a = b;
				}

				if (storageFmt == Storage_Fmt::FMT_ITEM)
				{
					switch (dataType)
					{
					case Var_Type::PLT_FLOAT : mapSurfaceData_ITEM<float> (meshData, surf, m_NFT, a); break;
					case Var_Type::PLT_VEC3F : mapSurfaceData_ITEM<vec3f> (meshData, surf, m_NFT, a); break;
					case Var_Type::PLT_MAT3FS: mapSurfaceData_ITEM<mat3fs>(meshData, surf, m_NFT, a); break;
					case Var_Type::PLT_MAT3F : mapSurfaceData_ITEM<mat3f> (meshData, surf, m_NFT, a); break;
					default:
						assert(false);
					}

				}
				else assert(false);
			}
		}
	}
}

BOX CGLMonitorScene::GetBoundingBox()
{
	return m_postModel->GetBoundingBox();
}


BOX CGLMonitorScene::GetSelectionBox()
{
	Post::CGLModel* mdl = GetGLModel();
	if (mdl == nullptr) return BOX(-1, -1, -1, 1, 1, 1);

	FESelection* sel = m_fmdoc->GetCurrentSelection();
	if (sel) return sel->GetBoundingBox();

	return BOX();
}
