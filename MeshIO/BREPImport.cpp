#include "stdafx.h"
#include "BREPImport.h"
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

BREPImport::BREPImport(FEProject& prj) : FEFileImport(prj)
{
}

BREPImport::~BREPImport()
{
}

bool BREPImport::Load(const char* szfile)
{
#ifdef HAS_OCC
	FEProject& prj = m_prj;
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
IGESImport::IGESImport(FEProject& prj) : FEFileImport(prj)
{
}

IGESImport::~IGESImport()
{
}

bool IGESImport::Load( const char* szfile)
{
#ifdef HAS_OCC
	FEProject& prj = m_prj;

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
