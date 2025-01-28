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
#include <QTreeWidget>
#include <vector>
#include <map>

class CModelViewer;
class CModelDocument;
class CPropertyList;
class FSModel;
class FSStep;
class FSObject;
class CObjectProps;
class FSMaterial;
class FSReactionMaterial;
class GMaterial;
class FSProject;
class FSModelComponent;

class CObjectValidator;
class CFSObjectProps;

enum ModelTreeType
{
	MT_FEOBJECT,
	MT_OBJECT,
	MT_PART,
	MT_SURFACE,
	MT_EDGE,
	MT_NODE,
	MT_FEMODEL,
	MT_MATERIAL_LIST,
	MT_SOLUTES_LIST,
	MT_SOLUTE,
	MT_SBM_LIST,
	MT_SBM,
	MT_NAMED_SELECTION,
	MT_OBJECT_LIST,
	MT_PART_LIST,
	MT_FACE_LIST,
	MT_EDGE_LIST,
	MT_NODE_LIST,
	MT_BC_LIST,
	MT_LOAD_LIST,
	MT_IC_LIST,
	MT_CONTACT_LIST,
	MT_CONSTRAINT_LIST,
	MT_RIGID_COMPONENT_LIST,
	MT_RIGID_BC,
	MT_RIGID_IC,
	MT_RIGID_LOAD,
	MT_RIGID_CONNECTOR,
	MT_DISCRETE_LIST,
	MT_DISCRETE_SET,
	MT_DISCRETE,
	MT_STEP_LIST,
	MT_LOAD_CONTROLLERS,
	MT_LOAD_CONTROLLER,
	MT_PROJECT_OUTPUT,
	MT_PROJECT_OUTPUT_PLT,
	MT_PROJECT_OUTPUT_LOG,
	MT_PART_GROUP,
	MT_FACE_GROUP,
	MT_EDGE_GROUP,
	MT_NODE_GROUP,
	MT_FEPART_GROUP,
	MT_FEELEM_GROUP,
	MT_FEFACE_GROUP,
	MT_FEEDGE_GROUP,
	MT_FENODE_GROUP,
	MT_BC,
	MT_LOAD,
	MT_IC,
	MT_CONTACT,
	MT_CONSTRAINT,
	MT_STEP,
	MT_MATERIAL,
	MT_MATERIAL_REACTANTS,
	MT_MATERIAL_PRODUCTS,
	MT_DATAMAP,
	MT_JOBLIST,
	MT_JOB,
	MT_POST_MODEL,
	MT_POST_PLOT,
	MT_3DIMAGE,
	MT_IMGANALYSIS,
	MT_MESH_DATA_LIST,
	MT_MESH_DATA,
	MT_MESH_DATA_GENERATOR,
	MT_MESH_ADAPTOR_LIST,
	MT_MESH_ADAPTOR
};

enum ModelTreeFilter
{
	FILTER_NONE,
	FILTER_GEOMETRY,
	FILTER_MATERIALS,
	FILTER_PHYSICS,
	FILTER_STEPS,
	FILTER_JOBS,
    FILTER_IMAGES
};

struct CModelTreeItem
{
	FSObject*			obj;	// the object
	int					flag;	// 0 = list view, 1 = form view
	int					type;	// the object type
	const char*			szicon = nullptr;	// the icon
};

struct CPropertyItem
{
	CFSObjectProps* props = nullptr;
	CObjectValidator* val = nullptr;
};

class CModelTree : public QTreeWidget
{
	Q_OBJECT

public:
	enum ModelFlags
	{
		SHOW_PROPERTY_FORM  = 1,	// show the properties in a form or in a list
		OBJECT_NOT_EDITABLE = 2,	// is the object associated with the item editable
		DUPLICATE_ITEM      = 4,	// this item is a duplicate
		NAME_NOT_EDITABLE   = 8		// the name cannot be edited
	};

public:
	CModelTree(CModelViewer* view, QWidget* parent = 0);

	void SetFilter(int n);

	// build the model tree from the document
	void Build(CModelDocument* doc);

	void ShowItem(QTreeWidgetItem* item);

	void Select(FSObject* po);
	void Select(const std::vector<FSObject*>& objList);
	
	void UpdateObject(FSObject* po);
	void UpdateItem(QTreeWidgetItem* item);

	void contextMenuEvent(QContextMenuEvent* ev) override;

	int Items() const { return (int) m_data.size(); }
	CModelTreeItem& GetItem(int n) { return m_data[n]; }

	QTreeWidgetItem* FindItem(FSObject* o);

	bool GetSelection(std::vector<FSObject*>& sel);

	QStringList GetAllWarnings();

protected:
	void ClearData();

	QTreeWidgetItem* AddTreeItem(QTreeWidgetItem* parent, const QString& name, int ntype = 0, int ncount = 0, FSObject* po = nullptr, int flags = 0, const char* szicon = nullptr);

	void UpdateModelData      (QTreeWidgetItem* t1, FSModel& fem);
	void UpdateObjects        (QTreeWidgetItem* t1, FSModel& fem);
	void UpdateGroups         (QTreeWidgetItem* t1, FSModel& fem);
	void UpdateBC             (QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep);
	void UpdateLoads          (QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep);
	void UpdateICs            (QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep);
	void UpdateContact        (QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep);
	void UpdateConstraints    (QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep);
	void UpdateRigid          (QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep);
	void UpdateSteps          (QTreeWidgetItem* t1, FSProject& fem);
	void UpdateMaterials      (QTreeWidgetItem* t1, FSModel& fem);
	void UpdateDiscrete       (QTreeWidgetItem* t1, FSModel& fem);
	void UpdateLoadControllers(QTreeWidgetItem* t1, FSModel& fem);
	void UpdateOutput         (QTreeWidgetItem* t1, FSProject& prj);
	void UpdateJobs           (QTreeWidgetItem* t1, CModelDocument* doc);
	void UpdateImages         (QTreeWidgetItem* t1, CModelDocument* doc);
	void UpdateMeshData       (QTreeWidgetItem* t1, FSModel& fem);
	void UpdateMeshAdaptors   (QTreeWidgetItem* t1, FSModel& fem, FSStep* pstep);

	CModelTreeItem* GetCurrentData();

private:
	void BuildPropertyLists(CModelDocument* doc);

private:
	std::vector<CModelTreeItem>		m_data;
	std::map<int, CPropertyItem>	m_props;

	CModelViewer*				m_view;
	int							m_nfilter;

	friend class CModelViewer;
};
