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
#include "Document.h"
#include "FEBioJob.h"
#include <MeshIO/FSFileImport.h>
#include <vector>

//-----------------------------------------------------------------------------
typedef FSObjectList<CFEBioJob> CFEBioJobList;

//-----------------------------------------------------------------------------
class CModelContext;
class FSObject;

//-----------------------------------------------------------------------------
class CModelDocument : public CGLDocument
{
	Q_OBJECT

public:
	CModelDocument(CMainWindow* wnd);
	~CModelDocument();

	bool LoadTemplate(int n);

	void Clear() override;

	// save/load project
	void Load(IArchive& ar) override;
	void Save(OArchive& ar) override;

	bool Initialize() override;

public:
	//! Get the project
	FSProject& GetProject();

	// get the FE model
	FSModel* GetFSModel();

	// get the geometry
	GModel* GetGModel();

	// return the active object
	GObject* GetActiveObject() override;

	BOX GetModelBox();

public:
	void Activate() override;
	void Deactivate() override;

	void SetActiveItem(FSObject* po);
	FSObject* GetActiveItem();

public:
	void AddObject(GObject* po);

	void DeleteObject(FSObject* po);
	void DeleteObjects(std::vector<FSObject*> objList);

	// helper function for applying a modifier
	bool ApplyFEModifier(FEModifier& modifier, GObject* po, FESelection* sel = 0, bool clearSel = true);
	bool ApplyFESurfaceModifier(FESurfaceModifier& modifier, GSurfaceMeshObject* po, FSGroup* sel = 0);

public: // selection
	void UpdateSelection(bool report = true) override;

	void HideCurrentSelection();
	void HideUnselected();

	void SelectItems(FSObject* po, const std::vector<int>& l, int n);

public:
	int FEBioJobs() const;
	void AddFEbioJob(CFEBioJob* job);
	CFEBioJob* GetFEBioJob(int i);

	CFEBioJob* FindFEBioJob(const std::string& s);

	void DeleteAllJobs();

public:
	bool GenerateFEBioOptimizationFile(const std::string& fileName, FEBioOpt& opt);

	// import geometry (geometry is added to current project)
	bool ImportGeometry(FSFileImport* preader, const char* szfile);

public:
	// checks the model for issues and returns the warnings as a string array
	std::vector<MODEL_ERROR>	CheckModel();

	bool ExportMaterials(const std::string& fileName, const vector<GMaterial*>& matList);
	bool ImportMaterials(const std::string& fileName);
	bool ImportFEBioMaterials(const std::string& fileName);

	void SetUnitSystem(int unitSystem) override;

signals:
	void selectionChanged();

private:
	// the FE Project
	FSProject	m_Project;

	// the job list
	CFEBioJobList	m_JobList;

	CModelContext*	m_context;
};
