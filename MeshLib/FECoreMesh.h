#pragma once
#include "FENode.h"
#include "FEElement.h"
#include "FEMeshBase.h"
#include <vector>

//-----------------------------------------------------------------------------
//! This class defines a simple mesh structure that provides basic container
//! services for storing mesh data. It only stores nodes, edges, faces. It implements 
//! an interface for accessing element data, but derived classes need to implement this. 
class FECoreMesh : public FEMeshBase
{
public:
	//! constructor
	FECoreMesh();

	//! destructor
	virtual ~FECoreMesh();

	//! allocate space for mesh
	virtual void Create(int nodes, int elems, int faces = 0, int edges = 0) = 0;

	//! check the type of the mesh
	bool IsType(int ntype) const;

public: // interface for accessing elements

	//! total number of elements
	virtual int Elements() const = 0;

	//! return reference to element
	virtual FEElement_& ElementRef(int n) = 0;
	virtual const FEElement_& ElementRef(int n) const = 0;

	//! return pointer to element
	virtual  FEElement* ElementPtr(int n=0) = 0;
};
