/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.
See Copyright - FEBio - Studio.txt for details.
Copyright(c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :
The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once

#include "DICMatching.h"
#include <sitkImage.h>

class CDICQ4
{
public:
	enum { BOXFILTER, GAUSSFILTER };

public:
	CDICQ4(CDICMatching& match);
	void ApplyQ4();
	std::vector<vec2i> GetNodesQ4(double ind);
	std::vector<vec2i> GetDispNodesQ4(double ind);
	std::vector<double> Q4GetGaussPtDisp(std::vector<vec2i> NODES, std::vector<vec2i> DisplacedNodes);
	std::vector<double> Q4Functions(double xi, double eta);
	void Local2Global();
	void SavePointPairs();
	vec2d ScalePoint(vec2d localCoord);
	double NormVal(double VAL);
	std::vector<double> GetDispField(std::vector<double> GaussDisplacements);
	std::vector<std::vector<double>> SplitDisps(std::vector<double> displacements);
	std::vector<std::vector<double>> GetStrainField(std::vector<vec2i> NODES, std::vector<vec2i> DISP_NODES);
	std::vector < std::vector<double>> GetStrainDispMat(double xi, double eta, std::vector<vec2i> NODES);
	void GetNodeIndices();
	std::vector<int> findPoints(std::vector<vec2i> const& v, vec2i target);
	void SortNodalPoints();
	std::vector<int> findItem(std::vector<vec2i> const& v, int target, int xORy);
	// void PrintNodalGridData();
	void WriteVTK(int DOI, int imgNum);
	std::vector<double> GetRefDim();
	std::vector<double> SmoothData(std::vector<double> data);
	// cv::Mat Vec2Mat(std::vector<double>, int r, int c);
	// std::vector<double> Mat2Vec(cv::Mat);
	itk::simple::Image CreateKernel(int k_w, int k_h, int filterType);
	void WriteVTKFile();

private:
	CDICMatching& m_match;
	// CorrImg& m_ref_img;
	// CorrImg& m_def_img;
	int m_subs_per_row;
	int m_subs_per_col;
	int m_stop;
	std::vector<vec2i> m_ref_centers;
	std::vector<vec2i> m_match_centers;
	double m_subSize;
	// double m_subSpacing;
	std::vector<std::pair<vec2d, vec2d>> m_NormCoordPairs; // Pair of points [-1 1] <-> [0 subSize]
	std::vector<vec2i> m_globalCoords;
	std::vector<vec2d> m_xi_eta_points;
	std::vector<double> m_U;
	std::vector<double> m_V;
	std::vector<double> m_EXX;
	std::vector<double> m_EXY;
	std::vector<double> m_EYY;
	std::vector<double> m_n_U;
	std::vector<double> m_n_V;
	std::vector<double> m_n_EXX;
	std::vector<double> m_n_EXY;
	std::vector<double> m_n_EYY;
	std::vector<vec2i> m_NodalPositions;
	std::vector<double> m_NCC;
	std::vector<double> m_n_NCC;
};
