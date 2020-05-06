#include "stdafx.h"
#include "STEPImport.h"
#include <GeomLib/GOCCObject.h>
#include <MeshTools/GModel.h>
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


//=============================================================================
STEPImport::STEPImport(FEProject& prj) : FEFileImport(prj)
{
}

STEPImport::~STEPImport()
{
}

bool STEPImport::Load(const char* szfile)
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
		int count = 1;
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

				char szfiletitle[1024] = { 0 }, szname[1024] = { 0 };
				FileTitle(szfiletitle);

				sprintf(szname, "%s%02d", szfiletitle, count++);
				occ->SetName(szname);

				GModel& mdl = m_prj.GetFEModel().GetModel();
				mdl.AddObject(occ);

			}
		}
	}

	return true;
#else
	return false;
#endif
}
