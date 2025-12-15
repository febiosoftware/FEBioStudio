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

#include "WindowPanel.h"

namespace Ui
{
    class CImageParam;
    class CImageParam2;
    class CImageSettingsWidget;
    class CImageSettingsPanel;
}

class CImageModel;
class CMainWindow;
class FSObject;
class Param;

class CImageParam : public QWidget
{
    Q_OBJECT

public:
    CImageParam();

	void setParam(Param* param);

    ~CImageParam();

	double currentValue();

	void setValue(double v);

public slots:
    void updateParam();

signals:
    void paramChanged();

private:
    Ui::CImageParam* ui;
};

class CImageParam2 : public QWidget
{
	Q_OBJECT

public:
	CImageParam2();
	void setParams(Param* param1, Param* param2);

	~CImageParam2();

	void setColor(QColor c);

public slots:
	void updateSpinBox();
	void updateSlider();

signals:
	void paramChanged();

private:
	Ui::CImageParam2* ui;
};

class CImageSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    CImageSettingsWidget(QWidget* parent = nullptr);

public slots:
    void ImageModelChanged(CImageModel* model);
	void on_ParamChanged();
	void on_colorChanged(QColor c);

signals:
    void ParamChanged();

private:
    Ui::CImageSettingsWidget* ui;

};

class CImageSettingsPanel : public CWindowPanel
{
    Q_OBJECT

public:
    CImageSettingsPanel(CMainWindow* wnd, QWidget* parent);

public slots:
    void ModelTreeSelectionChanged(FSObject* obj);
    void on_ParamChanged();

private:
    Ui::CImageSettingsPanel* ui;
};