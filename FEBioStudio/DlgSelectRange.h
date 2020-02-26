#pragma once
#include <QDialog>

namespace Ui {
	class CDlgSelectRange;
};

class CDlgSelectRange : public QDialog
{
public:
	CDlgSelectRange(QWidget* parent);

public:
	double m_min, m_max;
	double m_brange;

	void accept();
	int exec();

private:
	Ui::CDlgSelectRange* ui;
};
