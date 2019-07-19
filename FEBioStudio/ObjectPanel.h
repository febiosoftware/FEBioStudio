#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class CColorButton;
class GObject;
class CMainWindow;

class CObjectPanel : public QWidget
{
	Q_OBJECT

private:
	QLineEdit*		name;
	QLabel*			type;
	CColorButton*	color;
	CMainWindow*	m_wnd;

protected slots:
	void onColorChanged(QColor c);

public:
	CObjectPanel(CMainWindow* wnd, QWidget* parent = 0);

	void Update();
};

