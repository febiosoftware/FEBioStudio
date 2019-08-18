#pragma once
#include <list>
using namespace std;

//-----------------------------------------------------------------------------
// Forward declaration of the FECoreMesh
class FECoreMesh;

//-----------------------------------------------------------------------------
// The FEItemList stores a list of pointers to FEItems (nodes, faces, elements)
//

template <class T> class FEItemList_T
{
public:
	struct ITEM
	{
		ITEM(FECoreMesh* pm, T* pi) { m_pm = pm; m_pi = pi; }
		FECoreMesh*	m_pm;
		T*		m_pi;
	};

	typedef typename list<ITEM>::iterator Iterator;

public:
	FEItemList_T(){}

	void Add(FECoreMesh* pm, T* pn) { m_Item.push_back(ITEM(pm, pn)); }
	int Size() { return (int)m_Item.size(); }
	Iterator First() { return m_Item.begin(); }
	Iterator End() { return m_Item.end(); }

protected:
	list<ITEM>	m_Item;
};

//-----------------------------------------------------------------------------
// Specialization for nodes
class FENode;
typedef FEItemList_T<FENode> FENodeList;

//-----------------------------------------------------------------------------
// Specialization for faces
class FEFace;
typedef FEItemList_T<FEFace> FEFaceList;

//-----------------------------------------------------------------------------
// Specialization for elements
class FEElement;
typedef FEItemList_T<FEElement> FEElemList;
