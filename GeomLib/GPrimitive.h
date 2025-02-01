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
#include "GMultiBox.h"
#include <FSCore/ParamBlock.h>

class FSMesh;

//-----------------------------------------------------------------------------
// The GPrimitive class manages the parts, surfaces and nodesets automatically
// The user can not alter this data. This class assumes that all parts, surfaces,
// and nodesets are created by whatever class created the mesh. For example, since
// most procedural meshes automatically create their own partitions, they use
// auto-objects to manager their geometry. This is contrast for instance with the
// the GMeshObject which uses the FE mesh to define the geometry.

class GPrimitive : public GObject
{
public:
	GPrimitive(int ntype) : GObject(ntype) {}

	// get the editable mesh
	FSMeshBase* GetEditableMesh() override;
	FSLineMesh* GetEditableLineMesh() override;

	GObject* Clone() override;
};

// use this base class for shell primitives
class GShellPrimitive : public GPrimitive
{
public:
	GShellPrimitive(int ntype) : GPrimitive(ntype) {}
};

//-----------------------------------------------------------------------------
// Simple rectangular box
class GBox : public GPrimitive
{
public:
	enum { WIDTH, HEIGHT, DEPTH };
	double	m_w, m_h, m_d;

public:
	GBox();
	GBox(double W, double H, double D);
	bool Update(bool b = true) override;

private:
	FEMesher* CreateDefaultMesher() override;
	void Create() override;
};

//-----------------------------------------------------------------------------
// Tapered cone
class GCone : public GPrimitive
{
public:
	enum {R0, R1, H};
	double	m_R0, m_R1, m_h;

public:
	GCone();
	bool Update(bool b = true) override;

	double BottomRadius() const;
	double TopRadius() const;
	double Height() const;

	void SetBottomRadius(double r0);
	void SetTopRadius(double r1);
	void SetHeight(double h);

private:
	FEMesher* CreateDefaultMesher() override;
	void Create() override;
	void BuildGMesh() override;
};

//-----------------------------------------------------------------------------
// Circular cylinder
class GCylinder : public GPrimitive
{
public:
	enum {RADIUS, HEIGHT};
	double	m_R, m_h;

public:
	GCylinder();
	bool Update(bool b = true) override;

	double Radius() const;
	double Height() const;

	void SetRadius(double R);
	void SetHeight(double H);

private:
	FEMesher* CreateDefaultMesher() override;
	void Create() override;
};

//-----------------------------------------------------------------------------
// Ellipsoidal cylinder
class GCylinder2 : public GPrimitive
{
public:
	enum {RADIUSX, RADIUSY, HEIGHT};
	double	m_Rx, m_Ry, m_h;

public:
	GCylinder2();
	bool Update(bool b = true) override;

private:
	FEMesher* CreateDefaultMesher() override;
	void Create() override;
};

//-----------------------------------------------------------------------------
// hollow concentric sphere
class GHollowSphere: public GPrimitive
{
public:
	enum {RIN, ROUT};
	double	m_Ri, m_Ro;

public:
	GHollowSphere();
	bool Update(bool b = true) override;

private:
	void Create() override;
	void BuildGMesh() override;
	int NodeIndex(int i, int j, int ND, int NZ);
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// hollow truncated concentric ellipsoid
class GTruncatedEllipsoid: public GPrimitive
{
public:
	enum {RA, RB, RC, WT, VEND};
	double	m_Ra, m_Rb, m_Rc;	// ellipsoid radii
	double	m_wt;				// wall thickness
	double	m_vend;				// end angle

public:
	GTruncatedEllipsoid();
	bool Update(bool b = true) override;

private:
	void Create() override;
	void BuildGMesh() override;
	int NodeIndex(int i, int j, int NS);
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// sphere
class GSphere : public GPrimitive
{
public:
	enum {RADIUS};
	double	m_R;

public:
	GSphere();
	bool Update(bool b = true) override;

	double Radius() const;
	void SetRadius(double R);

private:
	void Create() override;
	void BuildGMesh() override;
	int NodeIndex(int i, int j, int ND, int NZ);
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// circular torus
class GTorus : public GPrimitive
{
public:
	enum {RIN, ROUT};
	double	m_R0, m_R1;

public:
	GTorus();
	bool Update(bool b = true) override;

private:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// box with a cylindrical cavity
class GCylinderInBox : public GPrimitive
{
public:
	enum {WIDTH, HEIGHT, DEPTH, RADIUS};
	double	m_W, m_H, m_D, m_R;

public:
	GCylinderInBox();
	bool Update(bool b = true) override;

private:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// box with a spherical cavity
class GSphereInBox : public GPrimitive
{
public:
	enum {WIDTH, HEIGHT, DEPTH, RADIUS};

public:
	GSphereInBox();
	bool Update(bool b = true) override;

private:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// Tube, that is a hollow cylinder
class GTube : public GPrimitive
{
public:
	enum {RIN, ROUT, HEIGHT};
	double	m_Ri, m_Ro, m_h;

public:
	GTube();
	bool Update(bool b = true) override;

	double InnerRadius() const;
	double OuterRadius() const;
	double Height() const;

	void SetInnerRadius(double Ri);
	void SetOuterRadius(double Ro);
	void SetHeight(double h);

private:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// Elliptical Tube, that is a hollow cylinder
class GTube2 : public GPrimitive
{
public:
	enum {RINX, RINY, ROUTX, ROUTY, HEIGHT};
	double	m_Rix, m_Riy, m_Rox, m_Roy, m_h;

public:
	GTube2();
	bool Update(bool b = true) override;

private:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// Cylindrical slice
class GSlice : public GPrimitive
{
public:
	enum {RADIUS, HEIGHT, ANGLE};
	double	m_R, m_H, m_w;

public:
	GSlice();
	bool Update(bool b = true) override;

private:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
//! Quarter symmetry dog-bone
class GQuartDogBone : public GPrimitive
{
public:
	enum { CWIDTH, CHEIGHT, RADIUS, LENGTH, DEPTH, WING };

public:
	GQuartDogBone();
	bool Update(bool b = true) override;

protected:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
//! a 3D Solid Arc 
class GSolidArc : public GPrimitive
{
public:
	enum { RIN, ROUT, HEIGHT, ARC };

public:
	GSolidArc();
	bool Update(bool b = true) override;

protected:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
class GHexagon : public GPrimitive
{
public:
	enum { RADIUS, HEIGHT };

public:
	GHexagon();
	bool Update(bool b = true);

	double Radius() const;
	double Height() const;

	void SetRadius(double R);
	void SetHeight(double H);

protected:
	void Create();
};

//-----------------------------------------------------------------------------
// 2D circular disc
class GDisc : public GShellPrimitive
{
public:
	enum {RADIUS};

public:
	GDisc();
	GDisc(double radius);

	bool Update(bool b = true) override;

	double Radius() const;
	void SetRadius(double R);

public:
	FSMesh* CreateMesh(int ndiv, int nsegs, double ratio);

protected:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// 2D rectangular patch
class GPatch : public GShellPrimitive
{
public:
	enum {W, H};
	double	m_w, m_h;

public:
	GPatch();
	bool Update(bool b = true) override;

private:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// 2D ring
class GRing : public GShellPrimitive
{
public:
	enum {RIN, ROUT};
	double	m_Ri, m_Ro;

public:
	GRing();
	bool Update(bool b = true) override;

	void SetInnerRadius(double ri);
	void SetOuterRadius(double ro);

private:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// a shell tube (cylinder without capped ends)
class GThinTube  : public GShellPrimitive
{
public:
	enum {RAD, H};
	double	m_R, m_h;

public:
	GThinTube();
	bool Update(bool b = true) override;

	double Radius() const;
	double Height() const;

	void SetRadius(double r);
	void SetHeight(double h);

private:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
class GCylindricalPatch : public GShellPrimitive
{
public:
	enum { W, H, R };

	double Width() const;
	double Height() const;
	double Radius() const;

public:
	GCylindricalPatch();
	bool Update(bool b = true) override;

private:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};

//-----------------------------------------------------------------------------
// Gregory patch
class GGregoryPatch : public GShellPrimitive
{
public:
	GGregoryPatch(FSMesh* pm);

public:
	void UpdateMesh();
};

//-----------------------------------------------------------------------------
class GBoxInBox : public GPrimitive
{
public:
	GBoxInBox();
	bool Update(bool b = true) override;

	double OuterWidth() const;
	double OuterHeight() const;
	double OuterDepth() const;

	double InnerWidth() const;
	double InnerHeight() const;
	double InnerDepth() const;

private:
	void Create() override;
	FEMesher* CreateDefaultMesher() override;
};
