#pragma once
#include <FSCore/FSObject.h>

class GPart;

class GPartSection : public FSObject
{
public:
	GPartSection(GPart*);

	const GPart* GetPart() const;
	GPart* GetPart();

	virtual GPartSection* Copy() = 0;

private:
	GPart*	m_part;
};

class FESolidFormulation;
class FEShellFormulation;
class FEBeamFormulation;

//=============================================================================
class GSolidSection : public GPartSection
{
public:
	GSolidSection(GPart* pg);
	~GSolidSection();
	GSolidSection* Copy() override;

	void SetElementFormulation(FESolidFormulation* form);
	FESolidFormulation* GetElementFormulation();

	bool UpdateData(bool bsave) override;

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	FESolidFormulation* m_form;
};

//=============================================================================
class GShellSection : public GPartSection
{
public:
	GShellSection(GPart* pg);
	~GShellSection();
	GShellSection* Copy() override;

	void SetElementFormulation(FEShellFormulation* form);
	FEShellFormulation* GetElementFormulation();

	void SetShellThickness(double h);
	double shellThickness() const;

	bool UpdateData(bool bsave) override;

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	FEShellFormulation* m_form;
};

//=============================================================================
class GBeamSection : public GPartSection
{
public:
	GBeamSection(GPart* pg);
	~GBeamSection();
	GBeamSection* Copy() override;

	void SetElementFormulation(FEBeamFormulation* form);
	FEBeamFormulation* GetElementFormulation();

	bool UpdateData(bool bsave) override;

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	FEBeamFormulation* m_form;
};
