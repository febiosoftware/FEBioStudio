#pragma once
#include <QWidget>
#include <vector>
#include <QtCore/QVariant>
#include <QPushButton>
#include <QComboBox>
#include "PropertyList.h"
#include "CIntInput.h"
#include "CColorButton.h"

//-----------------------------------------------------------------------------
class QTableWidget;
class QLabel;
class CPropertyList;

#include <QPainter>
#include <QColorDialog>

//-----------------------------------------------------------------------------
class CEditVariableProperty : public QComboBox
{
	Q_OBJECT

public:
	CEditVariableProperty(QWidget* parent = nullptr);

	void setProperty(CProperty* p, QVariant data);

public slots:
	void onCurrentIndexChanged(int index);

signals:
	void typeChanged();

private:
	CProperty*	m_prop;
};

//-----------------------------------------------------------------------------

namespace Ui {
	class CPropertyListView;
};

class CPropertyListView : public QWidget
{
	Q_OBJECT

public:
	CPropertyListView(QWidget* parent = 0);

	void Update(CPropertyList* plist);

	void FitGeometry();

	CPropertyList* GetPropertyList();

signals:
	void dataChanged(int);

private slots:
	void on_modelProps_clicked(const QModelIndex& index);
	void onDataChanged();

private:
	Ui::CPropertyListView*	ui;
};
