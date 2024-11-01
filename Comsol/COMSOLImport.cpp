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

#include "COMSOLImport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
#include <vector>
//using namespace std;

COMSOLimport::COMSOLimport(FSProject& prj) : FSFileImport(prj)
{
	m_domainstosets = false;
	m_bautopart = true;
	m_eltypeseg = false;
	m_addtris = false;
	m_addquads = false;
	m_addtets = true;
	m_addprisms = true;
	m_addhexes = true;
	m_pyrstotets = false;
	m_addpyrs = true;

	m_totalelems = 0;
	m_node0 = 0;	

	AddBoolParam(m_domainstosets, "domains_to_sets", "Convert domains to named selections");
	AddBoolParam(m_bautopart    , "auto_partition" , "Auto-partition domains into parts");
	AddBoolParam(m_eltypeseg    , "seg_elem_type"  , "Segregate element type");
	AddBoolParam(m_addtris      , "add_tris"       , "Import TRI elements");
	AddBoolParam(m_addquads     , "add_quads"      , "Import QUAD elements");
	AddBoolParam(m_addtets      , "add_tet"        , "Import TET elements");
	AddBoolParam(m_addprisms    , "add_prisms"     , "Import PRISM elements");
	AddBoolParam(m_addhexes     , "add_hex"        , "Import HEX elements");
	AddBoolParam(m_pyrstotets   , "add_pyrtotet"   , "Import PYR elements (as 2 TETs each)");
	AddBoolParam(m_addpyrs      , "add_pyr"        , "Import PYR elements");
}

COMSOLimport::~COMSOLimport()
{
}

bool COMSOLimport::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_domainstosets = GetBoolValue(0);
		m_bautopart     = GetBoolValue(1);
		m_eltypeseg     = GetBoolValue(2);
		m_addtris       = GetBoolValue(3);
		m_addquads      = GetBoolValue(4);
		m_addtets       = GetBoolValue(5);
		m_addprisms     = GetBoolValue(6);
		m_addhexes      = GetBoolValue(7);
		m_pyrstotets    = GetBoolValue(8);
		m_addpyrs       = GetBoolValue(9);
	}
	else
	{
		SetBoolValue(0, m_domainstosets);
		SetBoolValue(1, m_bautopart);
		SetBoolValue(2, m_eltypeseg);
		SetBoolValue(3, m_addtris);
		SetBoolValue(4, m_addquads);
		SetBoolValue(5, m_addtets);
		SetBoolValue(6, m_addprisms);
		SetBoolValue(7, m_addhexes);
		SetBoolValue(8, m_pyrstotets);
		SetBoolValue(9, m_addpyrs);
	}

	return false;
}


bool COMSOLimport::Load(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();
	m_pfem = &fem;
	
	char szline[256] = {0};

	int eltypes=0;
	  
	// open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening COMSOL file %s", szfile);

	// Read header (first 9 non-comment lines)
	if (!ReadHeader(szline)) return errf("Failed reading header.", szfile);

	// Read nodes
	if (!ReadNodes(szline)) return errf("Failed reading nodes.", szfile);

	// Loop through element types
	eltypes = GetSingleIntLine(szline);
	for (int i=1; i<=eltypes; i++)
	{
		// Read each element type
		if (!ReadElementType(szline)) return errf("Failed reading element type.", szfile);
	}

	// now that all the elements are imported, parse the domain IDs and convert them to element sets.
	if (!ConvertMapToElSets(szline)) return errf("Failed converting COMSOL domains to element sets.",szfile);

	// close the file
	Close();
    
	// build the mesh
	if (!BuildMesh(fem)) return errf("Could not build mesh.", szfile);;
    
	char szname[256];
	strcpy(szname, szfile);
	char* ch = strrchr(szname, '.');
	if (ch) *ch = 0;
	ch = strrchr(szname, '\\');
	if (ch == 0) ch = strrchr(szname, '/');
	if (ch == 0) ch = szname; else ++ch;
	fem.GetModel().Object(fem.GetModel().Objects()-1)->SetName(ch); // this is setting the wrong name if geometry exists already

    
	return true;
}

bool COMSOLimport::ReadHeader(char* szline)
{
	// we're actually burning the header without verifying anything... :-/
    for (int n=1; n<=9; n++) {
        if (!NextGoodLine(szline)) 
			return false;
    }
	return true;
}

bool COMSOLimport::ReadNodes(char* szline)
{
    int nodes = GetSingleIntLine(szline);
    m_node0 = GetSingleIntLine(szline);
    
	NODE node;
    
    for (int n=m_node0; n<(nodes+m_node0); n++) {
        
        if (NextGoodLine(szline)) { //
            node.id = n;
            // convert line to node coordinates!
            if (!sscanf(szline, "%lg %lg %lg", &node.x, &node.y, &node.z))
				return false;
            // add the node to the list
            m_Node.push_back(node);
        } else {
            return errf("Error encountered while reading Nodes dataset (2411)");
        }
    }

	// no errors encountered
	return true;
}
bool COMSOLimport::ReadElementType(char* szline)
{
	int dummy = 0;
	char eltype[8] = {};
	char elset[20] = {};
	char domain_id[8] = {};
	ELEMENT el;
    int * n = el.n;

	if (NextGoodLine(szline)) {
		if (!sscanf(szline, "%i %s ", &dummy, eltype))
			return false;
	} else {
		return false;
	}

	// read element lines
	int nodesperel = GetSingleIntLine(szline);
	int newelems = GetSingleIntLine(szline);
	
	bool import_this_type;
	bool is_pyramid = false;
	
    if ((eltype[1]=='e') && (nodesperel==4)) {
        // detect the 'e' in tet (3D)
        el.ntype = 100;
    } else if ((eltype[0]=='p') && (nodesperel==5)) {
        // detect the 'p' in pyr (pyramid element to be split into 2 tets)
        if (m_pyrstotets) {
            el.ntype = 100;      // tet here as final element type for pyr
            is_pyramid = true;
            newelems=2*newelems; // each pyr => two tets
        }
        else
            el.ntype = 200;
    } else if ((eltype[0]=='p') && (nodesperel==6)) {
        // detect the 'p' in prism (pentahedral 3D)
        el.ntype = 112;
    } else if ((eltype[2]=='x') && (nodesperel==8)) {
        // detect the 'x' in hex (3D)
        el.ntype = 101;
    } else if ((eltype[1]=='r') && (nodesperel==3)) {
        // detect the 'r' in tri (2D)
        el.ntype = 91;
    } else if ((eltype[1]=='u') && (nodesperel==4)) {
        // detect the 'u' in quad (2D)
        el.ntype = 94;
    } else {
        import_this_type = false;
        //return errf("Unsupported node type. (not tet, tri, or hex)");
    }

	switch(el.ntype) {
	case 100: 
		if (nodesperel==5) // if these are pyramid elements, use the convert pyr option
			import_this_type = m_pyrstotets;
		else // otherwise use the tet option to decide whether to import
			import_this_type = m_addtets;
		break;
	case 112: 
		import_this_type = m_addprisms;
		break;
	case 101: 
		import_this_type = m_addhexes;
		break;
	case 91: 
		import_this_type = m_addtris;
		break;
	case 94: 
		import_this_type = m_addquads;
		break;
    case 200:
        import_this_type = m_addpyrs;
        break;
	default:
		import_this_type = false;
	}

	std::vector< std::list<ELEMENT>::iterator > newElems;

	for (int i=0;i<newelems;++i){
		if (!NextGoodLine(szline)) 
			return errf("Error encountered trying to parse an element.");
		if (import_this_type==true) {
			if (is_pyramid==true) { // pyramid elements
				int pyr[5];
				if (!sscanf(szline, "%d %d %d %d %d %d %d %d", &pyr[0], &pyr[1], &pyr[2], &pyr[3], &pyr[4], &dummy, &dummy, &dummy))
					return false; 
				// first tet element from single pyr
				el.id = m_totalelems+i;
				el.pid = 0;
				// convert line to node coordinates!
				//                        pyr numbering:    1     2      3       4       5
				n[0]=pyr[0];
				n[1]=pyr[1];
				n[2]=pyr[2];
				n[3]=pyr[4];
				// add the node to the list
				m_Elem.push_back(el);
				
				// second tet element from single pyr
				++i;
				el.id = m_totalelems+i;
				el.pid = 0;
				n[0]=pyr[1];
				n[1]=pyr[3];
				n[2]=pyr[2];
				n[3]=pyr[4];
				// add the node to the list
				m_Elem.push_back(el);

			} else { // all other elements
				el.id = m_totalelems+i;
				el.pid = 0;
				// convert line to node coordinates!
                if (el.ntype == 101) { // comsol uses different node numbering than febio for hexes
                    if (!sscanf(szline, "%d %d %d %d %d %d %d %d", &n[0], &n[1], &n[3], &n[2], &n[4], &n[5], &n[7], &n[6]))
                        return false;
                }
                else if (el.ntype == 200) { // comsol uses different node numbering than febio for pyramids
                    if (!sscanf(szline, "%d %d %d %d %d %d %d %d", &n[0], &n[1], &n[3], &n[2], &n[4], &n[5], &n[6], &n[7]))
                        return false;
                }
                else {
                    if (!sscanf(szline, "%d %d %d %d %d %d %d %d", &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7]))
                        return false;
                }
				// add the node to the list
				m_Elem.push_back(el);

				newElems.push_back(--m_Elem.end());
			}
		}
	}


	// ignore element parameters
	// note that if this section is ever implemented, pyramid element conversion must be handled
	// which means parameters should be duplicated for the two tet elements formed from each pyr
    bool oldformat = true;
	int paramperel = GetSingleIntLine(szline);
    if (strstr(szline,"geometric entity indices")) oldformat = false;
    if (oldformat) {
        int newparams = GetSingleIntLine(szline);
        for (int i=m_totalelems;i<newparams+m_totalelems;i++){
            if (!NextGoodLine(szline))
                return errf("Error encountered trying to parse element parameters.");
        }
        
        // Read element domains and treat them like element sets by adding them to the map.
        // We will name element sets according to element type and domain ID.
        // The map we're creating now is used for "create element sets" and "auto partition."
        int domains = GetSingleIntLine(szline);
        if (is_pyramid==true) domains=domains*2;
        if (domains==newelems) {
            //        for (int i=1;i<(1+domains);++i){
            for (int i=m_node0;i<domains+m_node0;++i){
                if (NextGoodLine(szline)){
                    if (import_this_type==true) {
                        if (!sscanf(szline, "%s", domain_id))
                            return false;
                        strcpy(elset,"dom_"); // begin element set with dom_
                        if (m_eltypeseg==true) { // add tet/tri to element set string
                            strcat(elset,eltype);
                            strcat(elset,"_");
                        }
                        strcat(elset,domain_id); // add domain ID to element set string
                        map_ElSet[m_totalelems+i]=elset; // this is the map containing id/set pairs.
                        //        ^ i is an int, elset is a string.
                        if (is_pyramid==true) { // pyramid elements // add set info for second half of a split pyramid
                            ++i;
                            map_ElSet[m_totalelems+i]=elset;
                        }
                    }
                } else {
                    return false;
                }
            }
            
        }
        
        // skip up/down pairs
        //
        int updowns = GetSingleIntLine(szline);
        for (int i=1;i<=updowns;i++){
            if (!NextGoodLine(szline))
                return errf("Error encountered trying to skip up/down pairs.");
        }
    }
    else {
        // ignore geometric entity indices
        for (int i=0;i<paramperel;i++) {
            if (!NextGoodLine(szline))
                return errf("Error encountered trying to parse element parameters.");

			if (import_this_type)
			{
				int domain_id = -1;
				if (!sscanf(szline, "%d", &domain_id))
					return false;

				if (is_pyramid)
				{
					newElems[2 * i]->pid = domain_id;
					newElems[2 * i + 1]->pid = domain_id;
				}
				else
					newElems[i]->pid = domain_id;
			}
        }
    }

	if (import_this_type==true)
		m_totalelems += newelems;

	// no errors encountered
	return true;
}
bool COMSOLimport::BuildMesh(FSModel& fem)
{
	int i, j;
    
	// count mesh items
	int nodes = (int)m_Node.size();
	int elems = (int)m_Elem.size();
    
	// create new mesh
	FSMesh* pm = new FSMesh;
	pm->Create(nodes, elems);
    
	// create the node-lookup table
	list<NODE>::iterator in = m_Node.begin();
	int imin = in->id;
	int imax = in->id;
	for (i=0; i<nodes; ++i, ++in)
	{
		if (in->id < imin) imin = in->id;
		if (in->id > imax) imax = in->id;
	}
    
	int nsize = imax - imin + 1;
	std::vector<int> NLT; NLT.resize(nsize);
	for (i=0; i<nsize; ++i) NLT[i] = -1;
    
	in = m_Node.begin();
	for (i=0; i<nodes; ++i, ++in) NLT[in->id - imin] = i;
    
	// create the nodes
	in = m_Node.begin();
	FSNode* pn = pm->NodePtr();
	for (i=0; i<nodes; ++i, ++in, ++pn)
	{
		pn->r.x = in->x;
		pn->r.y = in->y;
		pn->r.z = in->z;
	}
    
	// create the elements
	list<ELEMENT>::iterator ie = m_Elem.begin();
	int ne;
	for (i=0; i<elems; ++i, ++ie)
	{
		FEElement_* pe = pm->ElementPtr(i);

		switch (ie->ntype)
		{
            case  91: pe->SetType(FE_TRI3  ); break;
            case  94: pe->SetType(FE_QUAD4 ); break;
            case 100: pe->SetType(FE_TET4  ); break;
            case 112: pe->SetType(FE_PENTA6); break;
            case 101: pe->SetType(FE_HEX8  ); break;
            case 200: pe->SetType(FE_PYRA5 ); break;
            default:
                delete pm;
                return false;
		}
        
		pe->m_gid = ie->pid;
        
		// read the nodes
		ne = pe->Nodes();
		for (j=0; j<ne; ++j) pe->m_node[j] = NLT[ie->n[j] - imin];
	}
	
	// auto-partition
	int elsets = (int)m_ElSet.size();

	if (m_bautopart && (elsets > 0))
	{
		list<ELEMENT_SET>::iterator pes = m_ElSet.begin();
		for (i=0; i<elsets; ++i, ++pes)
		{
			int n = (int)pes->elem.size();
			list<Telem_itr>::iterator pe = pes->elem.begin();
			for (j=0; j<n; ++j, ++pe)
			{
				FSElement& el = pm->Element((*pe)->id);
				el.m_gid = i;
			}
		}
	}

	pm->RebuildMesh();

	// next, we will add the mesh surfaces. Before we 
	// can do that we need to build and update the mesh.
	// This is simply done by attaching the mesh to an object
	GMeshObject* po = new GMeshObject(pm);

	// rename the parts to correspond to the element sets
	if (m_bautopart && (po->Parts() == elsets))
	{
		list<ELEMENT_SET>::iterator pes = m_ElSet.begin();
		for (i=0; i<elsets; ++i, ++pes) po->Part(i)->SetName(pes->szname);						
	}
	
	// read element sets
	if (m_domainstosets)
	{
		int elsets = (int)m_ElSet.size();
		if (elsets)
		{
			list<ELEMENT_SET>::iterator pes = m_ElSet.begin();
			for (i=0; i<elsets; ++i, ++pes)
			{
				int n = (int)pes->elem.size(); // how many elements are in the element set? -> n
				FSElemSet* pg = new FSElemSet(pm);
				pg->SetName(pes->szname);
				list<Telem_itr>::iterator pe = pes->elem.begin();
				for (j=0; j<n; ++j, ++pe) pg->add((*pe)->id);
				pm->AddFEElemSet(pg);
			}
		}
	}

    
	// update the mesh
	pm->RebuildMesh();
    
	// if we get here we are good to go!
	fem.GetModel().AddObject(new GMeshObject(pm));
    
	// clean up
	m_Node.clear();
	m_Elem.clear();
	m_ElSet.clear();
	map_ElSet.clear();
    
	return true;
}

list<COMSOLimport::ELEMENT_SET>::iterator COMSOLimport::FindElementSet(const char* szname)
{
	int n = (int)m_ElSet.size();
	Telset_itr pe = m_ElSet.begin();
	for (int i=0; i<n; ++i, ++pe) if (strcmp(pe->szname, szname) == 0) return pe;
	return m_ElSet.end();
}

list<COMSOLimport::ELEMENT_SET>::iterator COMSOLimport::AddElementSet(const char* szname)
{
	ELEMENT_SET es;
	strcpy(es.szname, szname);
	m_ElSet.push_back(es);
	return --m_ElSet.end();
}

bool COMSOLimport::NextGoodLine(char* szline)
{
    while (fgets(szline, 255, m_fp)) {
        if ((szline[0] != '#') && (szline[0] != '\0') && (szline[0] != '\r') && (szline[0] != '\n')) {
            return true;
        }
    }
    return false;
}

bool COMSOLimport::ConvertMapToElSets(char* szline)
{
	// loop through elset map
	Telem_itr elemitr = m_Elem.begin();
	for (Telsetmap_itr setitr = map_ElSet.begin();
		 setitr != map_ElSet.end(); ++setitr, ++elemitr)
	{
		string elset = setitr->second;
		char *elsetchar=new char[elset.size()+1];
		elsetchar[elset.size()]=0;
		memcpy(elsetchar,elset.c_str(),elset.size());
		list<ELEMENT_SET>::iterator pset = FindElementSet(elsetchar);
		if (pset == m_ElSet.end()) pset = AddElementSet(elsetchar);
		pset->elem.push_back(elemitr);
	}
	return true;

}

int COMSOLimport::GetSingleIntLine(char* szline)
{
	int val=-999999;
	while (fgets(szline, 255, m_fp)) {
        if ((szline[0] != '#') && (szline[0] != '\0') && (szline[0] != '\r') && (szline[0] != '\n')) {
			if (sscanf(szline, "%d", &val))
            return val;
        }
    }
    return val;
}
