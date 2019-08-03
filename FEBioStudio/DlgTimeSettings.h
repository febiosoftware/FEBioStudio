#pragma once
#include <QDialog>

namespace Ui {
	class CDlgTimeSettings;
};

class CPostDoc;

class CDlgTimeSettings : public QDialog
{
public:
	CDlgTimeSettings(CPostDoc* doc, QWidget* parent);

	void accept();

private:
	Ui::CDlgTimeSettings* ui;
	CPostDoc*	m_doc;
};
