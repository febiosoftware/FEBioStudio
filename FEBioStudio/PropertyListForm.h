#pragma once
#include <QWidget>

class QFormLayout;
class CPropertyList;
class CProperty;
class QLineEdit;
class QPushButton;
class GItem;
class QListWidget;
class QVBoxLayout;

//-----------------------------------------------------------------------------
// builds a Ui for editing properties using a form view
class CPropertyListForm : public QWidget
{
	Q_OBJECT

public:
	CPropertyListForm(QWidget* parent = 0);

	// set the property list
	void setPropertyList(CPropertyList* pl);

	// update data
	void updateData();

private:
	// clear the property list
	void clear();

	// create an editor for a property
	QWidget* createPropertyEditor(CProperty& p, QVariant v);

private slots:
	// catch-all slot for when a widget gets changed
	void onDataChanged();

signals:
	// send when a widget is changed
	void dataChanged(bool itemModified);

private:
	QVBoxLayout*	ui;
	CPropertyList*	m_list;
	QList<QWidget*>	m_widget;
};
