/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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

#include "stdafx.h"
#include "ModelTypeInfoReader.h"
#include "ModelDocument.h"
#include "ModelFileReader.h"
#include "FEBioStudio.h"
#include <FEBio/FEBioImport.h>
#include <FEMLib/FSModel.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FEAnalysisStep.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include <FEMLib/FEDiscreteMaterial.h>
#include <FEBioLink/FEBioModule.h>
#include <QFileInfo>

#include <iostream>
#include <ios>

bool ModelTypeInfoReader::ReadTypeInfo(std::string filename)
{
    this->filename = filename;

    QString ext = QFileInfo(filename.c_str()).suffix();
	if ((ext.compare("fsm", Qt::CaseInsensitive) == 0) ||
		(ext.compare("fs2", Qt::CaseInsensitive) == 0) ||
		(ext.compare("prv", Qt::CaseInsensitive) == 0) ||
		(ext.compare("fsprj", Qt::CaseInsensitive) == 0))
	{
		ReadFSM();
	}
    else if (ext.compare("feb", Qt::CaseInsensitive) == 0)
	{
        ReadFEB();
    }
    else
    {
        return false;
    }

    return true;    
}

unordered_map<string, unordered_set<string>> ModelTypeInfoReader::GetTypeInfo()
{
    return typeInfo;
}

string ModelTypeInfoReader::GetModule()
{
    return module;
}

void ModelTypeInfoReader::ReadFSM()
{
    CModelDocument doc(FBS::getMainWindow());
	// doc.SetDocFilePath(szfile);

    FSModel* fsModel = doc.GetFSModel();
    fsModel->SetSkipGeometry(true);
    
    ModelFileReader reader(&doc);

    reader.Load(filename.c_str());

    ParseFSModel(doc.GetProject());
}

void ModelTypeInfoReader::ReadFEB()
{
    FSProject prj;
    FEBioFileImport reader(prj);
    reader.SetSkipGeometryFlag(true);

    reader.Load(filename.c_str());

    ParseFSModel(prj);
}

void ModelTypeInfoReader::ParseFSModel(FSProject& prj)
{
    FSModel& fsModel = prj.GetFSModel();

    if(fsModel.Materials() > 0)
    {
        typeInfo["Material"] = unordered_set<string>();

        for(int mat = 0; mat < fsModel.Materials(); mat++)
        {
            string type = fsModel.GetMaterial(mat)->GetMaterialProperties()->GetTypeString();

            if(!type.empty())
            {
                typeInfo["Material"].insert(type);
            }
        }
    }

    GModel& gModel = fsModel.GetModel();

    if(gModel.DiscreteObjects() > 0)
    {
        typeInfo["Discrete"] = unordered_set<string>();

        for(int mat = 0; mat < gModel.DiscreteObjects(); mat++)
        {
            string type;

            switch(gModel.DiscreteObject(mat)->GetType())
            {
                case FE_LINEAR_SPRING_SET: type = "linear spring"; break;
                case FE_NONLINEAR_SPRING_SET: type = "nonlinear spring"; break;
                case FE_DISCRETE_SPRING_SET:
                {
                    type = dynamic_cast<GDiscreteSpringSet*>(gModel.DiscreteObject(mat))->GetMaterial()->GetTypeString();
                    break;
                }
            }

            if(!type.empty())
            {
                typeInfo["Discrete"].insert(type);
            }
        }
    }

    for(int step = 0; step < fsModel.Steps(); step++)
    {
        FSStep* current = fsModel.GetStep(step);

        // Older files might not be using the febio modules, and so the module type
        // is still stored on the step. If that's the case, we just grab it from the
        // first analysis step
        if(module.empty())
        {
            switch (current->GetType())
            {
                case FE_STEP_MECHANICS         : module = "solid"; break;
                case FE_STEP_HEAT_TRANSFER     : module = "heat"; break;
                case FE_STEP_BIPHASIC          : module = "biphasic"; break;
                case FE_STEP_BIPHASIC_SOLUTE   : module = "solute"; break;
                case FE_STEP_MULTIPHASIC       : module = "multiphasic"; break;
                case FE_STEP_FLUID             : module = "fluid"; break;
                case FE_STEP_FLUID_FSI         : module = "fluid-FSI"; break;
                case FE_STEP_REACTION_DIFFUSION: module = "reaction-diffusion"; break;
                case FE_STEP_POLAR_FLUID       : module = "polar fluid"; break;
				case FE_STEP_EXPLICIT_SOLID    : module = "explicit-solid"; break;
                case FE_STEP_FLUID_SOLUTES     : module = "fluid-solutes"; break;
                case FE_STEP_THERMO_FLUID      : module = "thermo-fluid"; break;
				case FE_STEP_FEBIO_ANALYSIS    :
                {
                    int mod = prj.GetModule();
                    module = FEBio::GetModuleName(mod);

                    break;
                }
	        };
        }

        int analysisType = -1;

        FSAnalysisStep* aStep = dynamic_cast<FSAnalysisStep*>(current);
        if(aStep)
        {
            analysisType = aStep->GetSettings().nanalysis;
        }

        FEBioAnalysisStep* febStep = dynamic_cast<FEBioAnalysisStep*>(current);
        if(febStep)
        {
            analysisType = current->GetParam("analysis")->GetIntValue();
        }

        if(analysisType != -1)
        {
            string analysis;
            
            if(module.compare("solid") == 0)
            {
                analysis = analysisType == 0 ? "STATIC" : "DYNAMIC";
            }
            else if(module.compare("biphasic") == 0)
            {
                analysis = analysisType == 0 ? "STEADY-STATE" : "TRANSIENT";
            }
            else if(module.compare("solute") == 0)
            {
                analysis = analysisType == 0 ? "STEADY-STATE" : "TRANSIENT";
            }
            else if(module.compare("multiphasic") == 0)
            {
                analysis = analysisType == 0 ? "STEADY-STATE" : "TRANSIENT";
            }
            else if(module.compare("fluid") == 0)
            {
                analysis = analysisType == 0 ? "STEADY-STATE" : "DYNAMIC";
            }
            else if(module.compare("fluid-FSI") == 0)
            {
                analysis = analysisType == 0 ? "STEADY-STATE" : "DYNAMIC";
            }
            else if(module.compare("polar fluid") == 0)
            {
                analysis = analysisType == 0 ? "STEADY-STATE" : "DYNAMIC";
            }
            else if(module.compare("multiphasic-FSI") == 0)
            {
                analysis = analysisType == 0 ? "STEADY-STATE" : "DYNAMIC";
            }
            else if(module.compare("fluid-solutes") == 0)
            {
                analysis = analysisType == 0 ? "STEADY-STATE" : "DYNAMIC";
            }
            else if(module.compare("fluid-solutes2") == 0)
            {
                analysis = analysisType == 0 ? "STEADY-STATE" : "DYNAMIC";
            }
            else if(module.compare("thermo-fluid") == 0)
            {
                analysis = analysisType == 0 ? "STEADY-STATE" : "DYNAMIC";
            }
            else 
            {
                analysis = analysisType == 0 ? "STATIC" : "DYNAMIC";
            }

            if(typeInfo.find("Analysis") == typeInfo.end())
            {
                typeInfo["Analysis"] = unordered_set<string>();
            }

            typeInfo["Analysis"].insert(analysis);

        }

        if(current->BCs() > 0)
        {
            if(typeInfo.find("Boundary") == typeInfo.end())
            {
                typeInfo["Boundary"] = unordered_set<string>();
            }

            for(int item = 0; item < current->BCs(); item++)
            {
                string type = current->BC(item)->GetTypeString();

                if(!type.empty())
                {
                    typeInfo["Boundary"].insert(type);
                }
            }
        }

        if(current->ICs() > 0)
        {
            if(typeInfo.find("Initial") == typeInfo.end())
            {
                typeInfo["Initial"] = unordered_set<string>();
            }

            for(int item = 0; item < current->ICs(); item++)
            {
                string type = current->IC(item)->GetTypeString();

                if(!type.empty())
                {
                    typeInfo["Initial"].insert(type);
                }
            }
        }

        if(current->Constraints() > 0)
        {
            if(typeInfo.find("Constraints") == typeInfo.end())
            {
                typeInfo["Constraints"] = unordered_set<string>();
            }

            for(int item = 0; item < current->Constraints(); item++)
            {
                string type = current->Constraint(item)->GetTypeString();

                if(!type.empty())
                {
                    typeInfo["Constraints"].insert(type);
                }
            }
        }

        if(current->Interfaces() > 0)
        {
            if(typeInfo.find("Contact") == typeInfo.end())
            {
                typeInfo["Contact"] = unordered_set<string>();
            }

            for(int item = 0; item < current->Interfaces(); item++)
            {
                string type = current->Interface(item)->GetTypeString();

                if(!type.empty())
                {
                    typeInfo["Contact"].insert(type);
                }
            }
        }

        if(current->Loads() > 0)
        {
            if(typeInfo.find("Loads") == typeInfo.end())
            {
                typeInfo["Loads"] = unordered_set<string>();
            }

            for(int item = 0; item < current->Loads(); item++)
            {
                string type = current->Load(item)->GetTypeString();

                if(!type.empty())
                {
                    typeInfo["Loads"].insert(type);
                }
            }
        }

        if(current->RigidConnectors() > 0)
        {
            if(typeInfo.find("Rigid") == typeInfo.end())
            {
                typeInfo["Rigid"] = unordered_set<string>();
            }

            for(int item = 0; item < current->RigidConnectors(); item++)
            {
                string type = current->RigidConnector(item)->GetTypeString();

                if(!type.empty())
                {
                    typeInfo["Rigid"].insert(type);
                }
            }
        }

        if(current->RigidConstraints() > 0)
        {
            if(typeInfo.find("Rigid") == typeInfo.end())
            {
                typeInfo["Rigid"] = unordered_set<string>();
            }

            for(int item = 0; item < current->RigidConstraints(); item++)
            {
                string type = current->RigidConstraint(item)->GetTypeString();

                if(!type.empty())
                {
                    typeInfo["Rigid"].insert(type);
                }
            }
        }

        if(current->RigidLoads() > 0)
        {
            if(typeInfo.find("Rigid") == typeInfo.end())
            {
                typeInfo["Rigid"] = unordered_set<string>();
            }

            for(int item = 0; item < current->RigidLoads(); item++)
            {
                string type = current->RigidLoad(item)->GetTypeString();

                if(!type.empty())
                {
                    typeInfo["Rigid"].insert(type);
                }
            }
        }

    }

    PrintInfoForPython();
}

void ModelTypeInfoReader::PrintInfo()
{
    std::cout << filename << std::endl;

    std::cout << "Module" << std::endl;
    std::cout << "\t" << module << std::endl;

    for(auto& item : typeInfo)
    {
        std::cout << item.first << std::endl;
        for(auto& str : item.second)
        {
            std::cout << "\t" << str << std::endl;
        }
    }

    std::cout << std::endl;
}

void ModelTypeInfoReader::PrintInfoForPython()
{
    QString name = QFileInfo(filename.c_str()).baseName();

    std::cout << "'" << name.toStdString() << "': {";


    std::cout << "'Module': '" << module << "'";

    for(auto& item : typeInfo)
    {
        std::cout << ", '" << item.first << "': {";

        std::vector<string> vecItems;

        for(auto& str : item.second)
        {
            vecItems.push_back(str);
        }

        for(int index = 0; index < vecItems.size(); index++)
        {
            std::cout << "'" << vecItems[index] << "'";

            if(index != vecItems.size() - 1)
            {
                std::cout<< ", ";
            }
        }

        std::cout << "}";
    }

    std::cout << "}, ";

    std::cout << std::endl;
}
