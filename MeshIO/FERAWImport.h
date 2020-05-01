#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

//-----------------------------------------------------------------------------
//! This class imports an 8-bit 3D RAW image file and converts it to a FE model
//! by making each voxel a hex-element. Different gray scales are assigned to 
//! different partitions.
class FERAWImport : public FEFileImport
{
public:
	FERAWImport(FEProject& prj);
	~FERAWImport();

	//! Set the image dimensions
	void SetImageDimensions(int nx, int ny, int nz);

	//! Set box position and size
	void SetBoxSize(double x0, double y0, double z0, double w, double h, double d);

	//! Load the image data
	bool Load(const char* szfile);

protected:
	int	m_nx, m_ny, m_nz;		//!< image dimensions
	double	m_x0, m_y0, m_z0;	//!< position of box
	double	m_w, m_h, m_d;		//!< size of box
	FEModel*	m_pfem;
};
