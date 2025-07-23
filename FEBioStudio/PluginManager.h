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

#pragma once

#include <QObject>
#include <string>
#include <vector>
#include <unordered_set>

enum PluginStatus
{
    PLUGIN_NOT_INSTALLED = 0,
    PLUGIN_BROKEN,
    PLUGIN_UP_TO_DATE,
    PLUGIN_OUT_OF_DATE,
    PLUGIN_LOCAL
};

struct Plugin
{
    int id; // non-repo plugins have negative IDs
    int status;
    int downloads = 0;
    std::string name;
    std::string owner;
    std::string description;
    std::string sourceURL;
    QByteArray imageData; // thumbnail image data
    std::vector<QVariantMap> publications; // plugin publications
    std::vector<std::string> tags; // versions from the database
    
    bool localCopy = false;
    bool loaded = false;
    std::string localVersion;
    std::string localFebioVersion;
    std::vector<std::string> files; // paths to plugin files
    int mainFileIndex = 0; // index of the file that should be loaded

    int allocatorID = 0;
};


class CPluginManager : public QObject
{
    Q_OBJECT;

    class Imp;

public:
    CPluginManager();
    ~CPluginManager();

    bool LoadXML();
    void Connect(int force = true);
    void SetConnected(bool connected);

    void LoadAllPlugins();
    void ReadDatabase();

    std::unordered_set<int> SearchPlugins(const QString& searchTerm);

    const std::unordered_map<int, Plugin>& GetPlugins();
    Plugin* GetPlugin(int id);
    Plugin* AddPlugin(int id);

    Plugin* GetPluginFromAllocatorID(int allocId);

    void DownloadPlugin(int id);
    bool DeletePlugin(int id);

    bool LoadPlugin(int id);
    bool UnloadPlugin(int id);
    bool LoadNonRepoPlugin(std::string& path);

    void AddRepoPlugin(char** argv);
    void AddPublication(int pluginID, const QVariantMap& data);
    void AddTag(int pluginID, const std::string& tag);
    void AddPluginFile(int pluginID, const std::string& filePath, int main, const std::string& version, const std::string& febioVersion);

    void OnDownloadFinished(int id);

signals:
    void DownloadFinished(int id);
    void HTMLError(QString& message, bool close = false);
    void PluginsReady();
    void downloadProgress(qint64 bytesSent, qint64 bytesTotal, int id);

private:
    Plugin* AddNonRepoPlugin();
    void SyncWithFEBioPluginManager();
    bool LoadPluginFile(std::string& path);
    bool LoadFEBioPlugin(Plugin& plugin);
    void SetPluginStatus(Plugin& plugin);
    bool IsVersion2Newer(const std::string& version1, const std::string& version2);

private:
    Imp* imp;
};