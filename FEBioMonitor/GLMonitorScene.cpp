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
#include "../FEBioStudio/GLView.h"
#include <FECore/FEModel.h>
#include <FECore/FEMesh.h>
#include <FECore/FEDomain.h>
#include <FECore/FEMaterial.h>
#include <FECore/FEPlotDataStore.h>
#include <FECore/FEPlotData.h>
#include <PostLib/Palette.h>
#include <PostLib/FEMeshData_T.h>
#include <PostLib/FEDataField.h>
#include <GLLib/GLContext.h>
#include <PostGL/GLPlaneCutPlot.h>
#include <QtCore/QFileInfo>
#ifdef __APPLE__
#include <OpenGL/GLU.h>
#else
#include <GL/glu.h>
#endif

class CGLMonitorScene::DataField
{
public:
	DataField(Post::ModelDataField* dataField, FEPlotData* plotData) : m_dataField(dataField), m_plotData(plotData){}
	~DataField() { delete m_plotData; }

public:
	Post::ModelDataField* m_dataField = nullptr;
	FEPlotData* m_plotData = nullptr;
};

CGLMonitorScene::CGLMonitorScene(FEBioMonitorDoc* doc) : m_doc(doc)
{
	m_fem = nullptr;
	m_postModel = new Post::FEPostModel;
	m_glm = new Post::CGLModel(m_postModel);
}

CGLMonitorScene::~CGLMonitorScene()
{
	delete m_glm;
	delete m_postModel;
	for (DataField* p : m_dataFields) delete p;
}

void CGLMonitorScene::Render(CGLContext& rc)
{
	QMutexLocker lock(&m_mutex);

	CGLView* glview = (CGLView*)rc.m_view; assert(glview);
	if (glview == nullptr) return;

	int nfield = m_glm->GetColorMap()->GetEvalField();
	std::string dataFieldName = m_postModel->GetDataManager()->getDataString(nfield, Post::DATA_SCALAR);


	// Update GLWidget string table for post rendering
	QString febFile = m_doc->GetFEBioInputFile();
	QFileInfo fi(febFile);
	QString filename = fi.fileName();
	GLWidget::addToStringTable("$(filename)", filename.toStdString());
	GLWidget::addToStringTable("$(datafield)", dataFieldName);
	//	GLWidget::addToStringTable("$(units)", m_doc->GetFieldUnits());
	GLWidget::addToStringTable("$(time)", m_doc->GetTimeValue());

	// We need this for rendering post docs
	glEnable(GL_COLOR_MATERIAL);

	Post::CGLModel* glm = m_glm;

	CGLCamera& cam = *rc.m_cam;

	GLViewSettings& vs = glview->GetViewSettings();

	glm->m_nrender = vs.m_nrender + 1;
	glm->m_bnorm = vs.m_bnorm;
	glm->m_scaleNormals = vs.m_scaleNormals;
	glm->m_doZSorting = vs.m_bzsorting;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glview->PositionCamera();

	glDisable(GL_CULL_FACE);

	// match the selection mode
	int selectionMode = Post::SELECT_ELEMS;
	switch (m_doc->GetItemMode())
	{
	case ITEM_MESH:
	case ITEM_ELEM: selectionMode = Post::SELECT_ELEMS; break;
	case ITEM_FACE: selectionMode = Post::SELECT_FACES; break;
	case ITEM_EDGE: selectionMode = Post::SELECT_EDGES; break;
	case ITEM_NODE: selectionMode = Post::SELECT_NODES; break;
	}
	glm->SetSelectionMode(selectionMode);


	if (vs.m_bShadows)
	{
		BOX box = m_postModel->GetBoundingBox();

		float a = vs.m_shadow_intensity;
		GLfloat shadow[] = { a, a, a, 1 };
		GLfloat zero[] = { 0, 0, 0, 1 };
		GLfloat ones[] = { 1,1,1,1 };
		GLfloat lp[4] = { 0 };

		glEnable(GL_STENCIL_TEST);

		float inf = box.Radius() * 100.f;

		vec3d lpv = to_vec3d(glview->GetLightPosition());

		quatd q = cam.GetOrientation();
		q.Inverse().RotateVector(lpv);

		lp[0] = lpv.x;
		lp[1] = lpv.y;
		lp[2] = lpv.z;

		// set coloring for shadows
		glLightfv(GL_LIGHT0, GL_DIFFUSE, shadow);
		glLightfv(GL_LIGHT0, GL_SPECULAR, zero);

		glStencilFunc(GL_ALWAYS, 0x00, 0xff);
		glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

		// render the scene
		glm->Render(rc);

		// Create mask in stencil buffer
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);

		Post::FEPostModel* fem = glm->GetFSModel();
		glm->RenderShadows(fem, lpv, inf);

		glCullFace(GL_BACK);
		glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);

		glm->RenderShadows(fem, lpv, inf);

		// Render the scene in light
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);

		GLfloat d = vs.m_diffuse;
		GLfloat dv[4] = { d, d, d, 1.f };

		glLightfv(GL_LIGHT0, GL_DIFFUSE, dv);
		glLightfv(GL_LIGHT0, GL_SPECULAR, ones);

		glStencilFunc(GL_EQUAL, 0, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		glDisable(GL_CULL_FACE);

		glClear(GL_DEPTH_BUFFER_BIT);
	}

	glm->Render(rc);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// render the tracking
	if (glview->TrackModeActive()) glview->RenderTrack();

	// render the tags
	GLViewSettings& view = glview->GetViewSettings();
	if (view.m_bTags) glview->RenderTags();

	Post::CGLPlaneCutPlot::DisableClipPlanes();

	// render the image data
	glview->RenderImageData();

	// render the decorations
	glview->RenderDecorations();
}

void CGLMonitorScene::InitScene(FEModel* fem)
{
	m_fem = fem;

	BuildMesh();
	BuildGLModel();
	UpdateScene();
	BOX box = GetBoundingBox();
	if (box.IsValid())
	{
		double f = box.GetMaxExtent();
		if (f == 0) f = 1;
		CGLCamera& cam = GetCamera();
		cam.SetTarget(box.Center());
		cam.SetTargetDistance(2.0 * f);
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
	// store the current mesh
	m_postModel->AddMesh(pmesh);

	m_postModel->UpdateBoundingBox();
}

Post::ModelDataField* BuildModelDataField(FEPlotData* ps, Post::FEPostModel* fem)
{
	Post::ModelDataField* pdf = nullptr;
	Var_Type dataType = ps->DataType();
	Region_Type regionType = ps->RegionType();
	Storage_Fmt storageFmt = ps->StorageFormat();

	if (regionType == FE_REGION_NODE)
	{
		switch (dataType)
		{
		case PLT_FLOAT  : pdf = new Post::FEDataField_T<Post::FENodeData<float  > >(fem, Post::EXPORT_DATA); break;
		case PLT_VEC3F  : pdf = new Post::FEDataField_T<Post::FENodeData<vec3f  > >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FS : pdf = new Post::FEDataField_T<Post::FENodeData<mat3fs > >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FD : pdf = new Post::FEDataField_T<Post::FENodeData<mat3fd > >(fem, Post::EXPORT_DATA); break;
		case PLT_TENS4FS: pdf = new Post::FEDataField_T<Post::FENodeData<tens4fs> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3F  : pdf = new Post::FEDataField_T<Post::FENodeData<mat3f  > >(fem, Post::EXPORT_DATA); break;
		default:
			assert(false);
			break;
		}
	}
	else if ((regionType == FE_REGION_DOMAIN) && (storageFmt == FMT_ITEM))
	{
		switch (dataType)
		{
		case PLT_FLOAT  : pdf = new Post::FEDataField_T<Post::FEElementData<float  ,Post::DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_VEC3F  : pdf = new Post::FEDataField_T<Post::FEElementData<vec3f  ,Post::DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FS : pdf = new Post::FEDataField_T<Post::FEElementData<mat3fs ,Post::DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3FD : pdf = new Post::FEDataField_T<Post::FEElementData<mat3fd ,Post::DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_TENS4FS: pdf = new Post::FEDataField_T<Post::FEElementData<tens4fs,Post::DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		case PLT_MAT3F  : pdf = new Post::FEDataField_T<Post::FEElementData<mat3f  ,Post::DATA_ITEM> >(fem, Post::EXPORT_DATA); break;
		default:
			assert(false);
			break;
		}
	}

	return pdf;
}

void CGLMonitorScene::BuildGLModel()
{
	QMutexLocker lock(&m_mutex);
	Post::FEPostModel& fem = *m_postModel;
	Post::FEDataManager* DM = m_postModel->GetDataManager();
	FEPlotDataStore& dataStore = m_fem->GetPlotDataStore();
	for (int i = 0; i < dataStore.PlotVariables(); ++i)
	{
		FEPlotVariable& var = dataStore.GetPlotVariable(i);
		std::string name = var.Name();

		// try to allocate the FEBio plot field
		FECoreKernel& febio = FECoreKernel::GetInstance();
		FEPlotData* ps = fecore_new<FEPlotData>(name.c_str(), m_fem);
		if (ps)
		{
			Post::ModelDataField* pdf = BuildModelDataField(ps, &fem);
			if (pdf)
			{
				DM->AddDataField(pdf, name);
				m_dataFields.push_back(new DataField(pdf, ps));
			}
			else delete ps;
		}
	}

	m_postModel->AddState(new Post::FEState(0.f, m_postModel, m_postModel->GetFEMesh(0)));
	m_glm->Update(true);
}

void CGLMonitorScene::UpdateScene()
{
	QMutexLocker lock(&m_mutex);

	if (m_fem == nullptr) return;

	Post::FEState* ps = m_postModel->GetState(0);
	FEMesh& febioMesh = m_fem->GetMesh();
	for (int i = 0; i < febioMesh.Nodes(); ++i)
	{
		FENode& feNode = febioMesh.Node(i);
		ps->m_NODE[i].m_rt = to_vec3f(feNode.m_rt);
		ps->GetFEMesh()->Node(i).r = feNode.m_rt; // TODO: How can I use the states?
	}
	m_postModel->UpdateMeshState(0);
	ps->GetFEMesh()->UpdateNormals();

	m_postModel->UpdateBoundingBox();

	UpdateModelData();
}

void CGLMonitorScene::UpdateModelData()
{
	Post::FEState* ps = m_postModel->GetState(0);
	for (int n=0; n<m_dataFields.size(); ++n)
	{
		DataField* dataField = m_dataFields[n];
		FEPlotData* pd = dataField->m_plotData;
		Region_Type rgn = pd->RegionType();
		Var_Type dataType = pd->DataType();
		Storage_Fmt storageFmt = pd->StorageFormat();

		int ndata = pd->VarSize(pd->DataType());

		if (rgn == Region_Type::FE_REGION_NODE)
		{
			int N = m_fem->GetMesh().Nodes();
			FEDataStream a; a.reserve(ndata * N);
			if (pd->Save(m_fem->GetMesh(), a))
			{
				// pad mismatches
				assert(a.size() == N * ndata);
				if (a.size() != N * ndata) a.resize(N * ndata, 0.f);

				std::vector<float>& data = a.data();

				if      (dataType == Var_Type::PLT_FLOAT)
				{
					Post::FENodeData<float>& d = dynamic_cast<Post::FENodeData<float>&>(ps->m_Data[n]);
					for (int i = 0; i < N; ++i) d[i] = data[i];
				}
				else if (dataType == Var_Type::PLT_VEC3F)
				{
					Post::FENodeData<vec3f>& d = dynamic_cast<Post::FENodeData<vec3f>&>(ps->m_Data[0]);
					for (int i = 0; i < N; ++i)
					{
						float* p = &data[3 * i];
						d[i] = vec3f(p[0], p[1], p[2]);
					}
				}
				else if (dataType == Var_Type::PLT_MAT3FS)
				{
					Post::FENodeData<mat3fs>& d = dynamic_cast<Post::FENodeData<mat3fs>&>(ps->m_Data[0]);
					for (int i = 0; i < N; ++i)
					{
						float* p = &data[6 * i];
						d[i] = mat3fs(p[0], p[1], p[2], p[3], p[4], p[5]);
					}
				}
				else assert(false);
			}
		}
		else if ((rgn == Region_Type::FE_REGION_DOMAIN) && (storageFmt == Storage_Fmt::FMT_ITEM))
		{
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
						if (dataType == Var_Type::PLT_FLOAT)
						{
							Post::FEElementData<float, Post::DATA_ITEM>& d = dynamic_cast<Post::FEElementData<float, Post::DATA_ITEM>&>(ps->m_Data[n]);
							std::vector<float>& data = a.data();
							for (int i = 0; i < NE; ++i, ++elementCounter)
							{
								if (d.active(elementCounter))
								{
									d.set(elementCounter, data[i]);
								}
								else
								{
									d.add(elementCounter, data[i]);
								}
							}
						}
						else if (dataType == Var_Type::PLT_VEC3F)
						{
							Post::FEElementData<vec3f, Post::DATA_ITEM>& d = dynamic_cast<Post::FEElementData<vec3f, Post::DATA_ITEM>&>(ps->m_Data[n]);
							std::vector<float>& data = a.data();
							for (int i = 0; i < NE; ++i, ++elementCounter)
							{
								float* p = &data[3 * i];
								vec3f v(p[0], p[1], p[2]);
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
							Post::FEElementData<mat3fs, Post::DATA_ITEM>& d = dynamic_cast<Post::FEElementData<mat3fs, Post::DATA_ITEM>&>(ps->m_Data[n]);
							std::vector<float>& data = a.data();
							for (int i = 0; i < NE; ++i, ++elementCounter)
							{
								float* p = &data[6 * i];
								mat3fs m(p[0], p[1], p[2], p[3], p[4], p[5]);
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
				}
				else elementCounter += NE;
			}
		}
	}
	m_glm->Update(true);
}

BOX CGLMonitorScene::GetBoundingBox()
{
	return m_postModel->GetBoundingBox();
}

// get the bounding box of the current selection
BOX CGLMonitorScene::GetSelectionBox()
{
	return GetBoundingBox();
}
