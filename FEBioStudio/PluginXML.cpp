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

#include "PluginXML.h"
#include "PluginManager.h"

#ifdef UPDATER
    #include <XMLReader.h>
    #include <XMLWriter.h>
#else
    #include <FEBioXML/XMLReader.h>
    #include <FEBioXML/XMLWriter.h>
#endif

CPluginXML::CPluginXML(CPluginManager* manager) 
    :  m_manager(manager), m_busy(false)
{

}

bool CPluginXML::LoadXML()
{
    if(m_busy) return false;

    m_busy = true;
    XMLReader xml;
    if (xml.Open(m_path.c_str()))
    {
        XMLTag tag;

        if(!xml.FindTag("plugins", tag))
        {
            m_busy = false;
            return false;
        }

        if(!tag.isleaf())
        {
            ++tag;
            do
            {
                if(tag == "plugin")
                {
                    int ID = tag.AttributeValue("ID", 0);

                    // If the plugin is a repo plugin, it will have a positive ID,
                    // and will get added to the manager. If it's not, the ID will 
                    // be negative, and this pointer remains null
                    Plugin* plugin = nullptr;

                    if(ID > 0)
                    {
                        plugin = m_manager->GetPlugin(ID);

                        if(!plugin)
                        {
                            plugin = m_manager->AddPlugin(ID);
                        }
                    }

                    // Only do this for repo plugins
                    if(plugin)
                    {
                        plugin->localVersion = tag.AttributeValue("version", std::string());
                        plugin->localFebioVersion = tag.AttributeValue("febioVersion", std::string());
                        plugin->localTimeStamp = std::stoull(tag.AttributeValue("timestamp", std::string("0")));
                    }

                    if(!tag.isleaf())
                    {
                        ++tag;
                        do
                        {
                            if (tag == "file")
                            {
                                int main = tag.AttributeValue("main", 1);

                                std::string filePath;
                                tag.value(filePath);

                                // Only do this for repo plugins
                                if(plugin)
                                {
                                    plugin->files.push_back(filePath);

                                    if(main == 1)
                                    {
                                        // Set the last file as the main file
                                        plugin->mainFileIndex = plugin->files.size() - 1;
                                    }

                                    plugin->localCopy = true;
                                }
                                // For nonrepo plugins we load them using this function which 
                                // initializes them properly. LoadNonRepoPlugin has an empty 
                                // definition when building the updater, so we don't have to 
                                // worry about the updater trying to fetch these
                                else
                                {
                                    m_manager->LoadNonRepoPlugin(filePath);
                                }
                                
                            }

                            ++tag;
                        } while(!tag.isend());
                    }
                }
                ++tag;
            } while(!tag.isend());
        }
    }
    else
    {
        m_busy = false;
        return false;
    }

    m_busy = false;
    return true;
}

void CPluginXML::WriteXML()
{
    if(m_busy) return;

    m_busy = false;

    XMLWriter xml;
    xml.open(m_path.c_str());
    xml.add_branch("plugins");
    const std::unordered_map<int, Plugin>& plugins = m_manager->GetPlugins();

    for (const auto& [id, plugin] : plugins)
    {
        int plID = plugin.id;

        if(plugin.localCopy)
        {
            XMLElement pluginElement("plugin");
            pluginElement.add_attribute("ID", plID);
            pluginElement.add_attribute("version", plugin.localVersion);
            pluginElement.add_attribute("febioVersion", plugin.localFebioVersion);
            pluginElement.add_attribute("timestamp", std::to_string(plugin.localTimeStamp));
            xml.add_branch(pluginElement);

            for(int index = 0; index < plugin.files.size(); ++index)
            {
                const std::string& file = plugin.files[index];
                XMLElement fileElement("file");
                fileElement.add_attribute("main", (index == plugin.mainFileIndex) ? 1 : 0);
                fileElement.value(file);
                xml.add_leaf(fileElement);
            }
            
            xml.close_branch(); // Close Plugin branch
        }  
    }
    xml.close_branch(); // Close plugins branch
    xml.close(); // Close the XML file

    m_busy = false;
}

void CPluginXML::WriteConfigFile(const std::string& fileName)
{
    XMLWriter xml;
    xml.open(fileName.c_str());
    
    XMLElement febioConfig("febio_config");
    febioConfig.add_attribute("version", "3.0");
    xml.add_branch(febioConfig);

    const std::unordered_map<int, Plugin>& plugins = m_manager->GetPlugins();

    for (const auto& [id, plugin] : plugins)
    {
        if(plugin.localCopy)
        {
            XMLElement import("import");
            import.value(plugin.files[plugin.mainFileIndex]);
            xml.add_leaf(import);
        }  
    }

    xml.close_branch(); // Close febio_config branch
    xml.close(); // Close the XML file
}