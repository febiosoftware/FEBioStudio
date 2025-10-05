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


static const char *__doc_ConvertToEditableMesh = R"doc(Convert an object to an editable mesh)doc";

static const char *__doc_EdgeType = R"doc(Edge types)doc";

static const char *__doc_EdgeType_EDGE_3P_ARC = R"doc()doc";

static const char *__doc_EdgeType_EDGE_3P_CIRC_ARC = R"doc()doc";

static const char *__doc_EdgeType_EDGE_BEZIER = R"doc()doc";

static const char *__doc_EdgeType_EDGE_LINE = R"doc()doc";

static const char *__doc_EdgeType_EDGE_MESH = R"doc()doc";

static const char *__doc_EdgeType_EDGE_UNKNOWN = R"doc()doc";

static const char *__doc_EdgeType_EDGE_YARC = R"doc()doc";

static const char *__doc_EdgeType_EDGE_ZARC = R"doc()doc";

static const char *__doc_ExtractSelection = R"doc(Extract a selection from an object and create a new GMeshObject)doc";

static const char *__doc_FEMesher = R"doc()doc";

static const char *__doc_FSCurveMesh = R"doc()doc";

static const char *__doc_FSEdgeSet = R"doc()doc";

static const char *__doc_FSEdgeSet_2 = R"doc()doc";

static const char *__doc_FSElemSet = R"doc()doc";

static const char *__doc_FSGroup = R"doc()doc";

static const char *__doc_FSLineMesh = R"doc()doc";

static const char *__doc_FSMesh = R"doc()doc";

static const char *__doc_FSMesh_2 = R"doc()doc";

static const char *__doc_FSMeshBase = R"doc()doc";

static const char *__doc_FSNode = R"doc()doc";

static const char *__doc_FSNodeSet = R"doc()doc";

static const char *__doc_FSPartSet = R"doc()doc";

static const char *__doc_FSSurface = R"doc()doc";

static const char *__doc_FSSurfaceMesh = R"doc(Forward declaration for surface mesh)doc";

static const char *__doc_FSSurfaceMesh_2 = R"doc()doc";

static const char *__doc_FaceType = R"doc(Face types)doc";

static const char *__doc_FaceType_FACE_EXTRUDE = R"doc()doc";

static const char *__doc_FaceType_FACE_POLYGON = R"doc()doc";

static const char *__doc_FaceType_FACE_QUAD = R"doc()doc";

static const char *__doc_FaceType_FACE_REVOLVE = R"doc()doc";

static const char *__doc_FaceType_FACE_REVOLVE_WEDGE = R"doc()doc";

static const char *__doc_FaceType_FACE_UNKNOWN = R"doc()doc";

static const char *__doc_GBaseObject =
R"doc(This is a base class for GObject. I hope to describe all geometry in
terms of this base class instead of GObject)doc";

static const char *__doc_GBaseObject_2 = R"doc(Forward declarations)doc";

static const char *__doc_GBaseObject_AddArcSection = R"doc(Add an arc section through three nodes)doc";

static const char *__doc_GBaseObject_AddBeamPart = R"doc(Add a beam part to the object)doc";

static const char *__doc_GBaseObject_AddBezierSection = R"doc(Add a Bezier section using control points)doc";

static const char *__doc_GBaseObject_AddCircularArc = R"doc(Add a circular arc through three nodes)doc";

static const char *__doc_GBaseObject_AddEdge = R"doc(Add an edge to the object)doc";

static const char *__doc_GBaseObject_AddEdge_2 = R"doc(Add an edge to the object)doc";

static const char *__doc_GBaseObject_AddFace = R"doc(Add a face to the object)doc";

static const char *__doc_GBaseObject_AddFace_2 = R"doc(Add a face to the object)doc";

static const char *__doc_GBaseObject_AddFacet = R"doc(Add a facet with specified nodes and edges)doc";

static const char *__doc_GBaseObject_AddFacet_2 = R"doc(Add a facet with specified edges)doc";

static const char *__doc_GBaseObject_AddLine = R"doc(Add a line between two nodes)doc";

static const char *__doc_GBaseObject_AddNode = R"doc(Add a node to the object)doc";

static const char *__doc_GBaseObject_AddNode_2 = R"doc(Add a node at specified position with given type)doc";

static const char *__doc_GBaseObject_AddPart = R"doc(Add a part to the object)doc";

static const char *__doc_GBaseObject_AddShellPart = R"doc(Add a shell part to the object)doc";

static const char *__doc_GBaseObject_AddSolidPart = R"doc(Add a solid part to the object)doc";

static const char *__doc_GBaseObject_AddSurface = R"doc(Add a surface to the object)doc";

static const char *__doc_GBaseObject_AddYArc = R"doc(Add a Y-axis arc between two nodes)doc";

static const char *__doc_GBaseObject_AddZArc = R"doc(Add a Z-axis arc between two nodes)doc";

static const char *__doc_GBaseObject_ClearAll = R"doc(Clear all geometry data)doc";

static const char *__doc_GBaseObject_ClearEdges = R"doc(Clear all edges)doc";

static const char *__doc_GBaseObject_ClearFaces = R"doc(Clear all faces)doc";

static const char *__doc_GBaseObject_ClearNodes = R"doc(Clear all nodes)doc";

static const char *__doc_GBaseObject_ClearParts = R"doc(Clear all parts)doc";

static const char *__doc_GBaseObject_Copy = R"doc(Copy constructor from another GBaseObject)doc";

static const char *__doc_GBaseObject_CopyTransform = R"doc(Copy transform info from another object)doc";

static const char *__doc_GBaseObject_DeletePart = R"doc(Delete a part from the object)doc";

static const char *__doc_GBaseObject_Edge = R"doc(Return pointer to an edge using local ID)doc";

static const char *__doc_GBaseObject_Edge_2 = R"doc(Return const pointer to an edge using local ID)doc";

static const char *__doc_GBaseObject_Edges = R"doc(Return number of edges)doc";

static const char *__doc_GBaseObject_Face = R"doc(Return pointer to a face using local ID)doc";

static const char *__doc_GBaseObject_Face_2 = R"doc(Return const pointer to a face using local ID)doc";

static const char *__doc_GBaseObject_Faces = R"doc(Return number of faces)doc";

static const char *__doc_GBaseObject_FindEdge = R"doc(Return pointer to an edge using global ID)doc";

static const char *__doc_GBaseObject_FindFace = R"doc(Return pointer to a face using global ID)doc";

static const char *__doc_GBaseObject_FindNode = R"doc(Return pointer to a node using global ID)doc";

static const char *__doc_GBaseObject_FindPart = R"doc(Return pointer to a part using global ID)doc";

static const char *__doc_GBaseObject_FindPartFromName = R"doc(Find a part by name)doc";

static const char *__doc_GBaseObject_GBaseObject = R"doc(Default constructor)doc";

static const char *__doc_GBaseObject_GetPosition = R"doc(Get the object's position)doc";

static const char *__doc_GBaseObject_GetRenderTransform = R"doc(Get the object's render transform)doc";

static const char *__doc_GBaseObject_GetRenderTransform_2 = R"doc(Get the object's render transform (const version))doc";

static const char *__doc_GBaseObject_GetTransform = R"doc(Get the object's transform)doc";

static const char *__doc_GBaseObject_GetTransform_2 = R"doc(Get the object's transform (const version))doc";

static const char *__doc_GBaseObject_Node = R"doc(Return pointer to a node using local ID)doc";

static const char *__doc_GBaseObject_Node_2 = R"doc(Return const pointer to a node using local ID)doc";

static const char *__doc_GBaseObject_Nodes = R"doc(Return number of nodes)doc";

static const char *__doc_GBaseObject_Part = R"doc(Return pointer to a part using local ID)doc";

static const char *__doc_GBaseObject_Part_2 = R"doc(Return const pointer to a part using local ID)doc";

static const char *__doc_GBaseObject_Parts = R"doc(Return number of parts)doc";

static const char *__doc_GBaseObject_RemoveNode = R"doc(Remove a node by index)doc";

static const char *__doc_GBaseObject_ResizeCurves = R"doc(Resize the curves array)doc";

static const char *__doc_GBaseObject_ResizeNodes = R"doc(Resize the nodes array)doc";

static const char *__doc_GBaseObject_ResizeParts = R"doc(Resize the parts array)doc";

static const char *__doc_GBaseObject_ResizeSurfaces = R"doc(Resize the surfaces array)doc";

static const char *__doc_GBaseObject_SetPosition = R"doc(Set the object's position)doc";

static const char *__doc_GBaseObject_SetRenderTransform = R"doc(Set the object's render transform)doc";

static const char *__doc_GBaseObject_UpdateNodeTypes = R"doc(Update the node types)doc";

static const char *__doc_GBaseObject_m_Edge = R"doc(Edges vector)doc";

static const char *__doc_GBaseObject_m_Face = R"doc(Surfaces vector)doc";

static const char *__doc_GBaseObject_m_Node = R"doc(Nodes vector)doc";

static const char *__doc_GBaseObject_m_Part = R"doc(Parts vector)doc";

static const char *__doc_GBaseObject_m_renderTransform = R"doc(The objects' transform for rendering)doc";

static const char *__doc_GBaseObject_m_transform = R"doc(The object's transform)doc";

static const char *__doc_GBox = R"doc(Simple rectangular box)doc";

static const char *__doc_GBoxInBox = R"doc(Box within a box)doc";

static const char *__doc_GBoxInBox_Create = R"doc(Create the geometry)doc";

static const char *__doc_GBoxInBox_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GBoxInBox_GBoxInBox = R"doc(Default constructor)doc";

static const char *__doc_GBoxInBox_InnerDepth = R"doc(Get the inner depth)doc";

static const char *__doc_GBoxInBox_InnerHeight = R"doc(Get the inner height)doc";

static const char *__doc_GBoxInBox_InnerWidth = R"doc(Get the inner width)doc";

static const char *__doc_GBoxInBox_OuterDepth = R"doc(Get the outer depth)doc";

static const char *__doc_GBoxInBox_OuterHeight = R"doc(Get the outer height)doc";

static const char *__doc_GBoxInBox_OuterWidth = R"doc(Get the outer width)doc";

static const char *__doc_GBoxInBox_Update = R"doc(Update the object)doc";

static const char *__doc_GBox_Create = R"doc(Create the geometry)doc";

static const char *__doc_GBox_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GBox_GBox = R"doc(Default constructor)doc";

static const char *__doc_GBox_GBox_2 = R"doc(Constructor with dimensions)doc";

static const char *__doc_GBox_Update = R"doc(Update the object)doc";

static const char *__doc_GBox_m_d = R"doc(Depth of the box)doc";

static const char *__doc_GBox_m_h = R"doc(Height of the box)doc";

static const char *__doc_GBox_m_w = R"doc(Width of the box)doc";

static const char *__doc_GBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_67_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_67_2_DEPTH = R"doc()doc";

static const char *__doc_GBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_67_2_HEIGHT = R"doc()doc";

static const char *__doc_GBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_67_2_WIDTH = R"doc()doc";

static const char *__doc_GCone = R"doc(Tapered cone)doc";

static const char *__doc_GCone_BottomRadius = R"doc(Get the bottom radius)doc";

static const char *__doc_GCone_BuildGMesh = R"doc(Build the geometry mesh)doc";

static const char *__doc_GCone_Create = R"doc(Create the geometry)doc";

static const char *__doc_GCone_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GCone_GCone = R"doc(Default constructor)doc";

static const char *__doc_GCone_Height = R"doc(Get the height)doc";

static const char *__doc_GCone_SetBottomRadius = R"doc(Set the bottom radius)doc";

static const char *__doc_GCone_SetHeight = R"doc(Set the height)doc";

static const char *__doc_GCone_SetTopRadius = R"doc(Set the top radius)doc";

static const char *__doc_GCone_TopRadius = R"doc(Get the top radius)doc";

static const char *__doc_GCone_Update = R"doc(Update the object)doc";

static const char *__doc_GCone_m_R0 = R"doc(Bottom radius)doc";

static const char *__doc_GCone_m_R1 = R"doc(Top radius)doc";

static const char *__doc_GCone_m_h = R"doc(Height)doc";

static const char *__doc_GCone_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_95_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GCone_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_95_2_H = R"doc()doc";

static const char *__doc_GCone_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_95_2_R0 = R"doc()doc";

static const char *__doc_GCone_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_95_2_R1 = R"doc()doc";

static const char *__doc_GCylinder = R"doc(Circular cylinder)doc";

static const char *__doc_GCylinder2 = R"doc(Ellipsoidal cylinder)doc";

static const char *__doc_GCylinder2_Create = R"doc(Create the geometry)doc";

static const char *__doc_GCylinder2_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GCylinder2_GCylinder2 = R"doc(Default constructor)doc";

static const char *__doc_GCylinder2_Update = R"doc(Update the object)doc";

static const char *__doc_GCylinder2_m_Rx = R"doc(X-axis radius)doc";

static const char *__doc_GCylinder2_m_Ry = R"doc(Y-axis radius)doc";

static const char *__doc_GCylinder2_m_h = R"doc(Height of the cylinder)doc";

static const char *__doc_GCylinder2_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_171_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GCylinder2_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_171_2_HEIGHT = R"doc()doc";

static const char *__doc_GCylinder2_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_171_2_RADIUSX = R"doc()doc";

static const char *__doc_GCylinder2_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_171_2_RADIUSY = R"doc()doc";

static const char *__doc_GCylinderInBox = R"doc(Box with a cylindrical cavity)doc";

static const char *__doc_GCylinderInBox_Create = R"doc(Create the geometry)doc";

static const char *__doc_GCylinderInBox_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GCylinderInBox_GCylinderInBox = R"doc(Default constructor)doc";

static const char *__doc_GCylinderInBox_Update = R"doc(Update the object)doc";

static const char *__doc_GCylinderInBox_m_D = R"doc(Depth of the box)doc";

static const char *__doc_GCylinderInBox_m_H = R"doc(Height of the box)doc";

static const char *__doc_GCylinderInBox_m_R = R"doc(Radius of the cylindrical cavity)doc";

static const char *__doc_GCylinderInBox_m_W = R"doc(Width of the box)doc";

static const char *__doc_GCylinderInBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_314_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GCylinderInBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_314_2_DEPTH = R"doc()doc";

static const char *__doc_GCylinderInBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_314_2_HEIGHT = R"doc()doc";

static const char *__doc_GCylinderInBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_314_2_RADIUS = R"doc()doc";

static const char *__doc_GCylinderInBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_314_2_WIDTH = R"doc()doc";

static const char *__doc_GCylinder_Create = R"doc(Create the geometry)doc";

static const char *__doc_GCylinder_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GCylinder_GCylinder = R"doc(Default constructor)doc";

static const char *__doc_GCylinder_Height = R"doc(Get the height)doc";

static const char *__doc_GCylinder_Radius = R"doc(Get the radius)doc";

static const char *__doc_GCylinder_SetHeight = R"doc(Set the height)doc";

static const char *__doc_GCylinder_SetRadius = R"doc(Set the radius)doc";

static const char *__doc_GCylinder_Update = R"doc(Update the object)doc";

static const char *__doc_GCylinder_m_R = R"doc(Radius of the cylinder)doc";

static const char *__doc_GCylinder_m_h = R"doc(Height of the cylinder)doc";

static const char *__doc_GCylinder_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_137_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GCylinder_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_137_2_HEIGHT = R"doc()doc";

static const char *__doc_GCylinder_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_137_2_RADIUS = R"doc()doc";

static const char *__doc_GCylindricalPatch = R"doc(Cylindrical patch)doc";

static const char *__doc_GCylindricalPatch_Create = R"doc(Create the geometry)doc";

static const char *__doc_GCylindricalPatch_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GCylindricalPatch_GCylindricalPatch = R"doc(Default constructor)doc";

static const char *__doc_GCylindricalPatch_Height = R"doc(Get the height)doc";

static const char *__doc_GCylindricalPatch_Radius = R"doc(Get the radius)doc";

static const char *__doc_GCylindricalPatch_Update = R"doc(Update the object)doc";

static const char *__doc_GCylindricalPatch_Width = R"doc(Get the width)doc";

static const char *__doc_GCylindricalPatch_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_647_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GCylindricalPatch_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_647_2_H = R"doc()doc";

static const char *__doc_GCylindricalPatch_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_647_2_R = R"doc()doc";

static const char *__doc_GCylindricalPatch_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_647_2_W = R"doc()doc";

static const char *__doc_GDisc = R"doc(2D circular disc)doc";

static const char *__doc_GDisc_Create = R"doc(Create the geometry)doc";

static const char *__doc_GDisc_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GDisc_CreateMesh = R"doc(Create mesh with specified parameters)doc";

static const char *__doc_GDisc_GDisc = R"doc(Default constructor)doc";

static const char *__doc_GDisc_GDisc_2 = R"doc(Constructor with radius)doc";

static const char *__doc_GDisc_Radius = R"doc(Get the radius)doc";

static const char *__doc_GDisc_SetRadius = R"doc(Set the radius)doc";

static const char *__doc_GDisc_Update = R"doc(Update the object)doc";

static const char *__doc_GDisc_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_528_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GDisc_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_528_2_RADIUS = R"doc()doc";

static const char *__doc_GEdge = R"doc(Defines the edge of the object)doc";

static const char *__doc_GEdge_GEdge = R"doc(Default constructor)doc";

static const char *__doc_GEdge_GEdge_2 = R"doc(Constructor with parent object)doc";

static const char *__doc_GEdge_GEdge_3 = R"doc(Copy constructor)doc";

static const char *__doc_GEdge_GetFEEdgeSet = R"doc(Get FE edge set)doc";

static const char *__doc_GEdge_HasNode = R"doc(Check if edge has a specific node)doc";

static const char *__doc_GEdge_Length = R"doc(Calculate edge length)doc";

static const char *__doc_GEdge_Load = R"doc(Load from archive)doc";

static const char *__doc_GEdge_Node = R"doc(Get node at index)doc";

static const char *__doc_GEdge_Point = R"doc(Get point at parameter l)doc";

static const char *__doc_GEdge_Save = R"doc(Save to archive)doc";

static const char *__doc_GEdge_Tangent = R"doc(Get tangent vector at parameter l)doc";

static const char *__doc_GEdge_Type = R"doc(Get edge type)doc";

static const char *__doc_GEdge_m_cnode = R"doc(Additional shape nodes)doc";

static const char *__doc_GEdge_m_node = R"doc(Indices of start and end nodes)doc";

static const char *__doc_GEdge_m_ntype = R"doc(Type identifier)doc";

static const char *__doc_GEdge_m_orient = R"doc(Orientation for arcs)doc";

static const char *__doc_GEdge_operator_assign = R"doc(Assignment operator)doc";

static const char *__doc_GEdge_operator_eq = R"doc(Equality operator)doc";

static const char *__doc_GFace = R"doc(Defines a face of the object)doc";

static const char *__doc_GFace_EDGE = R"doc(Edge structure containing ID and winding)doc";

static const char *__doc_GFace_EDGE_nid = R"doc(Local ID of edge)doc";

static const char *__doc_GFace_EDGE_nwn = R"doc(Winding (+1 or -1))doc";

static const char *__doc_GFace_Edges = R"doc(Get number of edges)doc";

static const char *__doc_GFace_GFace = R"doc(Default constructor)doc";

static const char *__doc_GFace_GFace_2 = R"doc(Constructor with parent object)doc";

static const char *__doc_GFace_GFace_3 = R"doc(Copy constructor)doc";

static const char *__doc_GFace_HasEdge = R"doc(Check if face has a specific edge)doc";

static const char *__doc_GFace_Invert = R"doc(Invert the face orientation)doc";

static const char *__doc_GFace_IsExternal = R"doc(Check if face is external)doc";

static const char *__doc_GFace_Load = R"doc(Load from archive)doc";

static const char *__doc_GFace_Nodes = R"doc(Get number of nodes)doc";

static const char *__doc_GFace_Save = R"doc(Save to archive)doc";

static const char *__doc_GFace_m_edge = R"doc(Edges defining face)doc";

static const char *__doc_GFace_m_nPID = R"doc(Part ID's)doc";

static const char *__doc_GFace_m_node = R"doc(Node ID's)doc";

static const char *__doc_GFace_m_ntype = R"doc(Face type)doc";

static const char *__doc_GFace_operator_assign = R"doc(Assignment operator)doc";

static const char *__doc_GGregoryPatch = R"doc(Gregory patch)doc";

static const char *__doc_GGregoryPatch_GGregoryPatch = R"doc(Constructor with mesh)doc";

static const char *__doc_GGregoryPatch_UpdateMesh = R"doc(Update the mesh)doc";

static const char *__doc_GHexagon = R"doc(Hexagon primitive)doc";

static const char *__doc_GHexagon_Create = R"doc(Create the geometry)doc";

static const char *__doc_GHexagon_GHexagon = R"doc(Default constructor)doc";

static const char *__doc_GHexagon_Height = R"doc(Get the height)doc";

static const char *__doc_GHexagon_Radius = R"doc(Get the radius)doc";

static const char *__doc_GHexagon_SetHeight = R"doc(Set the height)doc";

static const char *__doc_GHexagon_SetRadius = R"doc(Set the radius)doc";

static const char *__doc_GHexagon_Update = R"doc(Update the object)doc";

static const char *__doc_GHexagon_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_500_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GHexagon_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_500_2_HEIGHT = R"doc()doc";

static const char *__doc_GHexagon_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_500_2_RADIUS = R"doc()doc";

static const char *__doc_GHollowSphere = R"doc(Hollow concentric sphere)doc";

static const char *__doc_GHollowSphere_BuildGMesh = R"doc(Build the geometry mesh)doc";

static const char *__doc_GHollowSphere_Create = R"doc(Create the geometry)doc";

static const char *__doc_GHollowSphere_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GHollowSphere_GHollowSphere = R"doc(Default constructor)doc";

static const char *__doc_GHollowSphere_NodeIndex = R"doc(Get node index)doc";

static const char *__doc_GHollowSphere_Update = R"doc(Update the object)doc";

static const char *__doc_GHollowSphere_m_Ri = R"doc(Inner radius)doc";

static const char *__doc_GHollowSphere_m_Ro = R"doc(Outer radius)doc";

static const char *__doc_GHollowSphere_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_197_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GHollowSphere_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_197_2_RIN = R"doc()doc";

static const char *__doc_GHollowSphere_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_197_2_ROUT = R"doc()doc";

static const char *__doc_GItem =
R"doc(Base class for items of geometry objects. GItem's have two ID's: -
m_gid: the global ID number identifying the item number in the model
(one-based) - m_lid: the local ID number which is the index in the
object's item list (zero-based))doc";

static const char *__doc_GItem_GItem = R"doc(Constructor. Takes parent object as parameter)doc";

static const char *__doc_GItem_GetID = R"doc(Get global ID)doc";

static const char *__doc_GItem_GetLocalID = R"doc(Get local ID)doc";

static const char *__doc_GItem_GetMeshWeight = R"doc(Get mesh weight for this item)doc";

static const char *__doc_GItem_GetState = R"doc(Get state flags)doc";

static const char *__doc_GItem_HideItem = R"doc(Hide this item)doc";

static const char *__doc_GItem_IsActive = R"doc(Check active status)doc";

static const char *__doc_GItem_IsRequired = R"doc(Check if item is required)doc";

static const char *__doc_GItem_IsSelected = R"doc(Check selection state)doc";

static const char *__doc_GItem_IsVisible = R"doc(Check visibility state (only used by GBaseObject's))doc";

static const char *__doc_GItem_Object = R"doc(Get the parent object (non-const version))doc";

static const char *__doc_GItem_Object_2 = R"doc(Get the parent object (const version))doc";

static const char *__doc_GItem_Select = R"doc(Select this item)doc";

static const char *__doc_GItem_SetActive = R"doc(Set active state)doc";

static const char *__doc_GItem_SetID = R"doc(Set global ID (pure virtual))doc";

static const char *__doc_GItem_SetLocalID = R"doc(Set local ID)doc";

static const char *__doc_GItem_SetMeshWeight = R"doc(Set mesh weight for this item)doc";

static const char *__doc_GItem_SetRequired = R"doc(Set required state)doc";

static const char *__doc_GItem_SetState = R"doc(Set state flags)doc";

static const char *__doc_GItem_ShowItem = R"doc(Show this item)doc";

static const char *__doc_GItem_T =
R"doc(Intermediate base class defining a counter for each derived item class
that can be used to determine a unique global ID number)doc";

static const char *__doc_GItem_T_CreateUniqueID = R"doc(Create a unique ID)doc";

static const char *__doc_GItem_T_DecreaseCounter = R"doc(Decrease the counter)doc";

static const char *__doc_GItem_T_GItem_T = R"doc(Constructor)doc";

static const char *__doc_GItem_T_GetCounter = R"doc(Get the counter value)doc";

static const char *__doc_GItem_T_IncreaseCounter = R"doc(Increase the counter)doc";

static const char *__doc_GItem_T_ResetCounter = R"doc(Reset the counter)doc";

static const char *__doc_GItem_T_SetCounter = R"doc(Set the counter value)doc";

static const char *__doc_GItem_T_SetID = R"doc(Set ID and update counter)doc";

static const char *__doc_GItem_UnSelect = R"doc(Unselect this item)doc";

static const char *__doc_GItem_m_gid = R"doc(Global ID (one-based))doc";

static const char *__doc_GItem_m_lid = R"doc(Local ID (zero-based))doc";

static const char *__doc_GItem_m_ntag = R"doc(Multi-purpose tag)doc";

static const char *__doc_GItem_m_po = R"doc(Pointer to object this item belongs to)doc";

static const char *__doc_GItem_m_state = R"doc(State variable)doc";

static const char *__doc_GItem_m_weight = R"doc(Weight used for meshing)doc";

static const char *__doc_GLMesh = R"doc()doc";

static const char *__doc_GMeshObject =
R"doc(The GMeshObject defines the geometry of a "naked" FE mesh. That is, a
mesh that does not correspond to a geometry object. There are two ways
to create GMeshObjects. The first is to import a FE model from a file.
Most FE models imported are considered naked. The second is by
converting a geometry object that has a mesh to a GMeshObject.

This object defines the geometry (such as parts, faces, edges, nodes)
using this FE geometry. This has one major limitation and that is that
when the user changes the mesh, it can not be assumed that the
geometry stays the same and therefore all geometry dependant
quantities (e.g. boundary conditions) need to be removed. The
GMeshObject is the only object for which the user can define their own
partitions.)doc";

static const char *__doc_GMeshObject_AddNode = R"doc(Add a new node at the specified position)doc";

static const char *__doc_GMeshObject_Attach = R"doc(Attach another object to this one)doc";

static const char *__doc_GMeshObject_BuildGMesh = R"doc(Build the geometry mesh)doc";

static const char *__doc_GMeshObject_BuildMesh = R"doc(Build the finite element mesh)doc";

static const char *__doc_GMeshObject_Clone = R"doc(Create a clone of this object)doc";

static const char *__doc_GMeshObject_DeletePart = R"doc(Delete a part from the object)doc";

static const char *__doc_GMeshObject_DeleteParts = R"doc(Delete multiple parts from the object)doc";

static const char *__doc_GMeshObject_DetachSelection = R"doc(Detach an element selection and create a new GMeshObject)doc";

static const char *__doc_GMeshObject_GMeshObject = R"doc(Constructor for creating a GMeshObject from a "naked" mesh)doc";

static const char *__doc_GMeshObject_GMeshObject_2 =
R"doc(Constructor for creating a GMeshObject from a "naked" surface mesh
(this creates a shell mesh))doc";

static const char *__doc_GMeshObject_GMeshObject_3 = R"doc(Constructor for converting a GObject to a GMeshObject)doc";

static const char *__doc_GMeshObject_GetEditableLineMesh = R"doc(Get the editable line mesh)doc";

static const char *__doc_GMeshObject_GetEditableMesh = R"doc(Get the editable mesh base)doc";

static const char *__doc_GMeshObject_Load = R"doc(Load object from archive during serialization)doc";

static const char *__doc_GMeshObject_MakeGNode = R"doc(Make a geometry node from mesh node index)doc";

static const char *__doc_GMeshObject_Update = R"doc(Update geometry information)doc";

static const char *__doc_GMeshObject_UpdateEdges = R"doc(Update the edges of the geometry)doc";

static const char *__doc_GMeshObject_UpdateFaceNodes = R"doc(Update the face nodes)doc";

static const char *__doc_GMeshObject_UpdateNodes = R"doc(Update the nodes of the geometry)doc";

static const char *__doc_GMeshObject_UpdateParts = R"doc(Update the parts of the geometry)doc";

static const char *__doc_GMeshObject_UpdateSections = R"doc(Update the sections (made virtual for PostObject override))doc";

static const char *__doc_GMeshObject_UpdateSurfaces = R"doc(Update the surfaces of the geometry)doc";

static const char *__doc_GNode = R"doc(Forward declaration)doc";

static const char *__doc_GNode_2 = R"doc(Forward declaration)doc";

static const char *__doc_GNode_GNode = R"doc(Default constructor)doc";

static const char *__doc_GNode_GNode_2 = R"doc(Constructor with parent object)doc";

static const char *__doc_GNode_GNode_3 = R"doc(Copy constructor)doc";

static const char *__doc_GNode_GetNodeIndex = R"doc(Get the node index in the mesh)doc";

static const char *__doc_GNode_Load = R"doc(Load from archive)doc";

static const char *__doc_GNode_LocalPosition = R"doc(Get the local position of this node (non-const version))doc";

static const char *__doc_GNode_LocalPosition_2 = R"doc(Get the local position of this node (const version))doc";

static const char *__doc_GNode_MakeRequired = R"doc(Make this node required)doc";

static const char *__doc_GNode_Position = R"doc(Get the global position of the node)doc";

static const char *__doc_GNode_Save = R"doc(Save to archive)doc";

static const char *__doc_GNode_SetNodeIndex = R"doc(Set the node index in the mesh)doc";

static const char *__doc_GNode_SetType = R"doc(Set the node type)doc";

static const char *__doc_GNode_Type = R"doc(Get the node type)doc";

static const char *__doc_GNode_m_node = R"doc(Index of node in mesh)doc";

static const char *__doc_GNode_m_ntype = R"doc(Node type)doc";

static const char *__doc_GNode_m_r = R"doc(Node position (in local coordinates))doc";

static const char *__doc_GNode_operator_assign = R"doc(Assignment operator)doc";

static const char *__doc_GO_Orientation = R"doc()doc";

static const char *__doc_GO_Orientation_CCW = R"doc()doc";

static const char *__doc_GO_Orientation_CW = R"doc()doc";

static const char *__doc_GObject = R"doc(GObject is the base class for all geometry objects)doc";

static const char *__doc_GObject_2 = R"doc(GObject is the base class for all geometry objects)doc";

static const char *__doc_GObjectManipulator = R"doc(Object manipulator class for handling object transformations)doc";

static const char *__doc_GObjectManipulator_2 = R"doc(Object manipulator class for handling object transformations)doc";

static const char *__doc_GObjectManipulator_GObjectManipulator = R"doc(Constructor)doc";

static const char *__doc_GObjectManipulator_GetObject = R"doc(Get the associated object)doc";

static const char *__doc_GObjectManipulator_TransformNode = R"doc(Transform a node with the given transformation)doc";

static const char *__doc_GObjectManipulator_m_po = R"doc(Pointer to the associated object)doc";

static const char *__doc_GObject_AddFEEdgeSet = R"doc(Add an FE edge set)doc";

static const char *__doc_GObject_AddFEElemSet = R"doc(Add an FE element set)doc";

static const char *__doc_GObject_AddFENodeSet = R"doc(Add an FE node set)doc";

static const char *__doc_GObject_AddFEPartSet = R"doc(Add an FE part set)doc";

static const char *__doc_GObject_AddFESurface = R"doc(Add an FE surface)doc";

static const char *__doc_GObject_BuildFERenderMesh = R"doc(Build the FE render mesh)doc";

static const char *__doc_GObject_BuildGMesh = R"doc(Build the render mesh)doc";

static const char *__doc_GObject_BuildMesh = R"doc(Build the FSMesh)doc";

static const char *__doc_GObject_CanDelete = R"doc(Check if the object has any dependencies)doc";

static const char *__doc_GObject_CanDeleteMesh = R"doc(Check if the mesh can be deleted)doc";

static const char *__doc_GObject_ClearFEGroups = R"doc(Clear all FE groups)doc";

static const char *__doc_GObject_Clone = R"doc(Create a clone of this object)doc";

static const char *__doc_GObject_CollapseTransform = R"doc(Collapse the transformation)doc";

static const char *__doc_GObject_Copy =
R"doc(Copy the geometry from another object. This does not copy the mesh
from the passed object. If a mesh exists then this mesh is deleted.)doc";

static const char *__doc_GObject_Create = R"doc(Create the object)doc";

static const char *__doc_GObject_CreateDefaultMesher = R"doc(Create a default mesher)doc";

static const char *__doc_GObject_DeleteFEMesh = R"doc(Delete the mesh)doc";

static const char *__doc_GObject_FEEdgeSets = R"doc(Get number of FE edge sets)doc";

static const char *__doc_GObject_FEElemSets = R"doc(Get number of FE element sets)doc";

static const char *__doc_GObject_FENodeSets = R"doc(Get number of FE node sets)doc";

static const char *__doc_GObject_FEPartSets = R"doc(Get number of FE part sets)doc";

static const char *__doc_GObject_FESurfaces = R"doc(Get number of FE surfaces)doc";

static const char *__doc_GObject_FindFEEdgeSet = R"doc(Find an FE edge set by name)doc";

static const char *__doc_GObject_FindFEGroup = R"doc(Find an FE group by ID)doc";

static const char *__doc_GObject_FindFENodeSet = R"doc(Find an FE node set by name)doc";

static const char *__doc_GObject_FindFEPartSet = R"doc(Find an FE part set by name)doc";

static const char *__doc_GObject_FindFESurface = R"doc(Find an FE surface by name)doc";

static const char *__doc_GObject_FindNodeFromTag = R"doc(Find a node from its tag)doc";

static const char *__doc_GObject_GObject = R"doc(Constructor)doc";

static const char *__doc_GObject_GetActiveObject = R"doc(Get the active object)doc";

static const char *__doc_GObject_GetColor = R"doc(Get object color)doc";

static const char *__doc_GObject_GetEditableLineMesh = R"doc(Get the editable line mesh)doc";

static const char *__doc_GObject_GetEditableMesh = R"doc(Get the editable mesh)doc";

static const char *__doc_GObject_GetFECurveMesh = R"doc(Get the mesh of an edge curve)doc";

static const char *__doc_GObject_GetFEEdgeSet = R"doc(Get an FE edge set by index)doc";

static const char *__doc_GObject_GetFEElemSet = R"doc(Get an FE element set by index)doc";

static const char *__doc_GObject_GetFEMesh = R"doc(Retrieve the FE mesh)doc";

static const char *__doc_GObject_GetFEMesh_2 = R"doc(Retrieve the FE mesh (const version))doc";

static const char *__doc_GObject_GetFEMesher = R"doc(Retrieve the mesher)doc";

static const char *__doc_GObject_GetFENode = R"doc(Retrieve an FE nodes from a GNode)doc";

static const char *__doc_GObject_GetFENodeSet = R"doc(Get an FE node set by index)doc";

static const char *__doc_GObject_GetFEPartSet = R"doc(Get an FE part set by index)doc";

static const char *__doc_GObject_GetFERenderMesh = R"doc(Get the mesh for rendering the FE mesh)doc";

static const char *__doc_GObject_GetFESurface = R"doc(Get an FE surface by index)doc";

static const char *__doc_GObject_GetGlobalBox = R"doc(Get the global bounding box)doc";

static const char *__doc_GObject_GetLocalBox = R"doc(Get the local bounding box)doc";

static const char *__doc_GObject_GetManipulator = R"doc(Get the object manipulator)doc";

static const char *__doc_GObject_GetRenderMesh = R"doc(Get the render mesh)doc";

static const char *__doc_GObject_GetType = R"doc(Return type of Object)doc";

static const char *__doc_GObject_Hide = R"doc(Hide the object)doc";

static const char *__doc_GObject_Imp = R"doc(Internal implementation class)doc";

static const char *__doc_GObject_InsertFEEdgeSet = R"doc(Insert an FE edge set at specified index)doc";

static const char *__doc_GObject_InsertFEElemSet = R"doc(Insert an FE element set at specified index)doc";

static const char *__doc_GObject_InsertFENodeSet = R"doc(Insert an FE node set at specified index)doc";

static const char *__doc_GObject_InsertFEPartSet = R"doc(Insert an FE part set at specified index)doc";

static const char *__doc_GObject_InsertFESurface = R"doc(Insert an FE surface at specified index)doc";

static const char *__doc_GObject_IsActiveObject = R"doc(Check if this is the active object)doc";

static const char *__doc_GObject_IsFaceVisible = R"doc(Check if a face is visible)doc";

static const char *__doc_GObject_IsValid = R"doc(Check if the object is valid)doc";

static const char *__doc_GObject_Load = R"doc(Load object from archive)doc";

static const char *__doc_GObject_Reindex = R"doc(Reindex the object)doc";

static const char *__doc_GObject_RemoveEmptyFEGroups = R"doc(Remove empty FE groups)doc";

static const char *__doc_GObject_RemoveFEEdgeSet = R"doc(Remove an FE edge set)doc";

static const char *__doc_GObject_RemoveFEElemSet = R"doc(Remove an FE element set)doc";

static const char *__doc_GObject_RemoveFENodeSet = R"doc(Remove an FE node set)doc";

static const char *__doc_GObject_RemoveFEPartSet = R"doc(Remove an FE part set)doc";

static const char *__doc_GObject_RemoveFESurface = R"doc(Remove an FE surface)doc";

static const char *__doc_GObject_RemoveUnusedFEGroups = R"doc(Remove unused FE groups)doc";

static const char *__doc_GObject_ReplaceFEMesh = R"doc(Replace the current mesh (returns old mesh))doc";

static const char *__doc_GObject_ReplaceSurfaceMesh = R"doc(Replace the current surface mesh)doc";

static const char *__doc_GObject_Save = R"doc(Save object to archive)doc";

static const char *__doc_GObject_SetActiveObject = R"doc(Set the active object)doc";

static const char *__doc_GObject_SetColor = R"doc(Set object color)doc";

static const char *__doc_GObject_SetFEMesh = R"doc(Set the FE mesh)doc";

static const char *__doc_GObject_SetFEMesher = R"doc(Set the mesher)doc";

static const char *__doc_GObject_SetFERenderMesh = R"doc(Set the FE render mesh)doc";

static const char *__doc_GObject_SetManipulator = R"doc(Set the object manipulator)doc";

static const char *__doc_GObject_SetRenderMesh = R"doc(Set the render mesh)doc";

static const char *__doc_GObject_SetValidFlag = R"doc(Set the valid flag)doc";

static const char *__doc_GObject_Show = R"doc(Show the object)doc";

static const char *__doc_GObject_ShowAllParts = R"doc(Show all parts)doc";

static const char *__doc_GObject_ShowElements = R"doc(Show elements in the provided list)doc";

static const char *__doc_GObject_ShowPart = R"doc(Show (or hide) a part by index)doc";

static const char *__doc_GObject_ShowPart_2 = R"doc(Show (or hide) a part by reference)doc";

static const char *__doc_GObject_Update = R"doc(Update data)doc";

static const char *__doc_GObject_UpdateFEElementMatIDs = R"doc(Update the element material IDs)doc";

static const char *__doc_GObject_UpdateFERenderMesh = R"doc(Update the FE render mesh)doc";

static const char *__doc_GObject_UpdateGNodes = R"doc(Update GNodes (only GMeshObject uses this))doc";

static const char *__doc_GObject_UpdateItemVisibility =
R"doc(Update the visibility of items (i.e. surfaces, edges, nodes, and mesh
items))doc";

static const char *__doc_GObject_UpdateMesh = R"doc(Update FE mesh)doc";

static const char *__doc_GObject_UpdateMeshData = R"doc(Update mesh data)doc";

static const char *__doc_GObject_imp = R"doc(Internal implementation pointer)doc";

static const char *__doc_GPart = R"doc(Defines a part of the object)doc";

static const char *__doc_GPart_Edges = R"doc(Get number of edges)doc";

static const char *__doc_GPart_Faces = R"doc(Get number of faces)doc";

static const char *__doc_GPart_GPart = R"doc(Default constructor)doc";

static const char *__doc_GPart_GPart_2 = R"doc(Constructor with parent object)doc";

static const char *__doc_GPart_GPart_3 = R"doc(Copy constructor)doc";

static const char *__doc_GPart_GetGlobalBox = R"doc(Get global bounding box)doc";

static const char *__doc_GPart_GetLocalBox = R"doc(Get local bounding box)doc";

static const char *__doc_GPart_GetMaterialID = R"doc(Get material ID)doc";

static const char *__doc_GPart_GetSection = R"doc(Get the part section)doc";

static const char *__doc_GPart_IsBeam = R"doc(Check if part is beam)doc";

static const char *__doc_GPart_IsShell = R"doc(Check if part is shell)doc";

static const char *__doc_GPart_IsSolid = R"doc(Check if part is solid)doc";

static const char *__doc_GPart_Load = R"doc(Load from archive)doc";

static const char *__doc_GPart_Nodes = R"doc(Get number of nodes)doc";

static const char *__doc_GPart_Save = R"doc(Save to archive)doc";

static const char *__doc_GPart_SetMaterialID = R"doc(Set material ID)doc";

static const char *__doc_GPart_SetSection = R"doc(Set the part section)doc";

static const char *__doc_GPart_Update = R"doc(Update the part)doc";

static const char *__doc_GPart_UpdateBoundingBox = R"doc(Update bounding box)doc";

static const char *__doc_GPart_m_box = R"doc(Bounding box in local coordinates)doc";

static const char *__doc_GPart_m_edge = R"doc(Edge indices)doc";

static const char *__doc_GPart_m_face = R"doc(Face indices)doc";

static const char *__doc_GPart_m_matid = R"doc(Material ID)doc";

static const char *__doc_GPart_m_node = R"doc(Node indices)doc";

static const char *__doc_GPart_m_section = R"doc(Part section)doc";

static const char *__doc_GPart_operator_assign = R"doc(Assignment operator)doc";

static const char *__doc_GPatch = R"doc(2D rectangular patch)doc";

static const char *__doc_GPatch_Create = R"doc(Create the geometry)doc";

static const char *__doc_GPatch_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GPatch_GPatch = R"doc(Default constructor)doc";

static const char *__doc_GPatch_Update = R"doc(Update the object)doc";

static const char *__doc_GPatch_m_h = R"doc(Height of the patch)doc";

static const char *__doc_GPatch_m_w = R"doc(Width of the patch)doc";

static const char *__doc_GPatch_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_560_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GPatch_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_560_2_H = R"doc()doc";

static const char *__doc_GPatch_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_560_2_W = R"doc()doc";

static const char *__doc_GPrimitive =
R"doc(The GPrimitive class manages the parts, surfaces and nodesets
automatically The user can not alter this data. This class assumes
that all parts, surfaces, and nodesets are created by whatever class
created the mesh. For example, since most procedural meshes
automatically create their own partitions, they use auto-objects to
manager their geometry. This is contrast for instance with the the
GMeshObject which uses the FE mesh to define the geometry.)doc";

static const char *__doc_GPrimitive_Clone = R"doc(Clone the object)doc";

static const char *__doc_GPrimitive_GPrimitive = R"doc(Constructor)doc";

static const char *__doc_GPrimitive_GetEditableLineMesh = R"doc(Get the editable line mesh)doc";

static const char *__doc_GPrimitive_GetEditableMesh = R"doc(Get the editable mesh)doc";

static const char *__doc_GQuartDogBone = R"doc(Quarter symmetry dog-bone)doc";

static const char *__doc_GQuartDogBone_Create = R"doc(Create the geometry)doc";

static const char *__doc_GQuartDogBone_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GQuartDogBone_GQuartDogBone = R"doc(Default constructor)doc";

static const char *__doc_GQuartDogBone_Update = R"doc(Update the object)doc";

static const char *__doc_GQuartDogBone_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_460_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GQuartDogBone_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_460_2_CHEIGHT = R"doc()doc";

static const char *__doc_GQuartDogBone_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_460_2_CWIDTH = R"doc()doc";

static const char *__doc_GQuartDogBone_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_460_2_DEPTH = R"doc()doc";

static const char *__doc_GQuartDogBone_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_460_2_LENGTH = R"doc()doc";

static const char *__doc_GQuartDogBone_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_460_2_RADIUS = R"doc()doc";

static const char *__doc_GQuartDogBone_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_460_2_WING = R"doc()doc";

static const char *__doc_GRing = R"doc(2D ring)doc";

static const char *__doc_GRing_Create = R"doc(Create the geometry)doc";

static const char *__doc_GRing_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GRing_GRing = R"doc(Default constructor)doc";

static const char *__doc_GRing_SetInnerRadius = R"doc(Set the inner radius)doc";

static const char *__doc_GRing_SetOuterRadius = R"doc(Set the outer radius)doc";

static const char *__doc_GRing_Update = R"doc(Update the object)doc";

static const char *__doc_GRing_m_Ri = R"doc(Inner radius)doc";

static const char *__doc_GRing_m_Ro = R"doc(Outer radius)doc";

static const char *__doc_GRing_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_584_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GRing_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_584_2_RIN = R"doc()doc";

static const char *__doc_GRing_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_584_2_ROUT = R"doc()doc";

static const char *__doc_GShellPrimitive = R"doc(Use this base class for shell primitives)doc";

static const char *__doc_GShellPrimitive_GShellPrimitive = R"doc(Constructor)doc";

static const char *__doc_GSlice = R"doc(Cylindrical slice)doc";

static const char *__doc_GSlice_Create = R"doc(Create the geometry)doc";

static const char *__doc_GSlice_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GSlice_GSlice = R"doc(Default constructor)doc";

static const char *__doc_GSlice_Update = R"doc(Update the object)doc";

static const char *__doc_GSlice_m_H = R"doc(Height of the slice)doc";

static const char *__doc_GSlice_m_R = R"doc(Radius of the slice)doc";

static const char *__doc_GSlice_m_w = R"doc(Angle of the slice)doc";

static const char *__doc_GSlice_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_434_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GSlice_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_434_2_ANGLE = R"doc()doc";

static const char *__doc_GSlice_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_434_2_HEIGHT = R"doc()doc";

static const char *__doc_GSlice_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_434_2_RADIUS = R"doc()doc";

static const char *__doc_GSolidArc = R"doc(A 3D Solid Arc)doc";

static const char *__doc_GSolidArc_Create = R"doc(Create the geometry)doc";

static const char *__doc_GSolidArc_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GSolidArc_GSolidArc = R"doc(Default constructor)doc";

static const char *__doc_GSolidArc_Update = R"doc(Update the object)doc";

static const char *__doc_GSolidArc_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_480_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GSolidArc_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_480_2_ARC = R"doc()doc";

static const char *__doc_GSolidArc_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_480_2_HEIGHT = R"doc()doc";

static const char *__doc_GSolidArc_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_480_2_RIN = R"doc()doc";

static const char *__doc_GSolidArc_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_480_2_ROUT = R"doc()doc";

static const char *__doc_GSphere = R"doc(Sphere)doc";

static const char *__doc_GSphereInBox = R"doc(Box with a spherical cavity)doc";

static const char *__doc_GSphereInBox_BuildGMesh = R"doc(Build the geometry mesh)doc";

static const char *__doc_GSphereInBox_Create = R"doc(Create the geometry)doc";

static const char *__doc_GSphereInBox_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GSphereInBox_GSphereInBox = R"doc(Default constructor)doc";

static const char *__doc_GSphereInBox_Update = R"doc(Update the object)doc";

static const char *__doc_GSphereInBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_342_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GSphereInBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_342_2_DEPTH = R"doc()doc";

static const char *__doc_GSphereInBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_342_2_HEIGHT = R"doc()doc";

static const char *__doc_GSphereInBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_342_2_RADIUS = R"doc()doc";

static const char *__doc_GSphereInBox_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_342_2_WIDTH = R"doc()doc";

static const char *__doc_GSphere_BuildGMesh = R"doc(Build the geometry mesh)doc";

static const char *__doc_GSphere_Create = R"doc(Create the geometry)doc";

static const char *__doc_GSphere_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GSphere_GSphere = R"doc(Default constructor)doc";

static const char *__doc_GSphere_NodeIndex = R"doc(Get node index)doc";

static const char *__doc_GSphere_Radius = R"doc(Get the radius)doc";

static const char *__doc_GSphere_SetRadius = R"doc(Set the radius)doc";

static const char *__doc_GSphere_Update = R"doc(Update the object)doc";

static const char *__doc_GSphere_m_R = R"doc(Radius of the sphere)doc";

static const char *__doc_GSphere_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_259_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GSphere_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_259_2_RADIUS = R"doc()doc";

static const char *__doc_GThinTube = R"doc(A shell tube (cylinder without capped ends))doc";

static const char *__doc_GThinTube_Create = R"doc(Create the geometry)doc";

static const char *__doc_GThinTube_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GThinTube_GThinTube = R"doc(Default constructor)doc";

static const char *__doc_GThinTube_Height = R"doc(Get the height)doc";

static const char *__doc_GThinTube_Radius = R"doc(Get the radius)doc";

static const char *__doc_GThinTube_SetHeight = R"doc(Set the height)doc";

static const char *__doc_GThinTube_SetRadius = R"doc(Set the radius)doc";

static const char *__doc_GThinTube_Update = R"doc(Update the object)doc";

static const char *__doc_GThinTube_m_R = R"doc(Radius of the tube)doc";

static const char *__doc_GThinTube_m_h = R"doc(Height of the tube)doc";

static const char *__doc_GThinTube_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_613_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GThinTube_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_613_2_H = R"doc()doc";

static const char *__doc_GThinTube_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_613_2_RAD = R"doc()doc";

static const char *__doc_GTorus = R"doc(Circular torus)doc";

static const char *__doc_GTorus_Create = R"doc(Create the geometry)doc";

static const char *__doc_GTorus_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GTorus_GTorus = R"doc(Default constructor)doc";

static const char *__doc_GTorus_Update = R"doc(Update the object)doc";

static const char *__doc_GTorus_m_R0 = R"doc(Inner radius)doc";

static const char *__doc_GTorus_m_R1 = R"doc(Outer radius)doc";

static const char *__doc_GTorus_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_290_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GTorus_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_290_2_RIN = R"doc()doc";

static const char *__doc_GTorus_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_290_2_ROUT = R"doc()doc";

static const char *__doc_GTruncatedEllipsoid = R"doc(Hollow truncated concentric ellipsoid)doc";

static const char *__doc_GTruncatedEllipsoid_BuildGMesh = R"doc(Build the geometry mesh)doc";

static const char *__doc_GTruncatedEllipsoid_Create = R"doc(Create the geometry)doc";

static const char *__doc_GTruncatedEllipsoid_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GTruncatedEllipsoid_GTruncatedEllipsoid = R"doc(Default constructor)doc";

static const char *__doc_GTruncatedEllipsoid_NodeIndex = R"doc(Get node index)doc";

static const char *__doc_GTruncatedEllipsoid_Update = R"doc(Update the object)doc";

static const char *__doc_GTruncatedEllipsoid_m_Ra = R"doc(A-axis radius)doc";

static const char *__doc_GTruncatedEllipsoid_m_Rb = R"doc(B-axis radius)doc";

static const char *__doc_GTruncatedEllipsoid_m_Rc = R"doc(C-axis radius)doc";

static const char *__doc_GTruncatedEllipsoid_m_vend = R"doc(End angle)doc";

static const char *__doc_GTruncatedEllipsoid_m_wt = R"doc(Wall thickness)doc";

static const char *__doc_GTruncatedEllipsoid_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_225_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GTruncatedEllipsoid_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_225_2_RA = R"doc()doc";

static const char *__doc_GTruncatedEllipsoid_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_225_2_RB = R"doc()doc";

static const char *__doc_GTruncatedEllipsoid_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_225_2_RC = R"doc()doc";

static const char *__doc_GTruncatedEllipsoid_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_225_2_VEND = R"doc()doc";

static const char *__doc_GTruncatedEllipsoid_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_225_2_WT = R"doc()doc";

static const char *__doc_GTube = R"doc(Tube, that is a hollow cylinder)doc";

static const char *__doc_GTube2 = R"doc(Elliptical Tube, that is a hollow cylinder)doc";

static const char *__doc_GTube2_Create = R"doc(Create the geometry)doc";

static const char *__doc_GTube2_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GTube2_GTube2 = R"doc(Default constructor)doc";

static const char *__doc_GTube2_Update = R"doc(Update the object)doc";

static const char *__doc_GTube2_m_Rix = R"doc(Inner X-axis radius)doc";

static const char *__doc_GTube2_m_Riy = R"doc(Inner Y-axis radius)doc";

static const char *__doc_GTube2_m_Rox = R"doc(Outer X-axis radius)doc";

static const char *__doc_GTube2_m_Roy = R"doc(Outer Y-axis radius)doc";

static const char *__doc_GTube2_m_h = R"doc(Height of the tube)doc";

static const char *__doc_GTube2_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_404_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GTube2_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_404_2_HEIGHT = R"doc()doc";

static const char *__doc_GTube2_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_404_2_RINX = R"doc()doc";

static const char *__doc_GTube2_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_404_2_RINY = R"doc()doc";

static const char *__doc_GTube2_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_404_2_ROUTX = R"doc()doc";

static const char *__doc_GTube2_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_404_2_ROUTY = R"doc()doc";

static const char *__doc_GTube_Create = R"doc(Create the geometry)doc";

static const char *__doc_GTube_CreateDefaultMesher = R"doc(Create the default mesher)doc";

static const char *__doc_GTube_GTube = R"doc(Default constructor)doc";

static const char *__doc_GTube_Height = R"doc(Get the height)doc";

static const char *__doc_GTube_InnerRadius = R"doc(Get the inner radius)doc";

static const char *__doc_GTube_OuterRadius = R"doc(Get the outer radius)doc";

static const char *__doc_GTube_SetHeight = R"doc(Set the height)doc";

static const char *__doc_GTube_SetInnerRadius = R"doc(Set the inner radius)doc";

static const char *__doc_GTube_SetOuterRadius = R"doc(Set the outer radius)doc";

static const char *__doc_GTube_Update = R"doc(Update the object)doc";

static const char *__doc_GTube_m_Ri = R"doc(Inner radius)doc";

static const char *__doc_GTube_m_Ro = R"doc(Outer radius)doc";

static const char *__doc_GTube_m_h = R"doc(Height of the tube)doc";

static const char *__doc_GTube_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_364_2 = R"doc(Parameter enumeration)doc";

static const char *__doc_GTube_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_364_2_HEIGHT = R"doc()doc";

static const char *__doc_GTube_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_364_2_RIN = R"doc()doc";

static const char *__doc_GTube_unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GPrimitive_h_364_2_ROUT = R"doc()doc";

static const char *__doc_IsSameFace = R"doc(Helper function to see if two faces are identical)doc";

static const char *__doc_NodeType = R"doc(Node types)doc";

static const char *__doc_NodeType_NODE_SHAPE = R"doc()doc";

static const char *__doc_NodeType_NODE_UNKNOWN = R"doc()doc";

static const char *__doc_NodeType_NODE_VERTEX = R"doc()doc";

static const char *__doc__unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GItem_h_35_1 = R"doc(State values for GItem state)doc";

static const char *__doc__unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GItem_h_35_1_GEO_ACTIVE = R"doc()doc";

static const char *__doc__unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GItem_h_35_1_GEO_REQUIRED = R"doc()doc";

static const char *__doc__unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GItem_h_35_1_GEO_SELECTED = R"doc()doc";

static const char *__doc__unnamed_enum_at_home_mherron_Projects_FEBioStudio_GeomLib_GItem_h_35_1_GEO_VISIBLE = R"doc()doc";

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

