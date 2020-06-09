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

private:
	void UpdatePalettes();

protected:
	CMainWindow*		m_pwnd;
	Ui::CDlgSettings*	ui;
};