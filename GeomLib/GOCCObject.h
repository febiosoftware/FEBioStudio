#pragma once
#include "GObject.h"

//-----------------------------------------------------------------------
class OCC_Data;
class TopoDS_Shape;

//-----------------------------------------------------------------------
// Experimental class for representing OpenCascade objects
class GOCCObject : public GObject
{
public:
	GOCCObject(int type = GOCCOBJECT);

	// create the default mesher for this object
	FEMesher* CreateDefaultMesher() override;

	// build the mesh for visualization
	void BuildGMesh() override;

	// set the shape
	void SetShape(TopoDS_Shape& shape, bool update = true);

	// return the OCC shape object
	TopoDS_Shape& GetShape();

	FEMeshBase* GetEditableMesh() override;
	FELineMesh* GetEditableLineMesh() override;

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

protected:
	void BuildGObject();

private:
	OCC_Data*	m_occ;
};

//-----------------------------------------------------------------------
// A Bottle built with OCC
class GOCCBottle : public GOCCObject
{
public:
	enum { WIDTH, HEIGHT, THICKNESS };

	GOCCBottle();

	// update the object's geometry
	bool Update(bool b = true) override;

private:
	void MakeBottle(double h, double w, double t);
};

//-----------------------------------------------------------------------
// A Box built with OCC
class GOCCBox : public GOCCObject
{
public:
	GOCCBox();

	// update the object's geometry
	bool Update(bool b = true) override;

private:
	void MakeBox();
};
