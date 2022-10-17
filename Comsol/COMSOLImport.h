#pragma once
#include <MeshIO/FileReader.h>
#include <MeshTools/FEProject.h>

#include <list>
#include <vector>
#include <string>
#include <map>
//using namespace std;

using std::map;

// This code imports COMSOL mesh files.
// Developed by Michael Fernandez (mjf2152@columbia.edu) at Columbia (10/22/13)
// based on FEIDEAS and AbaqusImport.

class COMSOLimport : public FSFileImport
{

public:
	COMSOLimport(FSProject& prj);
	virtual ~COMSOLimport();
	bool	Load(const char* szfile);
	bool	m_domainstosets;	// create element sets by comsol domain
	bool	m_bautopart;	// create parts out of sets	
	bool	m_addtris;	    // add triangle planar elements
	bool	m_addquads;	    // add quad planar elements
	bool	m_addtets;	    // add tet elements
	bool	m_addhexes;	    // add hex elements
	bool	m_addprisms;	// add prism elements (5 sided/pentahedral)
	bool	m_pyrstotets;	// convert pyramid elements to two tets by splitting
    bool    m_addpyrs;      // add pyramid elements
	bool	m_eltypeseg;	// segregate element types in partitioning and set generation
	

protected:
	
	struct NODE
	{
		int		id;
		double	x, y, z;
	};
    
	struct ELEMENT
	{
		int id;		// element ID
		int	pid;	// part ID
		int ntype;	// element type
		int	n[8];	// node labels
	};
	typedef list<ELEMENT>::iterator Telem_itr;

	struct ELEMENT_SET
	{
		char szname[16];	// element set name
		list<Telem_itr>	elem;
	};
	typedef list<ELEMENT_SET>::iterator Telset_itr;
		
	bool BuildMesh(FSModel& fem);
	bool ReadHeader(char* szline);
	bool ReadNodes(char* szline);
	bool ReadElementType(char* szline);
	bool ReadDomains(char* szline);
    bool ConvertMapToElSets(char* szline);
    bool NextGoodLine(char* szline);
	int GetSingleIntLine(char* szline);
	Telset_itr FindElementSet(const char* szname);	// find an element set with a particular name
	Telset_itr AddElementSet(const char* szname); 	// add an element set

	list<NODE>		m_Node;
	list<ELEMENT>	m_Elem;
	string intToString(int number);
	map<int,string> map_ElSet;     // map of element sets <int element id, string set name>
	typedef map<int,string>::iterator Telsetmap_itr;

	list<ELEMENT_SET>	m_ElSet;	// element sets, one per comsol domain/element type combination
	FSModel*	m_pfem;
    int m_totalelems;
    int m_node0;
};
