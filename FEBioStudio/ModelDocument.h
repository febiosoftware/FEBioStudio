/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

//-----------------------------------------------------------------------------
typedef FSObjectList<CFEBioJob> CFEBioJobList;

//-----------------------------------------------------------------------------
class CModelDocument : public CDocument
{
public:
	CModelDocument(CMainWindow* wnd);

	bool LoadTemplate(int n);

	void Clear() override;

	// save/load project
	void Load(IArchive& ar) override;
	void Save(OArchive& ar) override;

public:
	//! Get the project
	FEProject& GetProject();

	// get the FE model
	FEModel* GetFEModel();

	// get the geometry
	GModel* GetGModel();

	// return the active object
	GObject* GetActiveObject() override;

	BOX GetModelBox();

public:
	void AddObject(GObject* po);

	void DeleteObject(FSObject* po);

	// helper function for applying a modifier
	bool ApplyFEModifier(FEModifier& modifier, GObject* po, FEGroup* sel = 0, bool clearSel = true);
	bool ApplyFESurfaceModifier(FESurfaceModifier& modifier, GSurfaceMeshObject* po, FEGroup* sel = 0);

public: // selection
	FESelection* GetCurrentSelection();
	void UpdateSelection(bool report = true) override;

	void GrowNodeSelection(FEMeshBase* pm);
	void GrowFaceSelection(FEMeshBase* pm, bool respectPartitions = true);
	void GrowEdgeSelection(FEMeshBase* pm);
	void GrowElementSelection(FEMesh* pm, bool respectPartitions = true);
	void ShrinkNodeSelection(FEMeshBase* pm);
	void ShrinkFaceSelection(FEMeshBase* pm);
	void ShrinkEdgeSelection(FEMeshBase* pm);
	void ShrinkElementSelection(FEMesh* pm);

	void HideCurrentSelection();
	void HideUnselected();

public:
	int FEBioJobs() const;
	void AddFEbioJob(CFEBioJob* job);
	CFEBioJob* GetFEBioJob(int i);

	CFEBioJob* FindFEBioJob(const std::string& s);

public:
	bool GenerateFEBioOptimizationFile(const std::string& fileName, FEBioOpt& opt);

	// import geometry (geometry is added to current project)
	bool ImportGeometry(FEFileImport* preader, const char* szfile);

public:
	// checks the model for issues and returns the warnings as a string array
	std::vector<MODEL_ERROR>	CheckModel();

	bool ExportMaterials(const std::string& fileName, const vector<GMaterial*>& matList);
	bool ImportMaterials(const std::string& fileName);

private:
	// the FE Project
	FEProject	m_Project;

	// the job list
	CFEBioJobList	m_JobList;

	// current selection
	FESelection*	m_psel;
};
