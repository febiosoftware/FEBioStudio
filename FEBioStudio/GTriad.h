#pragma once
#include <MathLib/math3d.h>
#include <QColor>

class GTriad
{
public:
	GTriad(void);
	~GTriad(void);

	void Render();

	void SetTextColor(const QColor& c) { m_tcol = c; }
	void SetOrientation(quatd q) { m_rot = q; }

protected:
	QColor	m_tcol;	// text color
	quatd	m_rot;	// triad orientation
};
