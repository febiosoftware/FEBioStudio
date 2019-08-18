#include "stdafx.h"
#include "BREPImport.h"
#include <GeomLib/GOCCObject.h>
#ifdef HAS_OCC
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <STEPControl_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#endif

BREPImport::BREPImport()
{
}

BREPImport::~BREPImport()
{
}

bool BREPImport::Load(FEProject& prj, const char* szfile)
{
#ifdef HAS_OCC
	SetFileName(szfile);
	TopoDS_Shape aShape;
	BRep_Builder aBuilder;
	TCollection_AsciiString  aFilePath = szfile;
	Standard_Boolean result = BRepTools::Read(aShape, aFilePath.ToCString(), aBuilder);
	if (result)
	{
		GOCCObject* occ = new GOCCObject;
		occ->SetShape(aShape);

		char szname[256];
		FileTitle(szname);
		occ->SetName(szname);

		GModel& mdl = prj.GetFEModel().GetModel();
		mdl.AddObject(occ);
		return true;
	}
	else return false;
#else
	return false;
#endif
}

//=============================================================================
STEPImport::STEPImport()
{
}

STEPImport::~STEPImport()
{
}

bool STEPImport::Load(FEProject& prj, const char* szfile)
{
#ifdef HAS_OCC
	SetFileName(szfile);
	TCollection_AsciiString  aFilePath = szfile;
	STEPControl_Reader aReader;
	IFSelect_ReturnStatus status = aReader.ReadFile(aFilePath.ToCString());
	if (status != IFSelect_RetDone)
	{
		return false;
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
		for (int i = 1; i <= nbs; i++)
		{
			TopoDS_Shape shape = aReader.Shape(i);

            // load each solid as an own object
            TopExp_Explorer ex;
            for (ex.Init(shape, TopAbs_SOLID); ex.More(); ex.Next())
            {
                // get the shape
                TopoDS_Solid solid = TopoDS::Solid(ex.Current());
                
                GOCCObject* occ = new GOCCObject;
                occ->SetShape(solid);
                
                char szfiletitle[1024] = {0}, szname[1024] = {0};
                FileTitle(szfiletitle);
                
                sprintf(szname, "%s%02d", szfiletitle, i);
                occ->SetName(szname);
                
                GModel& mdl = prj.GetFEModel().GetModel();
                mdl.AddObject(occ);
                
            }
		}
	}

	return true;
#else
	return false;
#endif
}

//=============================================================================
IGESImport::IGESImport()
{
}

IGESImport::~IGESImport()
{
}

bool IGESImport::Load(FEProject& prj, const char* szfile)
{
#ifdef HAS_OCC
	SetFileName(szfile);
	TCollection_AsciiString  aFilePath = szfile;

	IGESControl_Reader Reader;
	int status = Reader.ReadFile(aFilePath.ToCString());

	if (status == IFSelect_RetDone)
	{
		Reader.TransferRoots();
		TopoDS_Shape aShape = Reader.OneShape();

		GOCCObject* occ = new GOCCObject;
		occ->SetShape(aShape);

		char szname[256];
		FileTitle(szname);
		occ->SetName(szname);

		GModel& mdl = prj.GetFEModel().GetModel();
		mdl.AddObject(occ);
	}
	else return false;

	return true; 
#else
	return false;
#endif
}
