#pragma once
#include <QWidget>
#include <QString>

namespace Ui {
	class CExternalLinkEdit;
}

//-----------------------------------------------------------------------------
// This class can be used to access a resource (e.g. file)
class CExternalLinkEdit : public QWidget
{
	Q_OBJECT

public:
	CExternalLinkEdit(QStringList& paths, QWidget* parent = 0);

private slots:
	void buttonPressed();

private:
	Ui::CExternalLinkEdit*	ui;
};
