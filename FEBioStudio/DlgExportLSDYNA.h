#pragma once
#include <QDialog>

class CDlgExportLSDYNA_UI;

class CDlgExportLSDYNA : public QDialog
{
public:
	CDlgExportLSDYNA(QWidget* parent);

	void accept();

public:
	bool	m_bsel;
	bool	m_bsurf;
	bool	m_bnode;

private:
	CDlgExportLSDYNA_UI*	ui;
};

class CDlgExportLSDYNAPlot_UI;

namespace Post {
	class FEModel;
}

class CDlgExportLSDYNAPlot : public QDialog
{
public:
	CDlgExportLSDYNAPlot(Post::FEModel* fem, QWidget* parent);

	void accept();

public:
	bool	m_flag[6];
	int		m_code[6];

private:
	CDlgExportLSDYNAPlot_UI*	ui;
};
