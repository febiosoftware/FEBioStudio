# rhdm — robust hex-dominant meshing (vendored)

Source: https://github.com/gaoxifeng/robust_hex_dominant_meshing (MIT, see `LICENSE-rhdm`)

> X. Gao, W. Jakob, M. Tarini, D. Panozzo. *Robust Hex-Dominant Mesh Generation
> using Field-Guided Polyhedral Agglomeration*. ACM TOG (SIGGRAPH 2017).

`pcg32.h` is from https://github.com/wjakob/pcg32 (Apache 2.0).

Used by `MeshTools/FEHexDominantRemesh.{h,cpp}` ("Hex-dominant Remesh" mesh modifier). The
conversion to FEBio elements and the post-processing (projection onto the input surface,
split points by linear programming, untangling) are FEBio Studio code, not part of this
directory: see `FEHexDominantRemesh.cpp`, `FESurfaceProjector.{h,cpp}` and `FEMeshUntangler.{h,cpp}`.
Built only when CMake finds Eigen (`USE_HEXDOM`, `EIGEN3_INC`), which defines `HAS_HEXDOM`.

## Changes from upstream

Only the volume (3D) pipeline is kept. Viewer, batch, mesh I/O, BVH, subdivision and
the 2D-only helpers are not included.

- All code is wrapped in `namespace rhdm`, so names like `Mesh` and `Frame` cannot collide with FEBio Studio's.
- `hierarchy.{h,cpp}`: the OBJ loader `load()` and the TetGen-based `tet_meshing()` are replaced by
  `setTetMesh(V, T)`. It takes an existing tet mesh, extracts and orients its boundary faces,
  and sets up the bounding box and scale. The BVH (unused in 3D) and the nanogui serializer are removed.
- `common.h`: TBB is replaced by `tbb_compat.h`, which provides `parallel_for` on `std::thread`,
  serial reductions, and a spin mutex. Single precision is forced, matching upstream.
- `extract3D.cpp`: `meshExtraction3D()` now returns a value. Upstream had no return statement, which is
  undefined behavior. The global edge queue and the outputs are reset at entry so the mesher can run
  more than once per process.
- `system("PAUSE")` debugging stops now throw `std::runtime_error`.
- `extract3D.cpp`: the depth-first ring search `recursive_ring()` (used to split polyhedra) has a
  budget of calls per search and per polyhedron, and polyhedra with more than 250 edges are not
  searched. `split_polyhedral3D()` skips polyhedra that could not be split before and have not
  changed (until the next reindexing). Without these limits the search can run for hours on
  large polyhedra, e.g. when the tets are coarser than the target size; with them, typical
  meshes are unchanged and the extraction is faster. The search also polls
  `MultiResolutionHierarchy::mCancel`, so the extraction can be canceled.
- `rhdm_api.{h,cpp}`: a small Eigen-free interface. It runs the upstream batch pipeline serially:
  hierarchy, then orientation and position field smoothing (coarse to fine), then extraction.
- Sharp features (new; upstream drops the boundary constraint where the face normals at a
  vertex disagree, and otherwise ignores sharp edges). `HexDomParams::featureType/featureDir`
  mark the input nodes on sharp edges (with the edge tangent) and on corners;
  `MultiResolutionHierarchy::setFeatures()` stores them, and `build()` carries them through the
  hierarchy (`mFType`, `mFT`, `mFC`; a coarse vertex keeps the strongest constraint of the pair it
  replaces). A vertex on a sharp edge aligns a frame axis with the edge (`smoothOrientationsTet`,
  `prolongOrientations`) and puts a lattice line on it; a corner puts a lattice point on itself
  (`alignToLine`, `alignToPoint` in `positions.h`, used by `smoothPositionsTet` and
  `prolongPositions`). The lattice then runs along sharp edges instead of cutting them.
