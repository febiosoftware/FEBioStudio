#include <gtest/gtest.h>
#include <MeshLib/FSMesh.h>
#include <MeshLib/MeshMetrics.h>
#include <MeshTools/FEModifier.h>
#include <MeshTools/FESplitModifier.h>
#include <MeshTools/FEMeshUntangler.h>
#include <GeomLib/GMeshObject.h>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

namespace {

	// collects the log of a modifier
	class TextLogger : public TaskLogger
	{
	public:
		void Log(const std::string& msg) override { text += msg; }
		std::string text;
	};

	// A box [0,3]x[0,1]x[0,1] of three unit cubes, each meshed with different elements:
	// [0,1] is a hex8; [1,2] is six pyra5 with their apex at the center of the cube,
	// except that the one on top is split into two tet4; and [2,3] is two penta6.
	FSMesh* buildMixedBox()
	{
		auto id = [](int i, int j, int k) { return i + 4 * (j + 2 * k); };
		FSMesh* pm = new FSMesh;
		pm->Create(17, 10);
		for (int k = 0; k < 2; ++k)
			for (int j = 0; j < 2; ++j)
				for (int i = 0; i < 4; ++i) pm->Node(id(i, j, k)).r = vec3d(i, j, k);
		pm->Node(16).r = vec3d(1.5, 0.5, 0.5);

		int ne = 0;
		auto add = [&](int type, std::vector<int> n) {
			FSElement& el = pm->Element(ne++);
			el.SetType(type);
			el.m_gid = 0;
			for (int i = 0; i < (int)n.size(); ++i) el.m_node[i] = n[i];
		};

		add(FE_HEX8, { id(0,0,0), id(1,0,0), id(1,1,0), id(0,1,0), id(0,0,1), id(1,0,1), id(1,1,1), id(0,1,1) });

		// the base of each pyramid is a face of the cube, ordered so that its normal points to the apex
		const int h[8] = { id(1,0,0), id(2,0,0), id(2,1,0), id(1,1,0), id(1,0,1), id(2,0,1), id(2,1,1), id(1,1,1) };
		const int F[5][4] = { {0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7},{3,2,1,0} };	// outward normals
		for (auto& f : F) add(FE_PYRA5, { h[f[3]], h[f[2]], h[f[1]], h[f[0]], 16 });
		add(FE_TET4, { h[7], h[6], h[5], 16 });
		add(FE_TET4, { h[7], h[5], h[4], 16 });

		add(FE_PENTA6, { id(2,0,0), id(3,0,0), id(3,1,0), id(2,0,1), id(3,0,1), id(3,1,1) });
		add(FE_PENTA6, { id(2,0,0), id(3,1,0), id(2,1,0), id(2,0,1), id(3,1,1), id(2,1,1) });

		pm->RebuildMesh();
		return pm;
	}

	// a mesh of one element
	FSMesh* buildElement(int type, const std::vector<vec3d>& r)
	{
		FSMesh* pm = new FSMesh;
		pm->Create((int)r.size(), 1);
		for (int i = 0; i < (int)r.size(); ++i) pm->Node(i).r = r[i];
		FSElement& el = pm->Element(0);
		el.SetType(type);
		el.m_gid = 0;
		for (int i = 0; i < (int)r.size(); ++i) el.m_node[i] = i;
		pm->RebuildMesh();
		return pm;
	}

	int countType(FSMesh& m, int type)
	{
		int n = 0;
		for (int i = 0; i < m.Elements(); ++i) if (m.Element(i).Type() == type) n++;
		return n;
	}

	// number of elements with a non-positive Jacobian at an integration point (FEBio's check)
	int invalidElements(FSMesh& m)
	{
		int n = 0;
		for (int i = 0; i < m.Elements(); ++i)
			if (FEMeshMetrics::SolidJacobian(m, m.Element(i)) <= 0) n++;
		return n;
	}

	// total volume of the elements (overlapping elements count twice)
	double elementVolume(FSMesh& m)
	{
		double V = 0;
		for (int i = 0; i < m.Elements(); ++i) V += FEMeshMetrics::ElementVolume(m, m.Element(i));
		return V;
	}

	// every face is shared by at most two elements, and the faces of the mesh are its boundary
	bool isConforming(FSMesh& m)
	{
		std::map<std::vector<int>, int> faceCount;
		for (int i = 0; i < m.Elements(); ++i)
		{
			FSElement& e = m.Element(i);
			for (int j = 0; j < e.Faces(); ++j)
			{
				FSFace f = e.GetFace(j);
				std::vector<int> key(f.n, f.n + f.Nodes());
				std::sort(key.begin(), key.end());
				faceCount[key]++;
			}
		}
		int nboundary = 0;
		for (auto& it : faceCount)
		{
			if (it.second > 2) return false;
			if (it.second == 1) nboundary++;
		}
		return (nboundary == m.Faces());
	}

	// area of each surface (face partition); the faces are planar here
	std::map<int, double> surfaceAreas(FSMesh& m)
	{
		std::map<int, double> A;
		for (int i = 0; i < m.Faces(); ++i)
		{
			FSFace& f = m.Face(i);
			vec3d r[4];
			for (int j = 0; j < f.Nodes(); ++j) r[j] = m.Node(f.n[j]).r;
			double a = ((r[1] - r[0]) ^ (r[2] - r[0])).Length() * 0.5;
			if (f.Nodes() == 4) a += ((r[2] - r[0]) ^ (r[3] - r[0])).Length() * 0.5;
			A[f.m_gid] += a;
		}
		return A;
	}

	// length of each feature edge (edge partition)
	std::map<int, double> edgeLengths(FSMesh& m)
	{
		std::map<int, double> L;
		for (int i = 0; i < m.Edges(); ++i)
		{
			FSEdge& e = m.Edge(i);
			if (e.m_gid >= 0) L[e.m_gid] += (m.Node(e.n[1]).r - m.Node(e.n[0]).r).Length();
		}
		return L;
	}

	int cornerNodes(FSMesh& m)
	{
		int n = 0;
		for (int i = 0; i < m.Nodes(); ++i) if (m.Node(i).m_gid >= 0) n++;
		return n;
	}

	void expectSameMaps(const std::map<int, double>& a, const std::map<int, double>& b)
	{
		ASSERT_EQ(a.size(), b.size());
		for (auto& it : a)
		{
			auto jt = b.find(it.first);
			ASSERT_NE(jt, b.end());
			EXPECT_NEAR(it.second, jt->second, 1e-12 * (1 + it.second));
		}
	}

	FSMesh* refine(FSMesh* pm, int iterations, bool smooth, TextLogger* logger = nullptr, std::string* err = nullptr)
	{
		RefineMesh mod;
		if (logger) mod.SetTaskLogger(logger);
		mod.SetIntValue(0, iterations);
		mod.SetBoolValue(1, smooth);
		mod.SetBoolValue(2, false);
		FSMesh* pnew = mod.Apply(pm);
		if (err) *err = mod.GetErrorString();
		return pnew;
	}
}

// Refine Mesh splits each element of a mesh of hex8, penta6, pyra5 and tet4 elements:
// the refined mesh is conforming and valid, has the same volume, and keeps the surfaces,
// feature edges and corners of the mesh.
TEST(RefineTests, MixedElementsAreSplit)
{
	FSMesh* pm = buildMixedBox();
	ASSERT_EQ(invalidElements(*pm), 0);
	ASSERT_TRUE(isConforming(*pm));

	TextLogger logger;
	FSMesh* pr = refine(pm, 1, false, &logger);
	ASSERT_NE(pr, nullptr);

	EXPECT_EQ(countType(*pr, FE_HEX8), 8);
	EXPECT_EQ(countType(*pr, FE_PENTA6), 16);
	EXPECT_EQ(countType(*pr, FE_PYRA5), 30);
	EXPECT_EQ(countType(*pr, FE_TET4), 2 * 8 + 5 * 4);
	EXPECT_EQ(pr->Elements(), 90);

	EXPECT_TRUE(isConforming(*pr));
	EXPECT_EQ(invalidElements(*pr), 0);
	EXPECT_NEAR(elementVolume(*pr), 3.0, 1e-12);
	EXPECT_EQ(pr->Faces(), 4 * pm->Faces());

	expectSameMaps(surfaceAreas(*pm), surfaceAreas(*pr));
	expectSameMaps(edgeLengths(*pm), edgeLengths(*pr));
	EXPECT_EQ(cornerNodes(*pr), cornerNodes(*pm));

	// every edge of the faces is in the edge list once (as after rebuilding the mesh)
	std::map<std::pair<int, int>, int> faceEdges;
	for (int i = 0; i < pr->Faces(); ++i)
	{
		FSFace& f = pr->Face(i);
		for (int j = 0; j < f.Nodes(); ++j)
		{
			int a = f.n[j], b = f.n[(j + 1) % f.Nodes()];
			faceEdges[{std::min(a, b), std::max(a, b)}] = 0;
		}
	}
	EXPECT_EQ((int)faceEdges.size(), pr->Edges());
	for (int i = 0; i < pr->Edges(); ++i)
	{
		FSEdge& e = pr->Edge(i);
		auto it = faceEdges.find({ std::min(e.n[0], e.n[1]), std::max(e.n[0], e.n[1]) });
		ASSERT_NE(it, faceEdges.end());
		it->second++;
	}
	for (auto& it : faceEdges) EXPECT_EQ(it.second, 1);

	EXPECT_NE(logger.text.find("Refined the mesh from 10 to 90 elements (8 hex8, 16 penta6, 30 pyra5 and 36 tet4)"), std::string::npos) << logger.text;
	EXPECT_EQ(logger.text.find("Warning"), std::string::npos) << logger.text;

	delete pr;
	delete pm;
}

// two iterations refine the children again
TEST(RefineTests, MixedElementsTwoIterations)
{
	FSMesh* pm = buildMixedBox();
	FSMesh* pr = refine(pm, 2, false);
	ASSERT_NE(pr, nullptr);
	EXPECT_EQ(countType(*pr, FE_HEX8), 64);
	EXPECT_EQ(countType(*pr, FE_PENTA6), 128);
	EXPECT_EQ(countType(*pr, FE_PYRA5), 180);
	EXPECT_EQ(countType(*pr, FE_TET4), 36 * 8 + 30 * 4);
	EXPECT_TRUE(isConforming(*pr));
	EXPECT_EQ(invalidElements(*pr), 0);
	EXPECT_NEAR(elementVolume(*pr), 3.0, 1e-12);
	expectSameMaps(surfaceAreas(*pm), surfaceAreas(*pr));
	expectSameMaps(edgeLengths(*pm), edgeLengths(*pr));
	delete pr;
	delete pm;
}

// Smoothing the surface keeps the edges between surfaces as creases and the corners in
// place, and only moves nodes normal to the surface: the flat sides and straight edges
// of the box do not change at all.
TEST(RefineTests, SmoothingKeepsFlatSidesAndStraightEdges)
{
	FSMesh* pm = buildMixedBox();
	FSMesh* pr = refine(pm, 2, true);
	FSMesh* plain = refine(pm, 2, false);
	ASSERT_NE(pr, nullptr);
	ASSERT_NE(plain, nullptr);
	ASSERT_EQ(plain->Nodes(), pr->Nodes());
	double dmax = 0;
	for (int i = 0; i < pr->Nodes(); ++i) dmax = std::max(dmax, (pr->Node(i).r - plain->Node(i).r).Length());
	EXPECT_LT(dmax, 1e-12);
	EXPECT_EQ(invalidElements(*pr), 0);
	EXPECT_NEAR(elementVolume(*pr), 3.0, 1e-12);
	delete plain;
	delete pr;
	delete pm;
}

// The mixed box bent into a quarter of a ring (inner radius 2): its inner and outer sides are
// curved (three flat facets each, at 30 degrees), its other sides flat. Smoothing makes the
// curved sides smoother, keeps the flat sides flat, and keeps the elements valid.
TEST(RefineTests, SmoothingSmoothsCurvedSurfaces)
{
	FSMesh* pm = buildMixedBox();
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		vec3d& r = pm->Node(i).r;
		double a = r.x * M_PI / 6.0, R = 3.0 - r.y;	// (R decreases with y, so that the elements are not inverted)
		r = vec3d(R * cos(a), R * sin(a), r.z);
	}
	pm->RebuildMesh();
	ASSERT_EQ(invalidElements(*pm), 0);
	std::set<int> surfaces;
	for (int i = 0; i < pm->Faces(); ++i) surfaces.insert(pm->Face(i).m_gid);
	ASSERT_EQ(surfaces.size(), 6);

	// largest angle between the normals of neighboring faces of the inner side (R = 2)
	auto maxAngle = [](FSMesh& m) {
		auto normal = [&](FSFace& f) {
			vec3d r[4];
			for (int j = 0; j < f.Nodes(); ++j) r[j] = m.Node(f.n[j]).r;
			vec3d n = (f.Nodes() == 4 ? (r[2] - r[0]) ^ (r[3] - r[1]) : (r[1] - r[0]) ^ (r[2] - r[0]));
			n.unit();
			return n;
		};
		auto inner = [&](FSFace& f) {
			vec3d c(0, 0, 0);
			for (int j = 0; j < f.Nodes(); ++j) c += m.Node(f.n[j]).r;
			c /= (double)f.Nodes();
			return (sqrt(c.x * c.x + c.y * c.y) < 2.1) && (fabs(normal(f).z) < 0.5);
		};
		double amax = 0;
		for (int i = 0; i < m.Faces(); ++i)
		{
			FSFace& f = m.Face(i);
			if (!inner(f)) continue;
			for (int j = 0; j < f.Edges(); ++j)
			{
				if (f.m_nbr[j] < 0) continue;
				FSFace& g = m.Face(f.m_nbr[j]);
				if (!inner(g)) continue;
				double c = normal(f) * normal(g);
				amax = std::max(amax, acos(std::max(-1.0, std::min(1.0, c))) * 180.0 / M_PI);
			}
		}
		return amax;
	};

	TextLogger logger;
	FSMesh* plain = refine(pm, 2, false);
	FSMesh* pr = refine(pm, 2, true, &logger);
	ASSERT_NE(plain, nullptr);
	ASSERT_NE(pr, nullptr);
	EXPECT_TRUE(isConforming(*pr));
	EXPECT_EQ(invalidElements(*pr), 0) << logger.text;

	double a0 = maxAngle(*plain), a1 = maxAngle(*pr);
	printf("largest angle between faces of the curved side: %.1lf degrees, %.1lf degrees after smoothing\n", a0, a1);
	EXPECT_NEAR(a0, 30.0, 1e-6);
	EXPECT_LT(a1, 0.75 * a0);

	// the top and bottom stay flat, and the corners stay in place
	for (int i = 0; i < pr->Nodes(); ++i)
	{
		const vec3d& r0 = plain->Node(i).r;
		const vec3d& r1 = pr->Node(i).r;
		if ((fabs(r0.z) < 1e-12) || (fabs(r0.z - 1) < 1e-12)) EXPECT_NEAR(r1.z, r0.z, 1e-12);
		if (pr->Node(i).m_gid >= 0) EXPECT_LT((r1 - r0).Length(), 1e-12);
	}
	EXPECT_EQ(surfaceAreas(*pr).size(), 6);

	delete pr;
	delete plain;
	delete pm;
}

// The surfaces, edges, nodes and parts of the object stay the same.
TEST(RefineTests, ObjectKeepsItsSurfacesAndEdges)
{
	GMeshObject* po = new GMeshObject(buildMixedBox());
	int nodes = po->Nodes(), edges = po->Edges(), faces = po->Faces(), parts = po->Parts();
	EXPECT_EQ(faces, 6);
	EXPECT_EQ(edges, 12);
	EXPECT_EQ(nodes, 8);

	FSMesh* pr = refine(po->GetFEMesh(), 1, false);
	ASSERT_NE(pr, nullptr);
	FSMesh* old = po->ReplaceFEMesh(pr);
	EXPECT_EQ(po->Nodes(), nodes);
	EXPECT_EQ(po->Edges(), edges);
	EXPECT_EQ(po->Faces(), faces);
	EXPECT_EQ(po->Parts(), parts);

	delete old;
	delete po;
}

// Each element type is split into valid children with the same total volume.
TEST(RefineTests, ElementsOfEachType)
{
	struct Case { int type; std::vector<vec3d> r; int children[4]; };	// hex8, penta6, pyra5, tet4
	std::vector<Case> cases = {
		{ FE_HEX8, { {0,0,0},{1,0,0},{1,1,0},{0,1,0},{0,0,1},{1,0,1},{1,1,1},{0,1,1} }, {8,0,0,0} },
		{ FE_HEX8, { {0,0,0},{1.2,0.1,0},{1.1,0.9,0.2},{-0.1,1,0},{0.1,0,1},{1,-0.1,1.3},{1.3,1.2,1},{0,0.8,0.9} }, {8,0,0,0} },
		{ FE_PENTA6, { {0,0,0},{1,0,0},{0,1,0},{0,0,1},{1,0,1},{0,1,1} }, {0,8,0,0} },
		{ FE_PENTA6, { {0,0,0},{1.1,0.1,0},{0.2,0.9,0.1},{0.1,0,1.2},{1,0.2,0.9},{-0.1,1,1} }, {0,8,0,0} },
		{ FE_PYRA5, { {-1,-1,0},{1,-1,0},{1,1,0},{-1,1,0},{0,0,1} }, {0,0,6,4} },
		{ FE_PYRA5, { {-1,-1,0},{1.2,-0.9,0.1},{0.9,1.1,0},{-1,0.8,-0.1},{0.3,-0.2,0.7} }, {0,0,6,4} },
		{ FE_TET4, { {0,0,0},{1,0,0},{0,1,0},{0,0,1} }, {0,0,0,8} },
		{ FE_TET4, { {0,0,0},{3,0,0},{0.2,0.4,0},{1,0.1,0.3} }, {0,0,0,8} },
	};

	for (auto& c : cases)
	{
		FSMesh* pm = buildElement(c.type, c.r);
		ASSERT_EQ(invalidElements(*pm), 0);
		double V0 = elementVolume(*pm);

		// (meshes of only hex8 or only tet4 elements are refined by other modifiers, so call it directly)
		FEMixedSplitModifier mod;
		FSMesh* pr = mod.Apply(pm);
		ASSERT_NE(pr, nullptr) << mod.GetErrorString();
		EXPECT_EQ(countType(*pr, FE_HEX8), c.children[0]);
		EXPECT_EQ(countType(*pr, FE_PENTA6), c.children[1]);
		EXPECT_EQ(countType(*pr, FE_PYRA5), c.children[2]);
		EXPECT_EQ(countType(*pr, FE_TET4), c.children[3]);
		EXPECT_EQ(invalidElements(*pr), 0) << "element type " << c.type;
		EXPECT_TRUE(isConforming(*pr));
		EXPECT_NEAR(elementVolume(*pr), V0, 1e-12 * V0) << "element type " << c.type;

		// every face is split in four and keeps its surface (the areas differ here,
		// since the quads of the distorted elements are not flat)
		std::map<int, int> nf0, nf1;
		for (int i = 0; i < pm->Faces(); ++i) nf0[pm->Face(i).m_gid] += 4;
		for (int i = 0; i < pr->Faces(); ++i) nf1[pr->Face(i).m_gid]++;
		EXPECT_EQ(nf0, nf1);
		delete pr;
		delete pm;
	}
}

// A low dome (a pyramid on a square base, split into four tets and a pyramid around a node just
// under its top). Smoothing pulls the top of the dome down past that node; the nodes there move
// less, so that no element is inverted.
TEST(RefineTests, SmoothingDoesNotInvertElements)
{
	FSMesh* pm = new FSMesh;
	pm->Create(6, 5);
	const vec3d r[6] = { {-1,-1,0}, {1,-1,0}, {1,1,0}, {-1,1,0}, {0,0,0.5}, {0,0,0.45} };
	for (int i = 0; i < 6; ++i) pm->Node(i).r = r[i];
	const int E[5][5] = { {0,1,2,3,5}, {1,0,4,5,-1}, {2,1,4,5,-1}, {3,2,4,5,-1}, {0,3,4,5,-1} };
	for (int i = 0; i < 5; ++i)
	{
		FSElement& el = pm->Element(i);
		el.SetType(i == 0 ? FE_PYRA5 : FE_TET4);
		el.m_gid = 0;
		for (int j = 0; j < el.Nodes(); ++j) el.m_node[j] = E[i][j];
	}
	pm->RebuildMesh();
	ASSERT_EQ(invalidElements(*pm), 0);

	TextLogger logger;
	FSMesh* plain = refine(pm, 1, false);
	FSMesh* pr = refine(pm, 1, true, &logger);
	ASSERT_NE(plain, nullptr);
	ASSERT_NE(pr, nullptr);
	EXPECT_EQ(invalidElements(*pr), 0) << logger.text;
	EXPECT_NE(logger.text.find("Smoothing moved"), std::string::npos) << logger.text;

	// the top still moved down
	EXPECT_LT(pr->Node(4).r.z, plain->Node(4).r.z - 1e-3);

	delete pr;
	delete plain;
	delete pm;
}

// A pyramid with a folded corner: the face between the hex and the middle cube is warped (one
// of its nodes slides along its edge of the box) and the apex of the middle cube's pyramids is
// close to it. FEBio's integration points see valid elements, but the pyramid's Jacobian is
// negative at a corner of its base, and only splitting the elements inverts the children in
// that corner. Refine Mesh optimizes the nodes around them: the refined mesh is valid, and its
// sides and edges are unchanged.
TEST(RefineTests, FoldedCornerIsRepaired)
{
	FSMesh* pm = buildMixedBox();
	pm->Node(1).r = vec3d(1.4, 0, 0);
	pm->Node(16).r = vec3d(1.08, 0.7, 0.7);
	pm->RebuildMesh();
	ASSERT_EQ(invalidElements(*pm), 0);
	std::vector<vec3d> r(pm->Nodes());
	for (int i = 0; i < pm->Nodes(); ++i) r[i] = pm->Node(i).r;
	double corner = 1;
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.Type() != FE_PYRA5) continue;
		FEMeshUntangler::Element e;
		e.type = FEMeshUntangler::PYRA5;
		for (int j = 0; j < 8; ++j) e.n[j] = el.m_node[j < 5 ? j : 0];
		corner = std::min(corner, FEMeshUntangler::CornerScaledJacobian(r, e));
	}
	EXPECT_LT(corner, 0.0);

	// only split
	FEMixedSplitModifier split;
	split.DoElementRepair(false);
	FSMesh* p0 = split.Apply(pm);
	ASSERT_NE(p0, nullptr);
	EXPECT_GT(invalidElements(*p0), 0);
	delete p0;

	TextLogger logger;
	FSMesh* pr = refine(pm, 1, false, &logger);
	ASSERT_NE(pr, nullptr);
	EXPECT_NE(logger.text.find("Optimized the nodes around"), std::string::npos) << logger.text;
	EXPECT_EQ(invalidElements(*pr), 0);
	EXPECT_TRUE(isConforming(*pr));
	EXPECT_GT(FEMixedSplitModifier::WorstElement(*pr), FEMixedSplitModifier::WorstElement(*pm));
	EXPECT_NEAR(elementVolume(*pr), 3.0, 1e-12);
	expectSameMaps(surfaceAreas(*pm), surfaceAreas(*pr));
	expectSameMaps(edgeLengths(*pm), edgeLengths(*pr));
	delete pr;
	delete pm;
}

// A mesh of only penta6 elements could not be refined before.
TEST(RefineTests, PentaMeshIsRefined)
{
	FSMesh* pm = buildElement(FE_PENTA6, { {0,0,0},{1,0,0},{0,1,0},{0,0,1},{1,0,1},{0,1,1} });
	FSMesh* pr = refine(pm, 1, false);
	ASSERT_NE(pr, nullptr);
	EXPECT_EQ(countType(*pr, FE_PENTA6), 8);
	delete pr;
	delete pm;
}

// Refining selected elements of a mixed mesh, or the hex 2d-split, are not supported.
TEST(RefineTests, UnsupportedOptionsReportErrors)
{
	FSMesh* pm = buildMixedBox();

	std::string err;
	pm->Element(3).Select();
	EXPECT_EQ(refine(pm, 1, false, nullptr, &err), nullptr);
	EXPECT_NE(err.find("Clear the selection"), std::string::npos) << err;

	// all elements selected: the whole mesh is refined
	for (int i = 0; i < pm->Elements(); ++i) pm->Element(i).Select();
	FSMesh* pr = refine(pm, 1, false);
	ASSERT_NE(pr, nullptr);
	EXPECT_EQ(pr->Elements(), 90);
	delete pr;
	for (int i = 0; i < pm->Elements(); ++i) pm->Element(i).Unselect();

	RefineMesh mod;
	mod.SetIntValue(0, 1);
	mod.SetBoolValue(2, true);
	EXPECT_EQ(mod.Apply(pm), nullptr);
	EXPECT_NE(mod.GetErrorString().find("hex 2d-split"), std::string::npos) << mod.GetErrorString();

	// a quadratic element
	pm->Element(0).SetType(FE_HEX20);
	EXPECT_EQ(refine(pm, 1, false, nullptr, &err), nullptr);
	EXPECT_NE(err.find("Refine Mesh supports"), std::string::npos) << err;

	delete pm;
}
