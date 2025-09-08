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
#include <MeshIO/FSFileImport.h>
#include <FEMLib/FSProject.h>
#include "AbaqusModel.h"

#include <list>
////using namespace std;

//-----------------------------------------------------------------------------
// Implements a class to import ABAQUS files
// 
class AbaqusImport : public FSFileImport
{
public:
	enum {MAX_ATTRIB = 16};
	enum {MAX_STRING = 256};

	// attributes
	struct ATTRIBUTE
	{
		char szatt[MAX_STRING] = { 0 };	// name of attribute
		char szval[MAX_STRING] = { 0 };	// value of attribute
	};

	enum SCOPE {
		GLOBAL_SCOPE,
		ASSEMBLY_SCOPE,
		INSTANCE_SCOPE,
		PART_SCOPE
	};

public:
	class Exception{};

public:	// import options
	bool	m_breadPhysics;	// read the physics (i.e. materials, bcs, etc).

public:
	AbaqusImport(FSProject& prj);
	virtual ~AbaqusImport();

	bool Load(const char* szfile);

protected:
	// read a line and increment line counter
	bool read_line(char* szline, FILE* fp);

	// build the model
	bool build_model();

	// Keyword parsers
	bool read_heading            (char* szline, FILE* fp);
	bool read_nodes              (char* szline, FILE* fp);
	bool read_ngen               (char* szline, FILE* fp);
	bool read_nfill              (char* szline, FILE* fp);
	bool read_elements           (char* szline, FILE* fp);
	bool read_element_sets       (char* szline, FILE* fp);
	bool read_node_sets          (char* szline, FILE* fp);
	bool read_surface            (char* szline, FILE* fp);
	bool read_surface_interaction(char* szline, FILE* fp);
	bool read_materials          (char* szline, FILE* fp);
	bool read_part               (char* szline, FILE* fp);
	bool read_end_part           (char* szline, FILE* fp);
	bool read_instance           (char* szline, FILE* fp);
	bool read_end_instance       (char* szline, FILE* fp);
	bool read_assembly           (char* szline, FILE* fp);
	bool read_end_assembly       (char* szline, FILE* fp);
	bool read_spring_elements    (char* szline, FILE* fp);
	bool read_step               (char* szline, FILE* fp);
	bool read_end_step           (char* szline, FILE* fp);
	bool read_boundary           (char* szline, FILE* fp);
	bool read_dsload             (char* szline, FILE* fp);
	bool read_cload              (char* szline, FILE* fp);
	bool read_solid_section      (char* szline, FILE* fp);
	bool read_shell_section      (char* szline, FILE* fp);
	bool read_static             (char* szline, FILE* fp);
	bool read_orientation        (char* szline, FILE* fp);
	bool read_distribution       (char* szline, FILE* fp);
	bool read_amplitude          (char* szline, FILE* fp);
	bool read_contact_pair       (char* szline, FILE* fp);
	bool read_tie                (char* szline, FILE* fp);
	bool read_spring             (char* szline, FILE* fp);
	bool read_include            (char* szline, FILE* fp);

	// skip until we find the next keyword
	bool skip_keyword(char* szline, FILE* fp);

protected:
	// parse a file for keywords
	bool parse_file(FILE* fp);

	AbaqusModel::PART* GetActivePart();

private:
	string		m_title;
	FSProject*	m_pprj;

	FSModel*	m_pfem;

	AbaqusModel		m_inp;

	SCOPE m_scope = SCOPE::GLOBAL_SCOPE;
	AbaqusModel::PART* m_currentPart = nullptr;
	AbaqusModel::STEP* m_currentStep = nullptr;

	int	m_nline;	// current line number
};
