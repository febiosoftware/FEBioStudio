/*
  This file contains docstrings for use in the Python bindings.
  Do not edit! They were automatically extracted by pybind11_mkdoc.
 */

#define __EXPAND(x)                                      x
#define __COUNT(_1, _2, _3, _4, _5, _6, _7, COUNT, ...)  COUNT
#define __VA_SIZE(...)                                   __EXPAND(__COUNT(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0))
#define __CAT1(a, b)                                     a ## b
#define __CAT2(a, b)                                     __CAT1(a, b)
#define __DOC1(n1)                                       __doc_##n1
#define __DOC2(n1, n2)                                   __doc_##n1##_##n2
#define __DOC3(n1, n2, n3)                               __doc_##n1##_##n2##_##n3
#define __DOC4(n1, n2, n3, n4)                           __doc_##n1##_##n2##_##n3##_##n4
#define __DOC5(n1, n2, n3, n4, n5)                       __doc_##n1##_##n2##_##n3##_##n4##_##n5
#define __DOC6(n1, n2, n3, n4, n5, n6)                   __doc_##n1##_##n2##_##n3##_##n4##_##n5##_##n6
#define __DOC7(n1, n2, n3, n4, n5, n6, n7)               __doc_##n1##_##n2##_##n3##_##n4##_##n5##_##n6##_##n7
#define DOC(...)                                         __EXPAND(__EXPAND(__CAT2(__DOC, __VA_SIZE(__VA_ARGS__)))(__VA_ARGS__))

#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif


static const char *__doc_FEFaceShape = R"doc(Face shapes)doc";

static const char *__doc_FEFaceShape_FE_FACE_INVALID_SHAPE = R"doc()doc";

static const char *__doc_FEFaceShape_FE_FACE_QUAD = R"doc()doc";

static const char *__doc_FEFaceShape_FE_FACE_TRI = R"doc()doc";

static const char *__doc_FEFaceType = R"doc(Different face types (NOTE: Do not change the order of these values))doc";

static const char *__doc_FEFaceType_FE_FACE_INVALID_TYPE = R"doc()doc";

static const char *__doc_FEFaceType_FE_FACE_QUAD4 = R"doc()doc";

static const char *__doc_FEFaceType_FE_FACE_QUAD8 = R"doc()doc";

static const char *__doc_FEFaceType_FE_FACE_QUAD9 = R"doc()doc";

static const char *__doc_FEFaceType_FE_FACE_TRI10 = R"doc()doc";

static const char *__doc_FEFaceType_FE_FACE_TRI3 = R"doc()doc";

static const char *__doc_FEFaceType_FE_FACE_TRI6 = R"doc()doc";

static const char *__doc_FEFaceType_FE_FACE_TRI7 = R"doc()doc";

static const char *__doc_FSCurveMesh =
R"doc(A class that represents a mesh for a curve This mesh only consists of
nodes and edges)doc";

static const char *__doc_FSCurveMesh_AddEdge =
R"doc(adds an edge between nodes n0, n1 If the edge already exists, it will
not be added Returns the index of the new (or existing) edge (or -1 if
adding the edge has failed (e.g. if n0==n1)) Note that this will set
the type to INVALID_CURVE Call Update to reevaluate the curve mesh)doc";

static const char *__doc_FSCurveMesh_AddNode =
R"doc(Add a node to the mesh. Returns the index of the newly added node If
the snap flag is on, then it will be checked if the point already
exists in the mesh)doc";

static const char *__doc_FSCurveMesh_Attach = R"doc(attach another curve to this one)doc";

static const char *__doc_FSCurveMesh_BoundingBox = R"doc(get the bounding box of the mesh)doc";

static const char *__doc_FSCurveMesh_BuildMesh = R"doc(rebuild mesh data)doc";

static const char *__doc_FSCurveMesh_Clear = R"doc(clear curve data)doc";

static const char *__doc_FSCurveMesh_Create = R"doc(allocate storage for mesh)doc";

static const char *__doc_FSCurveMesh_CreateFromPoints = R"doc(Create curve mesh from a vector of points)doc";

static const char *__doc_FSCurveMesh_CurveType = R"doc(curve types)doc";

static const char *__doc_FSCurveMesh_CurveType_CLOSED_CURVE = R"doc(a single closed loop)doc";

static const char *__doc_FSCurveMesh_CurveType_COMPLEX_CURVE = R"doc(anything else)doc";

static const char *__doc_FSCurveMesh_CurveType_EMPTY_CURVE = R"doc(an empty curve (no nodes or edges))doc";

static const char *__doc_FSCurveMesh_CurveType_INVALID_CURVE = R"doc(there is an error in this mesh)doc";

static const char *__doc_FSCurveMesh_CurveType_SIMPLE_CURVE = R"doc(a single curve with two end-points)doc";

static const char *__doc_FSCurveMesh_EndPointList = R"doc(return a list of end points)doc";

static const char *__doc_FSCurveMesh_EndPoints = R"doc(count the number of end points)doc";

static const char *__doc_FSCurveMesh_FSCurveMesh = R"doc(constructor)doc";

static const char *__doc_FSCurveMesh_FlipEdge = R"doc(Flip the orientation of an edge)doc";

static const char *__doc_FSCurveMesh_Invert = R"doc(invert the order of the nodes)doc";

static const char *__doc_FSCurveMesh_Length = R"doc(returns the total lenght of all edge segments)doc";

static const char *__doc_FSCurveMesh_Load = R"doc(Load from archive)doc";

static const char *__doc_FSCurveMesh_RemoveNode = R"doc(remove a node (and connecting edges))doc";

static const char *__doc_FSCurveMesh_ReorderEdges = R"doc(reorder edges based on look-up table)doc";

static const char *__doc_FSCurveMesh_ReorderNodes = R"doc(reorder nodes based on look-up table)doc";

static const char *__doc_FSCurveMesh_Save = R"doc(Serialization)doc";

static const char *__doc_FSCurveMesh_Segments = R"doc(count the curve segments (a segment is a simply-connected curve))doc";

static const char *__doc_FSCurveMesh_Sort = R"doc(sort the nodes and edges based on edge connectivity)doc";

static const char *__doc_FSCurveMesh_TagAllEdges = R"doc(tag all the edges of this mesh)doc";

static const char *__doc_FSCurveMesh_TagAllNodes = R"doc(tag all the nodes of this mesh)doc";

static const char *__doc_FSCurveMesh_Type = R"doc(return the type of the curve)doc";

static const char *__doc_FSCurveMesh_UpdateEdgeIDs = R"doc(Update edge IDs)doc";

static const char *__doc_FSCurveMesh_UpdateEdgeNeighbors = R"doc(Update edge neighbor information)doc";

static const char *__doc_FSCurveMesh_UpdateMesh = R"doc(update mesh data)doc";

static const char *__doc_FSCurveMesh_UpdateNodeIDs = R"doc(Update node IDs)doc";

static const char *__doc_FSCurveMesh_UpdateType = R"doc(Update the curve type classification)doc";

static const char *__doc_FSCurveMesh_m_type = R"doc(curve type)doc";

static const char *__doc_FSElemClass = R"doc(Element class)doc";

static const char *__doc_FSElemClass_ELEM_BEAM = R"doc()doc";

static const char *__doc_FSElemClass_ELEM_SHELL = R"doc()doc";

static const char *__doc_FSElemClass_ELEM_SOLID = R"doc()doc";

static const char *__doc_FSElemShape = R"doc(Element shapes)doc";

static const char *__doc_FSElemShape_ELEM_HEX = R"doc()doc";

static const char *__doc_FSElemShape_ELEM_LINE = R"doc()doc";

static const char *__doc_FSElemShape_ELEM_PENTA = R"doc()doc";

static const char *__doc_FSElemShape_ELEM_PYRA = R"doc()doc";

static const char *__doc_FSElemShape_ELEM_QUAD = R"doc()doc";

static const char *__doc_FSElemShape_ELEM_TET = R"doc()doc";

static const char *__doc_FSElemShape_ELEM_TRI = R"doc()doc";

static const char *__doc_FSElemTraits = R"doc(Element traits)doc";

static const char *__doc_FSElemTraits_edges = R"doc(number of edges (only for shell elements))doc";

static const char *__doc_FSElemTraits_faces = R"doc(number of faces (only for solid elements))doc";

static const char *__doc_FSElemTraits_nclass = R"doc(element class)doc";

static const char *__doc_FSElemTraits_nodes = R"doc(number of nodes)doc";

static const char *__doc_FSElemTraits_nshape = R"doc(shape of element)doc";

static const char *__doc_FSElemTraits_ntype = R"doc(type of element)doc";

static const char *__doc_FSElement =
R"doc(The FSElement_ class defines the data interface to the element data.
Specialized element classes are then defined by deriving from this
base class. A note on shells: - shells require a thickness, which is
stored in _h. - shells can lie on top of solids or sandwhiched between
solids For such shells, the _nbr[4] and _nbr[5] are used to identify
the elements on top of which they sit.)doc";

static const char *__doc_FSElement_2 =
R"doc(The FSElement class can be used to represent a general purpose
element. This class can represent an element of all different types.)doc";

static const char *__doc_FSElementBase = R"doc(Base element class template)doc";

static const char *__doc_FSElementBase_FSElementBase = R"doc(constructor)doc";

static const char *__doc_FSElementBase_FSElementBase_2 = R"doc(copy constructor)doc";

static const char *__doc_FSElementBase_face = R"doc(faces (-1 for interior faces))doc";

static const char *__doc_FSElementBase_h = R"doc(element thickness (only used by shells))doc";

static const char *__doc_FSElementBase_nbr = R"doc(neighbour elements)doc";

static const char *__doc_FSElementBase_node = R"doc(node array)doc";

static const char *__doc_FSElementBase_operator_assign = R"doc(assignment operator)doc";

static const char *__doc_FSElementData = R"doc()doc";

static const char *__doc_FSElementTraits = R"doc(Element traits classes)doc";

static const char *__doc_FSElementTraits_2 = R"doc()doc";

static const char *__doc_FSElementTraits_3 = R"doc()doc";

static const char *__doc_FSElementTraits_4 = R"doc()doc";

static const char *__doc_FSElementTraits_5 = R"doc()doc";

static const char *__doc_FSElementTraits_6 = R"doc()doc";

static const char *__doc_FSElementTraits_7 = R"doc()doc";

static const char *__doc_FSElementTraits_8 = R"doc()doc";

static const char *__doc_FSElementTraits_9 = R"doc()doc";

static const char *__doc_FSElementTraits_10 = R"doc()doc";

static const char *__doc_FSElementTraits_11 = R"doc()doc";

static const char *__doc_FSElementTraits_12 = R"doc()doc";

static const char *__doc_FSElementTraits_13 = R"doc()doc";

static const char *__doc_FSElementTraits_14 = R"doc()doc";

static const char *__doc_FSElementTraits_15 = R"doc()doc";

static const char *__doc_FSElementTraits_16 = R"doc()doc";

static const char *__doc_FSElementTraits_17 = R"doc()doc";

static const char *__doc_FSElementTraits_18 = R"doc()doc";

static const char *__doc_FSElementTraits_19 = R"doc()doc";

static const char *__doc_FSElementTraits_20 = R"doc()doc";

static const char *__doc_FSElementTraits_Type = R"doc()doc";

static const char *__doc_FSElementTraits_Type_2 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_3 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_4 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_5 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_6 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_7 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_8 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_9 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_10 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_11 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_12 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_13 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_14 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_15 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_16 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_17 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_18 = R"doc()doc";

static const char *__doc_FSElementTraits_Type_19 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_271_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_271_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_271_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_271_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_271_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_271_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_272_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_272_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_272_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_272_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_272_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_272_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_273_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_273_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_273_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_273_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_273_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_273_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_274_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_274_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_274_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_274_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_274_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_274_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_275_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_275_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_275_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_275_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_275_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_275_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_276_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_276_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_276_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_276_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_276_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_276_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_277_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_277_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_277_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_277_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_277_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_277_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_278_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_278_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_278_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_278_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_278_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_278_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_279_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_279_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_279_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_279_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_279_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_279_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_280_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_280_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_280_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_280_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_280_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_280_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_281_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_281_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_281_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_281_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_281_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_281_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_282_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_282_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_282_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_282_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_282_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_282_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_283_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_283_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_283_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_283_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_283_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_283_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_284_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_284_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_284_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_284_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_284_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_284_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_285_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_285_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_285_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_285_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_285_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_285_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_286_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_286_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_286_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_286_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_286_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_286_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_287_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_287_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_287_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_287_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_287_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_287_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_288_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_288_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_288_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_288_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_288_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_288_98_Edges = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_289_57 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_289_57_Nodes = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_289_78 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_289_78_Faces = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_289_98 = R"doc()doc";

static const char *__doc_FSElementTraits_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_289_98_Edges = R"doc()doc";

static const char *__doc_FSElementType =
R"doc(element types (NOTE: do not change the order or values of these
macros.))doc";

static const char *__doc_FSElementType_FE_BEAM2 = R"doc()doc";

static const char *__doc_FSElementType_FE_BEAM3 = R"doc()doc";

static const char *__doc_FSElementType_FE_HEX20 = R"doc()doc";

static const char *__doc_FSElementType_FE_HEX27 = R"doc()doc";

static const char *__doc_FSElementType_FE_HEX8 = R"doc()doc";

static const char *__doc_FSElementType_FE_INVALID_ELEMENT_TYPE = R"doc()doc";

static const char *__doc_FSElementType_FE_PENTA15 = R"doc()doc";

static const char *__doc_FSElementType_FE_PENTA6 = R"doc()doc";

static const char *__doc_FSElementType_FE_PYRA13 = R"doc()doc";

static const char *__doc_FSElementType_FE_PYRA5 = R"doc()doc";

static const char *__doc_FSElementType_FE_QUAD4 = R"doc()doc";

static const char *__doc_FSElementType_FE_QUAD8 = R"doc()doc";

static const char *__doc_FSElementType_FE_QUAD9 = R"doc()doc";

static const char *__doc_FSElementType_FE_TET10 = R"doc()doc";

static const char *__doc_FSElementType_FE_TET15 = R"doc()doc";

static const char *__doc_FSElementType_FE_TET20 = R"doc()doc";

static const char *__doc_FSElementType_FE_TET4 = R"doc()doc";

static const char *__doc_FSElementType_FE_TET5 = R"doc()doc";

static const char *__doc_FSElementType_FE_TRI10 = R"doc()doc";

static const char *__doc_FSElementType_FE_TRI3 = R"doc()doc";

static const char *__doc_FSElementType_FE_TRI6 = R"doc()doc";

static const char *__doc_FSElementType_FE_TRI7 = R"doc()doc";

static const char *__doc_FSElement_Class = R"doc(Get the element class)doc";

static const char *__doc_FSElement_Edges = R"doc(Get the number of edges)doc";

static const char *__doc_FSElement_FSElement = R"doc(constructor)doc";

static const char *__doc_FSElement_FSElement_2 = R"doc(constructor)doc";

static const char *__doc_FSElement_FSElement_3 = R"doc(copy constructor)doc";

static const char *__doc_FSElement_Faces = R"doc(Get the number of faces)doc";

static const char *__doc_FSElement_FindEdge = R"doc(Find the edge index of a shell)doc";

static const char *__doc_FSElement_FindFace = R"doc(Find the face. Returns local index in face array)doc";

static const char *__doc_FSElement_FindNodeIndex =
R"doc(get the index into the element's node array (or -1 of the element does
not have this node))doc";

static const char *__doc_FSElement_GetEdge = R"doc(Get the edge)doc";

static const char *__doc_FSElement_GetFace = R"doc(Get the face i)doc";

static const char *__doc_FSElement_GetFace_2 = R"doc(Get the face i)doc";

static const char *__doc_FSElement_GetLocalFaceIndices = R"doc(Get local face indices)doc";

static const char *__doc_FSElement_GetShellFace = R"doc(Get the face of a shell)doc";

static const char *__doc_FSElement_HasNode = R"doc(Check if element has a specific node)doc";

static const char *__doc_FSElement_IsBeam = R"doc(Check if element is a beam element)doc";

static const char *__doc_FSElement_IsShell = R"doc(Check if element is a shell element)doc";

static const char *__doc_FSElement_IsSolid = R"doc(Check if element is a solid element)doc";

static const char *__doc_FSElement_IsType = R"doc(Is the element of this type)doc";

static const char *__doc_FSElement_Nodes = R"doc(Get the number of nodes)doc";

static const char *__doc_FSElement_SetType = R"doc(Set the element type)doc";

static const char *__doc_FSElement_Shape = R"doc(Get the element shape)doc";

static const char *__doc_FSElement_Type = R"doc(Get the element type)doc";

static const char *__doc_FSElement_copy = R"doc(help class for copy-ing element data)doc";

static const char *__doc_FSElement_eval = R"doc(evaluate a vector expression at iso-points (r,s,t))doc";

static const char *__doc_FSElement_eval_2 = R"doc(evaluate a vector expression at iso-points (r,s,t))doc";

static const char *__doc_FSElement_eval_3 = R"doc(evaluate a vector expression at iso-points (r,s,t))doc";

static const char *__doc_FSElement_eval_4 = R"doc(evaluate a vector expression at iso-points (r,s))doc";

static const char *__doc_FSElement_face = R"doc(faces (-1 for interior faces))doc";

static const char *__doc_FSElement_h = R"doc(element thickness (only used by shells))doc";

static const char *__doc_FSElement_is_equal = R"doc(comparison operator)doc";

static const char *__doc_FSElement_iso_coord = R"doc(get iso-param coordinates of the nodes)doc";

static const char *__doc_FSElement_iso_coord_2d = R"doc(get iso-param coordinates of the nodes)doc";

static const char *__doc_FSElement_m_MatID = R"doc(material id)doc";

static const char *__doc_FSElement_m_Q = R"doc(local material orientation)doc";

static const char *__doc_FSElement_m_Qactive = R"doc(active local material orientation)doc";

static const char *__doc_FSElement_m_a0 = R"doc(cross-sectional area (only used by truss elements))doc";

static const char *__doc_FSElement_m_face = R"doc(faces (-1 for interior faces))doc";

static const char *__doc_FSElement_m_fiber = R"doc(fiber orientation \todo maybe I can add an element attribute section)doc";

static const char *__doc_FSElement_m_h = R"doc(element thickness (only used by shells))doc";

static const char *__doc_FSElement_m_lid = R"doc(local ID (zero-based index into element array))doc";

static const char *__doc_FSElement_m_nbr = R"doc(neighbour elements)doc";

static const char *__doc_FSElement_m_node = R"doc(pointer to node data)doc";

static const char *__doc_FSElement_m_tex = R"doc(element texture coordinate)doc";

static const char *__doc_FSElement_m_traits = R"doc(element traits)doc";

static const char *__doc_FSElement_nbr = R"doc(neighbour elements)doc";

static const char *__doc_FSElement_node = R"doc(nodal id's)doc";

static const char *__doc_FSElement_operator_assign = R"doc(assignment operator)doc";

static const char *__doc_FSElement_operator_ne = R"doc(inequality operator)doc";

static const char *__doc_FSElement_setAxes = R"doc(set the material axis)doc";

static const char *__doc_FSElement_shape = R"doc(evaluate shape function at iso-parameteric point (r,s,t))doc";

static const char *__doc_FSElement_shape_2d =
R"doc(evaluate shape function at iso-parameteric point (r,s) (for 2D
elements only!))doc";

static const char *__doc_FSElement_shape_deriv = R"doc(shape function derivatives)doc";

static const char *__doc_FSElement_shape_deriv_2d = R"doc(shape function derivatives (for 2D elements only))doc";

static const char *__doc_FSElement_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_244_2 = R"doc()doc";

static const char *__doc_FSElement_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_244_2_MAX_NODES = R"doc()doc";

static const char *__doc_FSFace =
R"doc(FSFace class stores face data. 
A face can either have 3, 4, 6, 7, 8, 9 or 10 nodes. 
 - 3  : linear triangle
 - 6,7: quadratic triangle
 - 10 : cubic triangle
 - 4  : linear quad
 - 8,9: quadratic quad

  4       7       3      3            3
  +-------o-------+      +            +
  |               |      |\           | \
  |               |      | \         9o    o7
 8o       x9      o6    6o  o5        | x10 \
  |               |      | x7 \       8o     o6 
  |               |      |     \      |       \
  +-------o-------+      +--o--+      +--o--o--+
  1       5       2      1  4  2      1  4  5  2

Note that for the first three nodes for a triangle and the first four nodes
of a quad are always the corner nodes.)doc";

static const char *__doc_FSFace_ELEM_REF = R"doc(Element reference structure)doc";

static const char *__doc_FSFace_ELEM_REF_eid = R"doc()doc";

static const char *__doc_FSFace_ELEM_REF_lid = R"doc()doc";

static const char *__doc_FSFace_Edges = R"doc(return number of edges)doc";

static const char *__doc_FSFace_FSFace = R"doc(constructor)doc";

static const char *__doc_FSFace_FindEdge = R"doc(See if a node list is an edge)doc";

static const char *__doc_FSFace_FindNode = R"doc(Find the array index of node with ID i)doc";

static const char *__doc_FSFace_GetEdge = R"doc(return an edge)doc";

static const char *__doc_FSFace_GetEdgeNodes = R"doc(get the edge node numbers)doc";

static const char *__doc_FSFace_HasEdge = R"doc(See if this face has an edge)doc";

static const char *__doc_FSFace_HasNode = R"doc(See if this face has node with ID i)doc";

static const char *__doc_FSFace_IsExternal = R"doc(Is this face internal or external)doc";

static const char *__doc_FSFace_Nodes = R"doc(return number of nodes)doc";

static const char *__doc_FSFace_SetType = R"doc(set the type)doc";

static const char *__doc_FSFace_Shape = R"doc(get the shape)doc";

static const char *__doc_FSFace_Type = R"doc(get the type)doc";

static const char *__doc_FSFace_eval = R"doc(evaluate a scalar expression at iso-points (r,s))doc";

static const char *__doc_FSFace_eval_2 = R"doc(evaluate a vector expression at iso-points (r,s))doc";

static const char *__doc_FSFace_eval_3 = R"doc(evaluate a vector expression at iso-points (r,s))doc";

static const char *__doc_FSFace_eval_deriv1 = R"doc(evaluate the derivative of a scalar expression at iso-points (r,s))doc";

static const char *__doc_FSFace_eval_deriv1_2 = R"doc(evaluate the derivative of a vector expression at iso-points (r,s))doc";

static const char *__doc_FSFace_eval_deriv2 = R"doc(evaluate the derivative of a scalar expression at iso-points (r,s))doc";

static const char *__doc_FSFace_eval_deriv2_2 = R"doc(evaluate the derivative of a vector expression at iso-points (r,s))doc";

static const char *__doc_FSFace_gauss = R"doc(evaluate gauss integration points and weights)doc";

static const char *__doc_FSFace_m_edge = R"doc(the edges (interior faces don't have edges!))doc";

static const char *__doc_FSFace_m_elem = R"doc(the elements to which this face belongs)doc";

static const char *__doc_FSFace_m_fn = R"doc(face normal)doc";

static const char *__doc_FSFace_m_nbr = R"doc(neighbour faces)doc";

static const char *__doc_FSFace_m_nn = R"doc(node normals)doc";

static const char *__doc_FSFace_m_sid = R"doc(smoothing ID)doc";

static const char *__doc_FSFace_m_tex = R"doc(nodal 1D-texture coordinates)doc";

static const char *__doc_FSFace_m_texe = R"doc(element texture coordinate)doc";

static const char *__doc_FSFace_m_type = R"doc(face type)doc";

static const char *__doc_FSFace_n = R"doc(nodal ID's)doc";

static const char *__doc_FSFace_operator_eq = R"doc(comparison operator)doc";

static const char *__doc_FSFace_shape = R"doc(evaluate shape function at iso-parameteric point (r,s))doc";

static const char *__doc_FSFace_shape_deriv = R"doc(evaluate derivatives of shape function at iso-parameteric point (r,s))doc";

static const char *__doc_FSFace_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSFace_h_81_2 = R"doc()doc";

static const char *__doc_FSFace_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSFace_h_81_2_MAX_NODES = R"doc()doc";

static const char *__doc_FSLineMesh =
R"doc(Class that manages a list of nodes and edges This serves as a base
class for most meshes)doc";

static const char *__doc_FSLineMesh_BuildMesh = R"doc(Should be called when mesh needs to be reconstructed)doc";

static const char *__doc_FSLineMesh_Edge = R"doc(Get a reference to an edge by index)doc";

static const char *__doc_FSLineMesh_Edge_2 = R"doc(Get a const reference to an edge by index)doc";

static const char *__doc_FSLineMesh_EdgeCenter = R"doc(Get the edge center)doc";

static const char *__doc_FSLineMesh_EdgePtr = R"doc(Get a pointer to an edge by index)doc";

static const char *__doc_FSLineMesh_EdgePtr_2 = R"doc(Get a const pointer to an edge by index)doc";

static const char *__doc_FSLineMesh_Edges = R"doc(Get the number of edges in the mesh)doc";

static const char *__doc_FSLineMesh_FSLineMesh = R"doc(Default constructor)doc";

static const char *__doc_FSLineMesh_GetGObject = R"doc(Get the parent object)doc";

static const char *__doc_FSLineMesh_GetGObject_2 = R"doc(Get the parent object (const version))doc";

static const char *__doc_FSLineMesh_GlobalToLocal = R"doc(Convert a global point to local coordinates)doc";

static const char *__doc_FSLineMesh_IsEditable = R"doc(Check if the mesh is editable)doc";

static const char *__doc_FSLineMesh_LocalToGlobal =
R"doc(Convert a local point to global coordinates (This uses the parent
object's transform))doc";

static const char *__doc_FSLineMesh_Node = R"doc(Get a reference to a node by index)doc";

static const char *__doc_FSLineMesh_Node_2 = R"doc(Get a const reference to a node by index)doc";

static const char *__doc_FSLineMesh_NodeEdgeList = R"doc(Get the node-edge list for a specific node)doc";

static const char *__doc_FSLineMesh_NodeLocalPosition = R"doc(Get the local node position)doc";

static const char *__doc_FSLineMesh_NodePosition = R"doc(Get the global node position)doc";

static const char *__doc_FSLineMesh_NodePtr = R"doc(Get a pointer to a node by index)doc";

static const char *__doc_FSLineMesh_NodePtr_2 = R"doc(Get a const pointer to a node by index)doc";

static const char *__doc_FSLineMesh_Nodes = R"doc(Get the number of nodes in the mesh)doc";

static const char *__doc_FSLineMesh_SetGObject = R"doc(Set the parent object)doc";

static const char *__doc_FSLineMesh_TagAllEdges = R"doc(Tag all edges with the specified tag value)doc";

static const char *__doc_FSLineMesh_TagAllNodes = R"doc(Tag all nodes with the specified tag value)doc";

static const char *__doc_FSLineMesh_UpdateBoundingBox = R"doc(Update the bounding box)doc";

static const char *__doc_FSLineMesh_UpdateMesh =
R"doc(Should be called when mesh needs to be updated (but not
reconstructred) E.g. for surface meshes, this will update face
normals, etc.)doc";

static const char *__doc_FSLineMesh_m_Edge = R"doc(Edge list)doc";

static const char *__doc_FSLineMesh_m_NLL = R"doc(Node-edge connectivity list)doc";

static const char *__doc_FSLineMesh_m_Node = R"doc(Node list)doc";

static const char *__doc_FSLineMesh_m_box = R"doc(Bounding box)doc";

static const char *__doc_FSLineMesh_m_pobj = R"doc(Owning object)doc";

static const char *__doc_FSLinearElement = R"doc(This element class can represent any of the linear elements.)doc";

static const char *__doc_FSLinearElement_FSLinearElement = R"doc(constructor)doc";

static const char *__doc_FSLinearElement_FSLinearElement_2 = R"doc(copy constructor)doc";

static const char *__doc_FSLinearElement_face = R"doc(faces (-1 for interior faces))doc";

static const char *__doc_FSLinearElement_h = R"doc(element thickness (only used by shells))doc";

static const char *__doc_FSLinearElement_nbr = R"doc(neighbour elements)doc";

static const char *__doc_FSLinearElement_node = R"doc(array of node IDs)doc";

static const char *__doc_FSLinearElement_operator_assign = R"doc(assignment operator)doc";

static const char *__doc_FSLinearElement_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_353_2 = R"doc()doc";

static const char *__doc_FSLinearElement_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSElement_h_353_2_MAX_NODES = R"doc()doc";

static const char *__doc_FSMesh =
R"doc(This class describes a finite element mesh. Every FSMesh must be owned
by a GObject class.)doc";

static const char *__doc_FSMesh_2 =
R"doc(This class describes a finite element mesh. Every FSMesh must be owned
by a GObject class.)doc";

static const char *__doc_FSMesh_3 = R"doc()doc";

static const char *__doc_FSMeshBase =
R"doc(Base class for mesh classes. Essentially manages the nodes, edges, and
faces)doc";

static const char *__doc_FSMeshBase_AutoSmooth = R"doc(Calculate smoothing IDs based on face normals)doc";

static const char *__doc_FSMeshBase_ClearFaceSelection = R"doc(Clear the selection of all faces)doc";

static const char *__doc_FSMeshBase_CountSelectedEdges = R"doc(Count the number of selected edges)doc";

static const char *__doc_FSMeshBase_CountSelectedFaces = R"doc(Count the number of selected faces)doc";

static const char *__doc_FSMeshBase_CountSelectedNodes = R"doc(Count the number of selected nodes)doc";

static const char *__doc_FSMeshBase_DeleteEdges = R"doc(Delete all edges)doc";

static const char *__doc_FSMeshBase_DeleteFaces = R"doc(Delete all faces)doc";

static const char *__doc_FSMeshBase_DeleteNodes = R"doc(Delete all nodes)doc";

static const char *__doc_FSMeshBase_FSMeshBase = R"doc(Default constructor)doc";

static const char *__doc_FSMeshBase_Face = R"doc(Get a reference to a face by index)doc";

static const char *__doc_FSMeshBase_Face_2 = R"doc(Get a const reference to a face by index)doc";

static const char *__doc_FSMeshBase_FaceArea = R"doc(Calculate the area of a face)doc";

static const char *__doc_FSMeshBase_FaceArea_2 = R"doc(Calculate the area of a face given vertices and face type)doc";

static const char *__doc_FSMeshBase_FaceCenter = R"doc(Calculate the center of a face)doc";

static const char *__doc_FSMeshBase_FaceNodeLocalPositions = R"doc(Get the local positions of a face)doc";

static const char *__doc_FSMeshBase_FaceNodeNormals = R"doc(Get the normals of face nodes)doc";

static const char *__doc_FSMeshBase_FaceNodePosition = R"doc(Get the world positions of face nodes)doc";

static const char *__doc_FSMeshBase_FaceNodeTexCoords = R"doc(Get the texture coordinates of face nodes)doc";

static const char *__doc_FSMeshBase_FacePtr = R"doc(Get a pointer to a face by index)doc";

static const char *__doc_FSMeshBase_FacePtr_2 = R"doc(Get a const pointer to a face by index)doc";

static const char *__doc_FSMeshBase_Faces = R"doc(Get the number of faces)doc";

static const char *__doc_FSMeshBase_FindEdge = R"doc(Find an edge if it exists (or null otherwise))doc";

static const char *__doc_FSMeshBase_GetBoundingBox = R"doc(Get the bounding box of the mesh)doc";

static const char *__doc_FSMeshBase_GetNodeNeighbors = R"doc(Get node neighbors within specified levels)doc";

static const char *__doc_FSMeshBase_IsCreaseEdge = R"doc(Check if an edge is a crease edge)doc";

static const char *__doc_FSMeshBase_IsEdge = R"doc(Check if two nodes form an edge)doc";

static const char *__doc_FSMeshBase_NodeFaceList = R"doc(Get the node-face list)doc";

static const char *__doc_FSMeshBase_RemoveEdges = R"doc(Remove edges with specified tag)doc";

static const char *__doc_FSMeshBase_RemoveFaces = R"doc(Remove faces with specified tag)doc";

static const char *__doc_FSMeshBase_SmoothByPartition = R"doc(Assign smoothing IDs based on surface partition)doc";

static const char *__doc_FSMeshBase_TagAllFaces = R"doc(Tag all faces with a specific tag value)doc";

static const char *__doc_FSMeshBase_UpdateItemVisibility = R"doc(Update item visibility)doc";

static const char *__doc_FSMeshBase_UpdateMesh = R"doc(Update the mesh structure (override from FSLineMesh))doc";

static const char *__doc_FSMeshBase_UpdateNormals = R"doc(Update the normals)doc";

static const char *__doc_FSMeshBase_m_Face = R"doc(Vector of FE faces)doc";

static const char *__doc_FSMeshBase_m_NFL = R"doc(Node-face list for efficient lookups)doc";

static const char *__doc_FSMeshBuilder = R"doc()doc";

static const char *__doc_FSMeshData = R"doc(Class for storing mesh data with various data types and formats)doc";

static const char *__doc_FSMeshData_2 = R"doc(Class for storing mesh data with various data types and formats)doc";

static const char *__doc_FSMeshData_DataItems = R"doc(Get the number of data items)doc";

static const char *__doc_FSMeshData_DataSize = R"doc(Get the size of data field)doc";

static const char *__doc_FSMeshData_FSMeshData = R"doc(Constructor that initializes the mesh data with a given data class)doc";

static const char *__doc_FSMeshData_GetData = R"doc(Get the data vector)doc";

static const char *__doc_FSMeshData_GetDataClass = R"doc(Get the data class of this mesh data)doc";

static const char *__doc_FSMeshData_GetDataFormat = R"doc(Get the data format)doc";

static const char *__doc_FSMeshData_GetDataType = R"doc(Get the data type of this mesh data)doc";

static const char *__doc_FSMeshData_GetMesh = R"doc(Return mesh this data field belongs to)doc";

static const char *__doc_FSMeshData_ItemSize = R"doc(Get the size of each data item)doc";

static const char *__doc_FSMeshData_SetData = R"doc(Set the data vector)doc";

static const char *__doc_FSMeshData_SetDataFormat = R"doc(Set the data format)doc";

static const char *__doc_FSMeshData_SetDataType = R"doc(Set the data type)doc";

static const char *__doc_FSMeshData_SetMesh = R"doc(Set the mesh this data field belongs to)doc";

static const char *__doc_FSMeshData_get = R"doc(Get a double value at the specified index)doc";

static const char *__doc_FSMeshData_get_2 = R"doc(Get multiple values starting at the specified index)doc";

static const char *__doc_FSMeshData_getMat3d = R"doc(Get a mat3d value at the specified index)doc";

static const char *__doc_FSMeshData_getScalar = R"doc(Get a scalar value at the specified index)doc";

static const char *__doc_FSMeshData_getVec3d = R"doc(Get a vec3d value at the specified index)doc";

static const char *__doc_FSMeshData_m_data = R"doc(Vector storing the actual data values)doc";

static const char *__doc_FSMeshData_m_dataClass = R"doc(Data class type)doc";

static const char *__doc_FSMeshData_m_dataFmt = R"doc(Data format)doc";

static const char *__doc_FSMeshData_m_dataType = R"doc(Data type (scalar, vector, matrix, etc.))doc";

static const char *__doc_FSMeshData_m_itemSize = R"doc(Size of each data item)doc";

static const char *__doc_FSMeshData_m_pMesh = R"doc(Pointer to the mesh this data belongs to)doc";

static const char *__doc_FSMeshData_operator_array = R"doc(Access operator for data elements)doc";

static const char *__doc_FSMeshData_set = R"doc(Set a double value at the specified index)doc";

static const char *__doc_FSMeshData_set_2 = R"doc(Set a vec3d value at the specified index)doc";

static const char *__doc_FSMeshData_set_3 = R"doc(Set a mat3d value at the specified index)doc";

static const char *__doc_FSMeshData_setScalar = R"doc(Set a scalar value at the specified index)doc";

static const char *__doc_FSMeshData_setVec3d = R"doc(Set a vec3d value at the specified index)doc";

static const char *__doc_FSMeshItem = R"doc(Base class for mesh item classes.)doc";

static const char *__doc_FSMeshItem_2 = R"doc(Base class for mesh item classes.)doc";

static const char *__doc_FSMeshItem_Activate = R"doc(Activate the item)doc";

static const char *__doc_FSMeshItem_Activate_2 = R"doc(Activate the item)doc";

static const char *__doc_FSMeshItem_CanExport = R"doc(Check if the item can be exported)doc";

static const char *__doc_FSMeshItem_CanExport_2 = R"doc(Check if the item can be exported)doc";

static const char *__doc_FSMeshItem_Deactivate = R"doc(Deactivate the item)doc";

static const char *__doc_FSMeshItem_Deactivate_2 = R"doc(Deactivate the item)doc";

static const char *__doc_FSMeshItem_Disable = R"doc(Disable the item)doc";

static const char *__doc_FSMeshItem_Disable_2 = R"doc(Disable the item)doc";

static const char *__doc_FSMeshItem_Enable = R"doc(Enable the item)doc";

static const char *__doc_FSMeshItem_Enable_2 = R"doc(Enable the item)doc";

static const char *__doc_FSMeshItem_FSMeshItem = R"doc(Default constructor)doc";

static const char *__doc_FSMeshItem_FSMeshItem_2 = R"doc(Default constructor)doc";

static const char *__doc_FSMeshItem_GetID = R"doc(Get the ID of the item)doc";

static const char *__doc_FSMeshItem_GetID_2 = R"doc(Get the ID of the item)doc";

static const char *__doc_FSMeshItem_GetState = R"doc(Get the current state flags)doc";

static const char *__doc_FSMeshItem_GetState_2 = R"doc(Get the current state flags)doc";

static const char *__doc_FSMeshItem_Hide = R"doc(Hide the item and unselect it)doc";

static const char *__doc_FSMeshItem_Hide_2 = R"doc(Hide the item and unselect it)doc";

static const char *__doc_FSMeshItem_IsActive = R"doc(Check if the item is active)doc";

static const char *__doc_FSMeshItem_IsActive_2 = R"doc(Check if the item is active)doc";

static const char *__doc_FSMeshItem_IsDisabled = R"doc(Check if the item is disabled)doc";

static const char *__doc_FSMeshItem_IsDisabled_2 = R"doc(Check if the item is disabled)doc";

static const char *__doc_FSMeshItem_IsEnabled = R"doc(Check if the item is enabled (opposite of disabled))doc";

static const char *__doc_FSMeshItem_IsEnabled_2 = R"doc(Check if the item is enabled (opposite of disabled))doc";

static const char *__doc_FSMeshItem_IsEroded = R"doc(Check if the item is eroded)doc";

static const char *__doc_FSMeshItem_IsEroded_2 = R"doc(Check if the item is eroded)doc";

static const char *__doc_FSMeshItem_IsExterior = R"doc(Check if the item is exterior)doc";

static const char *__doc_FSMeshItem_IsExterior_2 = R"doc(Check if the item is exterior)doc";

static const char *__doc_FSMeshItem_IsHidden = R"doc(Check if the item is hidden)doc";

static const char *__doc_FSMeshItem_IsHidden_2 = R"doc(Check if the item is hidden)doc";

static const char *__doc_FSMeshItem_IsInvisible = R"doc(Check if the item is invisible)doc";

static const char *__doc_FSMeshItem_IsInvisible_2 = R"doc(Check if the item is invisible)doc";

static const char *__doc_FSMeshItem_IsRequired = R"doc(Check if the item is required)doc";

static const char *__doc_FSMeshItem_IsRequired_2 = R"doc(Check if the item is required)doc";

static const char *__doc_FSMeshItem_IsSelected = R"doc(Check if the item is selected)doc";

static const char *__doc_FSMeshItem_IsSelected_2 = R"doc(Check if the item is selected)doc";

static const char *__doc_FSMeshItem_IsVisible =
R"doc(Check if the item is visible (not invisible, not hidden, and not
eroded))doc";

static const char *__doc_FSMeshItem_IsVisible_2 =
R"doc(Check if the item is visible (not invisible, not hidden, and not
eroded))doc";

static const char *__doc_FSMeshItem_Select = R"doc(Select the item)doc";

static const char *__doc_FSMeshItem_Select_2 = R"doc(Select the item)doc";

static const char *__doc_FSMeshItem_SetEroded = R"doc(Set the eroded state of the item)doc";

static const char *__doc_FSMeshItem_SetEroded_2 = R"doc(Set the eroded state of the item)doc";

static const char *__doc_FSMeshItem_SetExport = R"doc(Set the export state of the item)doc";

static const char *__doc_FSMeshItem_SetExport_2 = R"doc(Set the export state of the item)doc";

static const char *__doc_FSMeshItem_SetExterior = R"doc(Set the exterior state of the item)doc";

static const char *__doc_FSMeshItem_SetExterior_2 = R"doc(Set the exterior state of the item)doc";

static const char *__doc_FSMeshItem_SetID = R"doc(Set the ID of the item)doc";

static const char *__doc_FSMeshItem_SetID_2 = R"doc(Set the ID of the item)doc";

static const char *__doc_FSMeshItem_SetRequired = R"doc(Set the required state of the item)doc";

static const char *__doc_FSMeshItem_SetRequired_2 = R"doc(Set the required state of the item)doc";

static const char *__doc_FSMeshItem_SetState = R"doc(Set the state flags)doc";

static const char *__doc_FSMeshItem_SetState_2 = R"doc(Set the state flags)doc";

static const char *__doc_FSMeshItem_Show = R"doc(Show or hide the item (affects visibility))doc";

static const char *__doc_FSMeshItem_Show_2 = R"doc(Show or hide the item (affects visibility))doc";

static const char *__doc_FSMeshItem_Unhide = R"doc(Unhide the item)doc";

static const char *__doc_FSMeshItem_Unhide_2 = R"doc(Unhide the item)doc";

static const char *__doc_FSMeshItem_Unselect = R"doc(Unselect the item)doc";

static const char *__doc_FSMeshItem_Unselect_2 = R"doc(Unselect the item)doc";

static const char *__doc_FSMeshItem_m_gid = R"doc(Group ID member variable)doc";

static const char *__doc_FSMeshItem_m_gid_2 = R"doc(Group ID member variable)doc";

static const char *__doc_FSMeshItem_m_nid = R"doc(Node/Item ID member variable)doc";

static const char *__doc_FSMeshItem_m_nid_2 = R"doc(Node/Item ID member variable)doc";

static const char *__doc_FSMeshItem_m_ntag = R"doc(Tag member variable)doc";

static const char *__doc_FSMeshItem_m_ntag_2 = R"doc(Tag member variable)doc";

static const char *__doc_FSMeshItem_m_state = R"doc(The state flag of the mesh item)doc";

static const char *__doc_FSMeshItem_m_state_2 = R"doc(The state flag of the mesh item)doc";

static const char *__doc_FSMeshItem_operator_assign = R"doc(Assignment operator)doc";

static const char *__doc_FSMeshItem_operator_assign_2 = R"doc(Assignment operator)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2 =
R"doc(Item flags Even when not hidden, the item may not be shown since e.g.
the material is hidden)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_2 =
R"doc(Item flags Even when not hidden, the item may not be shown since e.g.
the material is hidden)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_ACTIVE = R"doc(Does the item contain data?)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_ACTIVE_2 = R"doc(Does the item contain data?)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_DISABLED = R"doc(Should the item be evaluated?)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_DISABLED_2 = R"doc(Should the item be evaluated?)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_ERODED = R"doc(The item is "eroded" and should be treated as no longer present)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_ERODED_2 = R"doc(The item is "eroded" and should be treated as no longer present)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_EXPORT = R"doc(Item should be exported (only used by FEBioExport4))doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_EXPORT_2 = R"doc(Item should be exported (only used by FEBioExport4))doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_EXTERIOR = R"doc(The item is "exterior")doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_EXTERIOR_2 = R"doc(The item is "exterior")doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_HIDDEN = R"doc(State Flags Was the item hidden by the user)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_HIDDEN_2 = R"doc(State Flags Was the item hidden by the user)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_INVISIBLE = R"doc(Is the item invisible because the parent material was hidden?)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_INVISIBLE_2 = R"doc(Is the item invisible because the parent material was hidden?)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_REQUIRED = R"doc(The item is required and should not be deleted during mesh operations)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_REQUIRED_2 = R"doc(The item is required and should not be deleted during mesh operations)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_SELECTED = R"doc(Is the item currently selected?)doc";

static const char *__doc_FSMeshItem_unnamed_enum_at_home_mherron_Projects_FEBioStudio_MeshLib_FSMeshItem_h_35_2_ITEM_SELECTED_2 = R"doc(Is the item currently selected?)doc";

static const char *__doc_FSMesh_AddElementDataField = R"doc(Add element data field)doc";

static const char *__doc_FSMesh_AddFEEdgeSet = R"doc(Add FE edge set)doc";

static const char *__doc_FSMesh_AddFEElemSet = R"doc(Add FE element set)doc";

static const char *__doc_FSMesh_AddFENodeSet = R"doc(Add FE node set)doc";

static const char *__doc_FSMesh_AddFEPartSet = R"doc(Add FE part set)doc";

static const char *__doc_FSMesh_AddFESurface = R"doc(Add FE surface)doc";

static const char *__doc_FSMesh_AddMeshDataField = R"doc(Add mesh data field)doc";

static const char *__doc_FSMesh_AddNodeDataField = R"doc(Add node data field)doc";

static const char *__doc_FSMesh_AddPartDataField = R"doc(Add part data field)doc";

static const char *__doc_FSMesh_AddSurfaceDataField = R"doc(Add surface data field)doc";

static const char *__doc_FSMesh_BuildELT = R"doc(Build element lookup table)doc";

static const char *__doc_FSMesh_BuildMesh = R"doc(Build all mesh data structures)doc";

static const char *__doc_FSMesh_BuildNLT = R"doc(Build node lookup table)doc";

static const char *__doc_FSMesh_Clear = R"doc(Clear this mesh)doc";

static const char *__doc_FSMesh_ClearELT = R"doc(Clear element lookup table)doc";

static const char *__doc_FSMesh_ClearFEGroups = R"doc(Clear all FE groups)doc";

static const char *__doc_FSMesh_ClearMeshData = R"doc(Clear all mesh data)doc";

static const char *__doc_FSMesh_ClearMeshPartitions = R"doc(Clear mesh partitions)doc";

static const char *__doc_FSMesh_ClearMeshTopo = R"doc(Clear mesh topology)doc";

static const char *__doc_FSMesh_ClearNLT = R"doc(Clear node lookup table)doc";

static const char *__doc_FSMesh_ClearSelections = R"doc(Clear all selections)doc";

static const char *__doc_FSMesh_CountSelectedElements = R"doc(Count selected elements)doc";

static const char *__doc_FSMesh_Create = R"doc(Allocate space for mesh)doc";

static const char *__doc_FSMesh_Element = R"doc(Return element reference)doc";

static const char *__doc_FSMesh_Element_2 = R"doc(Return const element reference)doc";

static const char *__doc_FSMesh_ElementIndexFromID = R"doc(Return element index from its element ID)doc";

static const char *__doc_FSMesh_ElementRef = R"doc(Return reference to element)doc";

static const char *__doc_FSMesh_ElementRef_2 = R"doc(Return const reference to element)doc";

static const char *__doc_FSMesh_Elements = R"doc(Return number of elements)doc";

static const char *__doc_FSMesh_ExtractFaces = R"doc(Extract faces and return as new mesh)doc";

static const char *__doc_FSMesh_ExtractFacesAsSurface = R"doc(Extract faces and return as surface mesh)doc";

static const char *__doc_FSMesh_FEEdgeSets = R"doc(Get number of FE edge sets)doc";

static const char *__doc_FSMesh_FEElemSets = R"doc(Get number of FE element sets)doc";

static const char *__doc_FSMesh_FENodeSets = R"doc(Get number of FE node sets)doc";

static const char *__doc_FSMesh_FEPartSets = R"doc(Get number of FE part sets)doc";

static const char *__doc_FSMesh_FESurfaces = R"doc(Get number of FE surfaces)doc";

static const char *__doc_FSMesh_FSMesh = R"doc(Default constructor)doc";

static const char *__doc_FSMesh_FSMesh_2 = R"doc(Copy constructor)doc";

static const char *__doc_FSMesh_FSMesh_3 = R"doc(Constructor from surface mesh)doc";

static const char *__doc_FSMesh_FindDuplicateEdges = R"doc(Find duplicate edges)doc";

static const char *__doc_FSMesh_FindDuplicateFaces = R"doc(Find duplicate faces)doc";

static const char *__doc_FSMesh_FindFEEdgeSet = R"doc(Find FE edge set by name)doc";

static const char *__doc_FSMesh_FindFEGroup = R"doc(Find FE group by ID)doc";

static const char *__doc_FSMesh_FindFENodeSet = R"doc(Find FE node set by name)doc";

static const char *__doc_FSMesh_FindFEPartSet = R"doc(Find FE part set by name)doc";

static const char *__doc_FSMesh_FindFESurface = R"doc(Find FE surface by name)doc";

static const char *__doc_FSMesh_FindFaceIndex = R"doc(Find face index)doc";

static const char *__doc_FSMesh_FindMeshDataField = R"doc(Find mesh data field by name)doc";

static const char *__doc_FSMesh_FindPartDataField = R"doc(Find part data field by name)doc";

static const char *__doc_FSMesh_GenerateElementIDs = R"doc((Re-)generate element IDs (startID must be larger than 0!))doc";

static const char *__doc_FSMesh_GenerateNodalIDs = R"doc(Re-generate nodal IDs (startID must be larger than 0!))doc";

static const char *__doc_FSMesh_GetElementsFromSelectedFaces = R"doc(Select elements based on face selection)doc";

static const char *__doc_FSMesh_GetFEEdgeSet = R"doc(Get FE edge set by index)doc";

static const char *__doc_FSMesh_GetFEElemSet = R"doc(Get FE element set by index)doc";

static const char *__doc_FSMesh_GetFENodeSet = R"doc(Get FE node set by index)doc";

static const char *__doc_FSMesh_GetFEPartSet = R"doc(Get FE part set by index)doc";

static const char *__doc_FSMesh_GetFESurface = R"doc(Get FE surface by index)doc";

static const char *__doc_FSMesh_GetMeshData = R"doc(Get mesh data reference)doc";

static const char *__doc_FSMesh_GetMeshDataField = R"doc(Get mesh data field by index)doc";

static const char *__doc_FSMesh_GetMeshDataIndex = R"doc(Get mesh data index)doc";

static const char *__doc_FSMesh_InsertFEEdgeSet = R"doc(Insert FE edge set at specified index)doc";

static const char *__doc_FSMesh_InsertFEElemSet = R"doc(Insert FE element set at specified index)doc";

static const char *__doc_FSMesh_InsertFENodeSet = R"doc(Insert FE node set at specified index)doc";

static const char *__doc_FSMesh_InsertFEPartSet = R"doc(Insert FE part set at specified index)doc";

static const char *__doc_FSMesh_InsertFESurface = R"doc(Insert FE surface at specified index)doc";

static const char *__doc_FSMesh_InsertMeshData = R"doc(Insert mesh data at specified index)doc";

static const char *__doc_FSMesh_Load = R"doc(Load mesh from archive)doc";

static const char *__doc_FSMesh_MapFEElemSets = R"doc(Map FE element sets from another mesh)doc";

static const char *__doc_FSMesh_MapFENodeSets = R"doc(Map FE node sets from another mesh)doc";

static const char *__doc_FSMesh_MapFESurfaces = R"doc(Map FE surfaces from another mesh)doc";

static const char *__doc_FSMesh_MarkExteriorEdges = R"doc(Mark exterior edges)doc";

static const char *__doc_FSMesh_MeshDataFields = R"doc(Get number of mesh data fields)doc";

static const char *__doc_FSMesh_MeshPartition = R"doc(Get mesh partition by index)doc";

static const char *__doc_FSMesh_MeshPartitions = R"doc(Get number of mesh partitions)doc";

static const char *__doc_FSMesh_NodeElementList = R"doc(Get node element list)doc";

static const char *__doc_FSMesh_NodeIndexFromID = R"doc(Return node index from its nodal ID)doc";

static const char *__doc_FSMesh_RebuildEdgeData = R"doc(Rebuild edge data)doc";

static const char *__doc_FSMesh_RebuildElementData = R"doc(Rebuild element data)doc";

static const char *__doc_FSMesh_RebuildFaceData = R"doc(Rebuild face data)doc";

static const char *__doc_FSMesh_RebuildMesh = R"doc(Reconstruct the mesh)doc";

static const char *__doc_FSMesh_RebuildNodeData = R"doc(Rebuild node data)doc";

static const char *__doc_FSMesh_RemoveElements = R"doc(Remove elements with specified tag)doc";

static const char *__doc_FSMesh_RemoveEmptyFEGroups = R"doc(Remove empty FE groups)doc";

static const char *__doc_FSMesh_RemoveFEEdgeSet = R"doc(Remove FE edge set)doc";

static const char *__doc_FSMesh_RemoveFEElemSet = R"doc(Remove FE element set)doc";

static const char *__doc_FSMesh_RemoveFENodeSet = R"doc(Remove FE node set)doc";

static const char *__doc_FSMesh_RemoveFEPartSet = R"doc(Remove FE part set)doc";

static const char *__doc_FSMesh_RemoveFESurface = R"doc(Remove FE surface)doc";

static const char *__doc_FSMesh_RemoveMeshDataField = R"doc(Remove mesh data field by index)doc";

static const char *__doc_FSMesh_RemoveMeshDataField_2 = R"doc(Remove mesh data field by pointer)doc";

static const char *__doc_FSMesh_RemoveUnusedFEGroups = R"doc(Remove unused FE groups)doc";

static const char *__doc_FSMesh_ResizeEdges = R"doc(Resize edge array)doc";

static const char *__doc_FSMesh_ResizeElems = R"doc(Resize element array)doc";

static const char *__doc_FSMesh_ResizeFaces = R"doc(Resize face array)doc";

static const char *__doc_FSMesh_ResizeNodes = R"doc(Resize node array)doc";

static const char *__doc_FSMesh_Save = R"doc(Save mesh to archive)doc";

static const char *__doc_FSMesh_SetUniformShellThickness = R"doc(Set uniform shell thickness)doc";

static const char *__doc_FSMesh_ShallowCopy = R"doc(Copy part of the mesh)doc";

static const char *__doc_FSMesh_TakeItemLists = R"doc(Take item lists from another mesh)doc";

static const char *__doc_FSMesh_TakeMeshData = R"doc(Take mesh data from another mesh)doc";

static const char *__doc_FSMesh_UpdateEdgeElementTable = R"doc(Update edge element table)doc";

static const char *__doc_FSMesh_UpdateEdgeNeighbors = R"doc(Update edge neighbors)doc";

static const char *__doc_FSMesh_UpdateEdgePartitions = R"doc(Update edge partitions)doc";

static const char *__doc_FSMesh_UpdateElementNeighbors = R"doc(Update element neighbors)doc";

static const char *__doc_FSMesh_UpdateElementPartitions = R"doc(Update element partitions)doc";

static const char *__doc_FSMesh_UpdateFaceElementTable = R"doc(Update face element table)doc";

static const char *__doc_FSMesh_UpdateFaceNeighbors = R"doc(Update face neighbors)doc";

static const char *__doc_FSMesh_UpdateFacePartitions = R"doc(Update face partitions)doc";

static const char *__doc_FSMesh_UpdateMesh = R"doc(Update the mesh)doc";

static const char *__doc_FSMesh_UpdateMeshPartitions = R"doc(Update mesh partitions)doc";

static const char *__doc_FSMesh_UpdateNodePartitions = R"doc(Update node partitions)doc";

static const char *__doc_FSMesh_UpdateSmoothingGroups = R"doc(Update smoothing groups)doc";

static const char *__doc_FSMesh_ValidateEdges = R"doc(Validate edges)doc";

static const char *__doc_FSMesh_ValidateElements = R"doc(Validate elements)doc";

static const char *__doc_FSMesh_ValidateFaces = R"doc(Validate faces)doc";

static const char *__doc_FSMesh_m_Dom = R"doc(Domains)doc";

static const char *__doc_FSMesh_m_ELT = R"doc(Element ID lookup table)doc";

static const char *__doc_FSMesh_m_Elem = R"doc(FE elements)doc";

static const char *__doc_FSMesh_m_NEL = R"doc(Node element list)doc";

static const char *__doc_FSMesh_m_NLT = R"doc(Node ID lookup table)doc";

static const char *__doc_FSMesh_m_data = R"doc(Mesh data (used for data evaluation))doc";

static const char *__doc_FSMesh_m_eltmin = R"doc(The minimum element ID)doc";

static const char *__doc_FSMesh_m_meshData = R"doc(Data fields)doc";

static const char *__doc_FSMesh_m_nltmin = R"doc(The minimum node ID)doc";

static const char *__doc_FSMesh_m_pFEEdgeSet = R"doc(Named edge sets)doc";

static const char *__doc_FSMesh_m_pFEElemSet = R"doc(Named element sets)doc";

static const char *__doc_FSMesh_m_pFENodeSet = R"doc(Named node sets)doc";

static const char *__doc_FSMesh_m_pFEPartSet = R"doc(Named part sets)doc";

static const char *__doc_FSMesh_m_pFESurface = R"doc(Named surfaces)doc";

static const char *__doc_FSNodeData = R"doc()doc";

static const char *__doc_FSPartData = R"doc()doc";

static const char *__doc_FSSurfaceData = R"doc()doc";

static const char *__doc_FSSurfaceMesh = R"doc()doc";

static const char *__doc_FSSurfaceMesh_2 = R"doc()doc";

static const char *__doc_GObject = R"doc()doc";

static const char *__doc_MeshTools_ConvertSurfaceToMesh = R"doc(Convert surface mesh to volume mesh)doc";

static const char *__doc_MeshTools_GetConnectedEdges = R"doc(Get connected edges starting from an edge)doc";

static const char *__doc_MeshTools_GetConnectedEdgesByPath = R"doc(Get connected edges by path between start and end edge)doc";

static const char *__doc_MeshTools_GetConnectedEdgesOnLineMesh = R"doc(Get connected edges on a line mesh starting from a given edge)doc";

static const char *__doc_MeshTools_GetConnectedElements = R"doc(Get connected elements)doc";

static const char *__doc_MeshTools_GetConnectedFaces =
R"doc(Get a list of face indices, connected to a face INPUT: pm : the mesh
nface : the index of the start face tolAngleDeg : angle of selection
tolerance (degrees). Set to zero to turn off. respectPartitions: do
not cross surface partitions if true)doc";

static const char *__doc_MeshTools_GetConnectedFacesByPath = R"doc(Get connected faces by path between start and end face)doc";

static const char *__doc_MeshTools_GetConnectedNodes = R"doc(Get connected nodes starting from a node)doc";

static const char *__doc_MeshTools_GetConnectedNodesByPath = R"doc(Get connected nodes by path between start and end node)doc";

static const char *__doc_MeshTools_TagConnectedNodes = R"doc(Tag connected nodes starting from a node)doc";

static const char *__doc_MeshTools_TagNodesByShortestPath = R"doc(Tag nodes by shortest path between two nodes)doc";

static const char *__doc_bias = R"doc(Bias function)doc";

static const char *__doc_gain = R"doc(Gain function)doc";

static const char *__doc_get = R"doc(Get a double value at the specified index)doc";

static const char *__doc_get_2 = R"doc(Get multiple values starting at the specified index)doc";

static const char *__doc_getMat3d = R"doc(Get a mat3d value at the specified index)doc";

static const char *__doc_getScalar = R"doc(Get a scalar value at the specified index)doc";

static const char *__doc_getVec3d = R"doc(Get a vec3d value at the specified index)doc";

static const char *__doc_set = R"doc(Set a double value at the specified index)doc";

static const char *__doc_set_2 = R"doc(Set a vec3d value at the specified index)doc";

static const char *__doc_set_3 = R"doc(Set a mat3d value at the specified index)doc";

static const char *__doc_setScalar = R"doc(Set a scalar value at the specified index)doc";

static const char *__doc_setVec3d = R"doc(Set a vec3d value at the specified index)doc";

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

