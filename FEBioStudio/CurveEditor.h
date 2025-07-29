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
#include <QMainWindow>
#include "CommandManager.h"
#include "Command.h"
#include <CUILib/PlotWidget.h>
#include <FSCore/LoadCurve.h>
#include <FSCore/ParamBlock.h>

class CMainWindow;
class LoadCurve;
class FSModelComponent;
class QTreeWidgetItem;
class CCurveEditorItem;
class FSObject;
class FSModel;
class FSLoadController;

namespace Ui {
	class CCurveEdior;
}

class CCurveEditor : public QMainWindow
{
	Q_OBJECT

	enum { FLT_ALL, FLT_GEO, FLT_MESH_DATA, FLT_MAT, FLT_BC, FLT_LOAD, FLT_CONTACT, FLT_NLCONSTRAINT, FLT_RIGID_CONSTRAINT, FLT_RIGID_CONNECTOR, FLT_DISCRETE, FLT_STEP, FLT_LOAD_CURVES };

public:
	CCurveEditor(CMainWindow* wnd);

	void Update();

	void closeEvent(QCloseEvent* ev);

	static QRect preferredSize();
	static void setPreferredSize(const QRect& rt);

private:
	bool Filter(int n) { if ((m_nflt == FLT_ALL) || (m_nflt == n)) return true; return false; }
	void AddModelComponent(QTreeWidgetItem* t1, FSModelComponent* po);

	void SetActiveLoadController(FSLoadController* plc);

private:
	void BuildLoadCurves();
	void BuildModelTree();
	void BuildLoadCurves(QTreeWidgetItem* t1, FSModelComponent* po, const std::string& name = "");

private slots:
	void on_tree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev);
	void on_filter_currentIndexChanged(int n);
	void on_newLC_clicked(bool b);
	void on_selectLC_currentIndexChanged(int index);
	void on_plot_dataChanged();
	void on_math_mathChanged(QString s);
	void on_math2_mathChanged(QString s);
	void on_math2_leftExtendChanged(int n);
	void on_math2_rightExtendChanged(int n);
	void on_math2_minChanged(double v);
	void on_math2_maxChanged(double v);

private:
	Ui::CCurveEdior*	ui;
	CCurveEditorItem*	m_currentItem;
	CMainWindow*		m_wnd;
	int					m_nflt;
	FSModel*			m_fem;
	FSLoadController*	m_plc;

	static QRect	m_preferredSize;
};
