#pragma once
#include <QWidget>

namespace Ui {
	class CResourceEdit;
}

//-----------------------------------------------------------------------------
// This class can be used to access a resource (e.g. file)
class CResourceEdit : public QWidget
{
	Q_OBJECT

public:
	CResourceEdit(QWidget* parent = 0);

	void setResourceFilter(const QStringList& flt);

	QString resourceName() const;
	void setResourceName(const QString& t);

	private slots:
	void buttonPressed();
	void nameChanged();

signals:
	void resourceChanged();

private:
	Ui::CResourceEdit*	ui;
};
