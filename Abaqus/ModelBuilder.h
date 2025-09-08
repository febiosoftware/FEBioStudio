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

#include "AbaqusModel.h"

class FSModel;
class GModel;
class Transform;
class FSNodeSet;
class FSSurface;
class FSStep;

// this class builds an FSModel from an Abaqus model
class ModelBuilder
{
public:
	ModelBuilder();

	bool BuildGeometry(AbaqusModel& inp, GModel& g);

	bool BuildPhysics(AbaqusModel& inp, FSModel& fem);

private:
	void build_nodes(AbaqusModel::PART* pg, FSMesh* pm, const Transform& T, int& n);
	void build_elems(AbaqusModel::PART* pg, FSMesh* pm, int& m);
	void build_nodesets(AbaqusModel::PART& pg, FSMesh* pm);
	void build_surfaces(AbaqusModel::PART* pg, FSMesh* pm);
	void build_elemsets(AbaqusModel::PART& pg, FSMesh* pm);

	void partition_elems(AbaqusModel::PART* pg, FSMesh* pm);

	void build_materials(AbaqusModel& inp, FSModel& fem);
	void build_springs(AbaqusModel::PART& part, FSModel& fem);
	void assign_distribution(AbaqusModel::PART& part, FSMesh* pm);
	void build_steps(AbaqusModel& inp, FSModel& fem, FSMesh* pm);
	void build_step(AbaqusModel::STEP& step, FSModel& fem, FSMesh* pm, FSStep* festep);

	FSNodeSet* build_nodeset(AbaqusModel::NODE_SET& ns, FSMesh* pm);
	FSSurface* build_surface(AbaqusModel::SURFACE& si, FSMesh* pm);

	vector<int> get_element_indices(AbaqusModel::ELEMENT_SET* pg);
	vector<int> get_nodal_indices(AbaqusModel::NODE_SET* pg);
	int get_nodal_id(AbaqusModel::PART& part, std::string& ref);

private:
	AbaqusModel* m_inp = nullptr;
	vector<pair<string, string> > m_partitions;
	map<AbaqusModel::NODE_SET*, FSNodeSet*> m_nodeset_map;
	map<AbaqusModel::SURFACE*, FSSurface*> m_surface_map;
};
