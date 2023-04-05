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

#include <algorithm>
#include <string>
#include <fstream>
#include "DICQ4.h"

namespace sitk = itk::simple;

CDICQ4::CDICQ4(CDICMatching& match)
	: m_match(match), m_subs_per_row(match.GetRefImage().GetSubsPerRow()), m_subs_per_col(match.GetRefImage().GetSubsPerCol()),
        m_subSize(match.GetRefImage().GetSubSize()),m_match_centers(match.GetMatchResults()),m_ref_centers(match.GetRefCenterPoints())
{
	int temp = m_subs_per_row + 1;
	m_stop = m_match_centers.size() - temp; //cannot do interp with bottom row 

	ApplyQ4();
	WriteVTKFile();

}

void CDICQ4::ApplyQ4()
{
	Local2Global();
	//loop through all subsets

	std::vector<double> m_U_i, m_V_i;

	for (double i = 0; i < m_stop; i++)
	{
		int temp = i + 1;

		if (temp % m_subs_per_row == 0)
		{
			continue;
		}
		else
		{
			//std::cout << "-------------ELEMENT #: " << i << " -------------" << std::endl;

			std::vector<vec2i> NODES = GetNodesQ4(i);
			std::vector<vec2i> DISP_NODES = GetDispNodesQ4(i);

			std::vector<double> gauss_disps = Q4GetGaussPtDisp(NODES, DISP_NODES);
			std::vector<double> dispField = GetDispField(gauss_disps);

			std::vector<std::vector<double>> splitDisps = SplitDisps(dispField);

			std::vector<double> u_disps = splitDisps[0];
			std::vector<double> v_disps = splitDisps[1];

			for (int l = 0; l < u_disps.size(); l++)
			{
				m_U_i.push_back(u_disps[l]);
				m_V_i.push_back(v_disps[l]);
			}

		}

		

	}


    m_U = m_U_i;
	m_V = m_V_i;

	GetNodeIndices();
}


std::vector<vec2i> CDICQ4::GetNodesQ4(double ind)
{
	std::vector<vec2i> nodes;

    nodes.push_back(m_match.GetRefImage().GetSubset(ind).Center());
    nodes.push_back(m_match.GetRefImage().GetSubset(ind + 1).Center());
    nodes.push_back(m_match.GetRefImage().GetSubset(ind + m_subs_per_row).Center());
    nodes.push_back(m_match.GetRefImage().GetSubset(ind + m_subs_per_row + 1).Center());

	return nodes;
}


std::vector<vec2i> CDICQ4::GetDispNodesQ4(double ind)
{
	std::vector<vec2i> dispNodes;

    dispNodes.push_back(m_match_centers[ind]);
    dispNodes.push_back(m_match_centers[ind+1]);
    dispNodes.push_back(m_match_centers[ind + m_subs_per_row]);
    dispNodes.push_back(m_match_centers[ind + m_subs_per_row + 1]);

	return dispNodes;
}

///// GAUSS INTEGRATION POINT CALCULATIONS (STRAIN & DISP) //////////
std::vector<double> CDICQ4::Q4GetGaussPtDisp(std::vector<vec2i> NODES, std::vector<vec2i> DisplacedNodes)
{
	std::vector<vec2i> nodes = NODES;
	std::vector<vec2i> displacedNodes = DisplacedNodes;

	//optimal CDICQ4 integration points
	std::vector<double> eta, xi;

	eta.push_back(-1 / sqrt(3));
	eta.push_back(1 / sqrt(3));
	eta.push_back(1 / sqrt(3));
	eta.push_back(-1 / sqrt(3));

	xi.push_back(-1 / sqrt(3));
	xi.push_back(-1 / sqrt(3));
	xi.push_back(1 / sqrt(3));
	xi.push_back(1 / sqrt(3));

	std::vector<double> GaussPtDisp;

	for (int ii = 0; ii < 4; ii++)
	{
		double eta_temp = eta[ii];
		double xi_temp = xi[ii];

		std::vector<double> N = Q4Functions(xi_temp, eta_temp);

		double Ni[2][8] = {
			{N[0],0,N[1],0,N[2],0,N[3],0},
			{0, N[0],0,N[1],0,N[2],0,N[3]}
		};

		//separate displaced nodal positions
		std::vector<double> dispX, dispY;
		for (int d = 0; d < 4; d++)
		{
			dispX.push_back(displacedNodes[d].x);
			dispY.push_back(displacedNodes[d].y);
		}

		//separate x and y of nodes
		std::vector<double> x, y;

		for (int j = 0; j < 4; j++)
		{
			x.push_back(nodes[j].x);
			y.push_back(nodes[j].y);
		}

		double u0, v0, u1, v1, u2, v2, u3, v3;
		u0 = dispX[0] - x[0];
		v0 = dispY[0] - y[0];
		u1 = dispX[1] - x[1];
		v1 = dispY[1] - y[1];
		u2 = dispX[2] - x[2];
		v2 = dispY[2] - y[2];
		u3 = dispX[3] - x[3];
		v3 = dispY[3] - y[3];

		//nodal displacements = {u1 v1 u2 v2 u3 v3 u4 v4}
		double D[8] = { u0,v0, u1, v1, u2, v2, u3, v3 };


		//Displacement Approximation => [u(x,y) v(x,y)] = [Ni]*[D]
		double u{}, v{};

		for (int i = 0; i < 8; i++)
		{
			u += Ni[0][i] * D[i];
			v += Ni[1][i] * D[i];
		}

		GaussPtDisp.push_back(u);
		GaussPtDisp.push_back(v);
	}
	return GaussPtDisp;
}




/////////// Full Displacement and Strain Field Calculations //////////////////////////////////////
std::vector<double> CDICQ4::GetDispField(std::vector<double> GaussDisplacements)
{
	std::vector<double> disp;

	for (double s = 0; s < m_NormCoordPairs.size(); s++)
	{
		double xi = m_NormCoordPairs[s].first.x();
		double eta = m_NormCoordPairs[s].first.y();

		//convert eta and xi values (query point location) to prime coordinate system
		double xi_prime = xi * sqrt(3);
		double eta_prime = eta * sqrt(3);

		std::vector<double> N = Q4Functions(xi_prime, eta_prime);

		double Ni[2][8] = {
			{N[0],0,N[1],0,N[2],0,N[3],0},
			{0, N[0],0,N[1],0,N[2],0,N[3]}
		};

		double u{}, v{};

		for (int i = 0; i < 8; i++)
		{
			u += Ni[0][i] * GaussDisplacements[i];
			v += Ni[1][i] * GaussDisplacements[i];
		}

		disp.push_back(u);
		disp.push_back(v);
	}

	return disp;
}

std::vector<std::vector<double>> CDICQ4::SplitDisps(std::vector<double> displacements)
{
	std::vector<std::vector<double>> splitDisp;
	std::vector<double> u{}, v{};

	for (int i = 0; i < displacements.size(); i++)
	{
		if (i % 2 == 0)
		{
			u.push_back(displacements[i]);
		}
		else
		{
			v.push_back(displacements[i]);
		}

	}
	return splitDisp = { u,v };
}

std::vector<std::vector<double>> CDICQ4::GetStrainField(std::vector<vec2i> NODES, std::vector<vec2i> DISP_NODES)
{
	std::vector<vec2i> nodes = NODES;
	std::vector<vec2i> displacedNodes = DISP_NODES;

	//separate displaced nodal positions
	std::vector<double> dispX, dispY;
	for (int d = 0; d < 4; d++)
	{
		dispX.push_back(displacedNodes[d].x);
		dispY.push_back(displacedNodes[d].y);
	}

	//separate x and y of nodes
	std::vector<double> x, y;

	for (int j = 0; j < 4; j++)
	{
		x.push_back(nodes[j].x);
		y.push_back(nodes[j].y);
	}

	double u0, v0, u1, v1, u2, v2, u3, v3;
	u0 = dispX[0] - x[0];
	v0 = dispY[0] - y[0];
	u1 = dispX[1] - x[1];
	v1 = dispY[1] - y[1];
	u2 = dispX[2] - x[2];
	v2 = dispY[2] - y[2];
	u3 = dispX[3] - x[3];
	v3 = dispY[3] - y[3];

	//nodal displacements = {u1 v1 u2 v2 u3 v3 u4 v4}
	double D[8] = { u0,v0, u1, v1, u2, v2, u3, v3 };


	std::vector<std::vector<double>> exxx_pix;
	std::vector<double> EXX, EYY, EXY;

	for (double s = 0; s < m_NormCoordPairs.size(); s++)
	{
		double xi = m_NormCoordPairs[s].first.x();
		double eta = m_NormCoordPairs[s].first.y();

		// convert eta and xi values(query point location) to prime coordinate system
		double xi_prime = xi * sqrt(3);
		double eta_prime = eta * sqrt(3);


		//get Strain-Disp matrix
		std::vector<std::vector<double>> SD = GetStrainDispMat(xi_prime, eta_prime, nodes);

		std::vector<double> row1 = SD[0];
		std::vector<double> row2 = SD[1];
		std::vector<double> row3 = SD[2];

		double StrainDisp[3][8];

		for (int r = 0; r < row1.size(); r++)
		{
			StrainDisp[0][r] = row1[r];
			StrainDisp[1][r] = row2[r];
			StrainDisp[2][r] = row3[r];
		}

		double exx{ 0.0 }, eyy{ 0.0 }, exy{ 0.0 };

		for (int i = 0; i < 8; i++)
		{
			exx += StrainDisp[0][i] * D[i];
			eyy += StrainDisp[1][i] * D[i];
			exy += StrainDisp[2][i] * D[i];
		}

		EXX.push_back(exx);
		EYY.push_back(eyy);
		EXY.push_back(exy);

	}

	exxx_pix.push_back(EXX);
	exxx_pix.push_back(EYY);
	exxx_pix.push_back(EXY);

	return exxx_pix;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////




////////////// Shape Function and Strain-Disp Matrix Return Functions ////////////////////////////////////////////////

std::vector<double> CDICQ4::Q4Functions(double xi, double eta)
{
	std::vector<double> Ni;

	Ni.push_back((1.0 / 4.0) * (1.0 - xi) * (1.0 - eta));
	Ni.push_back((1.0 / 4.0) * (1.0 + xi) * (1.0 - eta));
	Ni.push_back((1.0 / 4.0) * (1.0 + xi) * (1.0 + eta));
	Ni.push_back((1.0 / 4.0) * (1.0 - xi) * (1.0 + eta));

	return Ni;
}

std::vector < std::vector<double>> CDICQ4::GetStrainDispMat(double xi, double eta, std::vector<vec2i> NODES)
{
	double temp_xi = xi;
	double temp_eta = eta;
	std::vector<vec2i> nodes = NODES;


	//calculate partials of the shape functions
	std::vector<double> dNxi, dNeta;

	dNxi.push_back(-(1.0 - temp_eta) / 4.0);
	dNxi.push_back((1.0 - temp_eta) / 4.0);
	dNxi.push_back((1.0 + temp_eta) / 4.0);
	dNxi.push_back(-(1.0 + temp_eta) / 4.0);

	dNeta.push_back(-(1.0 - temp_xi) / 4.0);
	dNeta.push_back(-(1.0 + temp_xi) / 4.0);
	dNeta.push_back((1.0 + temp_xi) / 4.0);
	dNeta.push_back((1.0 - temp_xi) / 4.0);

	//separate x and y of nodes
	std::vector<double> x, y;

	for (int j = 0; j < 4; j++)
	{
		x.push_back(nodes[j].x);
		y.push_back(nodes[j].y);
	}

	//chain rule
	double dxdxi, dydxi, dxdeta, dydeta;
	dxdxi = dNxi[0] * x[0] + dNxi[1] * x[1] + dNxi[2] * x[2] + dNxi[3] * x[3];
	dydxi = dNxi[0] * y[0] + dNxi[1] * y[1] + dNxi[2] * y[2] + dNxi[3] * y[3];
	dxdeta = dNeta[0] * x[0] + dNeta[1] * x[1] + dNeta[2] * x[2] + dNeta[3] * x[3];
	dydeta = dNeta[0] * y[0] + dNeta[1] * y[1] + dNeta[2] * y[2] + dNeta[3] * y[3];

	//form Jacobian Matrix and invert
	double J[2][2] = { {dxdxi, dydxi}, {dxdeta, dydeta} };
	double J_inv[2][2];
	double determinant;

	determinant = J[0][0] * J[1][1] - J[0][1] * J[1][0];

	//J^-1 : partial of eta and xi w.r.t x and y
	J_inv[0][0] = (1.0 / determinant) * J[1][1];
	J_inv[0][1] = (1.0 / determinant) * (-1) * J[0][1];
	J_inv[1][0] = (1.0 / determinant) * (-1) * J[1][0];
	J_inv[1][1] = (1.0 / determinant) * J[0][0];

	//chain rule for x,y partials of SF
	std::vector<double> dNx, dNy;
	for (int n = 0; n < 4; n++)
	{
		dNx.push_back(dNxi[n] * J_inv[0][0] + dNeta[n] * J_inv[0][1]);
		dNy.push_back(dNxi[n] * J_inv[1][0] + dNeta[n] * J_inv[1][1]);
	}

	std::vector < std::vector<double>> StrainDispMat;

	std::vector<double> SD0 = { dNx[0], 0, dNx[1], 0, dNx[2], 0, dNx[3], 0 };
	std::vector<double> SD1 = { 0,dNy[0],0,dNy[1],0,dNy[2],0,dNy[3] };
	std::vector<double> SD2 = { dNy[0], dNx[0], dNy[1],dNx[1], dNy[2], dNx[2], dNy[3],dNx[3] };

	StrainDispMat = { SD0, SD1, SD2 };

	return StrainDispMat;

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////// Convert Local Coordinates w/in element to global w/in image & associated functions //////////////
/////// point in [-1 1] ---> point in [0 60] LOCAL ---> point in image [0 width][0 height] ///////////////
void CDICQ4::Local2Global()
{
	SavePointPairs();

	std::ofstream debug;
	debug.open("C:\\Users\\elana\\Documents\\FEBio DIC\\DEBUG\\debug.txt");
	debug << "match points size: " << m_match_centers.size() << std::endl;
	debug << "m stop: " << m_stop << "subs per row = " << m_subs_per_row << " " << m_subs_per_col << std::endl;
	for (int i = 0; i < m_stop; i++)
	{
		int temp = i + 1;

		//L2G << "Subset #" << i << "\n";

		if (temp % (int)m_subs_per_row == 0)
		{
			debug << "cont'd" << std::endl;
			continue;
		}
		else
		{
			//std::cout << "-------------ELEMENT #: " << i << " -------------" << std::endl;

			std::vector<vec2i> NODES = GetNodesQ4(i); //check ref sub centers

			//cv::Point2d R_i(NODES[0].x + (m_subSize / 2.0), NODES[0].y + (m_subSize / 2.0));
			vec2i R_i(NODES[0].x + (m_subSize / 2.0), NODES[0].y + (m_subSize / 2.0));

			for (int p = 0; p < m_NormCoordPairs.size(); p++)
			{
				//cv::Point2d u_i = m_NormCoordPairs[p].second;
				vec2d u_i = m_NormCoordPairs[p].second;

				//cv::Point2d p_global(R_i.x + u_i.x - (m_subSize / 2.0), R_i.y + u_i.y - (m_subSize / 2.0));
				auto xp = R_i.x + u_i[0]; 
				auto yp = R_i.y + u_i[1];

				vec2i p_global(xp - (m_subSize / 2.0) + 1, yp - (m_subSize / 2.0) + 1);

				debug << p_global.x  << ", " << p_global.y << std::endl;

				//L2G << p_global << "\n";

				m_globalCoords.push_back(p_global);

				m_xi_eta_points.push_back(m_NormCoordPairs[p].first);

			}
		}
	}

	debug.close();
}

void CDICQ4::SavePointPairs()
{
	double sampInt = 2.0 / m_subSize;

	for (double xi = -1.0; xi <= 1.0; xi += sampInt)
	{
		for (double eta = -1.0; eta <= 1.0; eta += sampInt)
		{
			vec2d local(xi, eta);
            
			std::pair<vec2d, vec2d> coordPair;

			vec2d normed = ScalePoint(vec2d(xi, -eta));

			coordPair.first = local;
			coordPair.second = normed;

			m_NormCoordPairs.push_back(coordPair);
		}
	}
}

vec2d CDICQ4::ScalePoint(vec2d localCoord) //local coord: [-1 1]
{
	double xi, eta, normX, normY;
	xi = localCoord.x();
	eta = localCoord.y();

	normX = NormVal(xi);
	normY = NormVal(eta);

	vec2d normalized(normX, normY);

	return normalized;
}

double CDICQ4::NormVal(double VAL)
{
	double norm = (m_subSize) * ((VAL + 1) / 2);
	return norm;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

// finding indices of nodal points

void CDICQ4::GetNodeIndices()
{
	int summation = 0;
	std::vector<vec2i> trackNodes;

	//loop through all subsets
	for (double i = 0; i < m_stop; i++)
	{
		int temp = i + 1;

		if (temp % m_subs_per_row == 0)
		{
			continue;
		}
		else
		{
			//std::cout << "-------------ELEMENT #: " << i << " -------------" << std::endl;

			std::vector<vec2i> NODES = GetNodesQ4(i);

			summation += NODES.size();

			for (int n = 0; n < NODES.size(); n++)
			{
				vec2i N = NODES[n];

				if (std::count(trackNodes.begin(), trackNodes.end(), N))
				{
					continue;
				}
				else
				{
					trackNodes.push_back(N);

					std::vector<vec2i> const& v = m_globalCoords;

					//std::cout << "----- NODE: " << N << " -----" << std::endl;

					std::vector<int> indices = findPoints(v, N);

					double U = 0, V = 0;

					for (int in = 0; in < indices.size(); in++)
					{
						//std::cout << "index: " << indices[in] << std::endl;
						U += m_U[indices[in]];
						V += m_V[indices[in]];
						//std::cout << "exx [" << in << "] = " << m_EXX[indices[in]] << std::endl;
					}

					//average data for each nodal point
					double avg_u = U / indices.size();
					double avg_v = V / indices.size();

					m_n_U.push_back(avg_u);
					m_n_V.push_back(avg_v);

				}

			}
		}

	}

	m_NodalPositions = trackNodes;

	SortNodalPoints();

	//PrintNodalGridData();
}


std::vector<int> CDICQ4::findPoints(std::vector<vec2i> const& v, vec2i target)
{
	std::vector<int> indices;

	for (int i = 0; i < v.size(); i++) {
		if (v[i].x == target.x && v[i].y == target.y) {
			indices.push_back(i);
		}
	}
	return indices;
}

void CDICQ4::SortNodalPoints()
{
	std::vector<int> indices_first, indices_second;
	std::vector<vec2i> first;

	for (int i = 0; i < m_subs_per_row * 2.0; i++)
	{
		first.push_back(m_NodalPositions[i]);
	}

	int first_row = m_NodalPositions[0].y;
	int sec_row = m_NodalPositions[2].y;

	indices_first = findItem(first, first_row, 1);
	indices_second = findItem(first, sec_row, 1);

	indices_second.insert(indices_second.begin(), indices_first.begin(), indices_first.end());

	std::vector<vec2i> SORTED_PTs;
	std::vector<double> S_U, S_V;

	for (int f = 0; f < m_subs_per_row * 2.0; f++)
	{
		SORTED_PTs.push_back(m_NodalPositions[indices_second[f]]);
		S_U.push_back(m_n_U[indices_second[f]]);
		S_V.push_back(m_n_V[indices_second[f]]);
	}

	m_NodalPositions.erase(m_NodalPositions.begin(), m_NodalPositions.begin() + (m_subs_per_row * 2.0));
	m_n_U.erase(m_n_U.begin(), m_n_U.begin() + (m_subs_per_row * 2.0));
	m_n_V.erase(m_n_V.begin(), m_n_V.begin() + (m_subs_per_row * 2.0));

	SORTED_PTs.insert(SORTED_PTs.end(), m_NodalPositions.begin(), m_NodalPositions.end());
	S_U.insert(S_U.end(), m_n_U.begin(), m_n_U.end());
	S_V.insert(S_V.end(), m_n_V.begin(), m_n_V.end());

	m_NodalPositions = SORTED_PTs;
	m_n_U = S_U;
	m_n_V = S_V;

}

std::vector<int> CDICQ4::findItem(std::vector<vec2i> const& v, int target, int xORy)
{
	std::vector<int> indices;

	if (xORy == 0)
	{
		for (int i = 0; i < v.size(); i++)
		{
			if (v[i].x == target)
			{
				indices.push_back(i);
			}
		}
	}
	else
	{
		for (int i = 0; i < v.size(); i++)
		{
			if (v[i].y == target)
			{
				indices.push_back(i);
			}
		}

	}


	return indices;
}

void CDICQ4::WriteVTKFile()
{
	std::string dataName;

	dataName = "DIC_Results";
	std::ofstream VTKFile;
	std::string img_name = "deformedImg";

	int imgNum = 0;

	std::string str = "C:\\Users\\elana\\Documents\\FEBio DIC\\DEBUG\\" + dataName + "_0" + std::to_string(imgNum) + ".vtk";
	const char* c = const_cast<char*>(str.c_str());

	VTKFile.open(c);

	VTKFile << "# vtk DataFile Version 2.0 \n";
	VTKFile << img_name << ", subset size = " << m_subSize << "\n";
	VTKFile << "ASCII\n";
	VTKFile << "DATASET POLYDATA\n";

	VTKFile << "POINTS " << m_NodalPositions.size() << " float\n";

	for (int i = 0; i < m_NodalPositions.size(); i++)
	{
		VTKFile << m_NodalPositions[i].x << " " << m_NodalPositions[i].y << " " << 0 << " \n";
	}

	double R = m_subs_per_row;
	double C = m_subs_per_col;

	double n = (R - 1) * (C - 1);

	VTKFile << "POLYGONS " << n << " " << n * 5 << "\n";

	for (int row = 0; row < R - 1; row++)
	{
		for (int col = 0; col < C - 1; col++)
		{
			int n0 = col * (R)+row;
			int n1 = col * (R)+row + 1;
			int n2 = (col + 1) * (R)+row + 1;
			int n3 = (col + 1) * (R)+row;

			VTKFile << 4 << " " << n0 << " " << n1 << " " << n2 << " " << n3 << " \n";
		}

	}

	VTKFile << "POINT_DATA " << m_n_U.size() << " double\n";
	VTKFile << "VECTORS displacement float\n";

	for (int d = 0; d < m_n_U.size(); d++)
	{
		VTKFile << m_n_U[d] << " " << m_n_V[d] << " " << 0 << "\n";
	}

	VTKFile.close();
}


//////////// PRINT /////////////////////////////
// void CDICQ4::PrintNodalGridData()
// {
// 	std::ofstream NodalPointFile;
// 	NodalPointFile.open("NodalGridData.csv");

// 	NodalPointFile << "Grid Size:," << m_subs_per_row << " rows," << m_subs_per_col << " columns," << "\n";
// 	NodalPointFile << "X-Position," << "Y-Position," << "Exx," << "Exy," << "Eyy," << "U [pix]," << "V [pix]," << "\n";

// 	for (int i = 0; i < m_n_EXX.size(); i++)
// 	{
// 		NodalPointFile << m_NodalPositions[i].x << "," << m_NodalPositions[i].y << "," << m_n_EXX[i]
// 			<< "," << m_n_EXY[i] << "," << m_n_EYY[i] << "," << m_n_U[i]
// 			<< "," << m_n_V[i] << "\n";
// 		//std::cout << "EXX: " << mI_n_EXX[i].second << std::endl;
// 	}

// 	NodalPointFile.close();

// }

//////////// WRITING VTK FILES FOR VIS IN FEBIO STUDIO /////////////////////////
//void CDICQ4::WriteVTK(int DOI, int imgNum)
//{
//	std::vector<double> DATA;
//	std::string dataName;
//
//	switch (DOI)
//	{
//	case 0:
//		dataName = "V_EXX";
//		DATA = m_n_EXX;
//		break;
//	case 1:
//		dataName = "V_EYY";
//		DATA = m_n_EYY;
//		break;
//	case 2:
//		dataName = "V_EXY";
//		DATA = m_n_EXY;
//		break;
//	case 3:
//		dataName = "V_U";
//		DATA = m_n_U;
//		break;
//	case 4:
//		dataName = "V_V";
//		DATA = m_n_V;
//		break;
//	}
//
//	std::ofstream VTKFile;
//	// std::string img_name = m_match.GetDefImage().GetFileName();
//    std::string img_name = "def image";
//
//	std::string str = dataName + "_0" + std::to_string(imgNum) + ".vtk";
//	const char* c = const_cast<char*>(str.c_str());
//
//	VTKFile.open(c);
//
//	VTKFile << "# vtk DataFile Version 2.0 \n";
//	VTKFile << img_name << ", subset size = " << m_subSize << "\n";
//	VTKFile << "ASCII\n";
//	VTKFile << "DATASET POLYDATA\n";
//
//	VTKFile << "POINTS " << m_NodalPositions.size() << " float\n";
//
//	for (int i = 0; i < m_NodalPositions.size(); i++)
//	{
//		VTKFile << m_NodalPositions[i].x << " " << m_NodalPositions[i].y << " " << 0 << " \n";
//	}
//
//	//double R = trunc(GetRefDim()[0]/ m_subSpacing); //find # of full size subsets per row, round down to whole number
//	//double C = trunc(GetRefDim()[1] / m_subSpacing); //find # of full size subsets per col, round down to whole number
//
//	double R = m_subs_per_row;
//	double C = m_subs_per_col;
//
//	double n = (R-1) * (C-1);
//	//double n = m_ref_img.GetNumSubs();
//
//	//std::cout << "number subs total (n): " << n << std::endl;
//
//	VTKFile << "POLYGONS " << n << " " << n * 5 << "\n";
//
//	for (int row = 0; row < R-1 ; row++)
//	{
//		for (int col = 0; col < C-1 ; col++)
//		{
//			int n0 = col * (R)+row;
//			int n1 = col * (R)+row + 1;
//			int n2 = (col + 1) * (R)+row + 1;
//			int n3 = (col + 1) * (R)+row;
//
//			VTKFile << 4 << " " << n0 << " " << n1 << " " << n2 << " " << n3 << " \n";
//		}
//
//	}
//
//	VTKFile << "POINT_DATA " << m_n_EXX.size() << " double\n";
//	//VTKFile << "VECTORS displacement float\n";
//	VTKFile << "SCALARS data float 1\n";
//	VTKFile << "LOOKUP_TABLE default\n";
//
//
//	for (int d = 0; d < DATA.size(); d++)
//	{
//		//VTKFile << DATA[d] << " " << DATA[d] << " " << 0 << "\n";
//		VTKFile <<  DATA[d] << "\n";
//	}
//
//	VTKFile.close();
//
//}




















////// DATA SMOOTHING /////////

std::vector<double> CDICQ4::SmoothData(std::vector<double> data)
{
	// std::vector<double> temp = data; //data

	// cv::Mat mat = Vec2Mat(temp, (m_subs_per_col-1)*(m_subSize+1), (m_subs_per_row-1)*(m_subSize+1)); //convert vector to matrix (aka grid)

    int rows = (m_subs_per_row-1)*(m_subSize+1);
    int cols = (m_subs_per_col-1)*(m_subSize+1);


	//std::cout << mat << std::endl;

	//add padding
	// int pad_rows = (m_match.GetRefImage().GetWindowSize() - 1.0) / 2.0;
	// int pad_cols = (m_match.GetRefImage().GetWindowSize() - 1.0) / 2.0;

    // sitk::Image padded(cols+2*pad_cols, rows+2*pad_rows, sitk::sitkFloat64);
    // double* buffer = padded.GetBufferAsDouble();


    // for (int i = 0; i < cols; i++)
    // {
    //     for (int j = 0; j < rows; j++)
    //     {
    //         buffer[(i+pad_cols)*rows+j+pad_rows] = data[i*rows+j];
    //     }

    // }

    sitk::Image image(cols, rows, sitk::sitkFloat64);
    double* buffer = image.GetBufferAsDouble();

    for (int i = 0; i < cols; i++)
    {
        for (int j = 0; j < rows; j++)
        {
            buffer[i*rows+j] = data[i*rows+j];
        }

    }

	// cv::Mat padded(cv::Size(mat.cols + 2 * pad_cols, mat.rows + 2 * pad_rows), CV_64F, cv::Scalar(0));
	// mat.copyTo(padded(cv::Rect(pad_cols, pad_rows, mat.cols, mat.rows)));

	//create kernel
	sitk::Image kernel = CreateKernel(m_match.GetRefImage().GetWindowSize(), m_match.GetRefImage().GetWindowSize(), BOXFILTER);

	// cv::Mat smoothedMat = cv::Mat::zeros(mat.size(), CV_64F);
	// cv::Mat dst;

	// //convolve kernel to smooth
	// for (int i = 0; i < mat.rows; i++)
	// {
	// 	for (int j = 0; j < mat.cols; j++)
	// 	{
	// 		smoothedMat.at<double>(i, j) = sum(kernel.mul(padded(cv::Rect(j, i, m_ref_img.GetWindowSize(), m_ref_img.GetWindowSize())))).val[0];
	// 	}
	// }

	// smoothedMat.convertTo(dst, CV_8UC1);

    auto output = sitk::Convolution(image, kernel);
    buffer = output.GetBufferAsDouble();

    int size = output.GetSize()[0]*output.GetSize()[1];

    std::vector<double> smoothedVec(size);
    for(int i = 0; i < size; i++)
    {
        smoothedVec[i] = buffer[i];
    }


	// std::vector<double> smoothedVec = Mat2Vec(smoothedMat);

	//std::cout << "SMOOTHED DATA:" << std::endl;

	//for (int ii = 0; ii < smoothedVec.size(); ii++)
	//{
	//	std::cout << smoothedVec[ii] << std::endl;
	//}

	return smoothedVec;
}

// cv::Mat CDICQ4::Vec2Mat(std::vector<double> vec,int r, int c)
// {
// 	cv::Mat M(r, c,CV_64F);

// 	memcpy(M.data, vec.data(), vec.size()*sizeof(double));
	

// 	return M;
// }

// std::vector<double> CDICQ4::Mat2Vec(cv::Mat mat)
// {
// 	std::vector<double> vec;

// 	for (int i = 0; i < mat.rows; i++)
// 	{
// 		for (int j = 0; j < mat.cols; j++)
// 		{
// 			vec.push_back(mat.at<double>(i, j));
// 		}
// 	}

// 	return vec;
// }

itk::simple::Image CDICQ4::CreateKernel(int k_w, int k_h, int filterType)
{
	if (filterType == BOXFILTER)
	{
		int size = k_w * k_h;

        sitk::Image kernel(k_w, k_h, sitk::sitkFloat64);
        double* buffer = kernel.GetBufferAsDouble();
        double val = 1/size;

        for(int i = 0; i < size; i++)
        {
            buffer[i] = val;
        }

		return kernel;
	}
	else if (filterType == GAUSSFILTER)
	{
		double s = m_match.GetRefImage().GetSigma();
		double k = 1;
		int pad_rows = (m_match.GetRefImage().GetWindowSize() - 1.0) / 2.0;
		int pad_cols = (m_match.GetRefImage().GetWindowSize() - 1.0) / 2.0;
		
		sitk::Image kernel(k_w, k_h, sitk::sitkFloat64);
        double* buffer = kernel.GetBufferAsDouble();

        double sum = 0;
        int size = m_match.GetRefImage().GetWindowSize();

		for (int i = -pad_rows; i <= pad_rows; i++)
		{
			for (int j = -pad_cols; j <= pad_cols; j++)
			{
                double val = exp(-(i * i + j * j) / 2.0);
				buffer[(i + pad_rows)*size + j + pad_cols] = val;

                sum += val;
			}
		}

        for(int i = 0; i < size; i++)
        {
            buffer[i] /= sum;
        }

		return kernel;
	}
	
}