#pragma once
#include <QDialog>

class QRadioButton;

class CDlgImportSTL : public QDialog
{
public:
	CDlgImportSTL(QWidget* parent);

	void accept();

public:
	int	m_nselect;

private:
	QRadioButton*	m_sel[2];
};
