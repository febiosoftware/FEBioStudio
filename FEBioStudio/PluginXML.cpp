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
#include <XML/XMLReader.h>
#include <XML/XMLWriter.h>

CPluginXML::CPluginXML(CPluginManager* manager) 
    :  m_manager(manager)
{

}

bool CPluginXML::LoadXML()
{
    XMLReader xml;
    if (xml.Open(m_path.c_str()))
    {
        XMLTag tag;

        if(!xml.FindTag("plugins", tag)) return false;

        if(!tag.isleaf())
        {
            ++tag;
            do
            {
                if(tag == "plugin")
                {
                    int ID = tag.AttributeValue("ID", 0);

                    Plugin* plugin = m_manager->GetPlugin(ID);

                    if(!plugin)
                    {
                        plugin = m_manager->AddPlugin(ID);
                    }

                    plugin->localVersion = tag.AttributeValue("version", std::string());
                    plugin->localFebioVersion = tag.AttributeValue("febioVersion", std::string());

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
                                plugin->files.push_back(filePath);

                                if(main == 1)
                                {
                                    plugin->mainFileIndex = plugin->files.size() - 1; // Set the last file as the main file
                                }

                                plugin->localCopy = true;
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
        return false;
    }

    return true;
}

void CPluginXML::WriteXML()
{
    XMLWriter xml;
    xml.open(m_path.c_str());
    xml.add_branch("plugins");
    const std::unordered_map<int, Plugin>& plugins = m_manager->GetPlugins();

    for (const auto& [id, plugin] : plugins)
    {
        if(plugin.localCopy)
        {
            XMLElement pluginElement("plugin");
            pluginElement.add_attribute("ID", plugin.id);
            pluginElement.add_attribute("version", plugin.localVersion);
            pluginElement.add_attribute("febioVersion", plugin.localFebioVersion);
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
}