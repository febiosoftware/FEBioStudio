#include <QFileSystemModel>
#include <QListView>
#include <QBoxLayout>
#include <QComboBox>
#include "FileViewer.h"
#include "MainWindow.h"
#include <QToolButton>

class Ui::CFileViewer
{
public:
	QComboBox*	m_fileFilter;
	QListView*	m_fileList;
	QComboBox*	m_folder;

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* pg = new QVBoxLayout(parent);
		QHBoxLayout* pg2 = new QHBoxLayout;

		QToolButton* pb = new QToolButton(parent);
		pb->setObjectName(QStringLiteral("toolUp"));
		pb->setIcon(QIcon(":/icons/up.png"));
		pb->setAutoRaise(true);
		pb->setToolTip("<font color=\"black\">Parent folder");

		m_folder = new QComboBox;
		m_folder->setObjectName("folder");
		m_folder->setEditable(true);
		m_folder->setInsertPolicy(QComboBox::NoInsert);
		m_folder->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
		m_folder->setMaxCount(15);

		m_fileFilter = new QComboBox(parent);
		m_fileFilter->setObjectName(QStringLiteral("fileFilter"));

		m_fileList = new QListView(parent);
		m_fileList->setObjectName(QStringLiteral("fileList"));

		pg2->addWidget(pb);
		pg2->addWidget(m_folder);
		pg->addLayout(pg2);
		pg->addWidget(m_fileFilter);
		pg->addWidget(m_fileList);
	}

	void addFolder(const QString& folder)
	{
		// see if this item already exists
		int n = m_folder->findText(folder);
		if (n != -1) return;

		// if not, add it
		n = m_folder->count();
		if (n == m_folder->maxCount())
		{
			m_folder->removeItem(0);
		}
		m_folder->addItem(folder);
		m_folder->setCurrentIndex(m_folder->count() - 1);
	}
};

CFileViewer::CFileViewer(CMainWindow* pwnd, QWidget* parent) : QWidget(parent), m_wnd(pwnd), ui(new Ui::CFileViewer)
{
	// build Ui
	ui->setupUi(this);

	// build the filter list
	// Make sure this list matches the one in CMainWindow::on_actionOpen_triggered()
	// TODO: Can I somehow ensure that this is the case ?
    m_filters.push_back(pair<QString, QString>("FEBio Studio Projects (*.fsprj)", "*.fsprj"));
	m_filters.push_back(pair<QString, QString>("PreView files (*.prv)", "*.prv"));

	// add filters to drop down
	int nflts = (int)m_filters.size();
	for (int i = 0; i<nflts; ++i)
	{
		pair<QString, QString>& flt = m_filters[i];
		ui->m_fileFilter->addItem(flt.first);
	}

	// create a model for the file system
	m_fileSystem = new QFileSystemModel;
	m_fileSystem->setRootPath("C:\\");
	QStringList flt;
	flt << m_filters[0].second;
    flt << m_filters[1].second;
	m_fileSystem->setNameFilters(flt);
	m_fileSystem->setNameFilterDisables(false);

	// set the file system model
	ui->m_fileList->setModel(m_fileSystem);
	ui->m_fileList->setRootIndex(m_fileSystem->index("C:\\Users\\steve\\Documents"));

	QMetaObject::connectSlotsByName(this);
}

QString CFileViewer::currentPath() const
{
	return m_fileSystem->filePath(ui->m_fileList->rootIndex());
}

void CFileViewer::setCurrentPath(const QString& s)
{
	ui->m_fileList->setRootIndex(m_fileSystem->index(s));

	int n = ui->m_folder->findText(s);
	if (n >= 0) ui->m_folder->setCurrentIndex(n);
	else ui->m_folder->setEditText(currentPath());
}

QStringList CFileViewer::FolderList()
{
	QStringList folders;
	for (int i = 0; i<ui->m_folder->count(); ++i)
	{
		folders << ui->m_folder->itemText(i);
	}
	return folders;
}

void CFileViewer::SetFolderList(const QStringList& folders)
{
	ui->m_folder->clear();
	ui->m_folder->addItems(folders);
	ui->m_folder->setCurrentIndex(0);
}

void CFileViewer::on_fileList_doubleClicked(const QModelIndex& index)
{
	if (m_fileSystem->isDir(index))
	{
		ui->m_fileList->setRootIndex(index);
		ui->m_folder->setEditText(currentPath());
	}
	else
	{
		m_wnd->OpenDocument(m_fileSystem->filePath(index));

		QString filePath = currentPath();
		m_wnd->SetCurrentFolder(filePath);

		ui->addFolder(filePath);
	}
}

void CFileViewer::on_fileFilter_currentIndexChanged(int index)
{
	if ((index >= 0) && (index < m_filters.size()))
	{
		QStringList filters;
		pair<QString, QString>& flt = m_filters[index];
		filters << flt.second;
		m_fileSystem->setNameFilters(filters);
	}
}

void CFileViewer::on_toolUp_clicked()
{
	QModelIndex n = ui->m_fileList->rootIndex();
	n = m_fileSystem->parent(n);
	ui->m_fileList->setRootIndex(n);
	ui->m_folder->setEditText(currentPath());
}

void CFileViewer::on_folder_currentIndexChanged(const QString& text)
{
	setCurrentPath(text);
}

void CFileViewer::on_folder_editTextChanged(const QString& text)
{
	setCurrentPath(text);
}
