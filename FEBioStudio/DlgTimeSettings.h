#pragma once
#include <QDialog>

namespace Ui {
	class CDlgTimeSettings;
};

class CPostDocument;

class CDlgTimeSettings : public QDialog
{
public:
	CDlgTimeSettings(CPostDocument* doc, QWidget* parent);

	void accept();

private:
	Ui::CDlgTimeSettings* ui;
	CPostDocument*	m_doc;
};
