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
#include <vector>
//using namespace std;

using std::vector;
using std::pair;

class CMainWindow;
class FSObject;
class FEItemListBuilder;
class GMaterial;
class CPropertyList;
class QLineEdit;
class QLabel;
class CColorButton;
class QComboBox;
class QCheckBox;
class GDiscreteElementSet;

namespace Ui {
	class CModelPropsPanel;
}

class CObjectPropsPanel : public QWidget
{
	Q_OBJECT

public:
	CObjectPropsPanel(QWidget* parent = 0);

	void setName(const QString& name);

	void setType(const QString& name);

	void setColor(const QColor& col);

	void setStatus(bool b);

	void showColor(bool b);

	void showStatus(bool b);

	void setNameReadOnly(bool b);

protected slots:
	void on_name_textEdited(const QString&);
	void on_col_colorChanged(QColor c);
	void on_status_clicked(bool b);

signals:
	void nameChanged(const QString& newName);
	void colorChanged(const QColor& c);
	void statusChanged(bool b);

private:
	QLineEdit*		m_name;
	QLabel*			m_type;
	CColorButton*	m_col;
	QCheckBox*		m_status;
	QLabel*			m_statusLabel;
};

class CBCObjectPropsPanel : public QWidget
{
	Q_OBJECT

public:
	CBCObjectPropsPanel(QWidget* parent = 0);

	void setName(const QString& name);

	void setType(const QString& name);

	void setStepValues(const vector<pair<QString, int> >& l);

	void setStepID(int n);

	int currentStepID();

	void showActiveState(bool b);

	void setActiveState(bool b);

protected slots:
	void on_name_textEdited(const QString&);
	void on_list_currentIndexChanged(int n);
	void on_state_toggled(bool b);

signals:
	void nameChanged(const QString& newName);
	void stepChanged(int n);
	void stateChanged(bool isActive);

private:
	QLineEdit*		m_name;
	QLabel*			m_type;
	QComboBox*		m_list;
	QCheckBox*		m_state;
};

class CGItemPropsPanel : public QWidget
{
	Q_OBJECT

public:
	CGItemPropsPanel(QWidget* parent = 0);

	void setName(const QString& name);

	void setType(const QString& name);

	void setID(int id);

protected slots:
	void on_name_textEdited(const QString&);

signals:
	void nameChanged(const QString& newName);

private:
	QLineEdit*		m_name;
	QLabel*			m_type;
	QLabel*			m_id;
};


class CModelPropsPanel : public QWidget
{
	Q_OBJECT

public:
	CModelPropsPanel(CMainWindow* wnd, QWidget* parent = 0);

	void SetObjectProps(FSObject* po, CPropertyList* props, int flags);

	FSObject* GetCurrentObject() { return m_currentObject; }

	void Update();

	void Refresh();

private slots:
	void on_select1_addButtonClicked();
	void on_select1_subButtonClicked();
	void on_select1_delButtonClicked();
	void on_select1_selButtonClicked();
	void on_select1_clearButtonClicked();
	void on_select2_addButtonClicked();
	void on_select2_subButtonClicked();
	void on_select2_delButtonClicked();
	void on_select2_selButtonClicked();
	void on_select2_clearButtonClicked();
	void on_select1_nameChanged(const QString& t);
	void on_select2_nameChanged(const QString& t);
	void on_object_nameChanged(const QString&);
	void on_bcobject_nameChanged(const QString&);
	void on_gitem_nameChanged(const QString&);
	void on_object_colorChanged(const QColor& col);
	void on_props_dataChanged(int n);
	void on_form_dataChanged(bool itemModified);
	void on_bcobject_stepChanged(int n);
	void on_bcobject_stateChanged(bool isActive);
	void on_object_statusChanged(bool b);
	void on_math_mathChanged(QString m);
	void on_math2_mathChanged(QString m);
	void on_math2_leftExtendChanged(int n);
	void on_math2_rightExtendChanged(int n);
	void on_math2_minChanged(double vmin);
	void on_math2_maxChanged(double vmax);

	void on_plt_dataChanged();

private:
	void SetSelection(int n, FEItemListBuilder* it);
	void SetSelection(GDiscreteElementSet* set);

	void addSelection(int n);
	void subSelection(int n);
	void delSelection(int n);
	void selSelection(int n);
	void clearSelection(int n);

signals:
	void nameChanged(const QString& txt);
	void selectionChanged();
	void dataChanged(bool b);

private:
	Ui::CModelPropsPanel* ui;
	CMainWindow*		m_wnd;
	FSObject*			m_currentObject;
	bool				m_isUpdating;
};
