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

#include <QApplication>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QResizeEvent>
#include <QPixmap>
#include <QPainter>
#include "PluginListWidget.h"
#include "PluginManager.h"
#include "DlgPluginRepo.h"

class Ui::PluginThumbnail
{
public:
    QLabel* imageLabel;
    QLabel* nameLabel;
    QLabel* ownerLabel;
    QLabel* statusLabel;

public:

    PluginThumbnail(::PluginThumbnail* parent)
        : m_parent(parent), m_id(0), m_installed(false), m_selected(false)
    {

    }

    void setupUi(const Plugin* plugin)
    {
        m_parent->setAttribute(Qt::WA_Hover, true);
        m_parent->setCursor(Qt::PointingHandCursor);
        m_parent->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

        backgroundColor = qApp->palette().color(QPalette::Window);

        // This widget is also used to display the status of the repo while
        // it is connecting, or if there is an error. A nullptr value for
        // the plugin is the signal that this object is used for showing the
        // status 
        if(!plugin)
        {
            m_id = 0;

            QVBoxLayout* layout = new QVBoxLayout;
            layout->setAlignment(Qt::AlignHCenter);

            imageLabel = new QLabel;
            nameLabel = new QLabel;
            ownerLabel = new QLabel("<b>Connecting...</b>");
            statusLabel = new QLabel;

            layout->addWidget(nameLabel, 0, Qt::AlignHCenter);
            layout->addWidget(ownerLabel, 0, Qt::AlignHCenter);
            layout->addWidget(statusLabel, 0, Qt::AlignHCenter);

            m_parent->setLayout(layout);

            return;
        }

        QHBoxLayout* layout = new QHBoxLayout;
        layout->setAlignment(Qt::AlignLeft);
        
        imageLabel = new QLabel;

        if(plugin->id > 0)
        {
            QByteArray imageDataByteArray = QByteArray::fromBase64(plugin->imageData);
            image.loadFromData(imageDataByteArray);
        }
        else
        {
            image.load(":/icons/febio_large.png");
        }

        imageLabel->setAlignment(Qt::AlignVCenter);

        layout->addWidget(imageLabel);

        layout->addSpacing(5);

        QVBoxLayout* infoLayout = new QVBoxLayout;
        infoLayout->setContentsMargins(0,0,0,0);

        nameLabel = new QLabel(QString::fromStdString("<b>" + plugin->name + "</b>"));
        nameLabel->setAlignment(Qt::AlignLeft);
        ownerLabel = new QLabel(QString::fromStdString("<i>" + plugin->owner + "</i>"));
        ownerLabel->setAlignment(Qt::AlignLeft);
        statusLabel = new QLabel(QString::fromStdString(plugin->description));
        statusLabel->setAlignment(Qt::AlignLeft);

        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(ownerLabel);
        infoLayout->addWidget(statusLabel);

        layout->addLayout(infoLayout);

        m_parent->setLayout(layout);

        m_id = plugin->id;
        m_installed = plugin->localCopy;
    }

    void SetPixmap()
    {
        int left, top, right, bottom;
        m_parent->layout()->getContentsMargins(&left, &top, &right, &bottom);

        int height = m_parent->height() - top - bottom;

        imageLabel->setPixmap(image.scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imageLabel->setFixedWidth(height);
        imageLabel->setAlignment(Qt::AlignCenter);
    }

public:
    QWidget* m_parent;
    QColor backgroundColor;
    QPixmap image;
    int m_id;
    bool m_installed;
    bool m_selected;
};

PluginThumbnail::PluginThumbnail(const Plugin* plugin)
    : ui(new Ui::PluginThumbnail(this))
{
    ui->setupUi(plugin);

    if(plugin) SetStatus(plugin->status);
}


void PluginThumbnail::SetPixmap()
{
    ui->SetPixmap();
}

void PluginThumbnail::SetErrorText(const QString& text)
{
    ui->ownerLabel->setText(text);
}

void PluginThumbnail::SetStatus(int status)
{
    // QPixmap pluginImg = ui->imageLabel->pixmap();

    // QPixmap emblem;

    switch (status)
    {
    case PLUGIN_BROKEN:
        // emblem = QPixmap(":/icons/emblems/warning.png").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setToolTip("This plugin is broken and cannot be used.");
        ui->statusLabel->setText("Broken");
        break;
    case PLUGIN_NOT_INSTALLED:
        setToolTip("This plugin is not installed.");
        ui->statusLabel->setText("Not Installed");
        break;
    case PLUGIN_OUT_OF_DATE:
        // emblem = QPixmap(":/icons/emblems/caution.png").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setToolTip("This plugin is out of date.");
        ui->statusLabel->setText("Out of Date");
        break;
    case PLUGIN_UP_TO_DATE:
        // emblem = QPixmap(":/icons/emblems/greenCheck.png").scaled(25, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setToolTip("This plugin is up to date.");
        ui->statusLabel->setText("Up to Date");
        break;
    case PLUGIN_LOCAL:
        setToolTip("This plugin in not part of the repository, but was loaded from a local file.");
        ui->statusLabel->setText("Local Plugin");
        ui->ownerLabel->hide();
    }

    // QPainter painter(&pluginImg);
	// painter.drawPixmap(pluginImg.width() - emblem.width(), pluginImg.height() - emblem.height(), emblem);

    // ui->imageLabel->setPixmap(pluginImg);
}

int PluginThumbnail::getID()
{
    return ui->m_id;
}

void PluginThumbnail::setInstalled(bool installed)
{
    ui->m_installed = installed;
}

bool PluginThumbnail::installed()
{
    return ui->m_installed;
}

void PluginThumbnail::setSelected(bool selected)
{
    ui->m_selected = selected;
}

void PluginThumbnail::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QColor color = ui->backgroundColor;
    if(ui->m_selected) color = qApp->palette().color(QPalette::Highlight);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(rect(), 5, 5);
}

void PluginThumbnail::enterEvent(QEnterEvent* event)
{
    ui->backgroundColor = qApp->palette().color(QPalette::Highlight);
}

void PluginThumbnail::leaveEvent(QEvent* event)
{
    ui->backgroundColor = qApp->palette().color(QPalette::Window);
}

void PluginThumbnail::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) 
    {
        emit clicked(ui->m_id);
    }
}

class Ui::PluginListWidget
{
public:
    QLineEdit* searchInput;
    CCollapsibleHeader* installedPlugins;
    QVBoxLayout* installedLayout;
    CCollapsibleHeader* repoPlugins;
    QVBoxLayout* repoLayout;
    ::PluginThumbnail* statusThumbnail;

public:

    void setupUi(::PluginListWidget* parent)
    {
        m_parent = parent;

        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->setContentsMargins(0, 0, 0, 0);

        QScrollArea* scrollArea = new QScrollArea;
        QWidget* scrollWidget = new QWidget;
        QVBoxLayout* scrollLayout = new QVBoxLayout;
        scrollLayout->setContentsMargins(0,0,0,0);
        scrollWidget->setLayout(scrollLayout);

        installedPlugins = new CCollapsibleHeader("Installed");
        installedPlugins->SetExpanded(true);

        QWidget* installedWidget = new QWidget;
        installedLayout = new QVBoxLayout;
        installedLayout->setContentsMargins(0,0,0,0);
        installedWidget->setLayout(installedLayout);

        installedPlugins->SetContents(installedWidget);

        scrollLayout->addWidget(installedPlugins);

        repoPlugins = new CCollapsibleHeader("Repository");
        repoPlugins->SetExpanded(true);

        QWidget* repoWidget = new QWidget;
        repoLayout = new QVBoxLayout;
        repoLayout->setContentsMargins(0,0,0,0);
        repoWidget->setLayout(repoLayout);

        repoLayout->addWidget(statusThumbnail = new ::PluginThumbnail(nullptr));
        QObject::connect(statusThumbnail, &::PluginThumbnail::clicked, parent, &::PluginListWidget::on_pluginThumbnailClicked);

        repoPlugins->SetContents(repoWidget);

        scrollLayout->addWidget(repoPlugins);

        scrollLayout->addStretch();
        
        scrollArea->setWidget(scrollWidget);
        scrollArea->setWidgetResizable(true);
        
        mainLayout->addWidget(scrollArea);

        parent->setLayout(mainLayout);

        parent->setMinimumWidth(250);
    }

    void updateUi(int repoStatus = CPluginManager::CONNECTED)
    {
        // Removes the items from the layout, but does not delete the
        // widgets themselves.
        QLayoutItem* item;
        while((item = installedLayout->takeAt(0)) != nullptr) 
        {
            delete item;
        }

        while((item = repoLayout->takeAt(0)) != nullptr) 
        {
            delete item;
        }

        if(searchResults.empty())
        {
            for(::PluginThumbnail* thumbnail : pluginThumbnails) 
            {
                thumbnail->hide();
            }
        }

        bool showInstalled = false;
        bool showRepo = false;

        for(::PluginThumbnail* thumbnail : pluginThumbnails) 
        {
            thumbnail->show();

            // If there is only one search result and it's -1, show all plugins
            if(!((searchResults.size() == 1) && (searchResults.count(-1) == 1)))
            {
                // Otherwise skip if the thumbnail ID is not in the search results
                if(searchResults.count(thumbnail->getID()) == 0) 
                {
                    thumbnail->hide();
                    continue; // Skip if not in search results
                }
            }

            if(thumbnail->installed())
            {
                installedLayout->addWidget(thumbnail);
                thumbnail->show();
                showInstalled = true;
            }
            else
            {
                if(repoStatus == CPluginManager::CONNECTED)
                {
                    repoLayout->addWidget(thumbnail);
                    thumbnail->show();
                }
                else
                {
                    thumbnail->hide();
                }
                showRepo = true;
            }

            thumbnail->SetPixmap();
        }

        installedPlugins->setVisible(showInstalled);

        if(repoStatus != CPluginManager::CONNECTED)
        {
            statusThumbnail->show();

            if(repoStatus == CPluginManager::ERROR)
            {
                statusThumbnail->SetErrorText("<b>Connection Error</b>");
            }
            else
            {
                statusThumbnail->SetErrorText("<b>Connecting...</b>");
            }

            repoLayout->addWidget(statusThumbnail);
            
            showRepo = true;
        }
        else
        {
            statusThumbnail->hide();
        }

        repoPlugins->setVisible(showRepo);

        m_connected = false;
    }

public:
    ::PluginListWidget* m_parent;
    std::unordered_set<int> searchResults; 

    QList<::PluginThumbnail*> pluginThumbnails;

    bool m_connected;
};
    
PluginListWidget::PluginListWidget() : ui(new Ui::PluginListWidget)
{
    ui->setupUi(this);

    ui->searchResults.insert(-1); // Default to showing all plugins
}

void PluginListWidget::UpdateUi(int repoStatus)
{
    ui->updateUi(repoStatus);
}

void PluginListWidget::AddPlugin(const Plugin& plugin)
{
    PluginThumbnail* thumbnail = new PluginThumbnail(&plugin);
    thumbnail->setInstalled(plugin.localCopy);
    connect(thumbnail, &::PluginThumbnail::clicked, this, &PluginListWidget::on_pluginThumbnailClicked);
    ui->pluginThumbnails.append(thumbnail);
    
    ui->updateUi();
}

void PluginListWidget::Clear()
{
    qDeleteAll(ui->pluginThumbnails);
    ui->pluginThumbnails.clear();

    ui->searchResults.clear();
    ui->searchResults.insert(-1);

    ui->updateUi();
}

void PluginListWidget::SetSearchResults(const std::unordered_set<int>& results)
{
    ui->searchResults = results;

    ui->updateUi();
}

void PluginListWidget::SetPluginInstalled(int id, bool installed)
{
    for(auto plugin : ui->pluginThumbnails)
    {
        if(plugin->getID() == id)
        {
            plugin->setInstalled(installed);
            return;
        }
    }
}

void PluginListWidget::SetPluginStatus(int id, int status)
{
    for(auto plugin : ui->pluginThumbnails)
    {
        if(plugin->getID() == id)
        {
            plugin->SetStatus(status);
            return;
        }
    }
}

void PluginListWidget::on_pluginThumbnailClicked(int id)
{
    ui->statusThumbnail->setSelected(id==0);
    ui->statusThumbnail->update();

    for(auto plugin : ui->pluginThumbnails)
    {
        if(plugin->getID() == id)
        {
            plugin->setSelected(true);
        }
        else
        {
            plugin->setSelected(false);
        }

        plugin->update();
    }

    emit pluginThumbnailClicked(id);
}
