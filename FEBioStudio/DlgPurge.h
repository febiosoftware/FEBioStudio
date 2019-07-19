#pragma once
#include <QDialog>

class QRadioButton;

class CDlgPurge : public QDialog
{
public:
	CDlgPurge(QWidget* parent);

	int getOption() const { return m_option; }

private:
	void accept();

private:
	int		m_option;
	QRadioButton*	m_b[2];
};

