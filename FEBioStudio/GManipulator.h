#pragma once

// pivot selction mode
#define PIVOT_NONE	0
#define PIVOT_X		1
#define PIVOT_Y		2
#define PIVOT_Z		3
#define PIVOT_XY	4
#define PIVOT_YZ	5
#define PIVOT_XZ	6

class CGLView;

//-----------------------------------------------------------------------------
class GManipulator
{
public:
	GManipulator(CGLView* view);
	virtual ~GManipulator(void);

	void SetScale(double s) { m_scale = s; }

	virtual void Render(int npivot, bool bactive) = 0;

	virtual int Pick(int x, int y) = 0;

protected:
	double	m_scale;
	CGLView* m_view;
};

//-----------------------------------------------------------------------------

class GTranslator : public GManipulator
{
public:
	GTranslator(CGLView* view) : GManipulator(view) {}

	void Render(int npivot, bool bactive);

	int Pick(int x, int y);
};

//-----------------------------------------------------------------------------

class GRotator : public GManipulator
{
public:
	GRotator(CGLView* view) : GManipulator(view) {}

	void Render(int npivot, bool bactive);

	int Pick(int x, int y);
};

//-----------------------------------------------------------------------------

class GScalor : public GManipulator
{
public:
	GScalor(CGLView* view) : GManipulator(view) {}

	void Render(int npivot, bool bactive);

	int Pick(int x, int y);
};
