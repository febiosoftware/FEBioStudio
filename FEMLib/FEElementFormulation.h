#pragma once
#include <FSCore/FSObject.h>

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
	FEUT4Formulation();
};

class FEUDGHexFormulation : public FESolidFormulation
{
public:
	FEUDGHexFormulation();
};

class FEDefaultShellFormulation : public FEShellFormulation
{
public: 
	FEDefaultShellFormulation();

	void setShellNormalNodal(bool b);
	bool shellNormalNodal() const;

	void setLaugon(bool b);
	bool laugon() const;

	void setAugTol(double d);
	double augTol() const;
};
