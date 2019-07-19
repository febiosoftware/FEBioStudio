#pragma once
#include <QDialog>
#include <MathLib/math3d.h>

namespace Ui {
	class CDlgTransform;
};

class CDlgTransform : public QDialog
{
public:
	CDlgTransform(QWidget* parent);

	void Init();

	void accept();

public:
	vec3d	m_pos, m_relPos;
	vec3d	m_rot, m_relRot;
	vec3d	m_scl, m_relScl;	

private:
	Ui::CDlgTransform* ui;
};
