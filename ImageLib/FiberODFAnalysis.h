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

#pragma once

#include "ImageAnalysis.h"
#include <SimpleITK.h>
#include <MeshLib/GLMesh.h>
#include <GLLib/GLMeshRender.h>
#include <FECore/vec3d.h>

namespace sitk = itk::simple;

class matrix;

struct CODF
{
public:
    CODF();

    std::vector<double> m_odf;
    std::vector<double> m_sphHarmonics;
    vec3d m_position;
    GLMesh m_mesh;
    GLMesh m_radialMesh;
    GLMesh m_remesh;
    GLMesh m_radialRemesh;
    double m_radius;

	// fitting parameters
	vec3d	m_meanDir;	// mean direction
	double	m_FA;		// fractional anisotropy
};

class CFiberODFAnalysis : public CImageAnalysis
{
public:
    CFiberODFAnalysis(Post::CImageModel* img);
    ~CFiberODFAnalysis();

    void run() override;
    void render(CGLCamera* cam) override;

    int ODFs() { return m_ODFs.size(); }
    CODF* GetODF(int i);

    bool renderRemeshed();
    bool renderMeshLines();
    bool showRadial();

    bool display() override;

private:
    void clear();

    void butterworthFilter(sitk::Image& img);
    sitk::Image powerSpectrum(sitk::Image& img);
    void fftRadialFilter(sitk::Image& img);
    void reduceAmp(sitk::Image& img, std::vector<double>* reduced);

    std::unique_ptr<matrix> complLapBel_Coef();
    double GFA(std::vector<double>& vals);

    void buildMeshes();
    void remeshSphere(CODF* odf);

	void calculateFits();

	void updateProgress(double f);

private:
    std::vector<CODF*> m_ODFs;

    GLMeshRender m_render;

    double m_lengthScale;
    double m_hausd;
    double m_grad;

	// progress tracking
	int	m_stepsCompleted;
	int	m_totalSteps;
	double	m_progress;
	std::string	m_task;
};
