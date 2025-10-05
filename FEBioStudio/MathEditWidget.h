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
#include <CUILib/PlotWidget.h>
#include <FECore/MathObject.h>

class UIMathEditWidget;

class CMathEditWidget : public QWidget
{
	Q_OBJECT

public:
	CMathEditWidget(QWidget* parent = nullptr);

	void SetOrdinate(const QString& ord);
	void SetMath(const QString& txt);
	void ClearVariables();
	void SetVariable(const QString& name, double val);

	void showRangeOptions(bool b);

	void setMinMaxRange(double rmin, double rmax);
	void setLeftExtend(int n);
	void setRightExtend(int n);

public slots:
	void onEditingFinished();
	void onLeftExtendChanged();
	void onRightExtendChanged();
	void onRangeMinChanged();
	void onRangeMaxChanged();

signals:
	void mathChanged(QString s);
	void leftExtendChanged(int n);
	void rightExtendChanged(int n);
	void minChanged(double vmin);
	void maxChanged(double vmax);

private:
	UIMathEditWidget* ui;
};

class CMathPlotWidget : public CPlotWidget
{
	Q_OBJECT

public:
	// NOTE: Make sure this matches the definition in FEMathIntervalController
	enum ExtendMode { ZERO, CONSTANT, REPEAT };

public:
	CMathPlotWidget(QWidget* parent = nullptr);
	void DrawPlotData(QPainter& p, CPlotData& data) override;

	void SetOrdinate(const std::string& x);

	void SetMath(const QString& txt);

	void ClearVariables();
	void SetVariable(const QString& name, double val);

	void setLeftExtendMode(int n);
	void setRightExtendMode(int n);

private:
	double value(double x, MVariable* var, int& region);

public slots:
	void onRegionSelected(QRect rt);
	void onPointClicked(QPointF pt, bool shift);

private:
	MSimpleExpression	m_math;
	std::string			m_ord;

	std::vector< std::pair<QString, double> >	m_Var;

	int		m_leftExtend;
	int		m_rghtExtend;
};
