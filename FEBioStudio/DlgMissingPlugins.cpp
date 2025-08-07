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

#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QPixmap>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressBar>
#include <QVariantAnimation>
#include <QTimer>
#include "MainWindow.h"
#include "PluginManager.h"

#include "DlgMissingPlugins.h"

class Ui::CPluginWidget
{
public:
    QLabel* label;
    QLabel* okayLabel;
    QPushButton* button;
    QProgressBar* progress;

public:

    void setupUi(::CPluginWidget* parent, QString name, int id)
    {
        QHBoxLayout* layout = new QHBoxLayout;
        layout->setContentsMargins(0,0,0,0);
        
        layout->addStretch();

        label = new QLabel(name);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        layout->addWidget(label);

        okayLabel = new QLabel;
        QPixmap okayPixmap = QPixmap(":/icons/emblems/greenCheck.png").scaledToHeight(16, Qt::SmoothTransformation);
        okayLabel->setPixmap(okayPixmap);
        layout->addWidget(okayLabel);
        okayLabel->hide();

        QString text = "Connecting...";
        if(id < 0) text = "Locate Plugin";

        button = new QPushButton(text);
        if(id > 0) button->setDisabled(true);
        layout->addWidget(button);

        if(id > 0)
        {
            layout->addWidget(progress = new QProgressBar);
            progress->hide();
        }

        layout->addStretch();

        parent->setLayout(layout);
    }
};

CPluginWidget::CPluginWidget(int id, QString name)
    : m_fixed(false), m_name(name), m_id(id), ui(new Ui::CPluginWidget)
{
    ui->setupUi(this, name, id);

    connect(ui->button, &QPushButton::clicked, this, &CPluginWidget::on_button_clicked);
}

void CPluginWidget::MarkReady()
{
    if(m_id > 0)
    {
        show();
        ui->button->setText("Download");
        ui->button->setEnabled(true);
        ui->progress->hide();
        ui->button->show();
    }
}

void CPluginWidget::MarkFixed()
{
    m_fixed = true;

    ui->button->hide();
    ui->okayLabel->show();

    if(m_id > 0) ui->progress->hide();
}

void CPluginWidget::on_button_clicked()
{
    if(m_id > 0)
    {
        ui->button->hide();
        ui->progress->show();
    }

    emit button_clicked(this);
}

void CPluginWidget::on_downloadFinished(int id)
{
    if(m_id == id)
    {
        emit downloadFinished(this);
    }
}

void CPluginWidget::downloadProgress(qint64 bytesSent, qint64 bytesTotal, int id)
{
    if(m_id == id)
    {
        ui->progress->setRange(0, bytesTotal);

	    ui->progress->setValue(bytesSent);
    }
}

class Ui::CDlgMissingPlugins
{
public:
    QWidget* errorWidget;
    QLabel* errorLabel;
    QPushButton* retryButton;
    QDialogButtonBox* buttonBox;

public:
    CDlgMissingPlugins(::CMainWindow* wnd): m_skipPlugins(true), m_hasRepoPlugins(false)
    {
        m_manager = wnd->GetPluginManager();
    }

    void setupUi(::CDlgMissingPlugins* parent, vector<pair<int, string>> missingPlugins)
    {
        m_parent = parent;
        m_parent->setWindowTitle("Missing Plugins");

        QVBoxLayout* layout = new QVBoxLayout;

        QLabel* localPluginsLabel = new QLabel("This file needs the following plugin(s) which are not part "
            "of the Plugin Repository. If you have these plugins on your local machine, you can load them by "
            "clicking the Locate Plugin button next to each one.");
        localPluginsLabel->setWordWrap(true);
        layout->addWidget(localPluginsLabel);

        bool localPlugins = false;
        for(auto& [id, plugin] : missingPlugins)
        {
            if(id < 0)
            {
                localPlugins = true;

                ::CPluginWidget* localWidget = new ::CPluginWidget(id, plugin.c_str());
                layout->addWidget(localWidget);

                m_pluginWidets.append(localWidget);

                QObject::connect(localWidget, &::CPluginWidget::button_clicked, m_parent, &::CDlgMissingPlugins::on_pluginButton_clicked);
            }
        }

        if(!localPlugins) localPluginsLabel->hide();

        QLabel* repoPluginsLabel = new QLabel("This file needs the following plugin(s) from the Plugin Repository. "
            "You can download them now by clicking on the Download button next to each one.");
        repoPluginsLabel->setWordWrap(true);
        layout->addWidget(repoPluginsLabel);

        for(auto& [id, plugin] : missingPlugins)
        {
            if(id > 0)
            {
                m_hasRepoPlugins = true;

                ::CPluginWidget* repoWidget = new ::CPluginWidget(id, plugin.c_str());
                layout->addWidget(repoWidget);
                
                m_pluginWidets.append(repoWidget);

                QObject::connect(repoWidget, &::CPluginWidget::button_clicked, m_parent, &::CDlgMissingPlugins::on_pluginButton_clicked);
                QObject::connect(repoWidget, &::CPluginWidget::downloadFinished, m_parent, &::CDlgMissingPlugins::on_downloadFinished);
                QObject::connect(m_manager, &CPluginManager::DownloadFinished, repoWidget, &::CPluginWidget::on_downloadFinished);
                QObject::connect(m_manager, &CPluginManager::downloadProgress, repoWidget, &::CPluginWidget::downloadProgress);
            }
        }

        if(!m_hasRepoPlugins) repoPluginsLabel->hide();

        errorWidget = new QWidget;
        errorWidget->hide();
        QVBoxLayout* errorLayout = new QVBoxLayout;

        errorLabel = new QLabel;
        errorLabel->setWordWrap(true);
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLayout->addWidget(errorLabel);
        
        QHBoxLayout* retryLayout = new QHBoxLayout;
        retryLayout->setContentsMargins(0,0,0,0);
        retryLayout->addStretch();
        
        retryButton = new QPushButton("Retry Connection");
        retryLayout->addWidget(retryButton);

        retryLayout->addStretch();

        errorLayout->addLayout(retryLayout);

        errorWidget->setLayout(errorLayout);

        layout->addWidget(errorWidget);

        buttonBox = new QDialogButtonBox;
        buttonBox->addButton(QDialogButtonBox::Cancel);
        buttonBox->addButton("Attempt to loading file without plugins", QDialogButtonBox::AcceptRole);

        QObject::connect(buttonBox, &QDialogButtonBox::accepted, m_parent, &QDialog::accept);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, m_parent, &QDialog::reject);

        layout->addWidget(buttonBox);

        m_parent->setLayout(layout);
    }

    void MarkFixed(::CPluginWidget* widget)
    {
        widget->MarkFixed();

        bool allFixed = true;
        for(auto widget : m_pluginWidets)
        {
            if(!widget->m_fixed) allFixed = false;
        }

        if(allFixed)
        {
            buttonBox->clear();
            buttonBox->addButton(QDialogButtonBox::Cancel);
            buttonBox->addButton("Load File", QDialogButtonBox::AcceptRole);

            m_skipPlugins = false;
        }

        m_parent->adjustSize();
    }

    void ShowHTMLError(QString& message)
    {
        if(!m_hasRepoPlugins) return;

        errorWidget->show();

        errorLabel->setText(message);
        
        retryButton->setText("Retry Connection");
        retryButton->setEnabled(true);

        for(auto widget : m_pluginWidets)
        {
            if(widget->m_id > 0)
            {
                widget->hide();
            }
        }

        m_parent->adjustSize();

        flashErrorWidget();
    }

    void flashErrorWidget(int duration = 400)
    {
        QWidget* widget = errorWidget;

        // Store the original background color
        QColor originalColor = widget->palette().color(QPalette::Window);
        QColor highlightColor = widget->palette().color(QPalette::Highlight);

        // Apply auto fill background if not already set
        widget->setAutoFillBackground(true);

        // Set highlight color first
        QPalette palette = widget->palette();
        palette.setColor(QPalette::Window, highlightColor);
        widget->setPalette(palette);
        widget->update();

        // Animate back to the original color
        auto* animation = new QVariantAnimation(widget);
        animation->setDuration(duration);
        animation->setStartValue(highlightColor);
        animation->setEndValue(originalColor);

        QObject::connect(animation, &QVariantAnimation::valueChanged, widget, [=](const QVariant &value) {
            QPalette p = widget->palette();
            p.setColor(QPalette::Window, value.value<QColor>());
            widget->setPalette(p);
            widget->update();
        });

        // Clean up after done
        QObject::connect(animation, &QVariantAnimation::finished, widget, [=]() {
            animation->deleteLater();
        });

        animation->start();
    }

public:
    ::CDlgMissingPlugins* m_parent;

    QList<::CPluginWidget*> m_pluginWidets;

    CPluginManager* m_manager;

    bool m_hasRepoPlugins;
    bool m_skipPlugins;
};

CDlgMissingPlugins::CDlgMissingPlugins(CMainWindow* wnd, vector<pair<int, string>> missingPlugins, QWidget *parent)
    : QDialog(parent), ui(new Ui::CDlgMissingPlugins(wnd))
{
    ui->setupUi(this, missingPlugins);
    
    if(ui->m_hasRepoPlugins)
    {
        connect(ui->retryButton, &QPushButton::clicked, this, &CDlgMissingPlugins::RetryConnection);
        connect(ui->m_manager, &CPluginManager::PluginsReady, this, &CDlgMissingPlugins::on_pluginsReady);
        connect(ui->m_manager, &CPluginManager::HTMLError, this, &CDlgMissingPlugins::on_HTMLError);
        ui->m_manager->Connect(false);
    } 
}

bool CDlgMissingPlugins::SkipPlugins()
{
    return ui->m_skipPlugins;
}

void CDlgMissingPlugins::on_pluginButton_clicked(CPluginWidget* widget)
{
    if(widget->m_id < 0)
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
            QString("Locate %1").arg(widget->m_name), "", filter).toStdString();

        if(filename.empty()) return;

        if(ui->m_manager->LoadNonRepoPlugin(filename))
        {
            ui->MarkFixed(widget);
        }
        else
        {
            QString message = QString("Unable to load %1").arg(filename.c_str());
            QMessageBox::critical(this, "Plugin Error", message);

            return;
        }
    }
    else
    {
        ui->m_manager->DownloadPlugin(widget->m_id);
    }
}

void CDlgMissingPlugins::on_pluginsReady()
{
    ui->errorWidget->hide();

    for(auto widget : ui->m_pluginWidets)
    {
        widget->MarkReady();
    }

    adjustSize();
}

void CDlgMissingPlugins::on_downloadFinished(CPluginWidget* widget)
{
    ui->MarkFixed(widget);
}

void CDlgMissingPlugins::on_HTMLError(QString& message)
{
    ui->ShowHTMLError(message);
}

void CDlgMissingPlugins::RetryConnection()
{
    ui->retryButton->setText("Connecting...");
    ui->retryButton->setDisabled(true);

    ui->m_manager->Connect();
}