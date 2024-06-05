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
#include <QTreeView>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QLineEdit>
#include <MeshLib/FEItemListBuilder.h>

class FSCoreBase;
class FSModel;
class FEClassPropsModel;
class Param;
class FSProperty;
class CMainWindow;
class GModel;

class CPropertySelector : public QComboBox
{
	Q_OBJECT

public:
	CPropertySelector(FSProperty* pp, FSCoreBase* pc, int index, FSModel* fem, QWidget* parent = nullptr);

public slots:
	void onSelectionChanged(int n);

signals:
	void currentDataChanged(int n);

private:
	FSModel* m_fem;
	FSCoreBase* m_pc;
	FSProperty* m_pp;
	int m_index;
};

class CMeshItemPropertySelector : public QComboBox
{
	Q_OBJECT

public:
	CMeshItemPropertySelector(GModel& m, FSProperty* pp, DOMAIN_TYPE domainType, QWidget* parent = nullptr);

public slots:
	void onSelectionChanged(int n);

signals:
	void currentDataChanged(int n);

private:
	GModel&		m_mdl;
	FSProperty* m_pp;
	DOMAIN_TYPE m_domainType;
	std::vector<FEItemListBuilder*> m_itemList;
};

class FEClassPropsDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit FEClassPropsDelegate(QObject* parent = nullptr);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	void setEditorData(QWidget* editor, const QModelIndex& index) const override;

	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	bool editorEvent(QEvent* event,
		QAbstractItemModel* model,
		const QStyleOptionViewItem& option,
		const QModelIndex& index) override;

private slots:
	void OnEditorSignal();
};

class FEClassPropsView : public QTreeView
{
	Q_OBJECT

public:
	FEClassPropsView(QWidget* parent = nullptr);

	void SetFEClass(FSCoreBase* pc, FSModel* fem);

	Param* getParam(const QModelIndex& index);
	FSProperty* getProperty(const QModelIndex& index);

	FSProperty* getSelectedProperty();

protected:
	void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
	void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private slots:
	void onModelDataChanged();
	void setFilter(const QString& flt);

private:
	FEClassPropsModel*	m_model;
};

//-----------------------------------------------------------------------------
class FEClassPropsWidget : public QWidget
{
	Q_OBJECT

public:
	FEClassPropsWidget(QWidget* parent = nullptr);

	void SetFEClass(FSCoreBase* pc, FSModel* fem);
	FSProperty* getProperty(const QModelIndex& index);

	FSProperty* getSelectedProperty();

private slots:
	void on_clicked(const QModelIndex& index);

signals:
	void clicked(const QModelIndex& index);

private:
	QLineEdit* m_flt;
	FEClassPropsView* m_view;
};

//-----------------------------------------------------------------------------
class FEClassEditUI;

class FEClassEdit : public QWidget
{
	Q_OBJECT

public:
	FEClassEdit(CMainWindow* wnd, QWidget* parent = nullptr);

	void SetFEClass(FSCoreBase* pc, FSModel* fem);

public slots:
	void onItemClicked(const QModelIndex& i);
	void onMathChanged(QString s);
	void onPlotChanged();

private:
	FEClassEditUI* ui;
};
