#pragma once
#include <MeshTools/FEFileExport.h>
#include <XML/XMLWriter.h>
#include "FEBioException.h"
#include <MeshTools/FEProject.h>

// export sections
enum FEBioExportSections
{
	FEBIO_MODULE,
	FEBIO_CONTROL,
	FEBIO_GLOBAL,
	FEBIO_MATERIAL,
	FEBIO_GEOMETRY,
	FEBIO_MESHDATA,			// new in FEBio 2.5
	FEBIO_BOUNDARY,
	FEBIO_LOADS,
	FEBIO_INITIAL,
	FEBIO_CONSTRAINTS,
	FEBIO_CONTACT,			// new in FEBio 2.0
	FEBIO_DISCRETE,			// new in FEBio 2.0
	FEBIO_LOADDATA,
	FEBIO_OUTPUT,
	FEBIO_STEPS,
	FEBIO_MAX_SECTIONS		// = max nr of sections
};

class FEBioExport : public FEFileExport
{
public:
	FEBioExport(FEProject& prj);

	void SetPlotfileCompressionFlag(bool b);
	void SetExportSelectionsFlag(bool b);

protected:
	void WriteParam(Param& p);
	void WriteParamList(ParamContainer& c);

	virtual void Clear();

	virtual bool PrepareExport(FEProject& prj);

	void WriteNote(FSObject* po);

private:
	void AddLoadCurve(FELoadCurve* plc);
	void AddLoadCurves(ParamContainer& PC);
	void MultiMaterialCurves(FEMaterial* pm);
	void BuildLoadCurveList(FEModel& fem);

protected:
	XMLWriter		m_xml;

	std::vector<FELoadCurve*>		m_pLC;		//!< array of loadcurve pointers

	bool	m_section[FEBIO_MAX_SECTIONS];	//!< write section flags

	bool	m_compress;				//!< compress plot file
	bool	m_exportSelections;		//!< export named selections as well
	bool	m_exportEnumStrings;	//!< export enums as strings (otherwise output numbers)

	bool	m_exportNonPersistentParams;
};
