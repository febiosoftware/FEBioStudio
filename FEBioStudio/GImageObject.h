#pragma once
#include <GeomLib/GObject.h>
#include <MeshLib/box.h>
#include <ImageLib/ImageSlice.h>

class ImageStack;

class GImageObject : public GObject
{
public:
	GImageObject();
	~GImageObject();

	// set the 3d image stack for the object
	void SetImageStack(ImageStack* im);

	// set the physical dimensions of the image
	void SetBox(BOX box);

	// render the images
	void Render();

	// update the image object
	void Update();

private:
	void RenderSlice(int i);

private:
	BOX				m_box;		// physical dimensions of image
	ImageStack*		m_im;		// the actual image data

private:
	RGBAImage	m_slice[3];			// image slices
	int			m_nx, m_ny, m_nz;	// dimensions of resampled images
	unsigned int m_tex[3];
	bool		m_reload[3];
	int			m_off[3];
};
