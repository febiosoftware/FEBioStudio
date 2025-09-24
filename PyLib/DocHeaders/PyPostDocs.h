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


static const char *__doc_FSMesh = R"doc(forward declaration of the mesh)doc";

static const char *__doc_Post_EDGEDATA = R"doc(Data structure for storing edge information)doc";

static const char *__doc_Post_EDGEDATA_m_ntag = R"doc(active flag)doc";

static const char *__doc_Post_EDGEDATA_m_nv = R"doc(nodal values)doc";

static const char *__doc_Post_EDGEDATA_m_val = R"doc(current value)doc";

static const char *__doc_Post_ELEMDATA = R"doc(Data structure for storing element information)doc";

static const char *__doc_Post_ELEMDATA_m_h = R"doc(shell thickness (TODO: Can we move this to the face data?))doc";

static const char *__doc_Post_ELEMDATA_m_state = R"doc(state flags)doc";

static const char *__doc_Post_ELEMDATA_m_val = R"doc(current element value)doc";

static const char *__doc_Post_FACEDATA = R"doc(Data structure for storing face information)doc";

static const char *__doc_Post_FACEDATA_m_ntag = R"doc(active flag)doc";

static const char *__doc_Post_FACEDATA_m_val = R"doc(current face value)doc";

static const char *__doc_Post_FEDataManager =
R"doc(The data manager stores the attributes (name and type) of the
different data fields.)doc";

static const char *__doc_Post_FEDataManager_AddDataField = R"doc(add a data field)doc";

static const char *__doc_Post_FEDataManager_Clear = R"doc(clear data)doc";

static const char *__doc_Post_FEDataManager_DataField = R"doc(find the data field given an index)doc";

static const char *__doc_Post_FEDataManager_DataFields = R"doc(get the nodal datafield)doc";

static const char *__doc_Post_FEDataManager_DeleteDataField = R"doc(delete a data field)doc";

static const char *__doc_Post_FEDataManager_FindDataField = R"doc(find the index of a datafield)doc";

static const char *__doc_Post_FEDataManager_FirstDataField = R"doc(get the nodal datafield)doc";

static const char *__doc_Post_FEModelDependant =
R"doc(Base class for derived classes that need to be informed when the model
changes)doc";

static const char *__doc_Post_FEModelDependant_FEModelDependant = R"doc(Default constructor)doc";

static const char *__doc_Post_FEModelDependant_Update =
R"doc(This function is called whenever the model changes When the model is
deleted it will call this function with 0)doc";

static const char *__doc_Post_FEPostModel =
R"doc(Class that describes an FEPostModel. A model consists of a mesh (in
the future there can be multiple meshes to support remeshing), a list
of materials and a list of states. The states contain the data
associated with the model)doc";

static const char *__doc_Post_FEPostModel_AddDataField = R"doc(Add a new data field)doc";

static const char *__doc_Post_FEPostModel_AddDataField_2 = R"doc(Add a new data field constrained to a set)doc";

static const char *__doc_Post_FEPostModel_AddDependant = R"doc(Add a dependant object that will be notified of model changes)doc";

static const char *__doc_Post_FEPostModel_AddLineObject = R"doc(Add a line object)doc";

static const char *__doc_Post_FEPostModel_AddMaterial = R"doc(Add a material to the model)doc";

static const char *__doc_Post_FEPostModel_AddMesh = R"doc(Add a mesh to the model)doc";

static const char *__doc_Post_FEPostModel_AddPointObject = R"doc(Add a point object)doc";

static const char *__doc_Post_FEPostModel_AddState = R"doc(Add a state to the mesh)doc";

static const char *__doc_Post_FEPostModel_AddState_2 = R"doc(Add a new state at time)doc";

static const char *__doc_Post_FEPostModel_Clear = R"doc(Clear all model data)doc";

static const char *__doc_Post_FEPostModel_ClearDependants = R"doc(Clear all dependant objects)doc";

static const char *__doc_Post_FEPostModel_ClearMaterials = R"doc(Clear all materials)doc";

static const char *__doc_Post_FEPostModel_ClearObjects = R"doc(Clear all plot objects)doc";

static const char *__doc_Post_FEPostModel_ClearStates = R"doc(Clear all states)doc";

static const char *__doc_Post_FEPostModel_CopyDataField = R"doc(Copy a data field)doc";

static const char *__doc_Post_FEPostModel_CreateCachedCopy = R"doc(Create a cached copy of a data field)doc";

static const char *__doc_Post_FEPostModel_CurrentState = R"doc(Get the current state)doc";

static const char *__doc_Post_FEPostModel_CurrentTime = R"doc(Get the current time value)doc";

static const char *__doc_Post_FEPostModel_CurrentTimeIndex = R"doc(Get the current time index)doc";

static const char *__doc_Post_FEPostModel_DeleteDataField = R"doc(Delete a data field)doc";

static const char *__doc_Post_FEPostModel_DeleteMeshes = R"doc(Delete all meshes from the model)doc";

static const char *__doc_Post_FEPostModel_DeleteState = R"doc(Remove a state from the mesh)doc";

static const char *__doc_Post_FEPostModel_EnableMaterial = R"doc(Enable or disable a material)doc";

static const char *__doc_Post_FEPostModel_EvalEdgeField = R"doc(Helper function for evaluating edge fields)doc";

static const char *__doc_Post_FEPostModel_EvalElemField = R"doc(Helper function for evaluating element fields)doc";

static const char *__doc_Post_FEPostModel_EvalFaceField = R"doc(Helper function for evaluating face fields)doc";

static const char *__doc_Post_FEPostModel_EvalNodeField = R"doc(Helper function for evaluating node fields)doc";

static const char *__doc_Post_FEPostModel_Evaluate = R"doc(Evaluate a field at a given time)doc";

static const char *__doc_Post_FEPostModel_EvaluateEdge = R"doc(Evaluate scalar functions at an edge)doc";

static const char *__doc_Post_FEPostModel_EvaluateElemTensor = R"doc(Evaluate tensor functions at an element)doc";

static const char *__doc_Post_FEPostModel_EvaluateElemVector = R"doc(Evaluate vector functions at an element)doc";

static const char *__doc_Post_FEPostModel_EvaluateElement = R"doc(Evaluate scalar functions at an element)doc";

static const char *__doc_Post_FEPostModel_EvaluateFace = R"doc(Evaluate scalar functions at a face)doc";

static const char *__doc_Post_FEPostModel_EvaluateFaceTensor = R"doc(Evaluate tensor functions at a face)doc";

static const char *__doc_Post_FEPostModel_EvaluateFaceVector = R"doc(Evaluate vector functions at a face)doc";

static const char *__doc_Post_FEPostModel_EvaluateNode = R"doc(Evaluate scalar functions at a node)doc";

static const char *__doc_Post_FEPostModel_EvaluateNode_2 = R"doc(Evaluate based on point)doc";

static const char *__doc_Post_FEPostModel_EvaluateNodeTensor = R"doc(Evaluate tensor functions at a node)doc";

static const char *__doc_Post_FEPostModel_EvaluateNodeVector = R"doc(Evaluate vector functions at a node)doc";

static const char *__doc_Post_FEPostModel_FEPostModel = R"doc(Default constructor)doc";

static const char *__doc_Post_FEPostModel_FaceNormal = R"doc(Get the normal of a face at a given time)doc";

static const char *__doc_Post_FEPostModel_GetBoundingBox = R"doc(Get the bounding box)doc";

static const char *__doc_Post_FEPostModel_GetClosestTime = R"doc(Get the state closest to time t)doc";

static const char *__doc_Post_FEPostModel_GetDataManager = R"doc(Get the data manager)doc";

static const char *__doc_Post_FEPostModel_GetDisplacementField = R"doc(Get the displacement field)doc";

static const char *__doc_Post_FEPostModel_GetElementCoords = R"doc(Get the nodal coordinates of an element at time)doc";

static const char *__doc_Post_FEPostModel_GetFEMesh = R"doc(Get a mesh by index)doc";

static const char *__doc_Post_FEPostModel_GetInstance = R"doc(Get the singleton instance)doc";

static const char *__doc_Post_FEPostModel_GetLineObject = R"doc(Get a line object by index)doc";

static const char *__doc_Post_FEPostModel_GetMaterial = R"doc(Get a particular material)doc";

static const char *__doc_Post_FEPostModel_GetMetaData = R"doc(Get the metadata)doc";

static const char *__doc_Post_FEPostModel_GetName = R"doc(Get the name of the model)doc";

static const char *__doc_Post_FEPostModel_GetPlotObject = R"doc(Get a plot object by index)doc";

static const char *__doc_Post_FEPostModel_GetPointObject = R"doc(Get a point object by index)doc";

static const char *__doc_Post_FEPostModel_GetState = R"doc(Retrieve pointer to a state)doc";

static const char *__doc_Post_FEPostModel_GetStates = R"doc(Get the nr of states)doc";

static const char *__doc_Post_FEPostModel_GetTimeValue = R"doc(Get the time value of state n)doc";

static const char *__doc_Post_FEPostModel_GetTitle = R"doc(Get the title of the model)doc";

static const char *__doc_Post_FEPostModel_InsertState = R"doc(Insert a state at a particular time)doc";

static const char *__doc_Post_FEPostModel_InterpolateStateData = R"doc(Interpolate data between its neighbors)doc";

static const char *__doc_Post_FEPostModel_IsValidFieldCode = R"doc(Check if the field code is valid for the given state)doc";

static const char *__doc_Post_FEPostModel_LineObject = R"doc(Line object for rendering line segments)doc";

static const char *__doc_Post_FEPostModel_LineObject_LineObject = R"doc(Default constructor)doc";

static const char *__doc_Post_FEPostModel_LineObject_m_r01 = R"doc(Original first endpoint of the line)doc";

static const char *__doc_Post_FEPostModel_LineObject_m_r02 = R"doc(Original second endpoint of the line)doc";

static const char *__doc_Post_FEPostModel_LineObject_m_r1 = R"doc(First endpoint of the line)doc";

static const char *__doc_Post_FEPostModel_LineObject_m_r2 = R"doc(Second endpoint of the line)doc";

static const char *__doc_Post_FEPostModel_LineObjects = R"doc(Get the number of line objects)doc";

static const char *__doc_Post_FEPostModel_Materials = R"doc(Return number of materials)doc";

static const char *__doc_Post_FEPostModel_Merge = R"doc(Merge another FEPostModel into this one)doc";

static const char *__doc_Post_FEPostModel_Meshes = R"doc(Get the number of meshes in the model)doc";

static const char *__doc_Post_FEPostModel_NodePosition = R"doc(Get the position of a node at a given time)doc";

static const char *__doc_Post_FEPostModel_NodePosition_2 = R"doc(Get the position of a node based on a reference point at a given time)doc";

static const char *__doc_Post_FEPostModel_PlotObject = R"doc(Base class for plot objects that can be rendered in the scene)doc";

static const char *__doc_Post_FEPostModel_PlotObject_Color = R"doc(Get the color of the plot object)doc";

static const char *__doc_Post_FEPostModel_PlotObject_FindObjectDataIndex = R"doc(Find the index of object data with the given name)doc";

static const char *__doc_Post_FEPostModel_PlotObject_PlotObject = R"doc(Default constructor)doc";

static const char *__doc_Post_FEPostModel_PlotObject_Scale = R"doc(Get the scale factor for the plot object)doc";

static const char *__doc_Post_FEPostModel_PlotObject_SetColor = R"doc(Set the color of the plot object)doc";

static const char *__doc_Post_FEPostModel_PlotObject_m_data = R"doc(Vector of data associated with the plot object)doc";

static const char *__doc_Post_FEPostModel_PlotObject_m_id = R"doc(Unique identifier for the plot object)doc";

static const char *__doc_Post_FEPostModel_PlotObject_m_pos = R"doc(Position of the plot object)doc";

static const char *__doc_Post_FEPostModel_PlotObject_m_rot = R"doc(Rotation of the plot object)doc";

static const char *__doc_Post_FEPostModel_PlotObject_m_tag = R"doc(Tag for the plot object)doc";

static const char *__doc_Post_FEPostModel_PlotObjects = R"doc(Get the number of plot objects)doc";

static const char *__doc_Post_FEPostModel_PointObject = R"doc(Point object for rendering single points)doc";

static const char *__doc_Post_FEPostModel_PointObject_PointObject = R"doc(Default constructor)doc";

static const char *__doc_Post_FEPostModel_PointObject_m_rt = R"doc(Position vector for the point)doc";

static const char *__doc_Post_FEPostModel_PointObjects = R"doc(Get the number of point objects)doc";

static const char *__doc_Post_FEPostModel_ProjectToMesh = R"doc(Project a point onto the mesh)doc";

static const char *__doc_Post_FEPostModel_RemoveDependant = R"doc(Remove a dependant object)doc";

static const char *__doc_Post_FEPostModel_ResetAllStates =
R"doc(Reset all the states so any update will force the state to be
evaluated)doc";

static const char *__doc_Post_FEPostModel_SetCurrentTimeIndex = R"doc(Set the current time index)doc";

static const char *__doc_Post_FEPostModel_SetDisplacementField = R"doc(Set the displacement field)doc";

static const char *__doc_Post_FEPostModel_SetInstance = R"doc(Set the singleton instance)doc";

static const char *__doc_Post_FEPostModel_SetName = R"doc(Set the name of the model)doc";

static const char *__doc_Post_FEPostModel_SetTimeValue = R"doc(Set the active state closest to time t)doc";

static const char *__doc_Post_FEPostModel_SetTitle = R"doc(Set the title of the model)doc";

static const char *__doc_Post_FEPostModel_UpdateBoundingBox = R"doc(Update the bounding box)doc";

static const char *__doc_Post_FEPostModel_UpdateDependants = R"doc(Update all dependant objects)doc";

static const char *__doc_Post_FEPostModel_UpdateMeshState = R"doc(Enable or disable mesh items based on material's state)doc";

static const char *__doc_Post_FEPostModel_UpdateMeshState_2 = R"doc(Update mesh state at a specific time)doc";

static const char *__doc_Post_FEPostModel_getDataString = R"doc(Get the field variable name)doc";

static const char *__doc_Post_FEPostModel_m_Dependants = R"doc(Vector of dependant objects)doc";

static const char *__doc_Post_FEPostModel_m_Lines = R"doc(Vector of line objects)doc";

static const char *__doc_Post_FEPostModel_m_Mat = R"doc(Array of materials)doc";

static const char *__doc_Post_FEPostModel_m_Points = R"doc(Vector of point objects)doc";

static const char *__doc_Post_FEPostModel_m_RefState = R"doc(Reference state for meshes)doc";

static const char *__doc_Post_FEPostModel_m_State = R"doc(Array of pointers to FE-state structures)doc";

static const char *__doc_Post_FEPostModel_m_bbox = R"doc(Bounding box of mesh)doc";

static const char *__doc_Post_FEPostModel_m_fTime = R"doc(Current time value)doc";

static const char *__doc_Post_FEPostModel_m_mesh = R"doc(The list of meshes)doc";

static const char *__doc_Post_FEPostModel_m_meta = R"doc(Metadata for the model)doc";

static const char *__doc_Post_FEPostModel_m_nTime = R"doc(Active time step)doc";

static const char *__doc_Post_FEPostModel_m_name = R"doc(Name (as displayed in model viewer))doc";

static const char *__doc_Post_FEPostModel_m_ndisp = R"doc(Vector field defining the displacement)doc";

static const char *__doc_Post_FEPostModel_m_pDM = R"doc(The Data Manager)doc";

static const char *__doc_Post_FEPostModel_m_title = R"doc(Title of project)doc";

static const char *__doc_Post_FERefState = R"doc(Class for storing reference state information)doc";

static const char *__doc_Post_FERefState_FERefState = R"doc(Constructor)doc";

static const char *__doc_Post_FERefState_m_Node = R"doc(nodal data in reference state)doc";

static const char *__doc_Post_FEState =
R"doc(This class stores a state of a model. A state is defined by data for
each of the field variables associated by the model.)doc";

static const char *__doc_Post_FEState_AddPointObjectData = R"doc(Add point object data)doc";

static const char *__doc_Post_FEState_FEState = R"doc(Constructor with time, model, and mesh)doc";

static const char *__doc_Post_FEState_FEState_2 = R"doc(Constructor with time, model, and existing state)doc";

static const char *__doc_Post_FEState_GetFEMesh = R"doc(Get the finite element mesh)doc";

static const char *__doc_Post_FEState_GetFSModel = R"doc(Get the FEPostModel)doc";

static const char *__doc_Post_FEState_GetID = R"doc(Get the ID of this state)doc";

static const char *__doc_Post_FEState_GetObjectData = R"doc(Get object data at index n)doc";

static const char *__doc_Post_FEState_NodePosition = R"doc(Get node position at given node index)doc";

static const char *__doc_Post_FEState_NodeRefPosition = R"doc(Get node reference position at given node index)doc";

static const char *__doc_Post_FEState_RebuildData = R"doc(Rebuild all data structures)doc";

static const char *__doc_Post_FEState_SetFEMesh = R"doc(Set the finite element mesh)doc";

static const char *__doc_Post_FEState_SetID = R"doc(Set the ID of this state)doc";

static const char *__doc_Post_FEState_m_Data = R"doc(mesh data list)doc";

static const char *__doc_Post_FEState_m_EDGE = R"doc(edge data)doc";

static const char *__doc_Post_FEState_m_ELEM = R"doc(element data)doc";

static const char *__doc_Post_FEState_m_EdgeData = R"doc(edge data array)doc";

static const char *__doc_Post_FEState_m_ElemData = R"doc(element data array)doc";

static const char *__doc_Post_FEState_m_FACE = R"doc(face data)doc";

static const char *__doc_Post_FEState_m_FaceData = R"doc(face data array)doc";

static const char *__doc_Post_FEState_m_NODE = R"doc(nodal data)doc";

static const char *__doc_Post_FEState_m_bsmooth = R"doc(smoothing flag)doc";

static const char *__doc_Post_FEState_m_fem = R"doc(model this state belongs to)doc";

static const char *__doc_Post_FEState_m_id = R"doc(index in state array of FEPostModel)doc";

static const char *__doc_Post_FEState_m_mesh = R"doc(The mesh this state uses)doc";

static const char *__doc_Post_FEState_m_nField = R"doc(the field whos values are contained in m_pval)doc";

static const char *__doc_Post_FEState_m_objLn = R"doc(object line data)doc";

static const char *__doc_Post_FEState_m_objPt = R"doc(object point data)doc";

static const char *__doc_Post_FEState_m_ref = R"doc(the reference state for this state)doc";

static const char *__doc_Post_FEState_m_status = R"doc(status flag)doc";

static const char *__doc_Post_FEState_m_time = R"doc(time value)doc";

static const char *__doc_Post_IntegrateEdges = R"doc(Integrate values over specified edges)doc";

static const char *__doc_Post_IntegrateElems =
R"doc(This function calculates the integral over a volume. Note that if the
volume is not hexahedral, then we calculate the integral from a
degenerate hex.)doc";

static const char *__doc_Post_IntegrateFaces =
R"doc(This function calculates the integral over a surface. Note that if the
surface is triangular, then we calculate the integral from a
degenerate quad.)doc";

static const char *__doc_Post_IntegrateNodes = R"doc(Integrate values over specified nodes)doc";

static const char *__doc_Post_IntegrateReferenceElems = R"doc(Integrate values over reference elements)doc";

static const char *__doc_Post_IntegrateReferenceFaces = R"doc(Integrate values over reference faces)doc";

static const char *__doc_Post_IntegrateSurfaceNormal = R"doc(Integrates the surface normal scaled by the data field)doc";

static const char *__doc_Post_MetaData = R"doc(Class for storing metadata information about the model)doc";

static const char *__doc_Post_MetaData_author = R"doc(author of model file)doc";

static const char *__doc_Post_MetaData_software = R"doc(software that generated the model file)doc";

static const char *__doc_Post_ModelDataField = R"doc(Base class describing a data field)doc";

static const char *__doc_Post_ModelDataField_Clone = R"doc(Create a copy)doc";

static const char *__doc_Post_ModelDataField_CreateData = R"doc(FEMeshData constructor)doc";

static const char *__doc_Post_ModelDataField_GetFieldID = R"doc(get the field ID)doc";

static const char *__doc_Post_ModelDataField_SetFieldID = R"doc(Set the field ID)doc";

static const char *__doc_Post_ModelDataField_Type = R"doc(type identifier)doc";

static const char *__doc_Post_ModelDataField_TypeStr = R"doc(type string)doc";

static const char *__doc_Post_ModelDataField_componentName = R"doc(return the name of a component)doc";

static const char *__doc_Post_ModelDataField_components = R"doc(number of components)doc";

static const char *__doc_Post_ModelDataField_dataComponents = R"doc(number of actual data components)doc";

static const char *__doc_Post_ModelDataField_m_arrayNames = R"doc((optional) names of array components)doc";

static const char *__doc_Post_ModelDataField_m_arraySize = R"doc(data size for arrays)doc";

static const char *__doc_Post_ModelDataField_m_flag = R"doc(flags)doc";

static const char *__doc_Post_ModelDataField_m_nclass = R"doc(data class)doc";

static const char *__doc_Post_ModelDataField_m_nfield = R"doc(field ID)doc";

static const char *__doc_Post_ModelDataField_m_nfmt = R"doc(data format)doc";

static const char *__doc_Post_ModelDataField_m_ntype = R"doc(data type)doc";

static const char *__doc_Post_ModelDataField_m_units = R"doc(units)doc";

static const char *__doc_Post_NODEDATA = R"doc(Data structure for storing nodal information)doc";

static const char *__doc_Post_NODEDATA_m_ntag = R"doc(active flag)doc";

static const char *__doc_Post_NODEDATA_m_rt = R"doc(nodal position determined by displacement map)doc";

static const char *__doc_Post_NODEDATA_m_val = R"doc(current nodal value)doc";

static const char *__doc_Post_OBJECTDATA = R"doc(Base class for object data)doc";

static const char *__doc_Post_OBJECTDATA_OBJECTDATA = R"doc(Constructor)doc";

static const char *__doc_Post_OBJECTDATA_data = R"doc(pointer to object data)doc";

static const char *__doc_Post_OBJECTDATA_pos = R"doc(position)doc";

static const char *__doc_Post_OBJECTDATA_rot = R"doc(rotation)doc";

static const char *__doc_Post_OBJ_LINE_DATA = R"doc(Object line data structure)doc";

static const char *__doc_Post_OBJ_LINE_DATA_m_r1 = R"doc(first point of line)doc";

static const char *__doc_Post_OBJ_LINE_DATA_m_r2 = R"doc(second point of line)doc";

static const char *__doc_Post_OBJ_POINT_DATA = R"doc(Object point data class)doc";

static const char *__doc_Post_OBJ_POINT_DATA_m_rt = R"doc(point position)doc";

static const char *__doc_Post_ObjectData = R"doc(Class for managing object data arrays)doc";

static const char *__doc_Post_ObjectData_ObjectData = R"doc(Constructor)doc";

static const char *__doc_Post_ObjectData_append = R"doc(Append n bytes to the data array)doc";

static const char *__doc_Post_ObjectData_data = R"doc(data array)doc";

static const char *__doc_Post_ObjectData_get = R"doc(Get data value at index n as type T)doc";

static const char *__doc_Post_ObjectData_nsize = R"doc(size of data array)doc";

static const char *__doc_Post_ObjectData_off = R"doc(offset array)doc";

static const char *__doc_Post_ObjectData_push_back = R"doc(Add a float value to the data array)doc";

static const char *__doc_Post_ObjectData_push_back_2 = R"doc(Add a vec3f value to the data array)doc";

static const char *__doc_Post_ObjectData_push_back_3 = R"doc(Add a mat3f value to the data array)doc";

static const char *__doc_Post_ObjectData_set = R"doc(Set data value at index n to value v of type T)doc";

static const char *__doc_Post_StatusFlags = R"doc(Status flags for mesh items)doc";

static const char *__doc_Post_StatusFlags_ACTIVE = R"doc(item has data)doc";

static const char *__doc_Post_StatusFlags_VISIBLE = R"doc(item is visible (i.e. not eroded))doc";

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

