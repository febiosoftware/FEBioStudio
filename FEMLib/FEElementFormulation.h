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

class FENewShellFormulation : public FEShellFormulation
{
public: 
	FENewShellFormulation(FSModel* fem);

	void setShellNormalNodal(bool b);
	bool shellNormalNodal() const;
};

class FEElasticEASShellFormulation : public FEShellFormulation
{
public:
	FEElasticEASShellFormulation(FSModel* fem);
};

class FEElasticANSShellFormulation : public FEShellFormulation
{
public:
	FEElasticANSShellFormulation(FSModel* fem);
};

class FE3FieldShellFormulation : public FEShellFormulation
{
public:
	FE3FieldShellFormulation(FSModel* fem);

public:
	void setShellNormalNodal(bool b);
	bool shellNormalNodal() const;

	void setLaugon(bool b);
	bool laugon() const;

	void setAugTol(double d);
	double augTol() const;
};

class FEOldShellFormulation : public FEShellFormulation
{
public:
	FEOldShellFormulation(FSModel* fem);
};
