#include "stdafx.h"
#include "ExternalLinkEdit.h"
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

class Ui::CExternalLinkEdit
{
public:
	QLineEdit*		m_relativePathEdit;
	QToolButton*	m_open;

	QString fullPath;
	QString relativePath;

public:
	void setup(QWidget* w, QStringList& paths)
	{
		fullPath = paths[0];
		relativePath = paths[1];

		m_relativePathEdit = new QLineEdit(relativePath);
		m_relativePathEdit->setDisabled(true);
		m_relativePathEdit->setToolTip(fullPath);

		m_open = new QToolButton;
		m_open->setIcon(QIcon(":/icons/open.png"));

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

CExternalLinkEdit::CExternalLinkEdit(QStringList& paths, QWidget* parent) : QWidget(parent), ui(new Ui::CExternalLinkEdit)
{
	ui->setup(this, paths);
}

void CExternalLinkEdit::buttonPressed()
{
	QFileInfo info = QFileInfo(ui->fullPath);

	if(!info.exists())
	{
		QMessageBox box;
		box.setText((QString("Cannot open file.\n%1 does not exist.").arg(ui->fullPath)));
		box.exec();

		return;
	}

	// Open xplt files internally. TODO: do we want this special case here?
	if(info.suffix().compare("xplt") == 0)
	{
		CMainWindow* wnd = dynamic_cast<CMainWindow*>(QApplication::activeWindow()); assert(wnd);

		wnd->OpenFile(ui->fullPath);
	}
	else
	{
		QDesktopServices::openUrl(QUrl::fromLocalFile(ui->fullPath));
	}


}
