#include <gtest/gtest.h>
#include <MeshLib/FSMesh.h>
#include <MeshTools/FEHexDominantRemesh.h>
#include <MeshLib/MeshMetrics.h>
#include <MeshTools/FEMeshUntangler.h>
#include <MeshTools/FEModifier.h>
#include <MeshTools/FESplitModifier.h>
#include <cmath>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <map>
#include <set>
#include <array>
#include <vector>
#include <algorithm>
#include <cstdio>

// prints the modifier's log (hex count, inverted elements, ...) to stdout
class StdoutLogger : public TaskLogger
{
public:
	void Log(const std::string& msg) override { printf("%s", msg.c_str()); text += msg; }
	std::string text;
};

// build a conforming TET4 mesh of the unit cube (n^3 cells, 6 tets per cell).
// If sphere == true, the cube is mapped onto the unit sphere.
static FSMesh* buildTetCube(int n, bool sphere)
{
	auto id = [=](int i, int j, int k) { return (k * (n + 1) + j) * (n + 1) + i; };
	int NN = (n + 1) * (n + 1) * (n + 1);
	int NE = 6 * n * n * n;
	FSMesh* pm = new FSMesh;
	pm->Create(NN, NE);
	for (int k = 0; k <= n; ++k)
		for (int j = 0; j <= n; ++j)
			for (int i = 0; i <= n; ++i)
			{
				double x = i / (double)n, y = j / (double)n, z = k / (double)n;
				if (sphere)
				{
					x = 2 * x - 1; y = 2 * y - 1; z = 2 * z - 1;
					double X = x * sqrt(1 - y * y / 2 - z * z / 2 + y * y * z * z / 3);
					double Y = y * sqrt(1 - z * z / 2 - x * x / 2 + z * z * x * x / 3);
					double Z = z * sqrt(1 - x * x / 2 - y * y / 2 + x * x * y * y / 3);
					x = X; y = Y; z = Z;
				}
				pm->Node(id(i, j, k)).r = vec3d(x, y, z);
			}

	const int K[6][4] = { {0,1,3,7},{0,3,2,7},{0,2,6,7},{0,6,4,7},{0,4,5,7},{0,5,1,7} };
	int ne = 0;
	for (int k = 0; k < n; ++k)
		for (int j = 0; j < n; ++j)
			for (int i = 0; i < n; ++i)
			{
				int c[8];
				for (int b = 0; b < 8; ++b) c[b] = id(i + (b & 1), j + ((b >> 1) & 1), k + ((b >> 2) & 1));
				for (int t = 0; t < 6; ++t)
				{
					FSElement& el = pm->Element(ne++);
					el.SetType(FE_TET4);
					el.m_gid = 0;
					for (int m = 0; m < 4; ++m) el.m_node[m] = c[K[t][m]];
				}
			}
	pm->RebuildMesh();
	return pm;
}

// Conforming TET4 mesh of a regular hexagonal prism (circumradius 1, height 1): the hexagon
// is split into three rhombi with n x n cells each, extruded in m layers; every hexahedral
// cell is split into six tets.
static FSMesh* buildHexPrism(int n, int m)
{
	std::map<std::array<long long, 3>, int> index;
	std::vector<vec3d> X;
	auto node = [&](const vec3d& r) {
		std::array<long long, 3> key = { llround(r.x * 1e9), llround(r.y * 1e9), llround(r.z * 1e9) };
		auto it = index.find(key);
		if (it != index.end()) return it->second;
		int id = (int)X.size();
		index[key] = id;
		X.push_back(r);
		return id;
	};
	const double kPi = 3.14159265358979323846;
	const int K[6][4] = { {0,1,3,7},{0,3,2,7},{0,2,6,7},{0,6,4,7},{0,4,5,7},{0,5,1,7} };
	std::vector<std::array<int, 4> > tets;
	for (int k = 0; k < 3; ++k)
	{
		vec3d a(cos(2 * k * kPi / 3), sin(2 * k * kPi / 3), 0);
		vec3d b(cos((2 * k + 2) * kPi / 3), sin((2 * k + 2) * kPi / 3), 0);
		for (int l = 0; l < m; ++l)
			for (int j = 0; j < n; ++j)
				for (int i = 0; i < n; ++i)
				{
					int c[8];
					for (int q = 0; q < 8; ++q)
					{
						double u = (i + (q & 1)) / (double)n, v = (j + ((q >> 1) & 1)) / (double)n, z = (l + ((q >> 2) & 1)) / (double)m;
						c[q] = node(a * u + b * v + vec3d(0, 0, z));
					}
					for (auto& t : K) tets.push_back({ c[t[0]], c[t[1]], c[t[2]], c[t[3]] });
				}
	}
	FSMesh* pm = new FSMesh;
	pm->Create((int)X.size(), (int)tets.size());
	for (size_t i = 0; i < X.size(); ++i) pm->Node((int)i).r = X[i];
	for (size_t i = 0; i < tets.size(); ++i)
	{
		FSElement& el = pm->Element((int)i);
		el.SetType(FE_TET4);
		el.m_gid = 0;
		for (int j = 0; j < 4; ++j) el.m_node[j] = tets[i][j];
	}
	pm->RebuildMesh();
	return pm;
}

static double tetVol(const vec3d& a, const vec3d& b, const vec3d& c, const vec3d& d)
{
	return ((b - a) ^ (c - a)) * (d - a) / 6.0;
}

// Conforming TET4 mesh from hexahedral cells (corners in the usual order: bottom face, then top
// face). Each cell gets a center node and each face a (shared) center node, and is split into 24
// tets, so neighboring cells need not have matching orientations.
static FSMesh* buildFromHexes(const std::vector<std::array<vec3d, 8> >& hexes)
{
	std::map<std::array<long long, 3>, int> index;
	std::vector<vec3d> X;
	auto node = [&](const vec3d& r) {
		std::array<long long, 3> key = { llround(r.x * 1e9), llround(r.y * 1e9), llround(r.z * 1e9) };
		auto it = index.find(key);
		if (it != index.end()) return it->second;
		int id = (int)X.size();
		index[key] = id;
		X.push_back(r);
		return id;
	};
	const int F[6][4] = { {0,3,2,1}, {4,5,6,7}, {0,1,5,4}, {1,2,6,5}, {2,3,7,6}, {3,0,4,7} };	// outward
	std::vector<std::array<int, 4> > tets;
	for (auto& h : hexes)
	{
		vec3d cc(0, 0, 0);
		for (auto& p : h) cc += p;
		int ic = node(cc / 8.0);
		for (auto& f : F)
		{
			vec3d fc = (h[f[0]] + h[f[1]] + h[f[2]] + h[f[3]]) / 4.0;
			int ifc = node(fc);
			for (int j = 0; j < 4; ++j)
			{
				// tet (a, b, face center, cell center) with positive volume for an outward face
				int a = node(h[f[j]]), b = node(h[f[(j + 1) % 4]]);
				tets.push_back({ b, a, ifc, ic });
			}
		}
	}
	FSMesh* pm = new FSMesh;
	pm->Create((int)X.size(), (int)tets.size());
	for (size_t i = 0; i < X.size(); ++i) pm->Node((int)i).r = X[i];
	for (size_t i = 0; i < tets.size(); ++i)
	{
		FSElement& el = pm->Element((int)i);
		el.SetType(FE_TET4);
		el.m_gid = 0;
		for (int j = 0; j < 4; ++j) el.m_node[j] = tets[i][j];
	}
	pm->RebuildMesh();
	return pm;
}

// L-shaped block: the unit cubes at (0,0), (1,0) and (0,1) in the xy-plane, height 1, with n
// cells per unit length. It has a reflex (270 degree) edge along z at x = y = 1.
static FSMesh* buildLBlock(int n)
{
	std::vector<std::array<vec3d, 8> > hexes;
	const int cubes[3][2] = { {0,0}, {1,0}, {0,1} };
	for (auto& cb : cubes)
		for (int k = 0; k < n; ++k)
			for (int j = 0; j < n; ++j)
				for (int i = 0; i < n; ++i)
				{
					std::array<vec3d, 8> h;
					for (int q = 0; q < 8; ++q)
					{
						int di = ((q + 1) / 2) % 2, dj = (q / 2) % 2, dk = q / 4;	// 0,1,1,0 / 0,0,1,1
						h[q] = vec3d(cb[0] + (i + di) / (double)n, cb[1] + (j + dj) / (double)n, (k + dk) / (double)n);
					}
					hexes.push_back(h);
				}
	return buildFromHexes(hexes);
}

// Cylinder of radius 1 and height 1 (axis z) from an O-grid: a central square with n x n cells
// and four blocks with n x m cells between the square and the circle, in l layers.
static FSMesh* buildCylinder(int n, int m, int l)
{
	const double kPi = 3.14159265358979323846;
	std::vector<std::array<vec3d, 8> > hexes;
	// point of the disk: block b (0..3 around the square, or 4 = the square), (u, v) in [0,1]^2
	auto point = [&](int b, double u, double v) {
		if (b == 4) return vec3d(u - 0.5, v - 0.5, 0);
		// block 0 is on the +x side: from the square's edge (0.5, 0.5..-0.5) to the arc 45..-45 deg
		// (u runs along -y so that the cells are right-handed)
		vec3d s(0.5, 0.5 - u, 0);
		double a = 0.25 * kPi - 0.5 * kPi * u;
		vec3d c(cos(a), sin(a), 0);
		vec3d p = s * (1 - v) + c * v;
		double t = 0.5 * kPi * b;
		return vec3d(p.x * cos(t) - p.y * sin(t), p.x * sin(t) + p.y * cos(t), 0);
	};
	for (int b = 0; b < 5; ++b)
	{
		int nv = (b == 4 ? n : m);
		for (int k = 0; k < l; ++k)
			for (int j = 0; j < nv; ++j)
				for (int i = 0; i < n; ++i)
				{
					std::array<vec3d, 8> h;
					for (int q = 0; q < 8; ++q)
					{
						int di = ((q + 1) / 2) % 2, dj = (q / 2) % 2, dk = q / 4;
						h[q] = point(b, (i + di) / (double)n, (j + dj) / (double)nv) + vec3d(0, 0, (k + dk) / (double)l);
					}
					hexes.push_back(h);
				}
	}
	return buildFromHexes(hexes);
}

// total volume of the mesh (hex/penta/pyra split into tets)
static double meshVolume(FSMesh& m)
{
	double V = 0;
	for (int i = 0; i < m.Elements(); ++i)
	{
		FSElement& e = m.Element(i);
		auto r = [&](int j) { return m.Node(e.m_node[j]).r; };
		switch (e.Type())
		{
		case FE_TET4 : V += tetVol(r(0), r(1), r(2), r(3)); break;
		case FE_PYRA5: V += tetVol(r(0), r(1), r(2), r(4)) + tetVol(r(0), r(2), r(3), r(4)); break;
		case FE_PENTA6: V += tetVol(r(0), r(1), r(2), r(3)) + tetVol(r(1), r(2), r(3), r(4)) + tetVol(r(2), r(3), r(4), r(5)); break;
		case FE_HEX8:
		{
			// 6-tet split along diagonal 0-6
			const int K[6][4] = { {0,1,2,6},{0,2,3,6},{0,3,7,6},{0,7,4,6},{0,4,5,6},{0,5,1,6} };
			for (auto& t : K) V += tetVol(r(t[0]), r(t[1]), r(t[2]), r(t[3]));
		}
		break;
		default: break;
		}
	}
	return V;
}

// number of elements with a non-positive Jacobian at an integration point (FEBio's check)
static int invalidElements(FSMesh& m)
{
	int n = 0;
	for (int i = 0; i < m.Elements(); ++i)
		if (FEMeshMetrics::SolidJacobian(m, m.Element(i)) <= 0) n++;
	return n;
}

// smallest scaled Jacobian of the elements at FEBio's integration points (1 = ideal element)
static double minScaledJacobian(FSMesh& m)
{
	std::vector<vec3d> r(m.Nodes());
	for (int i = 0; i < m.Nodes(); ++i) r[i] = m.Node(i).r;
	double q = 1e300;
	for (int i = 0; i < m.Elements(); ++i)
	{
		FSElement& el = m.Element(i);
		FEMeshUntangler::Element e;
		switch (el.Type())
		{
		case FE_TET4  : e.type = FEMeshUntangler::TET4; break;
		case FE_PYRA5 : e.type = FEMeshUntangler::PYRA5; break;
		case FE_PENTA6: e.type = FEMeshUntangler::PENTA6; break;
		default       : e.type = FEMeshUntangler::HEX8; break;
		}
		for (int j = 0; j < 8; ++j) e.n[j] = (j < el.Nodes() ? el.m_node[j] : 0);
		q = std::min(q, FEMeshUntangler::ScaledJacobian(r, e));
	}
	return q;
}

// total volume of the elements (integrated over each element; overlapping elements count twice)
static double elementVolume(FSMesh& m)
{
	double V = 0;
	for (int i = 0; i < m.Elements(); ++i) V += FEMeshMetrics::ElementVolume(m, m.Element(i));
	return V;
}

// every interior face must be shared by exactly two elements
static bool isConforming(FSMesh& m)
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
	for (auto& it : faceCount) if (it.second > 2) return false;
	// all boundary faces of the elements must be mesh faces
	int nboundary = 0;
	for (auto& it : faceCount) if (it.second == 1) nboundary++;
	return (nboundary == m.Faces());
}

TEST(HexDominantTests, CubeIsAllHex)
{
	FSMesh* tet = buildTetCube(12, false);
	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.25);	// target edge length
	FSMesh* hex = mod.Apply(tet);
	ASSERT_NE(hex, nullptr) << mod.GetErrorString();

	EXPECT_EQ(hex->Elements(), 64);
	for (int i = 0; i < hex->Elements(); ++i) EXPECT_EQ(hex->Element(i).Type(), FE_HEX8);
	EXPECT_NEAR(meshVolume(*hex), 1.0, 1e-4);
	EXPECT_TRUE(isConforming(*hex));
	EXPECT_EQ(invalidElements(*hex), 0);

	delete hex;
	delete tet;
}

TEST(HexDominantTests, SphereIsConformingAndValid)
{
	FSMesh* tet = buildTetCube(16, true);
	double V0 = meshVolume(*tet);
	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.3);
	mod.SetIntValue(2, 1);	// one thread: deterministic
	FSMesh* hd = mod.Apply(tet);
	ASSERT_NE(hd, nullptr) << mod.GetErrorString();

	int nhex = 0;
	for (int i = 0; i < hd->Elements(); ++i) if (hd->Element(i).Type() == FE_HEX8) nhex++;
	EXPECT_GT(nhex, 0);
	EXPECT_TRUE(isConforming(*hd));
	EXPECT_EQ(invalidElements(*hd), 0);

	// the boundary nodes lie on the input surface (a triangulation of the unit sphere)
	double rmin = 1e9, rmax = 0;
	for (int i = 0; i < hd->Faces(); ++i)
	{
		FSFace& f = hd->Face(i);
		for (int j = 0; j < f.Nodes(); ++j)
		{
			double R = hd->Node(f.n[j]).r.Length();
			rmin = std::min(rmin, R); rmax = std::max(rmax, R);
		}
	}
	EXPECT_LE(rmax, 1.0 + 1e-9);
	EXPECT_GE(rmin, 0.99);

	// With the boundary nodes on the input surface, the volume lost is the faceting of the
	// boundary faces, which is about 6% for this coarse mesh (R/H = 3.3).
	double V = meshVolume(*hd);
	printf("volume: input %lg, output %lg (%+.2lf%%)\n", V0, V, 100.0 * (V - V0) / V0);
	EXPECT_NEAR(V, V0, 0.07 * V0);

	delete hd;
	delete tet;
}

// With H = 0 the automatic element size (3.5 x the average boundary edge length) is
// reported in the log.
TEST(HexDominantTests, AutomaticSizeIsLogged)
{
	FSMesh* tet = buildTetCube(12, false);
	double Lsum = 0; int nedge = 0;
	for (int i = 0; i < tet->Faces(); ++i)
	{
		FSFace& f = tet->Face(i);
		for (int j = 0; j < 3; ++j) { Lsum += (tet->Node(f.n[j]).r - tet->Node(f.n[(j + 1) % 3]).r).Length(); nedge++; }
	}
	double Hexpected = 3.5 * Lsum / nedge;

	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.0);
	mod.SetIntValue(2, 1);
	FSMesh* hd = mod.Apply(tet);
	ASSERT_NE(hd, nullptr) << mod.GetErrorString();

	const std::string key = "Element size: H = ";
	size_t pos = logger.text.find(key);
	ASSERT_NE(pos, std::string::npos);
	EXPECT_NE(logger.text.find("(automatic", pos), std::string::npos);
	double H = atof(logger.text.c_str() + pos + key.size());
	EXPECT_NEAR(H, Hexpected, 1e-5 * Hexpected);
	EXPECT_EQ(invalidElements(*hd), 0);

	delete hd;
	delete tet;
}

// Terminate() (what the GUI's Cancel button calls) stops a running remesh, and the same
// modifier object can be applied again afterwards.
TEST(HexDominantTests, CancelStopsAndRerunWorks)
{
	FSMesh* tet = buildTetCube(12, false);
	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.25);
	mod.SetIntValue(2, 1);

	FSMesh* hd = nullptr;
	std::thread worker([&]() { hd = mod.Apply(tet); });
	// cancel once the field optimization is running
	auto t0 = std::chrono::steady_clock::now();
	while ((mod.GetProgress().percent < 10.0) && (std::chrono::steady_clock::now() - t0 < std::chrono::seconds(60)))
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	mod.Terminate();
	worker.join();
	EXPECT_EQ(hd, nullptr);
	EXPECT_NE(mod.GetErrorString().find("Canceled"), std::string::npos);
	EXPECT_NE(logger.text.find("Hex-dominant remesh canceled."), std::string::npos);

	// rerun: the cancel flag and the error of the previous run are cleared
	hd = mod.Apply(tet);
	ASSERT_NE(hd, nullptr) << mod.GetErrorString();
	EXPECT_TRUE(mod.GetErrorString().empty());
	EXPECT_EQ(hd->Elements(), 64);
	EXPECT_EQ(invalidElements(*hd), 0);

	delete hd;
	delete tet;
}

// The edges of a hexagonal prism meet at 120 degrees, which a hex lattice cannot follow on both
// sides. They are recovered by snapping boundary nodes onto them: the boundary is then exactly
// the prism, and the output surface splits into its eight faces.
TEST(HexDominantTests, PrismEdgesAreRecovered)
{
	FSMesh* tet = buildHexPrism(6, 6);
	EXPECT_TRUE(isConforming(*tet));
	const double V0 = 1.5 * sqrt(3.0);
	EXPECT_NEAR(meshVolume(*tet), V0, 1e-9);

	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.25);
	mod.SetIntValue(2, 1);
	FSMesh* hd = mod.Apply(tet);
	ASSERT_NE(hd, nullptr) << mod.GetErrorString();

	EXPECT_NE(logger.text.find("recovered 12 of 12 corners and 18 of 18 feature curves"), std::string::npos);
	EXPECT_TRUE(isConforming(*hd));
	EXPECT_EQ(invalidElements(*hd), 0);

	// volume enclosed by the boundary faces (exact for planar faces): the prism's volume
	double Vb = 0;
	for (int i = 0; i < hd->Faces(); ++i)
	{
		FSFace& f = hd->Face(i);
		int nn = f.Nodes();
		vec3d c(0, 0, 0);
		for (int j = 0; j < nn; ++j) c += hd->Node(f.n[j]).r;
		c /= (double)nn;
		for (int j = 0; j < nn; ++j) Vb += tetVol(vec3d(0, 0, 0), c, hd->Node(f.n[j]).r, hd->Node(f.n[(j + 1) % nn]).r);
	}
	EXPECT_NEAR(Vb, V0, 1e-9 * V0);

	// eight surfaces (top, bottom, six sides) and every boundary node on the prism's surface
	std::set<int> surfaces;
	double dmax = 0;
	const double kPi = 3.14159265358979323846;
	for (int i = 0; i < hd->Faces(); ++i)
	{
		FSFace& f = hd->Face(i);
		surfaces.insert(f.m_gid);
		for (int j = 0; j < f.Nodes(); ++j)
		{
			vec3d r = hd->Node(f.n[j]).r;
			double d = std::min(fabs(r.z), fabs(r.z - 1.0));
			// distance to the side planes (apothem sqrt(3)/2, normals at 30 + 60k degrees)
			double ds = 1e9;
			for (int k = 0; k < 6; ++k)
			{
				double phi = kPi / 6 + k * kPi / 3;
				ds = std::min(ds, fabs(r.x * cos(phi) + r.y * sin(phi) - 0.5 * sqrt(3.0)));
			}
			dmax = std::max(dmax, std::min(d, ds));
		}
	}
	EXPECT_EQ((int)surfaces.size(), 8);
	EXPECT_LT(dmax, 1e-9);

	delete hd;
	delete tet;
}

#ifdef HAS_MMG
// A tet mesh that is too coarse for the target size is remeshed with MMG first.
TEST(HexDominantTests, CoarseInputIsRemeshed)
{
	FSMesh* tet = buildTetCube(3, false);
	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.25);
	mod.SetIntValue(2, 1);
	FSMesh* hd = mod.Apply(tet);
	ASSERT_NE(hd, nullptr) << mod.GetErrorString();

	EXPECT_NE(logger.text.find("Remeshed the input with MMG"), std::string::npos);
	int nhex = 0;
	for (int i = 0; i < hd->Elements(); ++i) if (hd->Element(i).Type() == FE_HEX8) nhex++;
	EXPECT_GE(nhex, 48);	// 53 of 64 cells at the time of writing
	EXPECT_TRUE(isConforming(*hd));
	EXPECT_EQ(invalidElements(*hd), 0);
	EXPECT_NEAR(meshVolume(*hd), 1.0, 1e-3);

	delete hd;
	delete tet;
}
#endif

// Block [-1,1] x [-1,1] x [0,1] with a cylindrical hole of radius 0.5 along z, from four blocks
// with n x m cells between the hole and the outer square, in l layers.
static FSMesh* buildBlockWithHole(int n, int m, int l)
{
	const double kPi = 3.14159265358979323846;
	std::vector<std::array<vec3d, 8> > hexes;
	auto point = [&](int b, double u, double v) {
		// block 0 is on the +x side, from the arc 45..-45 deg to the edge (1, 1..-1)
		double a = 0.25 * kPi - 0.5 * kPi * u;
		vec3d c(0.5 * cos(a), 0.5 * sin(a), 0);
		vec3d s(1.0, 1.0 - 2.0 * u, 0);
		vec3d p = c * (1 - v) + s * v;
		double t = 0.5 * kPi * b;
		return vec3d(p.x * cos(t) - p.y * sin(t), p.x * sin(t) + p.y * cos(t), 0);
	};
	for (int b = 0; b < 4; ++b)
		for (int k = 0; k < l; ++k)
			for (int j = 0; j < m; ++j)
				for (int i = 0; i < n; ++i)
				{
					std::array<vec3d, 8> h;
					for (int q = 0; q < 8; ++q)
					{
						int di = ((q + 1) / 2) % 2, dj = (q / 2) % 2, dk = q / 4;
						h[q] = point(b, (i + di) / (double)n, (j + dj) / (double)m) + vec3d(0, 0, (k + dk) / (double)l);
					}
					hexes.push_back(h);
				}
	return buildFromHexes(hexes);
}

// Number of surfaces, and the volume enclosed by the boundary faces
static void boundaryStats(FSMesh& m, int& nsurf, double& Vb)
{
	std::set<int> surfaces;
	Vb = 0;
	for (int i = 0; i < m.Faces(); ++i)
	{
		FSFace& f = m.Face(i);
		surfaces.insert(f.m_gid);
		int nn = f.Nodes();
		vec3d c(0, 0, 0);
		for (int j = 0; j < nn; ++j) c += m.Node(f.n[j]).r;
		c /= (double)nn;
		for (int j = 0; j < nn; ++j) Vb += tetVol(vec3d(0, 0, 0), c, m.Node(f.n[j]).r, m.Node(f.n[(j + 1) % nn]).r);
	}
	nsurf = (int)surfaces.size();
}

// An L-shaped block has a reflex edge; all 18 edges and 12 corners are recovered.
TEST(HexDominantTests, LBlockEdgesAreRecovered)
{
	FSMesh* tet = buildLBlock(5);
	EXPECT_TRUE(isConforming(*tet));
	EXPECT_NEAR(meshVolume(*tet), 3.0, 1e-9);

	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.25);
	mod.SetIntValue(2, 1);
	mod.SetIntValue(4, 2);	// do not remesh the input
	FSMesh* hd = mod.Apply(tet);
	ASSERT_NE(hd, nullptr) << mod.GetErrorString();

	EXPECT_NE(logger.text.find("recovered 12 of 12 corners and 18 of 18 feature curves"), std::string::npos);
	EXPECT_TRUE(isConforming(*hd));
	EXPECT_EQ(invalidElements(*hd), 0);
	int nsurf = 0; double Vb = 0;
	boundaryStats(*hd, nsurf, Vb);
	EXPECT_EQ(nsurf, 8);
	EXPECT_NEAR(Vb, 3.0, 1e-9);
	delete hd;
	delete tet;
}

// The edges of a cylinder are closed curves without corners.
TEST(HexDominantTests, CylinderEdgesAreRecovered)
{
	FSMesh* tet = buildCylinder(6, 4, 5);
	EXPECT_TRUE(isConforming(*tet));
	double V0 = meshVolume(*tet);

	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.25);
	mod.SetIntValue(2, 1);
	mod.SetIntValue(4, 2);	// do not remesh the input
	FSMesh* hd = mod.Apply(tet);
	ASSERT_NE(hd, nullptr) << mod.GetErrorString();

	EXPECT_NE(logger.text.find("recovered 0 of 0 corners and 2 of 2 feature curves"), std::string::npos);
	EXPECT_TRUE(isConforming(*hd));
	EXPECT_EQ(invalidElements(*hd), 0);
	int nsurf = 0; double Vb = 0;
	boundaryStats(*hd, nsurf, Vb);
	EXPECT_EQ(nsurf, 3);
	EXPECT_NEAR(Vb, V0, 0.02 * V0);
	// all nodes inside the cylinder, and the boundary nodes of the top and bottom on their planes
	double rmax = 0, zmin = 1e9, zmax = -1e9;
	for (int i = 0; i < hd->Nodes(); ++i)
	{
		vec3d r = hd->Node(i).r;
		rmax = std::max(rmax, sqrt(r.x * r.x + r.y * r.y));
		zmin = std::min(zmin, r.z); zmax = std::max(zmax, r.z);
	}
	EXPECT_LE(rmax, 1.0 + 1e-9);
	EXPECT_NEAR(zmin, 0.0, 1e-9);
	EXPECT_NEAR(zmax, 1.0, 1e-9);
	printf("volume: input %lg, output %lg (%.2f%%)\n", V0, Vb, 100.0 * (Vb - V0) / V0);
	delete hd;
	delete tet;
}

// A block with a hole: straight edges and corners on the outside, closed curves at the hole.
// (At H = 0.25, i.e. half the hole's radius, the coarse faceting of the hole can leave single
// tilted faces near its rim that FEBio Studio's surface partition splits off.)
TEST(HexDominantTests, BlockWithHoleEdgesAreRecovered)
{
	FSMesh* tet = buildBlockWithHole(8, 4, 5);
	EXPECT_TRUE(isConforming(*tet));
	double V0 = meshVolume(*tet);

	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.2);
	mod.SetIntValue(2, 1);
	mod.SetIntValue(4, 2);	// do not remesh the input
	FSMesh* hd = mod.Apply(tet);
	ASSERT_NE(hd, nullptr) << mod.GetErrorString();

	EXPECT_NE(logger.text.find("recovered 8 of 8 corners and 14 of 14 feature curves"), std::string::npos);
	EXPECT_TRUE(isConforming(*hd));
	EXPECT_EQ(invalidElements(*hd), 0);
	int nsurf = 0; double Vb = 0;
	boundaryStats(*hd, nsurf, Vb);
	EXPECT_EQ(nsurf, 7);
	EXPECT_NEAR(Vb, V0, 0.02 * V0);
	printf("volume: input %lg, output %lg (%.2f%%)\n", V0, Vb, 100.0 * (Vb - V0) / V0);
	delete hd;
	delete tet;
}

// Near the 120 degree edges of a prism meshed without MMG, the extraction leaves cells that
// fold over or have no volume, with no valid split point. They are merged with the region
// around them that a single point sees completely, instead of being left to the optimization
// (which leaves elements with a scaled Jacobian of about 0.05 here).
TEST(HexDominantTests, FoldedCellsAreMerged)
{
	FSMesh* tet = buildHexPrism(8, 8);
	const double V0 = 1.5 * sqrt(3.0);
	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.15);
	mod.SetIntValue(2, 1);
	mod.SetIntValue(4, 2);	// never remesh the input with MMG
	FSMesh* hd = mod.Apply(tet);
	ASSERT_NE(hd, nullptr) << mod.GetErrorString();

	EXPECT_NE(logger.text.find("around folded or flat cells"), std::string::npos);
	EXPECT_NE(logger.text.find("recovered 12 of 12 corners and 18 of 18 feature curves"), std::string::npos);
	EXPECT_TRUE(isConforming(*hd));
	EXPECT_EQ(invalidElements(*hd), 0);
	EXPECT_GT(minScaledJacobian(*hd), 0.1);
	// the elements do not overlap: they fill exactly the prism's volume
	EXPECT_NEAR(elementVolume(*hd), V0, 1e-6 * V0);
	delete hd;
	delete tet;
}

// Refine Mesh splits the hex8, penta6, pyra5 and tet4 elements of a hex-dominant mesh. The
// refined mesh is conforming and valid, fills the same volume and keeps the 8 surfaces of the
// prism; with smoothing, the flat sides stay flat and the edges straight.
TEST(HexDominantTests, RefinedMeshIsValid)
{
	FSMesh* tet = buildHexPrism(8, 8);
	HexDominantRemesh hdmod;
	hdmod.SetFloatValue(0, 0.15);
	hdmod.SetIntValue(2, 1);
	hdmod.SetIntValue(4, 2);	// never remesh the input with MMG
	FSMesh* hd = hdmod.Apply(tet);
	ASSERT_NE(hd, nullptr) << hdmod.GetErrorString();

	int ntype[4] = { 0 };
	for (int i = 0; i < hd->Elements(); ++i)
	{
		switch (hd->Element(i).Type())
		{
		case FE_HEX8  : ntype[0]++; break;
		case FE_PENTA6: ntype[1]++; break;
		case FE_PYRA5 : ntype[2]++; break;
		case FE_TET4  : ntype[3]++; break;
		}
	}
	printf("hex-dominant mesh: %d hex8, %d penta6, %d pyra5, %d tet4\n", ntype[0], ntype[1], ntype[2], ntype[3]);
	EXPECT_GT(ntype[0], 0);
	EXPECT_GT(ntype[2], 0);
	EXPECT_GT(ntype[3], 0);

	int nsurf0; double Vb0;
	boundaryStats(*hd, nsurf0, Vb0);
	const double V0 = elementVolume(*hd);
	EXPECT_EQ(nsurf0, 8);

	for (int smooth = 0; smooth < 2; ++smooth)
	{
		RefineMesh mod;
		StdoutLogger logger;
		mod.SetTaskLogger(&logger);
		mod.SetIntValue(0, 1);
		mod.SetBoolValue(1, smooth == 1);
		mod.SetBoolValue(2, false);
		FSMesh* pr = mod.Apply(hd);
		ASSERT_NE(pr, nullptr) << mod.GetErrorString();

		EXPECT_EQ(pr->Elements(), 8 * (hd->Elements() - ntype[2]) + 10 * ntype[2]);
		EXPECT_TRUE(isConforming(*pr));
		EXPECT_EQ(invalidElements(*pr), 0);
		EXPECT_NEAR(elementVolume(*pr), V0, 1e-9 * V0);
		int nsurf; double Vb;
		boundaryStats(*pr, nsurf, Vb);
		EXPECT_EQ(nsurf, nsurf0);
		EXPECT_NEAR(Vb, Vb0, 1e-9 * Vb0);
		printf("smooth = %d: smallest scaled Jacobian %lg (was %lg)\n", smooth, minScaledJacobian(*pr), minScaledJacobian(*hd));
		delete pr;
	}

	delete hd;
	delete tet;
}

// Smoothing the surface of a refined hex-dominant sphere makes it smoother (smaller angles
// between neighboring faces) and keeps the elements valid.
TEST(HexDominantTests, RefinedSphereIsSmoothed)
{
	FSMesh* tet = buildTetCube(16, true);
	HexDominantRemesh hdmod;
	hdmod.SetFloatValue(0, 0.3);
	hdmod.SetIntValue(2, 1);
	FSMesh* hd = hdmod.Apply(tet);
	ASSERT_NE(hd, nullptr) << hdmod.GetErrorString();

	// largest angle between the normals of neighboring boundary faces
	auto maxAngle = [](FSMesh& m) {
		auto normal = [&](FSFace& f) {
			vec3d r[4];
			for (int j = 0; j < f.Nodes(); ++j) r[j] = m.Node(f.n[j]).r;
			vec3d n = (f.Nodes() == 4 ? (r[2] - r[0]) ^ (r[3] - r[1]) : (r[1] - r[0]) ^ (r[2] - r[0]));
			n.unit();
			return n;
		};
		double amax = 0;
		for (int i = 0; i < m.Faces(); ++i)
		{
			FSFace& f = m.Face(i);
			for (int j = 0; j < f.Edges(); ++j)
			{
				if (f.m_nbr[j] < 0) continue;
				double c = normal(f) * normal(m.Face(f.m_nbr[j]));
				amax = std::max(amax, acos(std::max(-1.0, std::min(1.0, c))) * 180.0 / M_PI);
			}
		}
		return amax;
	};

	double a[2];
	for (int smooth = 0; smooth < 2; ++smooth)
	{
		RefineMesh mod;
		StdoutLogger logger;
		mod.SetTaskLogger(&logger);
		mod.SetIntValue(0, 1);
		mod.SetBoolValue(1, smooth == 1);
		mod.SetBoolValue(2, false);
		FSMesh* pr = mod.Apply(hd);
		ASSERT_NE(pr, nullptr) << mod.GetErrorString();
		EXPECT_TRUE(isConforming(*pr));
		EXPECT_EQ(invalidElements(*pr), 0);
		a[smooth] = maxAngle(*pr);
		printf("smooth = %d: largest angle between neighboring faces %.1lf degrees, smallest scaled Jacobian %lg\n", smooth, a[smooth], minScaledJacobian(*pr));
		delete pr;
	}
	EXPECT_LT(a[1], 0.75 * a[0]);

	delete hd;
	delete tet;
}

// Near the surface, a polyhedron can get a pyramid on a warped face whose Jacobian is negative at
// a corner of its base (the apex is below the plane of the two base edges there), although FEBio's
// integration points see a valid element. Refining such a mesh inverts the children in that
// corner. The remesher replaces these pyramids (three here, where the worst element was 0.07 and
// only splitting the elements gave two inverted ones): no pyramid has a folded corner, and the
// elements of the mesh can be split.
TEST(HexDominantTests, FoldedPyramidsAreReplaced)
{
	FSMesh* tet = buildHexPrism(7, 7);
	const double V0 = 1.5 * sqrt(3.0);
	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.35);
	mod.SetIntValue(2, 1);
	FSMesh* hd = mod.Apply(tet);
	ASSERT_NE(hd, nullptr) << mod.GetErrorString();

	EXPECT_NE(logger.text.find("with a folded or nearly flat corner"), std::string::npos);
	EXPECT_EQ(logger.text.find("Warning"), std::string::npos);
	EXPECT_TRUE(isConforming(*hd));
	EXPECT_EQ(invalidElements(*hd), 0);
	EXPECT_GT(minScaledJacobian(*hd), 0.1);
	EXPECT_NEAR(elementVolume(*hd), V0, 1e-6 * V0);

	std::vector<vec3d> r(hd->Nodes());
	for (int i = 0; i < hd->Nodes(); ++i) r[i] = hd->Node(i).r;
	double corner = 1;
	for (int i = 0; i < hd->Elements(); ++i)
	{
		FSElement& el = hd->Element(i);
		if (el.Type() != FE_PYRA5) continue;
		FEMeshUntangler::Element e;
		e.type = FEMeshUntangler::PYRA5;
		for (int j = 0; j < 8; ++j) e.n[j] = el.m_node[j < 5 ? j : 0];
		corner = std::min(corner, FEMeshUntangler::CornerScaledJacobian(r, e));
	}
	EXPECT_GT(corner, 0.0);

	// only splitting the elements gives valid elements
	FEMixedSplitModifier split;
	split.DoElementRepair(false);
	FSMesh* p0 = split.Apply(hd);
	ASSERT_NE(p0, nullptr);
	EXPECT_EQ(invalidElements(*p0), 0);
	delete p0;

	delete hd;
	delete tet;
}

// Manual benchmark: run with --gtest_also_run_disabled_tests
TEST(HexDominantTests, DISABLED_FineSphere)
{
	FSMesh* tet = buildTetCube(32, true);
	double V0 = meshVolume(*tet);
	HexDominantRemesh mod;
	StdoutLogger logger;
	mod.SetTaskLogger(&logger);
	mod.SetFloatValue(0, 0.15);
	FSMesh* hd = mod.Apply(tet);
	ASSERT_NE(hd, nullptr) << mod.GetErrorString();
	EXPECT_TRUE(isConforming(*hd));
	EXPECT_EQ(invalidElements(*hd), 0);
	double V = meshVolume(*hd);
	printf("volume: input %lg, output %lg (%+.2lf%%)\n", V0, V, 100.0 * (V - V0) / V0);
	delete hd;
	delete tet;
}
