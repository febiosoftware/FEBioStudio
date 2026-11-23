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

#include "rhdm_api.h"
#include "hierarchy.h"
#include <memory>
#include <stdexcept>

namespace rhdm {

namespace {
	struct Canceled {};

	// Serial version of the upstream Optimizer thread loop (optimizer.cpp):
	// smooth on the coarsest level first, then prolong to finer levels.
	void optimizeField(MultiResolutionHierarchy& mRes, bool orientations, int itersPerLevel,
		const HexDomProgress& progress, double p0, double p1)
	{
		int levels = mRes.levels();
		int startLevel = std::max(levels - 2, 0);
		int totalIters = (startLevel + 1) * itersPerLevel;
		int count = 0;
		for (int level = startLevel; level >= 0; --level)
		{
			for (int it = 0; it < itersPerLevel; ++it, ++count)
			{
				if (orientations) mRes.smoothOrientationsTet(level, true, true);
				else mRes.smoothPositionsTet(level, true, true);

				if (progress && ((count % 10) == 0))
				{
					double f = p0 + (p1 - p0) * (double)count / (double)totalIters;
					if (!progress(f, orientations ? "Optimizing orientation field" : "Optimizing position field"))
						throw Canceled();
				}
			}
			if (level > 0)
			{
				if (orientations) mRes.prolongOrientations(level - 1);
				else mRes.prolongPositions(level - 1);
			}
		}
	}
}

bool HexDominantMesh(const std::vector<double>& nodes, const std::vector<int>& tets,
	const HexDomParams& params, HexDomResult& result, std::string& errMsg,
	HexDomProgress progress)
{
	result = HexDomResult();
	size_t NN = nodes.size() / 3;
	size_t NT = tets.size() / 4;
	if ((NN == 0) || (NT == 0) || (nodes.size() != 3 * NN) || (tets.size() != 4 * NT))
	{
		errMsg = "Invalid input mesh.";
		return false;
	}

	int oldThreads = tbb::max_threads();
	tbb::max_threads() = params.threads;

	try {
		std::unique_ptr<MultiResolutionHierarchy> mRes(new MultiResolutionHierarchy);

		MatrixXf V(3, NN);
		for (size_t i = 0; i < NN; ++i)
			for (int j = 0; j < 3; ++j) V(j, i) = (Float)nodes[3 * i + j];

		MatrixXu T(4, NT);
		for (size_t i = 0; i < NT; ++i)
			for (int j = 0; j < 4; ++j)
			{
				int n = tets[4 * i + j];
				if ((n < 0) || ((size_t)n >= NN)) throw std::runtime_error("Invalid node index in tetrahedral mesh.");
				T(j, i) = (uint32_t)n;
			}

		if (progress && !progress(0.0, "Building hierarchy")) throw Canceled();

		if (!mRes->setTetMesh(V, T)) throw std::runtime_error("Failed to initialize from tetrahedral mesh.");
		if (mRes->F().cols() == 0) throw std::runtime_error("The tetrahedral mesh has no boundary.");
		if ((params.featureType.size() == NN) && (params.featureDir.size() == 3 * NN))
			mRes->setFeatures(params.featureType, params.featureDir);

		mRes->build();

		// set the target edge length (mScale = diagonalLen * ratio)
		if (params.edgeLength > 0.0)
			mRes->setScale((Float)(params.edgeLength / mRes->diagonalLen));
		else
			mRes->setScale(mRes->ratio_scale);
		result.edgeLength = mRes->mScale;
		result.hierarchyLevels = mRes->levels();

		int iters = (params.iterationsPerLevel > 0 ? params.iterationsPerLevel : 200);
		optimizeField(*mRes, true , iters, progress, 0.05, 0.40);
		optimizeField(*mRes, false, iters, progress, 0.40, 0.75);

		if (progress && !progress(0.75, "Extracting hex-dominant mesh")) throw Canceled();

		// the extraction polls for cancellation
		if (progress) mRes->mCancel = [&progress]() { return !progress(0.75, "Extracting hex-dominant mesh"); };

		// same settings as the upstream batch mode
		mRes->re_color = true;
		mRes->splitting = params.splitting;
		mRes->decomposes = true;
		mRes->doublets = true;
		mRes->triangles = true;
		mRes->meshExtraction3D();

		// copy the result
		const MatrixXf& VR = mRes->mV_tag;
		result.nodes.resize(3 * VR.cols());
		for (int i = 0; i < VR.cols(); ++i)
			for (int j = 0; j < 3; ++j) result.nodes[3 * i + j] = VR(j, i);

		result.faces.resize(mRes->F_tag.size());
		for (size_t i = 0; i < mRes->F_tag.size(); ++i)
			result.faces[i].assign(mRes->F_tag[i].begin(), mRes->F_tag[i].end());

		result.cells.resize(mRes->P_tag.size());
		for (size_t i = 0; i < mRes->P_tag.size(); ++i)
			result.cells[i].assign(mRes->P_tag[i].begin(), mRes->P_tag[i].end());

		result.isHex = mRes->Hex_flag;
		result.isHex.resize(result.cells.size(), false);

		if (progress) progress(1.0, "Extracting hex-dominant mesh");
	}
	catch (MultiResolutionHierarchy::ExtractionCanceled&)
	{
		tbb::max_threads() = oldThreads;
		errMsg = "Canceled.";
		return false;
	}
	catch (Canceled&)
	{
		tbb::max_threads() = oldThreads;
		errMsg = "Canceled.";
		return false;
	}
	catch (std::exception& e)
	{
		tbb::max_threads() = oldThreads;
		errMsg = e.what();
		return false;
	}
	catch (...)
	{
		tbb::max_threads() = oldThreads;
		errMsg = "Unknown error in hex-dominant mesher.";
		return false;
	}

	tbb::max_threads() = oldThreads;
	return true;
}

} // namespace rhdm
