#pragma once
#include <QDialog>
#include <vector>

namespace Ui {
	class CDlgFind;
};

class CDlgFind : public QDialog
{
public:
	CDlgFind(QWidget* parent, int nsel = 0);

public:
	std::vector<int>	m_item;	// item list
	bool	m_bsel[4];
	bool	m_bclear;	// clear current selection

	void accept();

private:
	Ui::CDlgFind* ui;
};
