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
#include "UndoDocument.h"
#include <GLLib/GView.h>
#include <MeshTools/FESelection.h>
#include <FSCore/FSObjectList.h>

class GLScene;
class CImageModel;
class FileWriter;
class FileReader;

// Transform Modes
enum TransformMode
{
	TRANSFORM_NONE = 1,
	TRANSFORM_MOVE = 2,
	TRANSFORM_ROTATE = 3,
	TRANSFORM_SCALE = 4
};

// see CGLDocument::GetSelectionMode()
enum SelectionMode {
	SELECT_OBJECT = 1,
	SELECT_PART = 2,
	SELECT_FACE = 3,
	SELECT_EDGE = 4,
	SELECT_NODE = 5,
	SELECT_DISCRETE = 6
};

// Selection Styles
enum RegionSelectionStyle {
	REGION_SELECT_BOX    = 1,
	REGION_SELECT_CIRCLE = 2,
	REGION_SELECT_FREE   = 3
};

// Sub-object item Modes
enum ItemSelectionMode {
	ITEM_MESH = 1,
	ITEM_ELEM = 2,
	ITEM_FACE = 3,
	ITEM_NODE = 4,
	ITEM_EDGE = 5
};

// mesh selection modes
enum MeshSelectionMode {
	MESH_MODE_VOLUME  = 0,
	MESH_MODE_SURFACE = 1
};

// Base class for documents that require visualization
class CGLDocument : public CUndoDocument
{
public:
	enum UI_VIEW_MODE
	{
		MODEL_VIEW, SLICE_VIEW, TIME_VIEW_2D
	};

public:
	CGLDocument(CMainWindow* wnd);
	~CGLDocument();

	bool SaveDocument() override;

	bool AutoSaveDocument() override;

	void Activate() override;

	// set/get the file reader
	void SetFileReader(FileReader* fileReader);
	FileReader* GetFileReader();

	// set/get the file writer
	void SetFileWriter(FileWriter* fileWriter);
	FileWriter* GetFileWriter();

	// --- view state ---
	VIEW_STATE GetViewState() { return m_vs; }
	void SetViewState(VIEW_STATE vs);

	int GetTransformMode() { return m_vs.ntrans; }
	void SetTransformMode(TransformMode mode);

	int GetSelectionMode() { return m_vs.nselect; }
	void SetSelectionMode(int mode) { m_vs.nitem = ITEM_MESH; m_vs.nselect = mode; UpdateSelection(); }

	void SetSelectionStyle(int nstyle) { m_vs.nstyle = nstyle; }
	int GetSelectionStyle() { return m_vs.nstyle; }

	int GetItemMode() { return m_vs.nitem; }
	void SetItemMode(int mode) { m_vs.nitem = mode; UpdateSelection(); }

	static std::string GetTypeString(FSObject* po);

	UI_VIEW_MODE GetUIViewMode() { return m_uiMode; }
	void SetUIViewMode(UI_VIEW_MODE vm) { m_uiMode = vm; }

	virtual int GetMeshMode() { return MESH_MODE_VOLUME; }

	// return the current selection
	FESelection* GetCurrentSelection();
	void SetCurrentSelection(FESelection* psel);

	// get the selection bounding box
	BOX GetSelectionBox();

	virtual void UpdateSelection();

	virtual GObject* GetActiveObject();

	CGView* GetView();

	GLScene* GetScene();

	void Update() override;

public:
	bool ShowTitle() const { return m_showTitle; }
	bool ShowSubtitle() const { return m_showSubtitle; }
	bool ShowLegend() const { return m_showLegend; }

	void ShowLegend(bool b) { m_showLegend = b; }
	void SetDataRange(double vmin, double vmax) { m_dataRange[0] = vmin; m_dataRange[1] = vmax; }
	void GetDataRange(double v[2]) { v[0] = m_dataRange[0]; v[1] = m_dataRange[1]; }
	int GetColorGradient() const { return m_colorGradient; }
	int GetLegendDivisions() const { return m_legendDivisions; }	
	bool GetLegendSmoothing() const { return m_legendSmooth; }	

public:
	void setModelInfo(const std::string& s) { m_info = s; }
	std::string getModelInfo() const { return m_info; }

public:
	int ImageModels() const;
	virtual void AddImageModel(CImageModel* img);
	virtual void RemoveImageModel(CImageModel* img);
	CImageModel* GetImageModel(int i);
	void DeleteAllImageModels();

public:
	void GrowNodeSelection(FSMeshBase* pm);
	void GrowFaceSelection(FSMeshBase* pm, bool respectPartitions = true);
	void GrowEdgeSelection(FSMeshBase* pm);
	void GrowElementSelection(FSMesh* pm, bool respectPartitions = true);
	void ShrinkNodeSelection(FSMeshBase* pm);
	void ShrinkFaceSelection(FSMeshBase* pm);
	void ShrinkEdgeSelection(FSMeshBase* pm);
	void ShrinkElementSelection(FSMesh* pm);

protected:
	void SaveResources(OArchive& ar);
	void LoadResources(IArchive& ar);

public:
	virtual void SetUnitSystem(int unitSystem);
	int GetUnitSystem() const;

protected:
	GLScene* m_scene;

	VIEW_STATE	m_vs;	// the view state

	UI_VIEW_MODE m_uiMode;

	std::string		m_info;
	int				m_units;

	// current selection
	FESelection* m_psel;

	FSObjectList<CImageModel>	m_img;

	FileReader* m_fileReader;
	FileWriter* m_fileWriter;

	// GL widget parameters
	bool	m_showTitle;
	bool	m_showSubtitle;
	bool	m_showLegend;
	double	m_dataRange[2];
	int		m_colorGradient;
	int		m_legendDivisions;
	bool	m_legendSmooth;
};
