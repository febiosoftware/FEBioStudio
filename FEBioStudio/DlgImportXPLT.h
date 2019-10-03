#pragma once
#include <QDialog>
#include <vector>

namespace Ui {
	class CDlgImportXPLT;
};

class CDlgImportXPLT : public QDialog
{
public:
	CDlgImportXPLT(QWidget* parent);

	void accept();

public:
	int					m_nop;
	std::vector<int>	m_item;

private:
	Ui::CDlgImportXPLT* ui;
};
