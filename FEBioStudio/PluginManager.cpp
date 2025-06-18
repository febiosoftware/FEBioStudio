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
#include <FECore/version.h>
#include <FEBioLib/plugin.h>
#include <FEBioLib/febio.h>
#include <FECore/FEModule.h>
#include <sstream>

class CPluginManager::Imp
{
public:
    Imp(CPluginManager* parent) : m_db(parent), m_xml(parent), m_repo(parent, &m_db) 
    {
        m_xml.SetPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + "/plugins/plugins.xml");
    }

public:
    CPluginDatabaseHandler m_db;
    CPluginXML   m_xml;
    CPluginRepoConnectionHandler m_repo;

    std::unordered_map<int, Plugin> m_plugins;
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

void CPluginManager::LoadAllPlugins()
{
    for(auto& [id, plugin] : imp->m_plugins)
    {
        if(!plugin.loaded)
        {
            if(LoadFEBioPlugin(plugin)) plugin.loaded = true;
        }
    }
}

void CPluginManager::Connect()
{
    imp->m_repo.getSchema();
}

void CPluginManager::ReadDatabase()
{
    imp->m_db.GetPlugins();

    // Ensure that this plugin manager agrees with the FEBioPluginManager
    // about what is loaded
    FEBioPluginManager* pm = FEBioPluginManager::GetInstance(); assert(pm);
    for(auto& [id, plugin] : imp->m_plugins)
    {
        if(plugin.localCopy && plugin.files.size() > 0)
        {
            plugin.loaded = false;

            for(int index = 0; index < pm->Plugins(); index++)
            {
                const FEBioPlugin& fbPlugin = pm->GetPlugin(index);

                if(fbPlugin.GetFilePath() == plugin.files[0])
                {
                    plugin.loaded = true;
                    break;
                }
            }
        }
    }

    for(auto& [id, plugin] : imp->m_plugins)
    {
        plugin.publications.clear();
        imp->m_db.GetPluginPubs(id);
        SetPluginStatus(plugin);
    }

    emit PluginsReady();
}

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

void CPluginManager::DownloadPlugin(int id)
{
    DeletePlugin(id);

    imp->m_repo.getPluginFiles(id);
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
    plugin.files.clear();
    plugin.status = PLUGIN_NOT_INSTALLED;

    imp->m_xml.WriteXML();

    return true;
}

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

void CPluginManager::AddPublication(int pluginID, const QVariantMap& data)
{
    if(imp->m_plugins.count(pluginID) == 0)
    {
        return; // Plugin does not exist
    }

    imp->m_plugins[pluginID].publications.push_back(data);
}

void CPluginManager::AddPluginFile(int pluginID, const std::string& filePath, const std::string& version, const std::string& febioVersion)
{
    if(imp->m_plugins.count(pluginID) == 0)
    {
        return; // Plugin does not exist
    }

    Plugin& plugin = imp->m_plugins[pluginID];
    plugin.localCopy = true;
    plugin.localVersion = version;
    plugin.localFebioVersion = febioVersion;

    plugin.files.push_back(filePath);
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

    emit DownloadFinished();
}

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
    plugin.owner = argv[2];
    plugin.description = argv[3];
    plugin.sourceURL = argv[4];
    plugin.imageData = argv[5];
    plugin.downloads = std::stoi(argv[6]);

    imp->m_plugins[id] = plugin;
}

bool CPluginManager::LoadFEBioPlugin(Plugin& plugin)
{
	std::string sfile = plugin.files[0];

	// get the currently active module
	// We need this, since importing the plugin might change this.
	FECoreKernel& fecore = FECoreKernel::GetInstance();

	FEModule* activeMod = fecore.GetActiveModule();
	int modId = -1;
	if (activeMod) modId = activeMod->GetModuleID();

	// try to import the plugin
	bool bsuccess = febio::ImportPlugin(sfile.c_str());

	// restore active module
	fecore.SetActiveModule(modId);

	return bsuccess;
}

void CPluginManager::SetPluginStatus(Plugin& plugin)
{
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

    std::vector<std::string> dbVersions = imp->m_db.GetPluginVersions(plugin.id);

    if(dbVersions.empty())
    {
        plugin.status = PLUGIN_BROKEN;
        return;
    }

    for(auto& version : dbVersions)
    {
        if(IsVersion2Newer(plugin.localVersion, version))
        {
            plugin.status = PLUGIN_OUT_OF_DATE; // We have a newer version of the plugin
            return;
        }
    }

    plugin.status = PLUGIN_UP_TO_DATE;
}

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
