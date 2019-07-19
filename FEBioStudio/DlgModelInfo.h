#pragma once
#include <QDialog>

namespace Ui
{
	class CDlgModelInfo;
}

class CDlgModelInfo : public QDialog
{
	Q_OBJECT

public:
	CDlgModelInfo(QWidget* parent);

	QString text();
	void setText(const QString& s);

private:
	Ui::CDlgModelInfo*	ui;
};
