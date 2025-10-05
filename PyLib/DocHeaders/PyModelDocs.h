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


static const char *__doc_CountBCs =
R"doc(helper function for identifying the number of BCs of a specific type
that have been defined.)doc";

static const char *__doc_CountBCsByTypeString = R"doc(Count boundary conditions by type string)doc";

static const char *__doc_CountConnectors =
R"doc(helper function for identifying the number of BCs of a specific type
that have been defined.)doc";

static const char *__doc_CountConstraints =
R"doc(helper function for identifying the number of constraints of a
specific type that have been defined.)doc";

static const char *__doc_CountICs =
R"doc(helper function for identifying the number of BCs of a specific type
that have been defined.)doc";

static const char *__doc_CountInterfaces =
R"doc(helper function for identifying the number of interfaces of a specific
type that have been defined.)doc";

static const char *__doc_CountLoads =
R"doc(helper function for identifying the number of BCs of a specific type
that have been defined.)doc";

static const char *__doc_CountMeshAdaptors = R"doc(helper function for identifying the number of mesh adaptors.)doc";

static const char *__doc_CountRigidBCs =
R"doc(helper function for identifying the number of rigid constraints of a
specific type that have been defined.)doc";

static const char *__doc_CountRigidConstraints =
R"doc(helper function for identifying the number of rigid constraints of a
specific type that have been defined.)doc";

static const char *__doc_CountRigidICs =
R"doc(helper function for identifying the number of rigid constraints of a
specific type that have been defined.)doc";

static const char *__doc_CountRigidLoads =
R"doc(helper function for identifying the number of rigid constraints of a
specific type that have been defined.)doc";

static const char *__doc_FSModel =
R"doc(The FE model stores all FE data.

It stores the geometry, the material list, and the analysis data.)doc";

static const char *__doc_FSModel_AddDataVariable = R"doc(Add data variable)doc";

static const char *__doc_FSModel_AddLoadController = R"doc(Add load controller)doc";

static const char *__doc_FSModel_AddLoadCurve = R"doc(helper function for creating load curves)doc";

static const char *__doc_FSModel_AddMaterial = R"doc(add a material to the model)doc";

static const char *__doc_FSModel_AddMeshDataGenerator = R"doc(Add mesh data generator)doc";

static const char *__doc_FSModel_AddSBM = R"doc(Add SBM)doc";

static const char *__doc_FSModel_AddSolute = R"doc(Add solute)doc";

static const char *__doc_FSModel_AddStep = R"doc(Add analysis step)doc";

static const char *__doc_FSModel_AddVariable = R"doc(Add variable)doc";

static const char *__doc_FSModel_AssignComponentToStep = R"doc(Assign component to analysis step)doc";

static const char *__doc_FSModel_AssignMaterial = R"doc(assign material to object)doc";

static const char *__doc_FSModel_AssignMaterial_2 = R"doc(assign material to part)doc";

static const char *__doc_FSModel_AssignMaterial_3 = R"doc(assign material to part)doc";

static const char *__doc_FSModel_AssignMaterial_4 = R"doc(assign material to part)doc";

static const char *__doc_FSModel_BuildMLT = R"doc(Build material lookup table)doc";

static const char *__doc_FSModel_CanDeleteMaterial = R"doc(delete a material from the model)doc";

static const char *__doc_FSModel_Clear = R"doc(clear the model)doc";

static const char *__doc_FSModel_ClearMLT = R"doc(Clear material lookup table)doc";

static const char *__doc_FSModel_ClearSBMs = R"doc(Clear all SBMs)doc";

static const char *__doc_FSModel_ClearSelections = R"doc(clear the selections of all the bc, loads, etc.)doc";

static const char *__doc_FSModel_ClearSolutes = R"doc(Clear all solutes)doc";

static const char *__doc_FSModel_ClearVariables = R"doc(Clear all variables)doc";

static const char *__doc_FSModel_CountBCs = R"doc(Count boundary conditions by type)doc";

static const char *__doc_FSModel_CountICs = R"doc(Count initial conditions by type)doc";

static const char *__doc_FSModel_CountInterfaces = R"doc(Count interfaces by type)doc";

static const char *__doc_FSModel_CountLoads = R"doc(Count loads by type)doc";

static const char *__doc_FSModel_CountMeshDataFields =
R"doc(count the mesh data fields (includes mesh data fields stored on the
mesh and the mesh data generators))doc";

static const char *__doc_FSModel_CountRigidConnectors = R"doc(Count rigid connectors by type)doc";

static const char *__doc_FSModel_CountRigidConstraints = R"doc(Count rigid constraints by type)doc";

static const char *__doc_FSModel_DataVariable = R"doc(Get data variable by index)doc";

static const char *__doc_FSModel_DataVariables = R"doc(Get number of data variables)doc";

static const char *__doc_FSModel_DeleteAllBC = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllConstraints = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllContact = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllIC = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllLoadControllers = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllLoads = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllMaterials = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllMeshAdaptors = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllMeshData = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllMeshDataGenerators = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllRigidBCs = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllRigidConnectors = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllRigidICs = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllRigidLoads = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteAllSteps = R"doc(functions to delete all components)doc";

static const char *__doc_FSModel_DeleteMaterial = R"doc(delete a material from the model)doc";

static const char *__doc_FSModel_DeleteStep = R"doc(Delete analysis step)doc";

static const char *__doc_FSModel_FSModel = R"doc(constructor/destructor)doc";

static const char *__doc_FSModel_FindDataVariable = R"doc(Find data variable by ID)doc";

static const char *__doc_FSModel_FindGroupParent = R"doc(find (and assign) the group's parent)doc";

static const char *__doc_FSModel_FindMaterial = R"doc(find a material from its name)doc";

static const char *__doc_FSModel_FindSBM = R"doc(Find SBM by name)doc";

static const char *__doc_FSModel_FindSolute = R"doc(Find solute by name)doc";

static const char *__doc_FSModel_FindStep = R"doc(Find analysis step by ID)doc";

static const char *__doc_FSModel_GetDOFIndex = R"doc(Get DOF index by name)doc";

static const char *__doc_FSModel_GetDOFName = R"doc(Get DOF name by index)doc";

static const char *__doc_FSModel_GetDOFNames = R"doc(Get DOF names for variable as string buffer)doc";

static const char *__doc_FSModel_GetDOFNames_2 = R"doc(Get DOF names for variable as vector)doc";

static const char *__doc_FSModel_GetDOFSymbol = R"doc(Get DOF symbol by index)doc";

static const char *__doc_FSModel_GetDOFSymbols = R"doc(Get DOF symbols for variable as vector)doc";

static const char *__doc_FSModel_GetEnumIndex = R"doc(Get enum index from parameter)doc";

static const char *__doc_FSModel_GetEnumKey = R"doc(Get enum key from parameter)doc";

static const char *__doc_FSModel_GetEnumValue = R"doc(Get enum value from parameter)doc";

static const char *__doc_FSModel_GetEnumValues = R"doc(Get enum values and populate list)doc";

static const char *__doc_FSModel_GetLoadController = R"doc(Get load controller by index)doc";

static const char *__doc_FSModel_GetLoadControllerFromID = R"doc(Get load controller by ID)doc";

static const char *__doc_FSModel_GetMaterial = R"doc(return the material)doc";

static const char *__doc_FSModel_GetMaterialFromID = R"doc(find a material from its ID)doc";

static const char *__doc_FSModel_GetMaterialPartList = R"doc(Get list of parts assigned to a material)doc";

static const char *__doc_FSModel_GetMeshDataGenerator = R"doc(Get mesh data generator by index)doc";

static const char *__doc_FSModel_GetModel = R"doc(return the model geometry)doc";

static const char *__doc_FSModel_GetReaction = R"doc(Get reaction by ID)doc";

static const char *__doc_FSModel_GetRigidConnectorFromID = R"doc(find a rigid connector from its ID)doc";

static const char *__doc_FSModel_GetRigidMaterialNames = R"doc(Get rigid material names as string buffer)doc";

static const char *__doc_FSModel_GetSBMData = R"doc(Get SBM data by index)doc";

static const char *__doc_FSModel_GetSBMNames = R"doc(Get SBM names as string buffer)doc";

static const char *__doc_FSModel_GetSoluteData = R"doc(Get solute data by index)doc";

static const char *__doc_FSModel_GetSoluteNames = R"doc(Get solute names as string buffer)doc";

static const char *__doc_FSModel_GetSpeciesNames = R"doc(Get species names as string buffer)doc";

static const char *__doc_FSModel_GetStep = R"doc(Get analysis step by index)doc";

static const char *__doc_FSModel_GetStepIndex = R"doc(Get index of analysis step)doc";

static const char *__doc_FSModel_GetVariable = R"doc(Get variable by name)doc";

static const char *__doc_FSModel_GetVariableIndex = R"doc(Get variable index by name)doc";

static const char *__doc_FSModel_GetVariableName = R"doc(Get variable name by index)doc";

static const char *__doc_FSModel_GetVariableNames = R"doc(Get variable names as string buffer)doc";

static const char *__doc_FSModel_InsertMaterial = R"doc(delete a material from the model)doc";

static const char *__doc_FSModel_InsertStep = R"doc(Insert analysis step at position)doc";

static const char *__doc_FSModel_Load = R"doc(load FE data from the archive)doc";

static const char *__doc_FSModel_LoadControllers = R"doc(Get number of load controllers)doc";

static const char *__doc_FSModel_LoadData = R"doc(I/O helper functions)doc";

static const char *__doc_FSModel_LoadLoadControllers = R"doc(I/O helper functions)doc";

static const char *__doc_FSModel_LoadMaterials = R"doc(I/O helper functions)doc";

static const char *__doc_FSModel_LoadMeshDataGenerators = R"doc(I/O helper functions)doc";

static const char *__doc_FSModel_LoadSBMData = R"doc(I/O helper functions)doc";

static const char *__doc_FSModel_LoadSoluteData = R"doc(I/O helper functions)doc";

static const char *__doc_FSModel_LoadSteps = R"doc(I/O helper functions)doc";

static const char *__doc_FSModel_Materials = R"doc(return materials)doc";

static const char *__doc_FSModel_MeshDataGenerators = R"doc(Get number of mesh data generators)doc";

static const char *__doc_FSModel_New = R"doc(reset model data)doc";

static const char *__doc_FSModel_Purge = R"doc(purge the model)doc";

static const char *__doc_FSModel_Reactions = R"doc(Get number of reactions)doc";

static const char *__doc_FSModel_RemoveLoadController = R"doc(Remove load controller)doc";

static const char *__doc_FSModel_RemoveMeshDataGenerator = R"doc(Remove mesh data generator)doc";

static const char *__doc_FSModel_RemoveSBM = R"doc(Remove SBM)doc";

static const char *__doc_FSModel_RemoveSolute = R"doc(Remove solute)doc";

static const char *__doc_FSModel_RemoveUnusedLoadControllers = R"doc(Remove unused load controllers from the model)doc";

static const char *__doc_FSModel_ReplaceMaterial = R"doc(replace a material in the model)doc";

static const char *__doc_FSModel_ReplaceStep = R"doc(replaces step i with newStep. Returns pointer to old step)doc";

static const char *__doc_FSModel_SBMs = R"doc(Get number of SBMs)doc";

static const char *__doc_FSModel_Save = R"doc(save the FE data to the archive)doc";

static const char *__doc_FSModel_SetEnumIndex = R"doc(Set enum by index)doc";

static const char *__doc_FSModel_SetEnumKey = R"doc(Set enum by key)doc";

static const char *__doc_FSModel_SetEnumValue = R"doc(Set enum by value)doc";

static const char *__doc_FSModel_SetSkipGeometry = R"doc(Set flag to skip geometry when loading)doc";

static const char *__doc_FSModel_Solutes = R"doc(Get number of solutes)doc";

static const char *__doc_FSModel_Steps = R"doc(Get number of analysis steps)doc";

static const char *__doc_FSModel_SwapSteps = R"doc(Swap two analysis steps)doc";

static const char *__doc_FSModel_UpdateData = R"doc(Update model data)doc";

static const char *__doc_FSModel_UpdateLoadControllerReferenceCounts = R"doc(Update load controller reference counts)doc";

static const char *__doc_FSModel_UpdateMaterialPositions = R"doc(Update material positions)doc";

static const char *__doc_FSModel_Variable = R"doc(Get variable by index)doc";

static const char *__doc_FSModel_Variables = R"doc(Get number of variables)doc";

static const char *__doc_FSModel_m_DOF = R"doc(degree of freedom list)doc";

static const char *__doc_FSModel_m_LC = R"doc(load controllers)doc";

static const char *__doc_FSModel_m_MD = R"doc(mesh data generators)doc";

static const char *__doc_FSModel_m_MLT = R"doc(material look-up table)doc";

static const char *__doc_FSModel_m_MLT_offset = R"doc(material lookup table offset)doc";

static const char *__doc_FSModel_m_SBM = R"doc(solid-bound molecule data variables)doc";

static const char *__doc_FSModel_m_Sol = R"doc(solute data variables)doc";

static const char *__doc_FSModel_m_Var = R"doc(data variables)doc";

static const char *__doc_FSModel_m_pMat = R"doc(Material list)doc";

static const char *__doc_FSModel_m_pModel = R"doc(Model geometry)doc";

static const char *__doc_FSModel_m_pStep = R"doc(Analysis data)doc";

static const char *__doc_FSModel_m_skipGeometry = R"doc(Skip geometry section when loading file)doc";

static const char *__doc_Namify = R"doc(helper function for creating a valid name from a string.)doc";

static const char *__doc_defaultBCName = R"doc(functions for creating default names)doc";

static const char *__doc_defaultConstraintName = R"doc(functions for creating default names)doc";

static const char *__doc_defaultICName = R"doc(functions for creating default names)doc";

static const char *__doc_defaultInterfaceName = R"doc(functions for creating default names)doc";

static const char *__doc_defaultLoadName = R"doc(functions for creating default names)doc";

static const char *__doc_defaultMeshAdaptorName = R"doc(functions for creating default names)doc";

static const char *__doc_defaultRigidBCName = R"doc(functions for creating default names)doc";

static const char *__doc_defaultRigidConnectorName = R"doc(functions for creating default names)doc";

static const char *__doc_defaultRigidICName = R"doc(functions for creating default names)doc";

static const char *__doc_defaultRigidLoadName = R"doc(functions for creating default names)doc";

static const char *__doc_defaultStepName = R"doc(functions for creating default names)doc";

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

