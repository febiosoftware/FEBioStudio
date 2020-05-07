#pragma once
#include <QWidget>
#include <QString>

namespace Ui {
	class CLinkPropertyEdit;
}

//-----------------------------------------------------------------------------
// This class can be used to access a resource (e.g. file)
class CLinkPropertyEdit : public QWidget
{
	Q_OBJECT

public:
	CLinkPropertyEdit(QStringList& paths, bool internal=false, QWidget* parent = 0);

private slots:
	void buttonPressed();

private:
	Ui::CLinkPropertyEdit*	ui;
};
