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
#include "GLDocument.h"
#include <QtCore/QDir>
#include <GLLib/GLScene.h>
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GCurveMeshObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GMultiPatch.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GGroup.h>
#include <PostGL/GLPlot.h>
#include <PostGL/GLDisplacementMap.h>
#include <PostGL/GLColorMap.h>
#include <PostGL/GLModel.h>
#include <PostLib/GLImageRenderer.h>
#include <ImageLib/ImageModel.h>
#include <ImageLib/ImageSource.h>
#include <ImageLib/ImageFilter.h>
#include <ImageLib/ImageAnalysis.h>
#include <GeomLib/GModel.h>
#include <MeshLib/FSNodeData.h>
#include <MeshLib/FSSurfaceData.h>
#include <MeshLib/FSElementData.h>
#include <FSCore/FileReader.h>
#include <FSCore/FileWriter.h>
#include <FSCore/ColorMapManager.h>
#include "FEBioJob.h"
#include "units.h"
#include "MainWindow.h"
#include "Commands.h"
#include <sstream>

CGLDocument::CGLDocument(CMainWindow* wnd) : CUndoDocument(wnd)
{
	m_fileWriter = nullptr;
	m_fileReader = nullptr;

	// Set default modes
	m_vs.nselect = SELECT_OBJECT;
	m_vs.ntrans = TRANSFORM_NONE;
	m_vs.nitem = ITEM_MESH;
	m_vs.nstyle = REGION_SELECT_BOX;

	m_uiMode = MODEL_VIEW;

	// set default unit system (0 == no unit system)
	m_units = 0;

	m_psel = nullptr;

	m_scene = nullptr;

	m_showTitle = false;
	m_showSubtitle = false;
	m_showLegend = false;
	m_dataRange[0] = m_dataRange[1] = 0;
	m_colorGradient = ColorMapManager::JET;
	m_legendDivisions = 10;
	m_legendSmooth = true;
}

CGLDocument::~CGLDocument()
{
	// remove autosave
	QFile autoSave(m_autoSaveFilePath.c_str());
	if (autoSave.exists()) autoSave.remove();
}

void CGLDocument::SetUnitSystem(int unitSystem)
{
	m_units = unitSystem;
}

int CGLDocument::GetUnitSystem() const
{
	return m_units;
}

FESelection* CGLDocument::GetCurrentSelection() { return m_psel; }

BOX CGLDocument::GetSelectionBox()
{
	if (!IsValid()) return BOX(-1, -1, -1, 1, 1, 1);

	BOX box;
	FESelection* currentSelection = GetCurrentSelection();
	if (currentSelection && currentSelection->Size())
	{
		box = currentSelection->GetBoundingBox();
	}

	if ((box.Width() < 1e-5) || (box.Height() < 1e-4) || (box.Depth() < 1e-4))
	{
		float R = box.Radius();
		box.InflateTo(R, R, R);
	}

	return box;
}

void CGLDocument::SetCurrentSelection(FESelection* psel)
{
	if (m_psel) delete m_psel;
	m_psel = psel;
}

void CGLDocument::UpdateSelection()
{

}

void CGLDocument::SetViewState(VIEW_STATE vs)
{
	m_vs = vs;
	UpdateSelection();
	//	if (m_wnd) m_wnd->UpdateUI();
}

void CGLDocument::SetTransformMode(TransformMode mode)
{
	m_vs.ntrans = (int)mode;
	UpdateSelection();
}

GObject* CGLDocument::GetActiveObject()
{
	return nullptr;
}

CGView* CGLDocument::GetView()
{
	return &m_scene->GetView();
}

GLScene* CGLDocument::GetScene()
{
	return m_scene;
}

void CGLDocument::Update()
{
	UpdateSelection();
	if (m_scene) m_scene->Update();
}

std::string CGLDocument::GetTypeString(FSObject* po)
{
	if (po == 0) return "(null)";

	if      (dynamic_cast<GPrimitive*>(po)) return "Primitive";
	else if (dynamic_cast<GMultiBox*>(po)) return "Multi-block";
	else if (dynamic_cast<GMultiPatch*>(po)) return "Multi-patch";
	else if (dynamic_cast<GMeshObject*>(po)) return "Editable mesh";
	else if (dynamic_cast<GCurveMeshObject*>(po)) return "Editable curve";
	else if (dynamic_cast<GSurfaceMeshObject*>(po)) return "Editable surface";
	else if (dynamic_cast<GMaterial*>(po))
	{
		GMaterial* gmat = dynamic_cast<GMaterial*>(po);
		FSMaterial* mat = gmat->GetMaterialProperties();
		if (mat)
		{
			std::stringstream ss;
			const char* sztype = mat->GetTypeString();
			if (sztype == 0) sztype = "";
			ss << "Material" << " [" << sztype << "]";
			return ss.str();
		}
	}
	else if (dynamic_cast<FSStepComponent*>(po))
	{
		FSStepComponent* pc = dynamic_cast<FSStepComponent*>(po);
		return pc->GetTypeString();
	}
	else if (dynamic_cast<FSStep*>(po))
	{
		FSStep* step = dynamic_cast<FSStep*>(po);
		std::stringstream ss;
		ss << "Step";
		const char* sztype = step->GetTypeString();
		if (sztype) ss << " [" << sztype << "]";
		return ss.str();
	}
	else if (dynamic_cast<GDiscreteSpringSet*>(po)) return "Discrete element set";
	else if (dynamic_cast<GDiscreteElement*>(po)) return "discrete element";
	else if (dynamic_cast<FSElemSet*>(po)) return "element selection";
	else if (dynamic_cast<FSSurface*>(po)) return "face selection";
	else if (dynamic_cast<FSEdgeSet*>(po)) return "edge selection";
	else if (dynamic_cast<FSNodeSet*>(po)) return "node selection";
	else if (dynamic_cast<GPart*>(po)) return "Part";
	else if (dynamic_cast<GFace*>(po)) return "Surface";
	else if (dynamic_cast<GEdge*>(po)) return "Curve";
	else if (dynamic_cast<GNode*>(po)) return "Node";
	else if (dynamic_cast<GGroup*>(po)) return "Named selection";
	else if (dynamic_cast<FSGroup*>(po)) return "Named selection";
	else if (dynamic_cast<GObject*>(po)) return "Object";
	else if (dynamic_cast<CFEBioJob*>(po)) return "Job";
	else if (dynamic_cast<CImageModel*>(po)) return "3D Image volume";
	else if (dynamic_cast<CImageAnalysis*>(po)) return "Image Analysis";
	else if (dynamic_cast<Post::CGLPlot*>(po)) return "Plot";
	else if (dynamic_cast<Post::CGLDisplacementMap*>(po)) return "Displacement map";
	else if (dynamic_cast<Post::CGLColorMap*>(po)) return "Color map";
	else if (dynamic_cast<Post::CGLModel*>(po)) return "post model";
	else if (dynamic_cast<GModel*>(po)) return "model";
	else if (dynamic_cast<Post::CGLImageRenderer*>(po)) return "volume image renderer";
	else if (dynamic_cast<CImageSource*>(po)) return "3D Image source";
	else if (dynamic_cast<CImageFilter*>(po)) return "Image filter";
	else if (dynamic_cast<FSMaterial*>(po))
	{
		FSMaterial* mat = dynamic_cast<FSMaterial*>(po);
		if (mat)
		{
			std::stringstream ss;
			const char* sztype = mat->GetTypeString();
			if (sztype == 0) sztype = "";
			ss << "Material" << " [" << sztype << "]";
			return ss.str();
		}
	}
	else if (dynamic_cast<FSLoadController*>(po))
	{
		FSLoadController* plc = dynamic_cast<FSLoadController*>(po);
		std::stringstream ss;
		const char* sztype = plc->GetTypeString();
		if (sztype == 0) sztype = "";
		ss << "Load controller" << " [" << sztype << "]";
		return ss.str();
	}
	else if (dynamic_cast<FSNodeDataGenerator*>(po))
	{
		FSNodeDataGenerator* plc = dynamic_cast<FSNodeDataGenerator*>(po);
		std::stringstream ss;
		const char* sztype = plc->GetTypeString();
		if (sztype == 0) sztype = "";
		ss << "Node data generator" << " [" << sztype << "]";
		return ss.str();
	}
	else if (dynamic_cast<FSEdgeDataGenerator*>(po))
	{
		FSEdgeDataGenerator* plc = dynamic_cast<FSEdgeDataGenerator*>(po);
		std::stringstream ss;
		const char* sztype = plc->GetTypeString();
		if (sztype == 0) sztype = "";
		ss << "Edge data generator" << " [" << sztype << "]";
		return ss.str();
	}
	else if (dynamic_cast<FSFaceDataGenerator*>(po))
	{
		FSFaceDataGenerator* plc = dynamic_cast<FSFaceDataGenerator*>(po);
		std::stringstream ss;
		const char* sztype = plc->GetTypeString();
		if (sztype == 0) sztype = "";
		ss << "Face data generator" << " [" << sztype << "]";
		return ss.str();
	}
	else if (dynamic_cast<FSElemDataGenerator*>(po))
	{
		FSElemDataGenerator* plc = dynamic_cast<FSElemDataGenerator*>(po);
		std::stringstream ss;
		const char* sztype = plc->GetTypeString();
		if (sztype == 0) sztype = "";
		ss << "Element data generator" << " [" << sztype << "]";
		return ss.str();
	}
	else if (dynamic_cast<FSMeshDataGenerator*>(po))
	{
		FSMeshDataGenerator* plc = dynamic_cast<FSMeshDataGenerator*>(po);
		std::stringstream ss;
		const char* sztype = plc->GetTypeString();
		if (sztype == 0) sztype = "";
		ss << "Mesh data generator" << " [" << sztype << "]";
		return ss.str();
	}
	else if (dynamic_cast<FSMeshData*>(po))
	{
		FSNodeData* nodeData = dynamic_cast<FSNodeData*>(po);
		if (nodeData) return "Node data";

		FSSurfaceData* surfData = dynamic_cast<FSSurfaceData*>(po);
		if (surfData) return "Surface data";

		FSElementData* elemData = dynamic_cast<FSElementData*>(po);
		if (elemData) return "Element data";

		FSPartData* partData = dynamic_cast<FSPartData*>(po);
		if (partData) return "Part data";

		assert(false);
		return "Mesh data";
	}
	else if (dynamic_cast<Post::CGLPlot*>(po))
	{
		Post::CGLPlot* plt = dynamic_cast<Post::CGLPlot*>(po); assert(plt);
		return plt->GetTypeString();
	}
	else if (po)
	{
		return po->GetTypeString();
	}

	return "(unknown)";
}

// Reads a single line of a text file while skipping comment lines
char* getline(char* szline, int n, FILE* fp)
{
	char* ch;

	do
	{
		ch = fgets(szline, n, fp);
	} while (ch && (*ch == '*'));

	if (ch == 0) return 0;

	if (ch)
	{
		ch = strrchr(szline, '\n');
		if (ch) *ch = 0;
	}

	return szline;
}

void CGLDocument::SaveResources(OArchive& ar)
{
	for (int i = 0; i < ImageModels(); ++i)
	{
		CImageModel& img = *GetImageModel(i);
		ar.BeginChunk(CID_RESOURCE_IMAGEMODEL);
		{
			img.Save(ar);
		}
		ar.EndChunk();
	}
}

void CGLDocument::LoadResources(IArchive& ar)
{
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case CID_RESOURCE_IMAGEMODEL:
		{
			CImageModel* img = new CImageModel(nullptr);
			img->Load(ar);
			AddImageModel(img);
		}
		break;
		}

		ar.CloseChunk();
	}
}

int CGLDocument::ImageModels() const
{
	return (int)m_img.Size();
}

void CGLDocument::AddImageModel(CImageModel* img)
{
	assert(img);
	m_img.Add(img);
	Update();
}

void CGLDocument::RemoveImageModel(CImageModel* img)
{
	m_img.Remove(img);
}

CImageModel* CGLDocument::GetImageModel(int i)
{
	return m_img[i];
}

void CGLDocument::DeleteAllImageModels()
{
	m_img.Clear();
}

// set/get the file reader
void CGLDocument::SetFileReader(FileReader* fileReader)
{
	if (fileReader != m_fileReader)
	{
		if (m_fileReader) delete m_fileReader;
		m_fileReader = fileReader;
	}
}

FileReader* CGLDocument::GetFileReader()
{
	return m_fileReader;
}

// set/get the file writer
void CGLDocument::SetFileWriter(FileWriter* fileWriter)
{
	if (fileWriter != m_fileWriter)
	{
		if (m_fileWriter) delete m_fileWriter;
		m_fileWriter = fileWriter;
	}
}

FileWriter* CGLDocument::GetFileWriter()
{
	return m_fileWriter;
}

bool CGLDocument::SaveDocument()
{
	if (m_fileWriter && (m_filePath.empty() == false))
	{
		bool success = m_fileWriter->Write(m_filePath.c_str());

		if (success)
		{
			SetModifiedFlag(false);
		}

		return success;
	}
	else
		return false;
}

bool CGLDocument::AutoSaveDocument()
{
	if (m_fileWriter && (m_autoSaveFilePath.empty() == false))
	{
		m_wnd->AddLogEntry(QString("Autosaving file: %1 ...").arg(m_title.c_str()));

		bool success = m_fileWriter->Write(m_autoSaveFilePath.c_str());

		m_wnd->AddLogEntry(success ? "SUCCESS\n" : "FAILED\n");
		SetModifiedFlag(false);
		return success;
	}
	else
		return false;
}

void CGLDocument::Activate()
{
	CDocument::Activate();

	// make sure the correct unit system is active
	Units::SetUnitSystem(m_units);
}

void CGLDocument::GrowElementSelection(FSMesh* pm, bool respectPartitions)
{
	int N = pm->Elements(), i;
	for (i = 0; i < N; ++i) pm->Element(i).m_ntag = 0;
	for (i = 0; i < N; ++i)
	{
		FSElement& e = pm->Element(i);
		if (e.IsSelected())
		{
			e.m_ntag = 1;
			int ne = 0;
			if (e.IsSolid()) ne = e.Faces();
			if (e.IsShell()) ne = e.Edges();

			for (int j = 0; j < ne; ++j)
				if (e.m_nbr[j] >= 0)
				{
					FSElement& ej = pm->Element(e.m_nbr[j]);
					if ((respectPartitions == false) || (e.m_gid == ej.m_gid))
					{
						ej.m_ntag = 1;
					}
				}
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i < N; ++i) if (pm->Element(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pe = new int[n];
		n = 0;
		for (i = 0; i < N; ++i) if (pm->Element(i).m_ntag) pe[n++] = i;
		DoCommand(new CCmdSelectElements(pm, pe, n, false));
		delete[] pe;
	}
}


void CGLDocument::GrowNodeSelection(FSMeshBase* pm)
{
	FSNodeFaceList NFT;
	NFT.Build(pm);

	int i;
	int NN = pm->Nodes();
	int NF = pm->Faces();
	for (i = 0; i < NN; ++i) pm->Node(i).m_ntag = 0;
	for (i = 0; i < NF; ++i) pm->Face(i).m_ntag = 0;
	for (i = 0; i < NN; ++i)
	{
		if (pm->Node(i).IsSelected())
		{
			int nf = NFT.Valence(i);
			for (int j = 0; j < nf; ++j) NFT.Face(i, j)->m_ntag = 1;
		}
	}

	for (i = 0; i < NF; ++i)
	{
		FSFace& f = pm->Face(i);
		if (f.m_ntag == 1)
		{
			for (int j = 0; j < f.Nodes(); ++j) pm->Node(f.n[j]).m_ntag = 1;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i < NN; ++i) if (pm->Node(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pn = new int[n];
		n = 0;
		for (i = 0; i < NN; ++i) if (pm->Node(i).m_ntag) pn[n++] = i;
		DoCommand(new CCmdSelectFENodes(pm, pn, n, false));
		delete[] pn;
	}
}

void CGLDocument::GrowFaceSelection(FSMeshBase* pm, bool respectPartitions)
{
	int N = pm->Faces(), i;
	for (i = 0; i < N; ++i) pm->Face(i).m_ntag = 0;
	for (i = 0; i < N; ++i)
	{
		FSFace& f = pm->Face(i);
		if (f.IsSelected())
		{
			f.m_ntag = 1;
			int n = f.Nodes();
			if (n == 6) n = 3;
			if (n == 8) n = 4;
			for (int j = 0; j < n; ++j)
			{
				if (f.m_nbr[j] >= 0)
				{
					FSFace& fj = pm->Face(f.m_nbr[j]);
					if ((respectPartitions == false) || (f.m_gid == fj.m_gid))
					{
						fj.m_ntag = 1;
					}
				}
			}
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i < N; ++i) if (pm->Face(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pf = new int[n];
		n = 0;
		for (i = 0; i < N; ++i) if (pm->Face(i).m_ntag) pf[n++] = i;
		DoCommand(new CCmdSelectFaces(pm, pf, n, false));
		delete[] pf;
	}
}

void CGLDocument::GrowEdgeSelection(FSMeshBase* pm)
{
	// TODO: implement this
}

void CGLDocument::ShrinkNodeSelection(FSMeshBase* pm)
{
	FSNodeFaceList NFT;
	NFT.Build(pm);

	int i;
	int NN = pm->Nodes();
	int NF = pm->Faces();
	for (i = 0; i < NN; ++i) pm->Node(i).m_ntag = (pm->Node(i).IsSelected() ? 1 : 0);
	for (i = 0; i < NF; ++i) pm->Face(i).m_ntag = 0;
	for (i = 0; i < NN; ++i)
	{
		if (pm->Node(i).IsSelected() == false)
		{
			int nf = NFT.Valence(i);
			for (int j = 0; j < nf; ++j) NFT.Face(i, j)->m_ntag = 1;
		}
	}

	for (i = 0; i < NF; ++i)
	{
		FSFace& f = pm->Face(i);
		if (f.m_ntag == 1)
		{
			for (int j = 0; j < f.Nodes(); ++j) pm->Node(f.n[j]).m_ntag = 0;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i < NN; ++i) if (pm->Node(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pn = new int[n];
		n = 0;
		for (i = 0; i < NN; ++i) if (pm->Node(i).m_ntag) pn[n++] = i;
		DoCommand(new CCmdSelectFENodes(pm, pn, n, false));
		delete[] pn;
	}
}

void CGLDocument::ShrinkFaceSelection(FSMeshBase* pm)
{
	int N = pm->Faces(), i;
	for (i = 0; i < N; ++i) pm->Face(i).m_ntag = 1;
	for (i = 0; i < N; ++i)
	{
		FSFace& f = pm->Face(i);
		if (f.IsSelected() == false)
		{
			f.m_ntag = 0;
			int n = f.Nodes();
			if (n == 6) n = 3;
			if (n == 8) n = 4;
			for (int j = 0; j < n; ++j) if (f.m_nbr[j] >= 0) pm->Face(f.m_nbr[j]).m_ntag = 0;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i < N; ++i) if (pm->Face(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pf = new int[n];
		n = 0;
		for (i = 0; i < N; ++i) if (pm->Face(i).m_ntag) pf[n++] = i;
		DoCommand(new CCmdSelectFaces(pm, pf, n, false));
		delete[] pf;
	}
}

void CGLDocument::ShrinkEdgeSelection(FSMeshBase* pm)
{
	// TODO: implement this
}

void CGLDocument::ShrinkElementSelection(FSMesh* pm)
{
	int N = pm->Elements(), i;
	for (i = 0; i < N; ++i) pm->Element(i).m_ntag = 1;
	for (i = 0; i < N; ++i)
	{
		FSElement& e = pm->Element(i);
		if (e.IsSelected() == false)
		{
			e.m_ntag = 0;
			if (e.IsSolid()) for (int j = 0; j < e.Faces(); ++j) if (e.m_nbr[j] >= 0) pm->Element(e.m_nbr[j]).m_ntag = 0;
			if (e.IsShell()) for (int j = 0; j < e.Edges(); ++j) if (e.m_nbr[j] >= 0) pm->Element(e.m_nbr[j]).m_ntag = 0;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i < N; ++i) if (pm->Element(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pe = new int[n];
		n = 0;
		for (i = 0; i < N; ++i) if (pm->Element(i).m_ntag) pe[n++] = i;
		DoCommand(new CCmdSelectElements(pm, pe, n, false));
		delete[] pe;
	}
}
