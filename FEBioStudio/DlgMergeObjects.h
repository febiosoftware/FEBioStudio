#pragma once
#include <QDialog>

namespace Ui {
	class CDlgMergeObjects;
};

class CDlgMergeObjects : public QDialog
{
public:
	CDlgMergeObjects(QWidget* parent);

	void accept();

public:
	std::string	m_name;
	bool		m_weld;
	double		m_tol;

private:
	Ui::CDlgMergeObjects* ui;
};
