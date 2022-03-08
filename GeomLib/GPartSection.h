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
	GPart* m_part;
};

class FESolidFormulation;
class FEShellFormulation;

class GSolidSection : public GPartSection
{
public:
	GSolidSection(GPart* pg);
	~GSolidSection();
	GSolidSection* Copy() override;

	void SetElementFormulation(FESolidFormulation* form);
	FESolidFormulation* GetElementFormulation();

	bool UpdateData(bool bsave) override;

private:
	FESolidFormulation* m_form;
};

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

private:
	FEShellFormulation* m_form;
};
