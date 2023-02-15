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
#include <GLWLib/GLWidget.h>
#include <PostLib/ColorMap.h>

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
	GLMesh m_remesh;
	double m_radius;

	vector<double>	newODF;
	vector<vec3d>	remeshCoord;

	BOX		m_box;
	bool	m_selected = false;
	bool	m_active = true;

	// fitting parameters
	double	m_el[3];	// eigenvalues
	vec3d	m_ev[3];	// eigenvectors

	vec3d	m_meanDir;	// mean direction
	double	m_FA;		// fractional anisotropy
	vector<double>	m_EFD_ODF;	// The EDF ODF
	vec3d	m_EFD_alpha;	// alpha values of EFD fit
	double	m_EFD_GFA;			// generalized fractional anisotropy
	double	m_EFD_FRD;			// Fisher-Rao distance

	vector<double>	m_VM3_ODF;	// The EDF ODF
	vec3d	m_VM3_beta;			// beta values of EFD fit
	double	m_VM3_GFA;			// generalized fractional anisotropy
	double	m_VM3_FRD;			// Fisher-Rao distance
};

class CFiberODFAnalysis : public CImageAnalysis
{
public:
    CFiberODFAnalysis(Post::CImageModel* img);
    ~CFiberODFAnalysis();

    void run() override;
    void render(CGLCamera* cam) override;

	int ODFs() const;
    CODF* GetODF(int i);

    bool renderMeshLines();
    bool showRadial();

    bool display() override;

	void SelectODF(int n);

	bool UpdateData(bool bsave) override;

public:
	void renderODFMesh(CODF* odf, CGLCamera* cam);

private:
    void clear();

    void butterworthFilter(sitk::Image& img);
    sitk::Image powerSpectrum(sitk::Image& img);
    void fftRadialFilter(sitk::Image& img);
    void reduceAmp(sitk::Image& img, std::vector<double>* reduced);

    std::unique_ptr<matrix> complLapBel_Coef();
    double GFA(std::vector<double>& vals);

	void processImage(sitk::Image& img);
	void normalizeODF(CODF* odf);
    void buildMesh(CODF* odf);
    void buildRemesh(CODF* odf);
	void calculateFits(CODF* odf);
	void UpdateMesh(CODF* odf, const vector<double>& val, bool bradial);
	void UpdateRemesh(CODF* odf, bool bradial);

	void UpdateAllMeshes();
	void UpdateStats();

	void updateProgressIncrement(double f);

	vector<double> optimize_edf(
		const vector<double>& alpha0,
		const vector<double>& odf,
		const vector<vec3d>& x,
		const matrix& V,
		const vector<double>& l);

	vector<double> optimize_vm3(
		const vector<double>& beta0,
		const vector<double>& odf,
		const vector<vec3d>& x);

private:
    std::vector<CODF*> m_ODFs;

    GLMeshRender m_render;

    double m_lengthScale;
    double m_hausd;
    double m_grad;

	// overall stats
	double	m_FAmin, m_FAmax;	// min, max range of FA

	// update settings
	int		m_nshowMesh;
	bool	m_bshowRadial;
	bool	m_nshowSelectionBox;
	int		m_ncolormode;

	// progress tracking
	int	m_stepsCompleted;
	int	m_totalSteps;
	double	m_progress;
	std::string	m_task;

	Post::CColorTexture	m_tex;
	GLLegendBar* m_pbar;
};
