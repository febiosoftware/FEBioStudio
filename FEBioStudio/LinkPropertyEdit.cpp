#include "stdafx.h"
#include "LinkPropertyEdit.h"
#include <QDesktopServices>
#include <QLineEdit>
#include <QToolButton>
#include <QBoxLayout>
#include <QMessageBox>
#include <QFileInfo>
#include <QApplication>
#include <QUrl>
#include <FSCore/FSDir.h>
#include "MainWindow.h"

class Ui::CLinkPropertyEdit
{
public:
	QLineEdit*		m_relativePathEdit;
	QToolButton*	m_open;

	QString fullPath;
	QString relativePath;
	bool internal;

public:
	CLinkPropertyEdit(QStringList& paths, bool internal)
		: internal(internal)
	{
		fullPath = paths[0];
		relativePath = paths[1];
	}

	void setup(QWidget* w)
	{
		m_relativePathEdit = new QLineEdit(relativePath);
		m_relativePathEdit->setDisabled(true);
		m_relativePathEdit->setToolTip(fullPath);

		m_open = new QToolButton;
		m_open->setIcon(QIcon(":/icons/open.png"));

		if(internal) m_open->setToolTip("Open file");
		else m_open->setToolTip("Open in external editor");

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(m_relativePathEdit);
		h->addWidget(m_open);
		h->setSpacing(0);
		h->setMargin(0);
		w->setLayout(h);

		QObject::connect(m_open, SIGNAL(clicked(bool)), w, SLOT(buttonPressed()));
	}
};


//=================================================================================================

CLinkPropertyEdit::CLinkPropertyEdit(QStringList& paths, bool internal, QWidget* parent)
	: QWidget(parent), ui(new Ui::CLinkPropertyEdit(paths, internal))
{
	ui->setup(this);
}

void CLinkPropertyEdit::buttonPressed()
{
	QFileInfo info = QFileInfo(ui->fullPath);

	if(!info.exists())
	{
		QMessageBox box;
		box.setText((QString("Cannot open file.\n%1 does not exist.").arg(ui->fullPath)));
		box.exec();

		return;
	}

	if(ui->internal)
	{
		CMainWindow* wnd = dynamic_cast<CMainWindow*>(QApplication::activeWindow()); assert(wnd);

		wnd->OpenFile(ui->fullPath);
	}
	else
	{
		QDesktopServices::openUrl(QUrl::fromLocalFile(ui->fullPath));
	}


}
