/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once
#include <QWidget>
#include <QFrame>
#include <unordered_map>

class QFormLayout;
class CPropertyList;
class CProperty;
class QLineEdit;
class QPushButton;
class GItem;
class QListWidget;
class QVBoxLayout;
class QComboBox;
class QGroupBox;
class QToolButton;

//-----------------------------------------------------------------------------
class CWrapperBox : public QFrame
{
	Q_OBJECT

public:
	QComboBox* m_pc;
	QGroupBox* m_pg;
	QToolButton* m_tb;

public:
	CWrapperBox(const QString& name, QWidget* parent = nullptr);

	void addItems(const QStringList& items);
	void setCurrentIndex(int n);

public slots:
	void OnExpandClicked();
};

//-----------------------------------------------------------------------------
// event filter for eating mouse scrolls on combo boxes
class QEvent;

class CMouseWheelFilter : public QObject
{
	Q_OBJECT
protected:
	bool eventFilter(QObject* po, QEvent* ev) override;
};

//-----------------------------------------------------------------------------
// builds a Ui for editing properties using a form view
class CPropertyListForm : public QWidget
{
	Q_OBJECT

public:
	CPropertyListForm(QWidget* parent = 0);

	// set the property list
	void setPropertyList(CPropertyList* pl);

	// get the property list
	CPropertyList* getPropertyList();

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
	void onCheckStateChanged(int n);

signals:
	// send when a widget is changed
	void dataChanged(bool itemModified, int index);

private:
	QVBoxLayout*	ui;
	CPropertyList*	m_list;
    std::unordered_map<QWidget*, int> m_widgets;
};
