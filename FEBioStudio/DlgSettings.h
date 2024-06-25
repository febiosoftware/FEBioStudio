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
#include <QDialog>
#include <QSpinBox>
#include <QGridLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <PostLib/ColorMap.h>

class CMainWindow;
class QAbstractButton;
class QLabel;

namespace Ui {
	class CDlgSettings;
};

class ColorGradient : public QWidget
{
public:
	ColorGradient(QWidget* parent = 0);

	QSize sizeHint() const override;

	void paintEvent(QPaintEvent* ev) override;

	void setColorMap(const Post::CColorMap& m);

private:
	Post::CColorMap	m_map;
};

class CColormapWidget : public QWidget
{
	Q_OBJECT

public:
	CColormapWidget(QWidget* parent = 0);

	void updateColorMap(const Post::CColorMap& map);

	void clearGrid();

protected slots:
	void currentMapChanged(int n);
	void onDataChanged();
	void onSpinValueChanged(int n);
	void onNew();
	void onDelete();
	void onEdit();
	void onInvert();
	void onSetDefault(int nstate);

private:
	void updateMaps();

private:
	QGridLayout*	m_grid;
	QSpinBox*		m_spin;
	QComboBox*		m_maps;
	ColorGradient*	m_grad;
	QCheckBox*		m_default;
	int				m_currentMap;
};

class CUnitWidget : public QWidget
{
	Q_OBJECT

public:
	CUnitWidget(CMainWindow* wnd, QWidget* parent = nullptr);

	void showAllOptions(bool b);

	int getOption();

	void setUnit(int n);

private slots:
	void OnUnitSystemChanged(int n);
	void OnUnitOptionChanged(int n);

public:
	int		m_unit;

private:
	CMainWindow*	m_wnd;
	QComboBox*		m_ops;
	QComboBox*		m_us;
	QWidget*		m_edit;
	QLabel*			m_name[12];
};

class CRepoSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	CRepoSettingsWidget(QWidget* parent = 0);

	void setupUi();

public:
	QString repoPath;
	QLineEdit* repoPathEdit;

protected slots:
	void pathButton_clicked();
};

class CUpdateSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	CUpdateSettingsWidget(QDialog* settings, CMainWindow* wnd, QWidget* parent = 0);

protected slots:
	void updateButton_clicked();
	void updateDevButton_clicked();

private:
	QDialog* m_settings;
	CMainWindow* m_wnd;
};

class CFEBioSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	CFEBioSettingsWidget(QWidget* parent = nullptr);

public:
	bool GetLoadConfigFlag();
	QString GetConfigFileName();

	void SetLoadConfigFlag(bool b);
	void SetConfigFileName(QString s);

	QString GetSDKIncludePath() const;
	void SetSDKIncludePath(const QString& s);

	QString GetSDKLibraryPath() const;
	void SetSDKLibraryPath(const QString& s);

	QString GetCreatePluginPath() const;
	void SetCreatePluginPath(const QString& s);

protected slots:
	void editConfigFilePath();

private:
	QCheckBox* m_loadConfig = nullptr;
	QLineEdit* m_configEdit = nullptr;
	QLineEdit* m_sdkInc = nullptr;
	QLineEdit* m_sdkLib = nullptr;
	QLineEdit* m_pluginPath = nullptr;
};

class CDlgSettings : public QDialog
{
	Q_OBJECT

public:
	CDlgSettings(CMainWindow* pwnd);

	void showEvent(QShowEvent* ev);

	void apply();

public slots:
	void accept();
	void onClicked(QAbstractButton*);
	void onReset();

private:
	void UpdatePalettes();
	void UpdateSettings();
	void UpdateUI();

protected:
	CMainWindow*		m_pwnd;
	Ui::CDlgSettings*	ui;
};
