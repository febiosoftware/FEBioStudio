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

using namespace std;

class CMainWindow;

namespace Ui {
    class CPluginWidget;
}

class CPluginWidget : public QWidget
{
    Q_OBJECT

public:
    CPluginWidget(int id, QString name);

    void MarkReady();

    void MarkFixed();

public slots:
    void on_button_clicked();
    void on_downloadFinished(int id);
    void downloadProgress(qint64 bytesSent, qint64 bytesTotal, int id);

signals:
    void button_clicked(CPluginWidget* widget);
    void downloadFinished(CPluginWidget* widget);

public:
    bool m_fixed;
    QString m_name;
    int m_id;

private:
    Ui::CPluginWidget* ui;
};

namespace Ui {
    class CDlgMissingPlugins;
}

class CDlgMissingPlugins : public QDialog
{
    Q_OBJECT

public:
    CDlgMissingPlugins(CMainWindow* wnd, vector<pair<int, string>> missingPlugins, QWidget *parent = nullptr);

    bool SkipPlugins();

public slots:
    void on_pluginButton_clicked(CPluginWidget* widget);
    void on_pluginsReady();
    void on_downloadFinished(CPluginWidget* widget);
    void on_HTMLError(QString& message);
    void RetryConnection();

private:
    Ui::CDlgMissingPlugins *ui;
};