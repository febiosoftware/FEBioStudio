#pragma once
#include "FEModelComponent.h"
#include <FSCore/LoadCurve.h>

enum LoadControllerType
{
	FE_FEBIO_LOAD_CONTROLLER = 1
};

class FSLoadController : public FSModelComponent
{
public:
	FSLoadController(FSModel* fem, int ntype);

	int Type() const;

public:
	int GetID() const;
	void SetID(int nid);

	static void ResetCounter();
	static void SetCounter(int n);
	static int GetCounter();

	// helper function for converting FEBio "loadcurve" to a LoadCurve
	virtual LoadCurve* CreateLoadCurve();

private:
	int	m_ntype;

private:
	int			m_nUID;	//!< unique ID
	static	int	m_nref;
};

class FEBioLoadController : public FSLoadController
{
public:
	FEBioLoadController(FSModel* fem = nullptr);
	~FEBioLoadController();

	LoadCurve* CreateLoadCurve() override;

	bool UpdateData(bool bsave) override;

private:
	LoadCurve* m_plc;
};

//==================================================================================================
enum FSFunction1DType
{
	FE_FEBIO_FUNCTION1D= 1
};

class FSFunction1D : public FSModelComponent
{
public:
	FSFunction1D(FSModel* fem, int ntype);

	int Type() const;

public:
	int GetID() const;
	void SetID(int nid);

	static void ResetCounter();
	static void SetCounter(int n);
	static int GetCounter();

	// helper function for converting FEBio "loadcurve" to a LoadCurve
	virtual LoadCurve* CreateLoadCurve();

private:
	int	m_ntype;

private:
	int			m_nUID;	//!< unique ID
	static	int	m_nref;
};

class FEBioFunction1D : public FSFunction1D
{
public:
	FEBioFunction1D(FSModel* fem = nullptr);
	~FEBioFunction1D();

	LoadCurve* CreateLoadCurve() override;

	bool UpdateData(bool bsave) override;

private:
	LoadCurve* m_plc;
};
