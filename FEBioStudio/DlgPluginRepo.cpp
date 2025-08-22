/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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
#include <FEBioLib/plugin.h>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QSplitter>
#include <QLineEdit>
#include <QProgressBar>
#include <QScrollArea>
#include <QApplication>
#include <QDesktopServices>
#include <QStyleHints>
#include <QPainter>
#include <QPainterPath>
#include <QAction>
#include <QPen>
#include <QPalette>
#include <QMouseEvent>
#include <QToolBar>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QTreeWidget>
#include <QHeaderView>
#include <FEBioLink/FEBioClass.h>
#include "DlgPluginRepo.h"
#include "PluginListWidget.h"
#include "IconProvider.h"
#include "PublicationWidgetView.h"
#include "PluginManager.h"
#include "WrapLabel.h"
#include "MainWindow.h"

#define IMAGE_SIZE 200

CFrameButton::CFrameButton(QString text, QWidget* parent) 
    : QWidget(parent), progress(1)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    label = new QLabel(text);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    label->setAlignment(Qt::AlignCenter);

    QFont font =  label->font();
    font.setPointSize(14);
    label->setFont(font);

    layout->addWidget(label);

    setLayout(layout);

    defaultColor = qApp->palette().color(QPalette::Highlight);

    if(qApp->styleHints()->colorScheme() != Qt::ColorScheme::Dark)
    {
        defaultColor = defaultColor.lighter();
    }
    else
    {
        defaultColor = defaultColor.darker();
    }

    backgroundColor = defaultColor;

    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void CFrameButton::SetText(QString text)
{
    label->setText(text);
}

void CFrameButton::SetProgress(float prog)
{
    progress = prog;
    update();
}

void CFrameButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPalette palette = qApp->palette();

    QColor borderColor = palette.color(QPalette::WindowText);
    int borderWidth = 2;
    int radius = 15;

    QRectF borderRect = rect().adjusted(borderWidth / 2.0, borderWidth / 2.0,
                                        -borderWidth / 2.0, -borderWidth / 2.0);

    painter.setPen(Qt::NoPen);

    if(progress < 1)
    {
        QPainterPath backgroundPath;
        backgroundPath.addRoundedRect(borderRect, radius, radius);
        painter.fillPath(backgroundPath, defaultColor);

        painter.setClipPath(backgroundPath);

        QRectF progressRect = borderRect;
        progressRect.setRight(borderRect.width() * progress);

        QPainterPath progressPath;
        progressPath.addRect(progressRect);

        painter.fillPath(progressPath, qApp->palette().color(QPalette::Highlight));
    }
    else
    {
        painter.setBrush(backgroundColor);
        painter.drawRoundedRect(borderRect, radius, radius);
    }

    // Then draw the border
    QPen pen(borderColor, borderWidth);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(borderRect, radius, radius);
}

QSize CFrameButton::sizeHint() const
{
    // Size hint = label's sizeHint + border padding
    QSize labelSize = label->sizeHint();
    int padding = 20; // 10 on each side
    return labelSize + QSize(padding, padding);
}

void CFrameButton::enterEvent(QEnterEvent* event)
{
    backgroundColor = qApp->palette().color(QPalette::Highlight);
    update();
}

void CFrameButton::leaveEvent(QEvent* event)
{
    backgroundColor = defaultColor;
    update();
}

void CFrameButton::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) 
    {
        emit clicked();
    }
}

CCollapsibleHeader::CCollapsibleHeader(QString text)
    : contents(nullptr), expanded(false)
{
    QVBoxLayout* outerLayout = new QVBoxLayout;
    outerLayout->setContentsMargins(20,0,20,0);

    QHBoxLayout* titleLayout = new QHBoxLayout;
    titleLayout->setContentsMargins(0,0,0,0);
    titleLayout->addWidget(label = new QLabel(text));
    label->setCursor(Qt::PointingHandCursor);

    QFrame* divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Sunken);
    divider->setCursor(Qt::PointingHandCursor);
    divider->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    titleLayout->addWidget(divider, 1);

    outerLayout->addLayout(titleLayout);

    setLayout(outerLayout);
}

void CCollapsibleHeader::SetContents(QWidget* widget)
{
    if(contents)
    {
        layout()->removeWidget(contents);
        contents->deleteLater();
    }

    contents = widget;

    if(contents)
    {
        layout()->addWidget(contents);

        contents->setVisible(expanded);
    }
}

void CCollapsibleHeader::SetExpanded(bool exp)
{
    expanded = exp;

    if(contents) contents->setVisible(expanded);
}

void CCollapsibleHeader::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int padding = 3;
    QRect box(padding, label->y() + padding, label->height() - padding*2, label->height() - padding*2); 

    int squash = 2; // used to squash triangle from base to tip by double squash value
    QPolygon triangle;
    if (expanded) {
        // Down-facing triangle
        triangle << QPoint(box.left(), box.top() + squash)
                << QPoint(box.right(), box.top() + squash)
                << QPoint(box.center().x(), box.bottom() - squash);
    } else {
        // Right-facing triangle
        triangle << QPoint(box.left() + squash, box.top())
                << QPoint(box.right() - squash, box.center().y())
                << QPoint(box.left() + squash, box.bottom());
    }

    // Draw filled triangle
    painter.setBrush(qApp->palette().color(QPalette::WindowText));
    painter.setPen(Qt::NoPen);
    painter.drawPolygon(triangle);
}

void CCollapsibleHeader::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton && contents) 
    {
        expanded = !expanded;

        contents->setVisible(expanded);
    }
}

class Ui::CDlgPluginRepo
{
public:
    ::PluginListWidget* pluginListWidget;

    QLineEdit* searchLineEdit;
    QAction* actionSearch;
    QAction* actionClear;

    QStackedWidget* stackedWidget;
    QLabel* repoStatusLabel;
    CFrameButton* retryButton;
    QLabel* imageLabel;
    QLabel* statusLabel;
    QLabel* downloadLabel;
    QLabel* downloadNumberLabel;
    QLabel* nameLabel;
    QLabel* byLabel;
    QLabel* ownerLabel;
    QLabel* descriptionLabel;
    QLabel* tagLabel;
    CFrameButton* downloadButton;
    CFrameButton* downloadingButton;
    CFrameButton* loadButton;
    CFrameButton* unloadButton;
    CFrameButton* deleteButton;
    CFrameButton* removeButton;
    CFrameButton* sourceButton;
    
    ::CPublicationWidgetView* publicationWidget;

    CCollapsibleHeader* advancedHeader;
    QTreeWidget* features;

    QDialogButtonBox* bb;

    void setupUI(::CDlgPluginRepo* dlg, CPluginManager* manager)
    {
        m_manager = manager;

        QVBoxLayout* layout = new QVBoxLayout;

        QToolBar* searchToolBar = new QToolBar;
        searchToolBar->addWidget(searchLineEdit = new QLineEdit);
        searchLineEdit->setPlaceholderText("Search plugins...");
        actionSearch = new QAction(CIconProvider::GetIcon("search"), "Search");
		searchToolBar->addAction(actionSearch);
		actionClear = new QAction(CIconProvider::GetIcon("clear"), "Clear");
		searchToolBar->addAction(actionClear);
        layout->addWidget(searchToolBar);

        QSplitter* splitter = new QSplitter(Qt::Horizontal);
        
        pluginListWidget = new ::PluginListWidget;
        splitter->addWidget(pluginListWidget);
        
        stackedWidget = new QStackedWidget;
        
        QWidget* welcomeCard = new QWidget;
        QVBoxLayout* welcomeLayout = new QVBoxLayout;
        welcomeCard->setLayout(welcomeLayout);

        welcomeLayout->addStretch();

        QLabel* welcomeLabel = new QLabel("Welcome to the FEBio Studio Plugin Repository!");
        QFont font = welcomeLabel->font();
        font.setBold(true);
        font.setPointSize(18);
        welcomeLabel->setFont(font);
        welcomeLayout->addWidget(welcomeLabel, 0, Qt::AlignHCenter);

        // Needs to be in its own layout for the horizontal spacing to work out
        QHBoxLayout* welcomeMessageLayout = new QHBoxLayout;
        welcomeMessageLayout->setContentsMargins(0,0,0,0);

        const char* welcomeMessage = "<br>Here you can find plugins for FEBio and FEBio Studio written by the FEBio Team " \
            "and others. Browse or search for plugins in the list on the left. Detailed information about the selected " \
            "plugin will be shown in this panel. After selecting the plugin, you can click the <i>Download</i> button " \
            "to download and automactially load the plugin into FEBio Studio.<br><br>Any downloaded plugins will be " \
            "automatically loaded into FEBio Studio each time it is launched. You can also load a plugin from a local " \
            "file by clicking the <i>Load Local Plugin</i> button.";

        QLabel* welcomeMessageLabel = new QLabel(welcomeMessage);
        welcomeMessageLabel->setWordWrap(true);
        welcomeMessageLabel->setAlignment(Qt::AlignCenter);
        
        welcomeMessageLabel->setMaximumWidth(700);
        
        welcomeMessageLayout->addWidget(welcomeMessageLabel);

        welcomeLayout->addLayout(welcomeMessageLayout);

        welcomeLayout->addStretch();

        welcomeLayout->addWidget(repoStatusLabel = new QLabel, 0, Qt::AlignHCenter);
        repoStatusLabel->setAlignment(Qt::AlignCenter);
        welcomeLayout->addWidget(retryButton = new CFrameButton("Retry Connection"), 0, Qt::AlignHCenter);
        
        welcomeLayout->addStretch();

        SetRepoStatus();

        stackedWidget->addWidget(welcomeCard);
        
        QWidget* pluginCard = new QWidget;
        QHBoxLayout* pluginCardLayout = new QHBoxLayout;

        QWidget* pluginLeftWidget = new QWidget;
        QVBoxLayout* pluginLeftLayout = new QVBoxLayout;
        pluginLeftLayout->setAlignment(Qt::AlignHCenter);
        pluginLeftLayout->addWidget(imageLabel = new QLabel);
        imageLabel->setAlignment(Qt::AlignHCenter);

        QHBoxLayout* statusLayout = new QHBoxLayout;
        statusLayout->setContentsMargins(0, 0, 0, 0);
        statusLayout->addStretch();
        statusLayout->addWidget(new QLabel("Status:"));
        statusLayout->addWidget(statusLabel = new QLabel);
        statusLayout->addStretch();
        pluginLeftLayout->addLayout(statusLayout);

        QHBoxLayout* downloadsLayout = new QHBoxLayout;
        downloadsLayout->setContentsMargins(0, 0, 0, 0);
        downloadsLayout->addStretch();
        downloadsLayout->addWidget(downloadLabel = new QLabel("Downloads:"));
        downloadsLayout->addWidget(downloadNumberLabel = new QLabel);
        downloadsLayout->addStretch();
        pluginLeftLayout->addLayout(downloadsLayout);

        downloadButton = new CFrameButton("Download Plugin");
        downloadButton->hide();
        pluginLeftLayout->addWidget(downloadButton, 0, Qt::AlignHCenter);

        downloadingButton = new CFrameButton("Downloading...");
        downloadingButton->hide();
        pluginLeftLayout->addWidget(downloadingButton, 0, Qt::AlignHCenter);

        loadButton = new CFrameButton("Load Plugin");
        loadButton->hide();
        pluginLeftLayout->addWidget(loadButton, 0, Qt::AlignHCenter);

        unloadButton = new CFrameButton("Unload Plugin");
        unloadButton->hide();
        pluginLeftLayout->addWidget(unloadButton, 0, Qt::AlignHCenter);

        deleteButton = new CFrameButton("Delete Plugin");
        deleteButton->hide();
        pluginLeftLayout->addWidget(deleteButton, 0, Qt::AlignHCenter);

        removeButton = new CFrameButton("Remove Plugin");
        removeButton->hide();
        pluginLeftLayout->addWidget(removeButton, 0, Qt::AlignHCenter);

        pluginLeftLayout->addStretch();
        pluginLeftWidget->setLayout(pluginLeftLayout);

        int left, top, right, bottom;
        pluginLeftLayout->getContentsMargins(&left, &top , &right, &bottom);
        pluginLeftWidget->setFixedWidth(IMAGE_SIZE + left + right);

        pluginCardLayout->addWidget(pluginLeftWidget);

        QVBoxLayout* pluginRightLayout = new QVBoxLayout;

        QHBoxLayout* nameLayout = new QHBoxLayout;
        nameLayout->setContentsMargins(0, 0, 0, 0);

        nameLayout->addWidget(nameLabel = new QLabel);
        QFont nameFont = nameLabel->font();
        nameFont.setBold(true);
        nameFont.setPointSize(18);
        nameLabel->setFont(nameFont);

        byLabel = new QLabel("by");
        QFont byFont = byLabel->font();
        byFont.setPointSize(18);
        byLabel->setFont(byFont);

        nameLayout->addWidget(byLabel);

        nameLayout->addWidget(ownerLabel = new QLabel);
        QFont font2 = ownerLabel->font();
        font2.setItalic(true);
        font2.setPointSize(18);
        ownerLabel->setFont(font2);

        nameLayout->addStretch();

        pluginRightLayout->addLayout(nameLayout);

        QScrollArea* scrollArea = new QScrollArea;
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        QWidget* scrollContent = new QWidget;
        QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
        scrollLayout->setContentsMargins(0, 0, 0, 0);

        scrollLayout->addWidget(descriptionLabel = new QLabel);
        descriptionLabel->setWordWrap(true);

        scrollLayout->addWidget(publicationWidget = new ::CPublicationWidgetView(::CPublicationWidgetView::LIST, false));

        scrollLayout->addWidget(tagLabel = new QLabel);
        tagLabel->setWordWrap(true);

        advancedHeader = new CCollapsibleHeader("<b>Advanced</b>");

        features = new QTreeWidget;
		features->setColumnCount(5);
		features->setHeaderLabels(QStringList() << "type string" << "super class" << "class name" << "base class" << "module");
        features->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        advancedHeader->SetContents(features);

        scrollLayout->addWidget(advancedHeader);

        scrollLayout->addStretch();

        scrollContent->setLayout(scrollLayout);
        scrollArea->setWidget(scrollContent);
        pluginRightLayout->addWidget(scrollArea);

        sourceButton = new CFrameButton("Source Code");
        pluginRightLayout->addWidget(sourceButton, 0, Qt::AlignRight);

        pluginCardLayout->addLayout(pluginRightLayout, 1);

        pluginCard->setLayout(pluginCardLayout);

        stackedWidget->addWidget(pluginCard);

        splitter->addWidget(stackedWidget);
        QList<int> sizes = {1, dlg->width()};
        splitter->setSizes(sizes);

        layout->addWidget(splitter);

		bb = new QDialogButtonBox(QDialogButtonBox::Close);
        bb->addButton("Load Local Plugin", QDialogButtonBox::ResetRole);
        
		layout->addWidget(bb);

        QObject::connect(pluginListWidget, &::PluginListWidget::pluginThumbnailClicked, dlg, &::CDlgPluginRepo::on_pluginThumbnail_clicked);
        QObject::connect(searchLineEdit, &QLineEdit::returnPressed, dlg, &::CDlgPluginRepo::on_actionSearch_triggered);
        QObject::connect(actionSearch, &QAction::triggered, dlg, &::CDlgPluginRepo::on_actionSearch_triggered);
        QObject::connect(actionClear, &QAction::triggered, dlg, &::CDlgPluginRepo::on_actionClear_triggered);
        QObject::connect(downloadButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_downloadButton_clicked);
        QObject::connect(deleteButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_deleteButton_clicked);
        QObject::connect(removeButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_removeButton_clicked);
        QObject::connect(loadButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_loadButton_clicked);
        QObject::connect(unloadButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_unloadButton_clicked);
        QObject::connect(sourceButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_sourceButton_clicked);
        QObject::connect(retryButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_retryButton_clicked);

		QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &::CDlgPluginRepo::reject);
        QObject::connect(bb, &QDialogButtonBox::clicked, dlg, &::CDlgPluginRepo::on_bbButton_clicked);
		
        dlg->setLayout(layout);
    }

    void SetRepoStatus(QString error = "")
    {
        switch (m_manager->Status())
        {
        case CPluginManager::UNCONNECTED:
        case CPluginManager::CONNECTING:
            repoStatusLabel->setText("Connecting...");
            repoStatusLabel->show();
            retryButton->hide();
            break;
        case CPluginManager::ERROR:
            repoStatusLabel->setText(error);
            repoStatusLabel->show();
            retryButton->show();
            break;
        default:
            repoStatusLabel->hide();
            retryButton->hide();
            break;
        }

        pluginListWidget->UpdateUi(m_manager->Status());
    }

    void setActivePlugin(int id)
    {
        if(id == 0)
        {
            pluginListWidget->UpdateUi(m_manager->Status());

            stackedWidget->setCurrentIndex(0);

            return;
        }

        Plugin* plugin = m_manager->GetPlugin(id);

        m_pluginID = plugin->id;

        if(id > 0)
        {
            nameLabel->setText(plugin->name.c_str());
            byLabel->show();
            ownerLabel->setText(plugin->owner.c_str());
            downloadLabel->show();
            downloadNumberLabel->setText(QString::number(plugin->downloads));
            downloadNumberLabel->show();
            descriptionLabel->setText(plugin->description.c_str());

            QByteArray image(plugin->imageData);
            if (image.size() > 0)
            {
                QByteArray imageDataByteArray = QByteArray::fromBase64(image);

                QPixmap pixmap;
                pixmap.loadFromData(imageDataByteArray);
                imageLabel->setPixmap(pixmap.scaledToHeight(IMAGE_SIZE, Qt::SmoothTransformation));
            }
            else
            {
                imageLabel->clear();
            }
        }
        else
        {
            nameLabel->setText(plugin->name.c_str());
            byLabel->hide();
            ownerLabel->setText("");
            downloadLabel->hide();
            downloadNumberLabel->hide();
            descriptionLabel->setText(QString::fromStdString( plugin->description + "\n\nPath: " + plugin->files[0]));

            QPixmap pixmap(":/icons/febio_large.png");
            imageLabel->setPixmap(pixmap.scaledToHeight(IMAGE_SIZE, Qt::SmoothTransformation));
        }

        if(plugin->publications.size() == 0)
        {
            publicationWidget->hide();
        }
        else
        {
            publicationWidget->clear();

            for(auto& pub : plugin->publications)
            {
                publicationWidget->addPublication(pub);
            }

            publicationWidget->show();
        }

        if(plugin->tags.size() > 0)
        {
            QString tagText("Tags: ");
            tagText += plugin->tags[0].c_str();

            for(size_t i = 1; i < plugin->tags.size(); ++i)
            {
                tagText += QString(", ") + plugin->tags[i].c_str();
            }

            tagLabel->setText(tagText);
            tagLabel->show();
        }
        else
        {
            tagLabel->hide();
        }

        sourceButton->setVisible(!plugin->repoName.empty());

        downloadButton->hide();
        downloadingButton->hide();
        loadButton->hide();
        unloadButton->hide();
        deleteButton->hide();
        removeButton->hide();
        
        if(plugin->status == PLUGIN_BROKEN)
        {
            statusLabel->setText("Broken");
            downloadButton->show();
        }
        if(plugin->status == PLUGIN_DOWNLOADING)
        {
            statusLabel->setText("Downloading...");
            downloadingButton->show();
        }
        else if(plugin->status ==  PLUGIN_NOT_INSTALLED)
        {
            statusLabel->setText("Not Installed");
            downloadButton->show();
        }
        else if(plugin->status == PLUGIN_OUT_OF_DATE)
        {
            statusLabel->setText("Out of Date");
            downloadButton->show();

            if(plugin->loaded)
            {
                unloadButton->show();
            }
            else
            {
                loadButton->show();
            }

            deleteButton->show();
        }
        else if(plugin->status == PLUGIN_UP_TO_DATE)
        {
            statusLabel->setText("Up to Date");

            if(plugin->loaded)
            {
                unloadButton->show();
            }
            else
            {
                loadButton->show();
            }

            deleteButton->show();
        }
        else if(plugin->status == PLUGIN_LOCAL)
        {
            statusLabel->setText("Local Plugin");

            if(plugin->loaded)
            {
                unloadButton->show();
            }
            else
            {
                loadButton->show();
            }

            removeButton->show();
        }

        advancedHeader->hide();
        if(plugin->loaded)
        {
            updateFeaturesList(plugin);
            advancedHeader->show();
        }

        pluginListWidget->UpdateUi(m_manager->Status());

        stackedWidget->setCurrentIndex(1);
    }

    void addFeature(const QString& type, const QString& superClass, const QString& className, const QString& baseClass, const QString moduleName)
	{
		QTreeWidgetItem* twi = new QTreeWidgetItem(features);
		twi->setText(0, type);
		twi->setText(1, superClass);
		twi->setText(2, className);
		twi->setText(3, baseClass);
		twi->setText(4, moduleName);
	}

	void updateFeaturesList(Plugin* plugin)
	{
		FEBioPluginManager* pm = FEBioPluginManager::GetInstance(); assert(pm);

		features->clear();

		std::vector<FEBio::FEBioClassInfo> classList = FEBio::FindAllPluginClasses(plugin->allocatorID);
		for (FEBio::FEBioClassInfo& ci : classList)
		{
			QString baseClass = QString::fromStdString(FEBio::GetBaseClassName(ci.baseClassId));
			QString superClass = FEBio::GetSuperClassString(ci.superClassId);
			addFeature(ci.sztype, superClass, ci.szclass, baseClass, ci.szmod);
		}

        int totalHeight = features->header()->height();
        for (int i = 0; i < features->topLevelItemCount(); ++i) {
            QTreeWidgetItem* item = features->topLevelItem(i);
            totalHeight += features->visualItemRect(item).height();
        }
        totalHeight += 10;
        features->setFixedHeight(totalHeight);
	}

public:
    CPluginManager* m_manager;
    std::vector<std::string> m_tags;
    int m_pluginID = -1;
};

CDlgPluginRepo::CDlgPluginRepo(CPluginManager* manager, QWidget *parent)
    : QDialog(parent), ui(new Ui::CDlgPluginRepo)
{
    resize(1100, 600);
    setWindowTitle("Plugin Repository");

    ui->setupUI(this, manager);

    connect(ui->m_manager, &CPluginManager::downloadProgress, this, &CDlgPluginRepo::downloadProgress);
    connect(ui->m_manager, &CPluginManager::DownloadFinished, this, &CDlgPluginRepo::DownloadFinished);
    connect(ui->m_manager, &CPluginManager::PluginsReady, this, &CDlgPluginRepo::on_PluginsReady);
    connect(ui->m_manager, &CPluginManager::HTMLError, this, &CDlgPluginRepo::on_HTMLError);

    ui->m_manager->ReadDatabase();

    ui->m_manager->Connect(false);
}

void CDlgPluginRepo::LoadLocalPlugin()
{
    QString filter;
#ifdef WIN32
        filter = "*.dll";
#elif __APPLE__
        filter = "*.dylib";
#else
        filter = "*.so";
#endif

    std::string filename = QFileDialog::getOpenFileName(this, 
        QString("Load Local Plugin"), "", filter).toStdString();

    if(filename.empty()) return;

    if(ui->m_manager->LoadNonRepoPlugin(filename))
    {
        on_PluginsReady();
    }
    else
    {
        QString message = QString("Unable to load %1").arg(filename.c_str());
        QMessageBox::critical(this, "Plugin Error", message);

        return;
    }
}

void CDlgPluginRepo::DownloadFinished(int id)
{
    ui->pluginListWidget->SetPluginInstalled(id, true);
    ui->pluginListWidget->SetPluginStatus(id, ui->m_manager->GetPlugin(id)->status);
    
    ui->setActivePlugin(ui->m_pluginID);
}

void CDlgPluginRepo::downloadProgress(qint64 bytesSent, qint64 bytesTotal, int id)
{
    float progress = (float) bytesSent / (float) bytesTotal;

    if(ui->m_pluginID == id)
    {
        ui->downloadingButton->SetProgress(progress);
    }

    ui->pluginListWidget->SetPluginProgress(id, progress);
}

void CDlgPluginRepo::on_PluginsReady()
{
    ui->pluginListWidget->Clear();

    auto& plugins = ui->m_manager->GetPlugins();

    for (auto& [id, plugin] : plugins)
    {
        ui->pluginListWidget->AddPlugin(plugin);
    }

    on_actionSearch_triggered();

    ui->SetRepoStatus();
}

void CDlgPluginRepo::on_HTMLError(QString& message)
{
    ui->SetRepoStatus(message);

    ui->setActivePlugin(0);
}

void CDlgPluginRepo::on_UnknownError(QString& message)
{
    ui->SetRepoStatus(message);

    ui->setActivePlugin(0);
}

void CDlgPluginRepo::on_pluginThumbnail_clicked(int id)
{
    ui->setActivePlugin(id);
}

void CDlgPluginRepo::on_actionSearch_triggered()
{
    QString searchTerm = ui->searchLineEdit->text();
    if(searchTerm.isEmpty())
    {
        on_actionClear_triggered();
        return;
    }

    std::unordered_set<int> searchResults = ui->m_manager->SearchPlugins(searchTerm);
    ui->pluginListWidget->SetSearchResults(searchResults);
}
void CDlgPluginRepo::on_actionClear_triggered()
{
    ui->searchLineEdit->clear();

    std::unordered_set<int> clearResults;
    clearResults.insert(-1);
    ui->pluginListWidget->SetSearchResults(clearResults);
}

void CDlgPluginRepo::on_downloadButton_clicked()
{
    ui->m_manager->DownloadPlugin(ui->m_pluginID);
    
    ui->pluginListWidget->SetPluginStatus(ui->m_pluginID, ui->m_manager->GetPlugin(ui->m_pluginID)->status);
    
    ui->setActivePlugin(ui->m_pluginID);
}

void CDlgPluginRepo::on_deleteButton_clicked()
{
    if(!ui->m_manager->DeletePlugin(ui->m_pluginID))
    {
        QMessageBox::warning(this, "Error", "Unable to delete plugin.");
        return;
    }

    ui->pluginListWidget->SetPluginInstalled(ui->m_pluginID, false);
    ui->pluginListWidget->SetPluginStatus(ui->m_pluginID, ui->m_manager->GetPlugin(ui->m_pluginID)->status);
    
    int id = ui->m_pluginID;
    if(ui->m_manager->Status() != CPluginManager::CONNECTED) id = 0;
    ui->setActivePlugin(id);
}

void CDlgPluginRepo::on_removeButton_clicked()
{
    if(!ui->m_manager->RemoveNonRepoPlugin(ui->m_pluginID))
    {
        QMessageBox::warning(this, "Error", "Unable to load plugin.");
        return;
    }

    ui->pluginListWidget->RemovePlugin(ui->m_pluginID);

    ui->setActivePlugin(0);
}

void CDlgPluginRepo::on_loadButton_clicked()
{
    if(!ui->m_manager->LoadPlugin(ui->m_pluginID))
    {
        QMessageBox::warning(this, "Error", "Unable to load plugin.");
    }

    ui->pluginListWidget->SetPluginStatus(ui->m_pluginID, ui->m_manager->GetPlugin(ui->m_pluginID)->status);
    ui->setActivePlugin(ui->m_pluginID);
}

void CDlgPluginRepo::on_unloadButton_clicked()
{
    if(!ui->m_manager->UnloadPlugin(ui->m_pluginID))
    {
        QMessageBox::warning(this, "Error", "Unable to unload plugin.");
    }

    ui->pluginListWidget->SetPluginStatus(ui->m_pluginID, ui->m_manager->GetPlugin(ui->m_pluginID)->status);
    ui->setActivePlugin(ui->m_pluginID);
}

void CDlgPluginRepo::on_sourceButton_clicked()
{
    Plugin* plugin = ui->m_manager->GetPlugin(ui->m_pluginID);
    assert(plugin);

    if(plugin)
    {
        QUrl url(QString("https://github.com/febiosoftware/") + plugin->repoName.c_str());
        QDesktopServices::openUrl(url);
    }
}

void CDlgPluginRepo::on_retryButton_clicked()
{
    ui->m_manager->Connect();

    ui->SetRepoStatus();
}

void CDlgPluginRepo::on_bbButton_clicked(QAbstractButton *button)
{
    if(ui->bb->buttonRole(button) == QDialogButtonBox::ResetRole)
    {
        LoadLocalPlugin();
    }
}