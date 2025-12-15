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
#include "ImageFilter.h"
#include <FSCore/ClassDescriptor.h>
#include <ImageLib/ImageModel.h>
#include <ImageLib/ImageSource.h>
#include "3DImage.h"
#include <PostGL/GLModel.h>
#include <MeshLib/FSFindElement.h>
#include "ImageFilterSITK.h"
#include <limits>

REGISTER_CLASS(ThresholdImageFilter, CLASS_IMAGE_FILTER, "Threshold Filter", 0);
REGISTER_CLASS(PadImageFilter, CLASS_IMAGE_FILTER, "Padding Filter", 0);

#ifdef HAS_ITK
REGISTER_CLASS(MeanImageFilter, CLASS_IMAGE_FILTER, "Mean Filter", 0);
REGISTER_CLASS(GaussianImageFilter, CLASS_IMAGE_FILTER, "Gaussian Filter", 0);
REGISTER_CLASS(AdaptiveHistogramEqualizationFilter, CLASS_IMAGE_FILTER, "Adaptive Histogram Equalization", 0);
#endif

CImageFilter::CImageFilter() : m_model(nullptr)
{

}

void CImageFilter::SetImageModel(CImageModel* model)
{
    m_model = model;
}

CImageModel* CImageFilter::GetImageModel()
{
	return m_model;
}

ThresholdImageFilter::ThresholdImageFilter()
{
    static int n = 1;

    char sz[64];
    sprintf(sz, "ThresholdImageFilter%02d", n);
    n += 1;
    SetName(sz);

    AddDoubleParam(255, "max");
    AddDoubleParam(0, "min");
}

template<class pType> void ThresholdImageFilter::FilterTemplate()
{
    C3DImage* image = m_model->GetImageSource()->Get3DImage();

    double max = GetFloatValue(0);
    double min = GetFloatValue(1);

    if(min >= max) return;

    pType* originalBytes = (pType*)image->GetBytes();

    int nx = image->Width();
	int ny = image->Height();
	int nz = image->Depth();
    uint8_t* dest_buf = new uint8_t[nx * ny * nz * image->BPS()];
    pType* filteredBytes = (pType*)dest_buf;
    
    for(int i = 0; i < image->Width()*image->Height()*image->Depth(); i++)
    {
        if(originalBytes[i] > max || originalBytes[i] < min)
        {
            filteredBytes[i] = 0;
        }
        else
        {
            filteredBytes[i] = originalBytes[i];
        }

    }

    C3DImage* imageToFilter = m_model->GetImageSource()->GetImageToFilter();
    imageToFilter->Create(nx, ny, nz, dest_buf, image->PixelType());
}

void ThresholdImageFilter::ApplyFilter()
{
    if(!m_model) return;

    C3DImage* image = m_model->GetImageSource()->Get3DImage();

    if(!image) return;

    switch (image->PixelType())
    {
    case CImage::UINT_8:
        FilterTemplate<uint8_t>();
        break;
    case CImage::INT_8:
        FilterTemplate<int8_t>();
        break;
    case CImage::UINT_16:
        FilterTemplate<uint16_t>();
        break;
    case CImage::INT_16:
        FilterTemplate<int16_t>();
        break;
    case CImage::UINT_32:
        FilterTemplate<uint32_t>();
        break;
    case CImage::INT_32:
        FilterTemplate<int32_t>();
        break;
    case CImage::UINT_RGB8:
        FilterTemplate<uint8_t>();
        break;
    case CImage::INT_RGB8:
        FilterTemplate<int8_t>();
        break;
    case CImage::UINT_RGB16:
        FilterTemplate<uint16_t>();
        break;
    case CImage::INT_RGB16:
        FilterTemplate<int16_t>();
        break;
    case CImage::REAL_32:
        FilterTemplate<float>();
        break;
    case CImage::REAL_64:
        FilterTemplate<double>();
        break;
    default:
        assert(false);
    }
}

void ThresholdImageFilter::SetImageModel(CImageModel* model)
{
    if(model && model->Get3DImage())
    {
        double min, max;
        model->Get3DImage()->GetMinMax(min, max);
        SetFloatValue(0, max);
        SetFloatValue(1, min);
    }

    CImageFilter::SetImageModel(model);
}

PadImageFilter::PadImageFilter()
{
    static int n = 1;
    char sz[64] = { 0 };
    sprintf(sz, "PadImageFilter%02d", n++);
    SetName(sz);

    
    AddIntParam(0, "pad_x_low", "Lower X Padding");
    AddIntParam(0, "pad_x_up", "Upper X Padding");
    AddIntParam(0, "pad_y_low", "Lower Y Padding");
    AddIntParam(0, "pad_y_up", "Upper Y Padding");
    AddIntParam(0, "pad_z_low", "Lower Z Padding");
    AddIntParam(0, "pad_z_up", "Upper Z Padding");

    AddChoiceParam(0, "value", "Value")->SetEnumNames("Min Used\0Max Used\0Min Possible\0Max Possible\0");
    AddChoiceParam(0, "scale", "Image Scaling")->SetEnumNames("Maintain Size\0Maintain Spacing\0");
}

template<class pType> void PadImageFilter::FilterTemplate()
{
    C3DImage* image = m_model->GetImageSource()->Get3DImage();

    int xLow = GetIntValue(0);
    int xUp = GetIntValue(1);
    int yLow = GetIntValue(2);
    int yUp = GetIntValue(3);
    int zLow = GetIntValue(4);
    int zUp = GetIntValue(5);
    int valueChoice = GetIntValue(6);
    int scaleChoice = GetIntValue(7);

    pType value;

    switch(valueChoice)
    {
        case 0:
        {
            double min, max;
            image->GetMinMax(min, max, true);
            value = (pType)min;
            break;
        }
        case 1:
        {
            double min, max;
            image->GetMinMax(min, max, true);
            value = (pType)max;
            break;
        }
        case 2:
        {
            value = std::numeric_limits<pType>::lowest();
            break;
        }
        case 3:
        {
            value = std::numeric_limits<pType>::max();
            break;
        }
        default:
            assert(false);
    }

    int originalX = image->Width();
    int originalY = image->Height();
    int originalZ = image->Depth();

    int nx = originalX + xUp + xLow;
	int ny = originalY + yUp + yLow;
	int nz = originalZ + zUp + zLow;
    uint8_t* dest_buf = new uint8_t[nx * ny * nz * image->BPS()];
    pType* filteredBytes = (pType*)dest_buf;

    #pragma omp parallel for
    for(int index = 0; index < nx*ny*nz; index++)
    {
        int x = index % nx;
        int y = (index / nx) % ny;
        int z = index / (nx*ny);

        if(x < xLow || x >= nx - xUp || y < yLow || y >= ny - yUp || z < zLow || z >= nz - zUp)
        {
            filteredBytes[index] = value;
        }
        else
        {
            int originalIndex = (x - xLow) + (y - yLow) * originalX + (z - zLow) * originalX * originalY;
            filteredBytes[index] = ((pType*)image->GetBytes())[originalIndex];
        }
    }

    C3DImage* imageToFilter = m_model->GetImageSource()->GetImageToFilter();
    imageToFilter->Create(nx, ny, nz, dest_buf, image->PixelType());

    // Scale the physical dimensions of the image
    if(scaleChoice == 1)
    {
        BOX box = image->GetBoundingBox();

        double xSpacing = box.Width() / originalX;
        double ySpacing = box.Height() / originalY;
        double zSpacing = box.Depth() / originalZ;

        BOX newBox(box.x0 - xLow * xSpacing, box.y0 - yLow * ySpacing, box.z0 - zLow * zSpacing,
            box.x1 + xUp * xSpacing, box.y1 + yUp * ySpacing, box.z1 + zUp * zSpacing);

        imageToFilter->SetBoundingBox(newBox);
    }
}

void PadImageFilter::ApplyFilter()
{
    if(!m_model) return;

    C3DImage* image = m_model->GetImageSource()->Get3DImage();

    if(!image) return;

    switch (image->PixelType())
    {
    case CImage::UINT_8:
        FilterTemplate<uint8_t>();
        break;
    case CImage::INT_8:
        FilterTemplate<int8_t>();
        break;
    case CImage::UINT_16:
        FilterTemplate<uint16_t>();
        break;
    case CImage::INT_16:
        FilterTemplate<int16_t>();
        break;
    case CImage::UINT_32:
        FilterTemplate<uint32_t>();
        break;
    case CImage::INT_32:
        FilterTemplate<int32_t>();
        break;
    case CImage::UINT_RGB8:
        FilterTemplate<uint8_t>();
        break;
    case CImage::INT_RGB8:
        FilterTemplate<int8_t>();
        break;
    case CImage::UINT_RGB16:
        FilterTemplate<uint16_t>();
        break;
    case CImage::INT_RGB16:
        FilterTemplate<int16_t>();
        break;
    case CImage::REAL_32:
        FilterTemplate<float>();
        break;
    case CImage::REAL_64:
        FilterTemplate<double>();
        break;
    default:
        assert(false);
    }
}


WarpImageFilter::WarpImageFilter(Post::CGLModel* glm) 
    : m_glm(glm)
{
	static int n = 1;
	char sz[64] = { 0 };
	sprintf(sz, "WarpImageFilter%02d", n++);
	SetName(sz);

	AddBoolParam(true, "scale_dim", "Scale dimensions");
}

template<class pType> void WarpImageFilter::FilterTemplate()
{
    if ((m_model == nullptr) || (m_glm == nullptr)) return;
	CImageModel* mdl = m_model;

	C3DImage* im = mdl->Get3DImage();
	pType* src = (pType*)im->GetBytes();

	Post::CGLModel& gm = *m_glm;
	Post::FEState* state = gm.GetActiveState();
	Post::FERefState* ps = state->m_ref;

	// get the original bounding box
	BOX box0 = mdl->GetBoundingBox();

	// get the (current) bounding box of the mesh
	FSMesh* mesh = gm.GetActiveMesh();
	BOX box = mesh->GetBoundingBox();

	// calculate scale factors
	double sx = box.Width() / box0.Width();
	double sy = box.Height() / box0.Height();
	double sz = (im->Depth() == 1 ? 1.0 : box.Depth() / box0.Depth());

	double w = box.Width();
	double h = box.Height();
	double d = box.Depth();

	bool dimScale = GetBoolValue(SCALE_DIM);

	int nx = (dimScale ? (int)(sx*im->Width ()) : im->Width ());
	int ny = (dimScale ? (int)(sy*im->Height()) : im->Height());
	int nz = (dimScale ? (int)(sz*im->Depth ()) : im->Depth ());

	uint8_t* dst_buf = new uint8_t[nx * ny * nz * im->BPS()];
	pType* dst = (pType*)dst_buf;

	double wx = (nx < 2 ? 0 : 1.0 / (nx - 1.0));
	double wy = (ny < 2 ? 0 : 1.0 / (ny - 1.0));
	double wz = (nz < 2 ? 0 : 1.0 / (nz - 1.0));

	box.Scale(0.9999f); // scale a bit to avoid roundoff errors near the edges.
	vec3d r0 = box.r0();
	vec3d r1 = box.r1();

	if (im->Depth() == 1)
	{
        #pragma omp parallel for
		for (int j = 0; j < ny; ++j)
		{
            int index = j*nx;
			for (int i = 0; i < nx; ++i)
			{
				// get the spatial coordinates of the voxel
				double x = r0.x + (r1.x - r0.x) * (i * wx);
				double y = r0.y + (r1.y - r0.y) * (j * wy);

				// find which element this belongs to
				int elem = -1;
				double q[2] = { 0 };
				vec2d p(x, y);
				if (FindElement2D(p, elem, q, mesh))
				{
					// map to reference configuration
					FSElement& el = mesh->Element(elem);
					int ne = el.Nodes();
					vec3f r[FSElement::MAX_NODES];
					for (int j = 0; j < el.Nodes(); ++j)
					{
						r[j] = ps->m_Node[el.m_node[j]].m_rt;
					}

					// sample 
					vec3f s = el.eval(r, q[0], q[1]);
					pType b = im->ValueAtGlobalPos(to_vec3d(s));
					dst[index+i] = b;
				}
				else
				{
					dst[index+i] = 0;
				}
			}
		}
	}
	else
	{
		FSFindElement fe(*mesh);
		fe.Init();

		// 3D case
		for (int k = 0; k < nz; ++k)
		{
            #pragma omp parallel for
			for (int j = 0; j < ny; ++j)
			{
                int index = k*ny*nx+j*nx;

				for (int i = 0; i < nx; ++i)
				{
					// get the spatial coordinates of the voxel
					double x = r0.x + (r1.x - r0.x) * (i * wx);
					double y = r0.y + (r1.y - r0.y) * (j * wy);
					double z = r0.z + (r1.z - r0.z) * (k * wz);

					// find which element this belongs to
					int elem = -1;
					double q[3] = { 0 };
					if (fe.FindElement(vec3f(x, y, z), elem, q))
					{
						// map to reference configuration
						FSElement& el = mesh->Element(elem);
						int ne = el.Nodes();
						vec3f p[FSElement::MAX_NODES];
						for (int j = 0; j < el.Nodes(); ++j)
						{
							p[j] = ps->m_Node[el.m_node[j]].m_rt;
						}

						// sample 
						vec3f s = el.eval(p, q[0], q[1], q[2]);
						pType b = im->ValueAtGlobalPos(to_vec3d(s));
						dst[index+i] = b;
					}
					else
					{
						dst[index+i] = 0;
					}
				}
			}
		}
	}

	C3DImage* im2 = mdl->GetImageSource()->GetImageToFilter();
	im2->Create(nx, ny, nz, dst_buf, im->PixelType());

	// update the model's box
	im2->SetBoundingBox(box);
}

void WarpImageFilter::ApplyFilter()
{
	if(!m_model) return;

    C3DImage* image = m_model->GetImageSource()->Get3DImage();

    if(!image) return;

    switch (image->PixelType())
    {
    case CImage::UINT_8:
        FilterTemplate<uint8_t>();
        break;
    case CImage::INT_8:
        FilterTemplate<int8_t>();
        break;
    case CImage::UINT_16:
        FilterTemplate<uint16_t>();
        break;
    case CImage::INT_16:
        FilterTemplate<int16_t>();
        break;
    case CImage::UINT_32:
        FilterTemplate<uint32_t>();
        break;
    case CImage::INT_32:
        FilterTemplate<int32_t>();
        break;
    case CImage::UINT_RGB8:
        FilterTemplate<uint8_t>();
        break;
    case CImage::INT_RGB8:
        FilterTemplate<int8_t>();
        break;
    case CImage::UINT_RGB16:
        FilterTemplate<uint16_t>();
        break;
    case CImage::INT_RGB16:
        FilterTemplate<int16_t>();
        break;
    case CImage::REAL_32:
        FilterTemplate<float>();
        break;
    case CImage::REAL_64:
        FilterTemplate<double>();
        break;
    default:
        assert(false);
    }
}

