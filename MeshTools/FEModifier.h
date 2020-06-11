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
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------

class FEModifier : public FSObject 
{
public:
	FEModifier(const char* sz) { SetName(sz); }
	virtual ~FEModifier() {}

	virtual FEMesh* Apply(FEMesh* pm) = 0;

	virtual FEMesh* Apply(FEGroup* pg) { return (pg ? Apply(pg->GetMesh()) : 0); }

	virtual FEMeshBase* ApplyModifier(FEMeshBase* pm) { return 0; }

	static bool SetError(const char* szerr, ...);

	static std::string GetErrorString();

protected:
	static std::string	m_error;
};

//-----------------------------------------------------------------------------
class FEPartitionSelection : public FEModifier
{
public:
	FEPartitionSelection();
	FEMesh* Apply(FEMesh* pm);	
	FEMesh* Apply(FEGroup* pg);

	bool UpdateData(bool bsave) override;
};

//-----------------------------------------------------------------------------
class FESmoothMesh : public FEModifier
{
public:
	FESmoothMesh();
	FEMesh* Apply(FEMesh* pm);

public:
	void SmoothMesh(FEMesh& mesh);
	void ShapeSmoothMesh(FEMesh& mesh, const FEMesh& backMesh);
};

//-----------------------------------------------------------------------------
class FERemoveDuplicateElements : public FEModifier
{
public:
	FERemoveDuplicateElements() : FEModifier("Remove duplicates"){}
	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------

class FEMirrorMesh : public FEModifier
{
public:
	FEMirrorMesh();

	FEMesh* Apply(FEMesh* pm);
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

	FEMesh* Apply(FEMesh* pm);

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
	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------

class FESetShellThickness : public FEModifier
{
public:
	FESetShellThickness();

	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------

class FESetFiberOrientation : public FEModifier
{
public:
	FESetFiberOrientation();

	FEMesh* Apply(FEMesh* pm);

	bool UpdateData(bool bsave);

protected:
	void SetFiberVector(FEMesh* pm);
	void SetFiberNodes (FEMesh* pm);
};

//-----------------------------------------------------------------------------

class FESetAxesOrientation : public FEModifier
{
public:
	FESetAxesOrientation();
	
	FEMesh* Apply(FEMesh* pm);

	bool UpdateData(bool bsave);
	
protected:
	bool SetAxesVectors(FEMesh* pm);
	bool SetAxesNodes (FEMesh* pm);
    bool SetAxesAngles(FEMesh* pm);
	bool SetAxesCopy  (FEMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a quad mesh to a tri-mesh
//
class FEQuad2Tri : public FEModifier
{
public:
	FEQuad2Tri() : FEModifier("Quad2Tri"){}
	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a hex mesh into a tet-mesh
//
class FEHex2Tet : public FEModifier
{
public:
	FEHex2Tet() : FEModifier("Hex2Tet"){}
	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// helper class for smoothing a tet10 mesh
class FETet10Smooth
{
public:
	void Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a tet4 mesh into a tet5 mesh
//
class FETet4ToTet5 : public FEModifier
{
public:
	FETet4ToTet5();
	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a tet5 mesh into a tet4 mesh
//
class FETet5ToTet4 : public FEModifier
{
public:
	FETet5ToTet4();
	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a tet4 mesh into a tet10 mesh
//
class FETet4ToTet10 : public FEModifier
{
public:
	FETet4ToTet10();
	FEMesh* Apply(FEMesh* pm);

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
	FETet4ToTet15();
	FEMesh* Apply(FEMesh* pm);

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
	FEMesh* Apply(FEMesh* pm);

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
	FETet4ToHex8();
	FEMesh* Apply(FEMesh* pm);

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
	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a tet15 mesh into a tet4 mesh
//
class FETet15ToTet4 : public FEModifier
{
public:
	FETet15ToTet4() : FEModifier("Tet15-to-Tet4"){}
	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// helper class for smoothing a hex20 mesh
class FEHex20Smooth
{
public:
	void Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a hex8 mesh to a hex20 mesh
class FEHex8ToHex20 : public FEModifier
{
public:
	FEHex8ToHex20();
	FEMesh* Apply(FEMesh* pm);
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
	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// helper class for smoothing a quad8 mesh
class FEQuad8Smooth
{
public:
    void Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a quad4 mesh to a quad8 mesh
class FEQuad4ToQuad8 : public FEModifier
{
public:
    FEQuad4ToQuad8();
    FEMesh* Apply(FEMesh* pm);
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
    FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// helper class for smoothing a tri6 mesh
class FETri6Smooth
{
public:
    void Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// Convert a quad4 mesh to a quad8 mesh
class FETri3ToTri6 : public FEModifier
{
public:
    FETri3ToTri6();
    FEMesh* Apply(FEMesh* pm);
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
    FEMesh* Apply(FEMesh* pm);
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
	FEMesh* Apply(FEMesh* pm);

	void SetPlaneCoefficients(double a[4]);

private:
	double	m_a[4];	//!< plane coefficients
};

//-----------------------------------------------------------------------------
class FERefineMesh : public FEModifier
{
public:
	FERefineMesh();

	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
class FEConvertMesh : public FEModifier
{
public:
	FEConvertMesh();

	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
class FEAddNode : public FEModifier
{
public:
	FEAddNode();

	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
class FEInvertMesh : public FEModifier
{
public:
	FEInvertMesh();
	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
class FEDetachElements : public FEModifier
{
public:
	FEDetachElements();
	FEMesh* Apply(FEMesh* pm);
};
