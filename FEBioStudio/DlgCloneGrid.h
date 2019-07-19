#pragma once
#include <QDialog>

namespace Ui {
	class CDlgCloneGrid;
}

class CDlgCloneGrid : public QDialog
{
	Q_OBJECT

public:
	CDlgCloneGrid(QWidget* parent);

	void accept();

public:
	int	m_rangeX[2];
	int	m_rangeY[2];
	int	m_rangeZ[2];
	double	m_inc[3];

protected:
	Ui::CDlgCloneGrid*	ui;
};
