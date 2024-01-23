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
#include "STEPImport.h"
#include <GeomLib/GOCCObject.h>
#include <GeomLib/GModel.h>
#ifdef HAS_OCC
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <STEPControl_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BOPAlgo_Builder.hxx>
#include <BOPAlgo_MakerVolume.hxx>
#endif


//=============================================================================
STEPImport::STEPImport(FSProject& prj) : FSFileImport(prj)
{
}

STEPImport::~STEPImport()
{
}

bool STEPImport::Load(const char* szfile)
{
#ifdef HAS_OCC
	SetFileName(szfile);
	STEPControl_Reader aReader;
	IFSelect_ReturnStatus status = aReader.ReadFile(szfile);
	if (status != IFSelect_RetDone)
	{
		return errf("Failed reading STEP file.");
	}

	//Interface_TraceFile::SetDefault();
	bool failsonly = false;
	aReader.PrintCheckLoad(failsonly, IFSelect_ItemsByEntity);

	int nbr = aReader.NbRootsForTransfer();
	aReader.PrintCheckTransfer(failsonly, IFSelect_ItemsByEntity);
	for (Standard_Integer n = 1; n <= nbr; n++)
	{
		aReader.TransferRoot(n);
	}

	int nbs = aReader.NbShapes();
	if (nbs > 0)
	{
        // empty compound, as nothing has been added yet
        BOPAlgo_MakerVolume aBuilder;
		int count = 1;
        int ns = 0;
		for (int i = 1; i <= nbs; i++)
		{
			TopoDS_Shape shape = aReader.Shape(i);

			// load each solid as an own object
			TopExp_Explorer ex;
            bool found_solid = false;
			for (ex.Init(shape, TopAbs_SOLID); ex.More(); ex.Next())
			{
				// get the shape
				TopoDS_Solid solid = TopoDS::Solid(ex.Current());
                aBuilder.AddArgument(solid);
                
                ++ns;

				GOCCObject* occ = new GOCCObject;
				occ->SetShape(solid);

				char szfiletitle[1024] = { 0 }, szname[1024] = { 0 };
				FileTitle(szfiletitle);

				sprintf(szname, "%s%02d", szfiletitle, count++);
				occ->SetName(szname);

				GModel& mdl = m_prj.GetFSModel().GetModel();
				mdl.AddObject(occ);

                found_solid = true;
			}
            if (!found_solid) {
                for (ex.Init(shape, TopAbs_SHELL); ex.More(); ex.Next())
                {
                    // get the shape
                    TopoDS_Shell shell = TopoDS::Shell(ex.Current());
                    
                    GOCCObject* occ = new GOCCObject;
                    occ->SetShape(shell);
                    
                    char szfiletitle[1024] = { 0 }, szname[1024] = { 0 };
                    FileTitle(szfiletitle);
                    
                    sprintf(szname, "%s%02d", szfiletitle, count++);
                    occ->SetName(szname);
                    
                    GModel& mdl = m_prj.GetFSModel().GetModel();
                    mdl.AddObject(occ);
                    
                }
            }
		}
        // merge all solids
        if (ns > 1) {
            aBuilder.SetIntersect(true);
            aBuilder.SetAvoidInternalShapes(false);
            aBuilder.Perform();
            TopoDS_Shape solid = aBuilder.Shape();
            GOCCObject* occ = new GOCCObject;
            occ->SetShape(solid);
            
            char szfiletitle[1024] = { 0 }, szname[1024] = { 0 };
            FileTitle(szfiletitle);
            
            sprintf(szname, "%s_merged", szfiletitle);
            occ->SetName(szname);
            
            GModel& mdl = m_prj.GetFSModel().GetModel();
            mdl.AddObject(occ);
        }
	}

	return true;
#else
	return errf("STEP reader not supported in this build.");
#endif
}
