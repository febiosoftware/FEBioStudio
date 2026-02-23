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

#include "PluginManager.h"
#include "PluginRepoConnectionHandler.h"
#include "PluginDatabaseHandler.h"
#include "PluginXML.h"
#include <QFileInfo>
#include <QStandardPaths>
#include <unordered_map>
#include <sstream>

#ifndef UPDATER
#include <FECore/version.h>
#include <FEBioLib/plugin.h>
#include <FEBioLib/febio.h>
#include <FECore/FEModule.h>
#endif

// This is need on Windows since one of the windows headers introduces a max macro. 
#ifdef WIN32
#undef max
#endif

class CPluginManager::Imp
{
public:
    Imp(CPluginManager* parent) 
        : m_db(parent), m_xml(parent), m_repo(parent, &m_db), m_localID(-1), m_status(CPluginManager::UNCONNECTED), m_develop(false)
    {
		string path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString() + "/plugins/plugins.xml";
        m_xml.SetPath(path);

        // Ensure that dev versions of FBS always get dev plugins
        #ifdef DEVCOMMIT
            m_develop = true;
        #endif
    }

public:
    CPluginDatabaseHandler m_db;
    CPluginXML   m_xml;
    CPluginRepoConnectionHandler m_repo;

    std::unordered_map<int, Plugin> m_plugins;
    int m_localID;

    int m_status;

    bool m_develop;
};

CPluginManager::CPluginManager() : imp(new Imp(this))
{

}

CPluginManager::~CPluginManager()
{
    delete imp;
}

bool CPluginManager::LoadXML()
{
    return imp->m_xml.LoadXML();
}

void CPluginManager::Connect(int force)
{   
    if((imp->m_status != CONNECTED && imp->m_status != CONNECTING) || force)
    {
        imp->m_status = CONNECTING;

        imp->m_repo.getSchema();
    }
    else
    {
        ReadDatabase();
    }
}

int CPluginManager::Status()
{
    return imp->m_status;
}

void CPluginManager::SetDevelop(bool develop)
{
    imp->m_develop = develop;
}

#ifndef UPDATER
void CPluginManager::LoadAllPlugins()
{
    for(auto& [id, plugin] : imp->m_plugins)
    {
        if(plugin.localCopy && !plugin.loaded)
        {
            if(LoadFEBioPlugin(plugin)) plugin.loaded = true;
        }
    }
}

void CPluginManager::ReadDatabase()
{
    imp->m_db.GetPlugins();

    SyncWithFEBioPluginManager();

    for(auto& [id, plugin] : imp->m_plugins)
    {
        plugin.publications.clear();
        imp->m_db.GetPluginPubs(id);

        plugin.tags.clear();
        imp->m_db.GetPluginTags(id);

        SetPluginStatus(plugin);
    }

    emit PluginsReady();
}

std::unordered_set<int> CPluginManager::SearchPlugins(const QString& searchTerm)
{
    return imp->m_db.GetPluginSearchResults(searchTerm);
}

bool CPluginManager::IsPluginNameInUse(QString& name)
{
    return imp->m_db.IsPluginNameInUse(name);
}

void CPluginManager::GetTags(QStringList& tags)
{
    imp->m_db.GetAllTags(tags);
}

#else
void CPluginManager::LoadAllPlugins() {}
void CPluginManager::ReadDatabase() {}
std::unordered_set<int> CPluginManager::SearchPlugins(const QString& searchTerm) { return std::unordered_set<int>(); }
bool CPluginManager::IsPluginNameInUse(QString& name) { return false; }
void CPluginManager::GetTags(QStringList& tags) { }
#endif

const std::unordered_map<int, Plugin>& CPluginManager::GetPlugins()
{
    return imp->m_plugins;
}

Plugin* CPluginManager::GetPlugin(int id)
{
    if(imp->m_plugins.count(id) != 0)
    {
        return &imp->m_plugins[id];
    }
    else
    {
        return nullptr;
    }
}

Plugin* CPluginManager::AddPlugin(int id)
{
    if(imp->m_plugins.count(id) == 0)
    {
        Plugin plugin;
        plugin.id = id;
        imp->m_plugins.insert({id, plugin});
    }
    return &imp->m_plugins[id];
}

Plugin* CPluginManager::GetPluginFromAllocatorID(int allocId)
{
    for(auto& [id, plugin] : imp->m_plugins)
    {
        if(plugin.allocatorID == allocId) return &plugin;
    }

    return nullptr;
}

void CPluginManager::DownloadPlugin(int id)
{
    DeletePlugin(id);

    Plugin* plugin = GetPlugin(id);
    if(plugin) plugin->status = PLUGIN_DOWNLOADING;

    imp->m_repo.getPluginFiles(id, 0, imp->m_develop);
}

bool CPluginManager::DeletePlugin(int id)
{
    if(imp->m_plugins.count(id) == 0)
    {
        assert(false);
        return false; // Plugin does not exist
    }

    Plugin& plugin = imp->m_plugins[id];

    UnloadPlugin(id);

    for (auto& path : plugin.files)
    {
        QFile file(path.c_str());
        if (file.exists())
        {
            if(!file.remove())
            {
                return false;
            }
        }
    }

    plugin.localCopy = false;
    plugin.loaded = false;
    plugin.localVersion.clear();
    plugin.localFebioVersion.clear();
    plugin.localTimeStamp = 0;
    plugin.files.clear();
    plugin.status = PLUGIN_NOT_INSTALLED;

    imp->m_xml.WriteXML();

    return true;
}

#ifndef UPDATER
bool CPluginManager::LoadPlugin(int id)
{
    if(imp->m_plugins.count(id) == 0)
    {
        assert(false);
        return false; // Plugin does not exist
    }

    Plugin& plugin = imp->m_plugins[id];

    if(!LoadFEBioPlugin(plugin)) return false;
    
    plugin.loaded = true;

    return true;
}

bool CPluginManager::UnloadPlugin(int id)
{
    if(imp->m_plugins.count(id) == 0)
    {
        assert(false);
        return false; // Plugin does not exist
    }

    Plugin& plugin = imp->m_plugins[id];

    if(plugin.files.size() > 0)
    {
        QFileInfo fileInfo(plugin.files[0].c_str());

        FEBioPluginManager* pm = FEBioPluginManager::GetInstance(); assert(pm);
        if(!pm->UnloadPlugin(fileInfo.fileName().toStdString())) return false;
    }

    plugin.loaded = false;

    return true;
}

bool CPluginManager::LoadNonRepoPlugin(std::string& path)
{
    bool success = LoadPluginFile(path);

    if(success)
    {
        FEBioPluginManager* pm = FEBioPluginManager::GetInstance();
        const FEBioPlugin& pl = pm->GetPlugin(pm->Plugins() - 1);

        Plugin* plugin = nullptr;
        
        // Check to see if we are already tracking a plugin with this name
        QFileInfo info(path.c_str());
        std::string filename = info.fileName().toStdString();
        for(auto& [id, pl] : imp->m_plugins)
        {
            if(id < 0 && filename == pl.name)
            {
                plugin = &pl;
            }
        }

        // If we're not, add a new plugin
        if(!plugin)plugin = AddNonRepoPlugin();

        plugin->name = pl.GetName();
        plugin->files.push_back(path);
        plugin->localCopy = true;
        plugin->loaded = true;
        plugin->status = PLUGIN_LOCAL;
        plugin->allocatorID = pl.GetAllocatorID();
    }

    imp->m_xml.WriteXML();

    return success;
}

bool CPluginManager::RemoveNonRepoPlugin(int id)
{
    if(!UnloadPlugin(id)) return false;

    if(imp->m_plugins.count(id) == 0)
    {
        assert(false);
        return false; // Plugin does not exist
    }

    imp->m_plugins.erase(id);

    imp->m_xml.WriteXML();

    return true;
}

void CPluginManager::AddPublication(int pluginID, const QVariantMap& data)
{
    if(imp->m_plugins.count(pluginID) == 0)
    {
        return; // Plugin does not exist
    }

    imp->m_plugins[pluginID].publications.push_back(data);
}

void CPluginManager::AddTag(int pluginID, const std::string& tag)
{
   if(imp->m_plugins.count(pluginID) == 0)
   {
       return; // Plugin does not exist
   }

   imp->m_plugins[pluginID].tags.push_back(tag);
}
#else
bool CPluginManager::LoadPlugin(int id) { return false; }
bool CPluginManager::UnloadPlugin(int id) { return false; }
bool CPluginManager::LoadNonRepoPlugin(std::string& path) { return false; }
bool CPluginManager::RemoveNonRepoPlugin(int id) { return false; }
void CPluginManager::AddPublication(int pluginID, const QVariantMap& data) {}
void CPluginManager::AddTag(int pluginID, const std::string& tag) {}
#endif

void CPluginManager::AddPluginFile(int pluginID, const std::string& filePath, int main, const std::string& version, 
    const std::string& febioVersion, uint64_t timestamp)
{
    if(imp->m_plugins.count(pluginID) == 0)
    {
        return; // Plugin does not exist
    }

    Plugin& plugin = imp->m_plugins[pluginID];
    plugin.localCopy = true;
    plugin.localVersion = version;
    plugin.localFebioVersion = febioVersion;
    plugin.localTimeStamp = timestamp;

    plugin.files.push_back(filePath);

    if(main == 1)
    {
        plugin.mainFileIndex = plugin.files.size() - 1; // Set the last file as the main file
    }
}

void CPluginManager::OnConnectionFinished()
{
    imp->m_status = CONNECTED;

    ReadDatabase();
}

void CPluginManager::OnDownloadFinished(int id)
{
    imp->m_xml.WriteXML();

    if(imp->m_plugins.count(id) == 0)
    {
        assert(false);
        return; // Plugin does not exist
    }

    Plugin& plugin = imp->m_plugins[id];
    plugin.downloads++;

    if(LoadFEBioPlugin(plugin)) plugin.loaded = true;

    SetPluginStatus(plugin);

    emit DownloadFinished(id);
}

void CPluginManager::OnHTMLError(QString& message, int pluginID)
{
    if(imp->m_plugins.count(pluginID) != 0)
    {
        SetPluginStatus(imp->m_plugins[pluginID]);
    }

    imp->m_status = ERROR;

    emit HTMLError(message);
}

void CPluginManager::SumbitPlugin(QByteArray& pluginInfo)
{
    imp->m_repo.sumbitPlugin(pluginInfo);
}

void CPluginManager::UploadImage(QByteArray& token, QString& filename)
{
    imp->m_repo.uploadImage(token, filename);
}

Plugin* CPluginManager::AddNonRepoPlugin()
{
    Plugin plugin;
    plugin.id = imp->m_localID;
    plugin.status = PLUGIN_LOCAL;
    plugin.description = "This plugin is not part of the repository. It was loaded locally from your machine.";
    imp->m_plugins.insert({imp->m_localID, plugin});

    imp->m_localID--;

    return &imp->m_plugins[imp->m_localID + 1];
}

void CPluginManager::WriteConfigFile(const std::string& fileName)
{
    imp->m_xml.WriteConfigFile(fileName);
}

#ifndef UPDATER

void CPluginManager::AddRepoPlugin(char** argv)
{
    int id = std::stoi(argv[0]);

    Plugin plugin;
    
    if(imp->m_plugins.count(id) > 0)
    {
        plugin = imp->m_plugins.at(id);
    }    

    plugin.id = id;
    plugin.name = argv[1];
    plugin.repoName = argv[2];
    plugin.owner = argv[3];
    plugin.description = argv[4];
    plugin.imageData = argv[5];
    plugin.downloads = std::stoi(argv[6]);

    imp->m_plugins[id] = plugin;
}

// Ensure that this plugin manager agrees with the FEBioPluginManager
// about what is loaded
void CPluginManager::SyncWithFEBioPluginManager()
{  
    // Set everything as unloaded since the FEBio plugin manager is 
    // the authority on that matter. We'll set the loaded flag on the
    // loaded plugins later
    for(auto& [id, plugin] : imp->m_plugins)
    {
        plugin.loaded = false;
    }

    FEBioPluginManager* pm = FEBioPluginManager::GetInstance(); assert(pm);
    for(int i = 0; i < pm->Plugins(); i++)
    {
        const FEBioPlugin& fbPlugin = pm->GetPlugin(i);

        bool foundPlugin = false;

        for(auto& [id, plugin] : imp->m_plugins)
        {
            if(plugin.localCopy && plugin.files.size() > 0)
            {
                if(fbPlugin.GetFilePath() == plugin.files[plugin.mainFileIndex])
                {
                    plugin.loaded = true;
                    foundPlugin = true;
                    break;
                }
            }
        }

        if(!foundPlugin)
        {
            Plugin* nrPlugin = AddNonRepoPlugin();
            nrPlugin->name = fbPlugin.GetName();
            nrPlugin->files.push_back(fbPlugin.GetFilePath());
            nrPlugin->allocatorID = fbPlugin.GetAllocatorID();
            nrPlugin->localCopy = true;
            nrPlugin->loaded = true;
        }
    }
}

bool CPluginManager::LoadPluginFile(std::string& path)
{
    // get the currently active module
	// We need this, since importing the plugin might change this.
	FECoreKernel& fecore = FECoreKernel::GetInstance();

    FEModule* activeMod = fecore.GetActiveModule();
	int modId = -1;
	if (activeMod) modId = activeMod->GetModuleID();

	// try to import the plugin
	bool success = febio::ImportPlugin(path.c_str());

    // restore active module
	fecore.SetActiveModule(modId);

    return success;
}

bool CPluginManager::LoadFEBioPlugin(Plugin& plugin)
{
	std::string sfile = plugin.files[plugin.mainFileIndex];
	
    bool success = LoadPluginFile(sfile);

    if(success)
    {
        FEBioPluginManager* pm = FEBioPluginManager::GetInstance();
        const FEBioPlugin& pl = pm->GetPlugin(pm->Plugins() - 1);

        plugin.allocatorID = pl.GetAllocatorID();
    }

	return success;
}

void CPluginManager::SetPluginStatus(Plugin& plugin)
{
    if(plugin.id < 0)
    {
        plugin.status = PLUGIN_LOCAL;
        return;
    }

    std::string currentFEBioVersion = std::to_string(FE_SDK_MAJOR_VERSION) + "." 
        + std::to_string(FE_SDK_SUB_VERSION) + "." + std::to_string(FE_SDK_SUBSUB_VERSION);

    std::vector<std::pair<std::string, uint64_t>> dbVersions = imp->m_db.GetPluginVersions(plugin.id, imp->m_develop);

    if(dbVersions.empty())
    {
        plugin.status = PLUGIN_UNAVAILABLE;
        return;
    }

    if(!plugin.localCopy)
    {
        plugin.status = PLUGIN_NOT_INSTALLED;
        return;
    }
    else
    {
        for(std::string& file : plugin.files)
        {
            // if the local copy doesn't exist, something is wrong
            if(!QFileInfo::exists(file.c_str()))
            {
                plugin.status = PLUGIN_BROKEN;
                return;
            }
        }
    }

    if(IsVersion2Newer(plugin.localFebioVersion, currentFEBioVersion))
    {
        plugin.status = PLUGIN_OUT_OF_DATE;
        return;
    }

    for(auto& version : dbVersions)
    {
        if(IsVersion2Newer(plugin.localVersion, version.first))
        {
            plugin.status = PLUGIN_OUT_OF_DATE; // We have a newer version of the plugin
            return;
        }
    }

    for(auto& version : dbVersions)
    {
        if((plugin.localVersion == version.first) && (plugin.localTimeStamp < version.second))
        {
            plugin.status = PLUGIN_OUT_OF_DATE; // We have a newer version of the plugin
            return;
        }
    }

    plugin.status = PLUGIN_UP_TO_DATE;
}

#else
void CPluginManager::AddRepoPlugin(char** argv) {}
void CPluginManager::SyncWithFEBioPluginManager() {};
bool CPluginManager::LoadFEBioPlugin(Plugin& plugin) { return false; }
void CPluginManager::SetPluginStatus(Plugin& plugin) {}
#endif

// Compare two version strings (e.g., "1.2.3" and "1.2.4")
// Returns true if version2 is newer than version1, false otherwise
bool CPluginManager::IsVersion2Newer(const std::string& version1, const std::string& version2)
{
    if(version1 == version2) return false;

    // Split the version strings into parts
    std::vector<int> parts1, parts2;
    std::istringstream iss1(version1);
    std::istringstream iss2(version2);
    std::string part;
    
    while(std::getline(iss1, part, '.'))
    {
        parts1.push_back(std::stoi(part));
    }

    while(std::getline(iss2, part, '.'))
    {
        parts2.push_back(std::stoi(part));
    }

    // Compare the version parts
    for(size_t i = 0; i < std::max(parts1.size(), parts2.size()); ++i)
    {
        int part1 = (i < parts1.size()) ? parts1[i] : 0;
        int part2 = (i < parts2.size()) ? parts2[i] : 0;
        
        if(part1 > part2) 
        {
            return false; // version1 is newer
        }
        else if(part1 < part2)
        {
            return true; // version2 is newer
        }
    }
    
    return false; // versions are equal
}