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
#include <MeshLib/FSMesh.h>
#include <FSCore/FSThreadedTask.h>
class FESelection;
class GObject;
class GFace;

//-----------------------------------------------------------------------------

class FEModifier : public FSThreadedTask 
{
public:
	FEModifier(const char* sz);
	virtual ~FEModifier();

	virtual FSMesh* Apply(FSMesh* pm);
	virtual FSMesh* Apply(FSGroup* pg);
	virtual FSMesh* Apply(GObject* po, FESelection* pg);

	virtual FSMeshBase* ApplyModifier(FSMeshBase* pm);

	// derived classes can return true if a nullptr return from Apply is allowed
	// (see e.g. FEDiscardMesh)
	virtual bool AllowNullMesh() { return false; }

	bool SetError(const char* szerr, ...);

	std::string GetErrorString();

protected:
	std::string	m_error;
};

//-----------------------------------------------------------------------------
class FEPartitionSelection : public FEModifier
{
public:
	FEPartitionSelection();
	FSMesh* Apply(FSMesh* pm) override;
	FSMesh* Apply(FSGroup* pg) override;

	bool UpdateData(bool bsave) override;
};

//-----------------------------------------------------------------------------
class FESmoothMesh : public FEModifier
{
public:
	FESmoothMesh();
	FSMesh* Apply(FSMesh* pm);

public:
	void SmoothMesh(FSMesh& mesh);
	void ShapeSmoothMesh(FSMesh& mesh, const FSMesh& backMesh);
};

//-----------------------------------------------------------------------------
class FERemoveDuplicateElements : public FEModifier
{
public:
	FERemoveDuplicateElements() : FEModifier("Remove duplicates"){}
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------

class FEMirrorMesh : public FEModifier
{
public:
	FEMirrorMesh();

	bool UpdateData(bool bsave) override;

	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
class FEFlattenFaces : public FEModifier
{
public:
	FEFlattenFaces() : FEModifier("Flatten faces")
	{
		m_rad = 1;
		m_na = vec3d(1,0,0);
		m_bun = false;
	}

	FSMesh* Apply(FSMesh* pm);

public:
	double	m_rad;
	vec3d	m_na;
	bool	m_bun;	// use user normal
};

//-----------------------------------------------------------------------------
class FEAlignNodes : public FEModifier
{
public:
	FEAlignNodes();
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
class FEProjectNodes : public FEModifier
{
public:
    FEProjectNodes();
    FSMesh* Apply(FSMesh* pm) override;
	FSMesh* Apply(GObject* po, FESelection* pg) override;

private:
	FSMesh* ProjectToSurface(GObject* po, GFace* pg);
	FSMesh* ProjectToUserPlane(GObject* po);

	bool UpdateData(bool bsave) override;

private:
	int m_method = 0;
};

class FEExtrudeToSurface : public FEModifier
{
public:
	FEExtrudeToSurface();
	FSMesh* Apply(GObject* po, FESelection* pg) override;
};

//-----------------------------------------------------------------------------

class FESetShellThickness : public FEModifier
{
public:
	FESetShellThickness();

	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------

class FESetFiberOrientation : public FEModifier
{
public:
	FESetFiberOrientation();

	FSMesh* Apply(FSMesh* pm);

	bool UpdateData(bool bsave);

protected:
	void SetFiberVector(FSMesh* pm);
	void SetFiberNodes (FSMesh* pm);
};

//-----------------------------------------------------------------------------

class FESetAxesOrientation : public FEModifier
{
public:
	FESetAxesOrientation();
	
	FSMesh* Apply(FSMesh* pm);

	bool UpdateData(bool bsave);
	
protected:
	bool SetAxesVectors(FSMesh* pm);
	bool SetAxesNodes (FSMesh* pm);
    bool SetAxesAngles(FSMesh* pm);
	bool SetAxesCopy  (FSMesh* pm);
	bool SetAxesCylindrical(FSMesh* pm);
	bool ClearAxes(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a quad mesh to a tri-mesh
//
class FEQuad2Tri : public FEModifier
{
public:
	FEQuad2Tri() : FEModifier("Quad2Tri"){}
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a tri3 mesh to a quad mesh
//
class FETri2Quad : public FEModifier
{
public:
	FETri2Quad() : FEModifier("Tri2Quad") {}
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a hex mesh into a tet-mesh
//
class FEHex2Tet : public FEModifier
{
public:
	FEHex2Tet() : FEModifier("Hex2Tet"){}
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// helper class for smoothing a tet10 mesh
class FETet10Smooth
{
public:
	void Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a tet4 mesh into a tet5 mesh
//
class FETet4ToTet5 : public FEModifier
{
public:
	FETet4ToTet5();
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a tet5 mesh into a tet4 mesh
//
class FETet5ToTet4 : public FEModifier
{
public:
	FETet5ToTet4();
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a tet4 mesh into a tet10 mesh
//
class FETet4ToTet10 : public FEModifier
{
public:
	FETet4ToTet10(bool bsmooth = false);
	FSMesh* Apply(FSMesh* pm);

	void SetSmoothing(bool b) { m_bsmooth = b; }
private:
	bool	m_bsmooth;
};

//-----------------------------------------------------------------------------
// Convert a tet4 mesh into a tet15 mesh
//
class FETet4ToTet15 : public FEModifier
{
public:
	FETet4ToTet15(bool bsmooth = false);
	FSMesh* Apply(FSMesh* pm);

	void SetSmoothing(bool b) { m_bsmooth = b; }
private:
	bool	m_bsmooth;
};

//-----------------------------------------------------------------------------
// Convert a tet4 mesh into a tet20 mesh
//
class FETet4ToTet20 : public FEModifier
{
public:
	FETet4ToTet20();
	FSMesh* Apply(FSMesh* pm);

	void SetSmoothing(bool b) { m_bsmooth = b; }
private:
	bool	m_bsmooth;
};

//-----------------------------------------------------------------------------
// Convert a tet4 mesh into a hex8 mesh
//
class FETet4ToHex8 : public FEModifier
{
public:
	FETet4ToHex8(bool bsmooth = false);
	FSMesh* Apply(FSMesh* pm);

	void SetSmoothing(bool b) { m_bsmooth = b; }
private:
	bool	m_bsmooth;
};

//-----------------------------------------------------------------------------
// Convert a tet10 mesh into a tet4 mesh
//
class FETet10ToTet4 : public FEModifier
{
public:
	FETet10ToTet4() : FEModifier("Tet10-to-Tet4"){}
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a tet15 mesh into a tet4 mesh
//
class FETet15ToTet4 : public FEModifier
{
public:
	FETet15ToTet4() : FEModifier("Tet15-to-Tet4"){}
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// helper class for smoothing a hex20 mesh
class FEHex20Smooth
{
public:
	void Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a hex8 mesh to a hex20 mesh
class FEHex8ToHex20 : public FEModifier
{
public:
	FEHex8ToHex20(bool bsmooth = false);
	FSMesh* Apply(FSMesh* pm);
	void SetSmoothing(bool b) { m_bsmooth = b; }

protected:
	bool m_bsmooth;
};

//-----------------------------------------------------------------------------
// Convert a hex20 mesh to a hex8 mesh
class FEHex20ToHex8 : public FEModifier
{
public:
	FEHex20ToHex8() : FEModifier("Hex20-to-Hex8"){}
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a penta6 mesh into a tet4 mesh
//
class FEPenta6ToTet4 : public FEModifier
{
public:
    FEPenta6ToTet4() : FEModifier("Penta6-to-Tet4"){}
    FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a penta6 mesh into a penta15 mesh
//
class FEPenta6ToPenta15 : public FEModifier
{
public:
	FEPenta6ToPenta15() : FEModifier("Penta6-to-Penta15") {}
	FSMesh* Apply(FSMesh* pm);
};


//-----------------------------------------------------------------------------
// helper class for smoothing a quad8 mesh
class FEQuad8Smooth
{
public:
    void Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a quad4 mesh to a quad8 mesh
class FEQuad4ToQuad8 : public FEModifier
{
public:
    FEQuad4ToQuad8(bool bsmooth = false);
    FSMesh* Apply(FSMesh* pm);
    void SetSmoothing(bool b) { m_bsmooth = b; }
    
protected:
    bool m_bsmooth;
};

//-----------------------------------------------------------------------------
// Convert a quad8 mesh to a quad4 mesh
class FEQuad8ToQuad4 : public FEModifier
{
public:
    FEQuad8ToQuad4() : FEModifier("Quad8-to-Quad4"){}
    FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// helper class for smoothing a tri6 mesh
class FETri6Smooth
{
public:
    void Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a quad4 mesh to a quad8 mesh
class FETri3ToTri6 : public FEModifier
{
public:
    FETri3ToTri6(bool bsmooth = false);
    FSMesh* Apply(FSMesh* pm);
    void SetSmoothing(bool b) { m_bsmooth = b; }
    
protected:
    bool m_bsmooth;
};

//-----------------------------------------------------------------------------
// Convert a quad8 mesh to a quad4 mesh
class FETri6ToTri3 : public FEModifier
{
public:
    FETri6ToTri3() : FEModifier("Tri6-to-Tri3"){}
    FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
// Split a mesh in two using a planar cut
class FEPlaneCut : public FEModifier
{
	struct EDGE
	{
		int n0, n1;	// node indices for edges
		double w;	// weight of edge
	};

	struct EDGELIST
	{
		int n[3];	// the three element edges
	};

public:
	FEPlaneCut();
	FSMesh* Apply(FSMesh* pm);

	void SetPlaneCoefficients(double a[4]);

private:
	double	m_a[4];	//!< plane coefficients
};

//-----------------------------------------------------------------------------
class RefineMesh : public FEModifier
{
public:
	RefineMesh();

	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
class FEConvertMesh : public FEModifier
{
public:
	FEConvertMesh();

	FSMesh* Apply(FSMesh* pm);

	bool UpdateData(bool bsave) override;

	// return progress
	FSTaskProgress GetProgress() override;

private:
	FEModifier* m_mod;
	int			m_currentType;
};

//-----------------------------------------------------------------------------
class FEAddNode : public FEModifier
{
public:
	FEAddNode();

	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
class FEAddTriangle : public FEModifier
{
public:
	FEAddTriangle();

	FSMesh* Apply(FSMesh* pm);

	void push_stack();
	void pop_stack();

private:
	std::vector<int> m_stack;
};

//-----------------------------------------------------------------------------
class FEInvertMesh : public FEModifier
{
public:
	FEInvertMesh();
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
class FEDetachElements : public FEModifier
{
public:
	FEDetachElements();
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
class FERezoneMesh : public FEModifier
{
public:
	FERezoneMesh();
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
class FEInflateMesh: public FEModifier
{
public:
	FEInflateMesh();
	FSMesh* Apply(FSMesh* pm);
};

//-----------------------------------------------------------------------------
class FEDeleteElements : public FEModifier
{
public:
	FEDeleteElements();
	FSMesh* Apply(FSMesh* pm);
};
