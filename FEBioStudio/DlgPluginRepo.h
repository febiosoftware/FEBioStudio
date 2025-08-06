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

#include <QDialog>

class QLabel;
class CPluginManager;

class CFrameButton : public QWidget
{
    Q_OBJECT

public:
    CFrameButton(QString text, QWidget* parent = nullptr);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

    QSize sizeHint() const override;
private:
    QLabel* label;
    QColor backgroundColor;
    QColor defaultColor;
};

class CCollapsibleHeader : public QWidget
{
public:
    CCollapsibleHeader(QString text);

    void SetContents(QWidget* widget);
    void SetExpanded(bool exp);

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent* event) override;
private:
    QLabel* label;
    bool expanded;
    QWidget* contents;
};

namespace Ui {
class CDlgPluginRepo;
}

class CDlgPluginRepo : public QDialog
{
    Q_OBJECT

public:
    CDlgPluginRepo(CPluginManager* manager, QWidget *parent);

    void DownloadFinished();

private:
    void LoadLocalPlugin();

public slots:
    void downloadProgress(qint64 bytesSent, qint64 bytesTotal);

private slots:
    void on_PluginsReady();
    void on_HTMLError(QString& message, bool bclose);
    void on_UnknownError(QString& message);
    void on_pluginThumbnail_clicked(int id);
    void on_actionSearch_triggered();
    void on_actionClear_triggered();
    void on_downloadButton_clicked();
    void on_deleteButton_clicked();
    void on_loadButton_clicked();
    void on_unloadButton_clicked();
    void on_sourceButton_clicked();
    void on_bbButton_clicked(QAbstractButton *button);
    
private:
    friend class Ui::CDlgPluginRepo;
    Ui::CDlgPluginRepo *ui;
};
