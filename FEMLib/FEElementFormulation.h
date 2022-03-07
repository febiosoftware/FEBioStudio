#pragma once
#include <FSCore/FSObject.h>

class FSModel;

class FEElementFormulation : public FSObject
{
public:
	FEElementFormulation() {}
};

class FESolidFormulation : public FEElementFormulation {};
class FEShellFormulation : public FEElementFormulation {};

class FEUT4Formulation : public FESolidFormulation
{
public:
	FEUT4Formulation(FSModel* fem);
};

class FEUDGHexFormulation : public FESolidFormulation
{
public:
	FEUDGHexFormulation(FSModel* fem);
};

class FEDefaultShellFormulation : public FEShellFormulation
{
public: 
	FEDefaultShellFormulation(FSModel* fem);

	void setShellNormalNodal(bool b);
	bool shellNormalNodal() const;

	void setLaugon(bool b);
	bool laugon() const;

	void setAugTol(double d);
	double augTol() const;
};
