#pragma once
#include <QDialog>
#include <MathLib/math3d.h>

namespace Ui {
	class CDlgCloneRevolve;
}

class CDlgCloneRevolve : public QDialog
{
	Q_OBJECT

public:
	CDlgCloneRevolve(QWidget* parent);

	void accept();

public:
	int		m_count;	    // number of copies
	double	m_range;	    // degrees
	double	m_spiral;	    // spiral distance
	vec3d	m_center;	    // rotation center
	vec3d	m_axis;		    // rotation axis
	bool	m_rotateClones;	// rotate the clones

protected:
	Ui::CDlgCloneRevolve*	ui;
};
