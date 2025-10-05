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

//! The GPrimitive class manages the parts, surfaces and nodesets automatically
//! The user can not alter this data. This class assumes that all parts, surfaces,
//! and nodesets are created by whatever class created the mesh. For example, since
//! most procedural meshes automatically create their own partitions, they use
//! auto-objects to manager their geometry. This is contrast for instance with the
//! the GMeshObject which uses the FE mesh to define the geometry.
class GPrimitive : public GObject
{
public:
	//! Constructor
	GPrimitive(int ntype) : GObject(ntype) {}

	//! Get the editable mesh
	FSMeshBase* GetEditableMesh() override;
	//! Get the editable line mesh
	FSLineMesh* GetEditableLineMesh() override;

	//! Clone the object
	GObject* Clone() override;
};

//! Use this base class for shell primitives
class GShellPrimitive : public GPrimitive
{
public:
	//! Constructor
	GShellPrimitive(int ntype) : GPrimitive(ntype) {}
};

//! Simple rectangular box
class GBox : public GPrimitive
{
public:
	//! Parameter enumeration
	enum { WIDTH, HEIGHT, DEPTH };
	//! Width of the box
	double	m_w;
	//! Height of the box
	double	m_h;
	//! Depth of the box
	double	m_d;

public:
	//! Default constructor
	GBox();
	//! Constructor with dimensions
	GBox(double W, double H, double D);
	//! Update the object
	bool Update(bool b = true) override;

private:
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
	//! Create the geometry
	void Create() override;
};

//! Tapered cone
class GCone : public GPrimitive
{
public:
	//! Parameter enumeration
	enum {R0, R1, H};
	//! Bottom radius
	double	m_R0;
	//! Top radius
	double	m_R1;
	//! Height
	double	m_h;

public:
	//! Default constructor
	GCone();
	//! Update the object
	bool Update(bool b = true) override;

	//! Get the bottom radius
	double BottomRadius() const;
	//! Get the top radius
	double TopRadius() const;
	//! Get the height
	double Height() const;

	//! Set the bottom radius
	void SetBottomRadius(double r0);
	//! Set the top radius
	void SetTopRadius(double r1);
	//! Set the height
	void SetHeight(double h);

private:
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
	//! Create the geometry
	void Create() override;
	//! Build the geometry mesh
	void BuildGMesh() override;
};

//! Circular cylinder
class GCylinder : public GPrimitive
{
public:
	//! Parameter enumeration
	enum {RADIUS, HEIGHT};
	//! Radius of the cylinder
	double	m_R;
	//! Height of the cylinder
	double	m_h;

public:
	//! Default constructor
	GCylinder();
	//! Update the object
	bool Update(bool b = true) override;

	//! Get the radius
	double Radius() const;
	//! Get the height
	double Height() const;

	//! Set the radius
	void SetRadius(double R);
	//! Set the height
	void SetHeight(double H);

private:
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
	//! Create the geometry
	void Create() override;
};

//! Ellipsoidal cylinder
class GCylinder2 : public GPrimitive
{
public:
	//! Parameter enumeration
	enum {RADIUSX, RADIUSY, HEIGHT};
	//! X-axis radius
	double	m_Rx;
	//! Y-axis radius
	double	m_Ry;
	//! Height of the cylinder
	double	m_h;

public:
	//! Default constructor
	GCylinder2();
	//! Update the object
	bool Update(bool b = true) override;

private:
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
	//! Create the geometry
	void Create() override;
};

//! Hollow concentric sphere
class GHollowSphere: public GPrimitive
{
public:
	//! Parameter enumeration
	enum {RIN, ROUT};
	//! Inner radius
	double	m_Ri;
	//! Outer radius
	double	m_Ro;

public:
	//! Default constructor
	GHollowSphere();
	//! Update the object
	bool Update(bool b = true) override;

private:
	//! Create the geometry
	void Create() override;
	//! Build the geometry mesh
	void BuildGMesh() override;
	//! Get node index
	int NodeIndex(int i, int j, int ND, int NZ);
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! Hollow truncated concentric ellipsoid
class GTruncatedEllipsoid: public GPrimitive
{
public:
	//! Parameter enumeration
	enum {RA, RB, RC, WT, VEND};
	//! A-axis radius
	double	m_Ra;
	//! B-axis radius
	double	m_Rb;
	//! C-axis radius
	double	m_Rc;
	//! Wall thickness
	double	m_wt;
	//! End angle
	double	m_vend;

public:
	//! Default constructor
	GTruncatedEllipsoid();
	//! Update the object
	bool Update(bool b = true) override;

private:
	//! Create the geometry
	void Create() override;
	//! Build the geometry mesh
	void BuildGMesh() override;
	//! Get node index
	int NodeIndex(int i, int j, int NS);
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! Sphere
class GSphere : public GPrimitive
{
public:
	//! Parameter enumeration
	enum {RADIUS};
	//! Radius of the sphere
	double	m_R;

public:
	//! Default constructor
	GSphere();
	//! Update the object
	bool Update(bool b = true) override;

	//! Get the radius
	double Radius() const;
	//! Set the radius
	void SetRadius(double R);

private:
	//! Create the geometry
	void Create() override;
	//! Build the geometry mesh
	void BuildGMesh() override;
	//! Get node index
	int NodeIndex(int i, int j, int ND, int NZ);
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! Circular torus
class GTorus : public GPrimitive
{
public:
	//! Parameter enumeration
	enum {RIN, ROUT};
	//! Inner radius
	double	m_R0;
	//! Outer radius
	double	m_R1;

public:
	//! Default constructor
	GTorus();
	//! Update the object
	bool Update(bool b = true) override;

private:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! Box with a cylindrical cavity
class GCylinderInBox : public GPrimitive
{
public:
	//! Parameter enumeration
	enum {WIDTH, HEIGHT, DEPTH, RADIUS};
	//! Width of the box
	double	m_W;
	//! Height of the box
	double	m_H;
	//! Depth of the box
	double	m_D;
	//! Radius of the cylindrical cavity
	double	m_R;

public:
	//! Default constructor
	GCylinderInBox();
	//! Update the object
	bool Update(bool b = true) override;

private:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! Box with a spherical cavity
class GSphereInBox : public GPrimitive
{
public:
	//! Parameter enumeration
	enum {WIDTH, HEIGHT, DEPTH, RADIUS};

public:
	//! Default constructor
	GSphereInBox();
	//! Update the object
	bool Update(bool b = true) override;

private:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
	//! Build the geometry mesh
	void BuildGMesh() override;
};

//! Tube, that is a hollow cylinder
class GTube : public GPrimitive
{
public:
	//! Parameter enumeration
	enum {RIN, ROUT, HEIGHT};
	//! Inner radius
	double	m_Ri;
	//! Outer radius
	double	m_Ro;
	//! Height of the tube
	double	m_h;

public:
	//! Default constructor
	GTube();
	//! Update the object
	bool Update(bool b = true) override;

	//! Get the inner radius
	double InnerRadius() const;
	//! Get the outer radius
	double OuterRadius() const;
	//! Get the height
	double Height() const;

	//! Set the inner radius
	void SetInnerRadius(double Ri);
	//! Set the outer radius
	void SetOuterRadius(double Ro);
	//! Set the height
	void SetHeight(double h);

private:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! Elliptical Tube, that is a hollow cylinder
class GTube2 : public GPrimitive
{
public:
	//! Parameter enumeration
	enum {RINX, RINY, ROUTX, ROUTY, HEIGHT};
	//! Inner X-axis radius
	double	m_Rix;
	//! Inner Y-axis radius
	double	m_Riy;
	//! Outer X-axis radius
	double	m_Rox;
	//! Outer Y-axis radius
	double	m_Roy;
	//! Height of the tube
	double	m_h;

public:
	//! Default constructor
	GTube2();
	//! Update the object
	bool Update(bool b = true) override;

private:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! Cylindrical slice
class GSlice : public GPrimitive
{
public:
	//! Parameter enumeration
	enum {RADIUS, HEIGHT, ANGLE};
	//! Radius of the slice
	double	m_R;
	//! Height of the slice
	double	m_H;
	//! Angle of the slice
	double	m_w;

public:
	//! Default constructor
	GSlice();
	//! Update the object
	bool Update(bool b = true) override;

private:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! Quarter symmetry dog-bone
class GQuartDogBone : public GPrimitive
{
public:
	//! Parameter enumeration
	enum { CWIDTH, CHEIGHT, RADIUS, LENGTH, DEPTH, WING };

public:
	//! Default constructor
	GQuartDogBone();
	//! Update the object
	bool Update(bool b = true) override;

protected:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! A 3D Solid Arc 
class GSolidArc : public GPrimitive
{
public:
	//! Parameter enumeration
	enum { RIN, ROUT, HEIGHT, ARC };

public:
	//! Default constructor
	GSolidArc();
	//! Update the object
	bool Update(bool b = true) override;

protected:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! Hexagon primitive
class GHexagon : public GPrimitive
{
public:
	//! Parameter enumeration
	enum { RADIUS, HEIGHT };

public:
	//! Default constructor
	GHexagon();
	//! Update the object
	bool Update(bool b = true);

	//! Get the radius
	double Radius() const;
	//! Get the height
	double Height() const;

	//! Set the radius
	void SetRadius(double R);
	//! Set the height
	void SetHeight(double H);

protected:
	//! Create the geometry
	void Create();
};

//! 2D circular disc
class GDisc : public GShellPrimitive
{
public:
	//! Parameter enumeration
	enum {RADIUS};

public:
	//! Default constructor
	GDisc();
	//! Constructor with radius
	GDisc(double radius);

	//! Update the object
	bool Update(bool b = true) override;

	//! Get the radius
	double Radius() const;
	//! Set the radius
	void SetRadius(double R);

public:
	//! Create mesh with specified parameters
	FSMesh* CreateMesh(int ndiv, int nsegs, double ratio);

protected:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! 2D rectangular patch
class GPatch : public GShellPrimitive
{
public:
	//! Parameter enumeration
	enum {W, H};
	//! Width of the patch
	double	m_w;
	//! Height of the patch
	double	m_h;

public:
	//! Default constructor
	GPatch();
	//! Update the object
	bool Update(bool b = true) override;

private:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! 2D ring
class GRing : public GShellPrimitive
{
public:
	//! Parameter enumeration
	enum {RIN, ROUT};
	//! Inner radius
	double	m_Ri;
	//! Outer radius
	double	m_Ro;

public:
	//! Default constructor
	GRing();
	//! Update the object
	bool Update(bool b = true) override;

	//! Set the inner radius
	void SetInnerRadius(double ri);
	//! Set the outer radius
	void SetOuterRadius(double ro);

private:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! A shell tube (cylinder without capped ends)
class GThinTube  : public GShellPrimitive
{
public:
	//! Parameter enumeration
	enum {RAD, H};
	//! Radius of the tube
	double	m_R;
	//! Height of the tube
	double	m_h;

public:
	//! Default constructor
	GThinTube();
	//! Update the object
	bool Update(bool b = true) override;

	//! Get the radius
	double Radius() const;
	//! Get the height
	double Height() const;

	//! Set the radius
	void SetRadius(double r);
	//! Set the height
	void SetHeight(double h);

private:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! Cylindrical patch
class GCylindricalPatch : public GShellPrimitive
{
public:
	//! Parameter enumeration
	enum { W, H, R };

	//! Get the width
	double Width() const;
	//! Get the height
	double Height() const;
	//! Get the radius
	double Radius() const;

public:
	//! Default constructor
	GCylindricalPatch();
	//! Update the object
	bool Update(bool b = true) override;

private:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};

//! Gregory patch
class GGregoryPatch : public GShellPrimitive
{
public:
	//! Constructor with mesh
	GGregoryPatch(FSMesh* pm);

public:
	//! Update the mesh
	void UpdateMesh();
};

//! Box within a box
class GBoxInBox : public GPrimitive
{
public:
	//! Default constructor
	GBoxInBox();
	//! Update the object
	bool Update(bool b = true) override;

	//! Get the outer width
	double OuterWidth() const;
	//! Get the outer height
	double OuterHeight() const;
	//! Get the outer depth
	double OuterDepth() const;

	//! Get the inner width
	double InnerWidth() const;
	//! Get the inner height
	double InnerHeight() const;
	//! Get the inner depth
	double InnerDepth() const;

private:
	//! Create the geometry
	void Create() override;
	//! Create the default mesher
	FEMesher* CreateDefaultMesher() override;
};