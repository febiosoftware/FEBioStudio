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


static const char *__doc_GLColor =
R"doc(OpenGL color class with RGBA components stored as 8-bit unsigned
integers)doc";

static const char *__doc_GLColor_Black = R"doc(Create black color (0, 0, 0))doc";

static const char *__doc_GLColor_Blue = R"doc(Create blue color (0, 0, 255))doc";

static const char *__doc_GLColor_FromRGBf = R"doc(Create color from normalized float RGB values (0.0-1.0))doc";

static const char *__doc_GLColor_GLColor = R"doc(Default constructor - creates black color with full opacity)doc";

static const char *__doc_GLColor_GLColor_2 = R"doc(Constructor with RGB components and optional alpha)doc";

static const char *__doc_GLColor_GLColor_3 = R"doc(Constructor from packed 32-bit unsigned integer (RGBA format))doc";

static const char *__doc_GLColor_Green = R"doc(Create green color (0, 255, 0))doc";

static const char *__doc_GLColor_Red = R"doc(Create red color (255, 0, 0))doc";

static const char *__doc_GLColor_White = R"doc(Create white color (255, 255, 255))doc";

static const char *__doc_GLColor_Yellow = R"doc(Create yellow color (255, 255, 0))doc";

static const char *__doc_GLColor_a = R"doc(Alpha component (0-255))doc";

static const char *__doc_GLColor_b = R"doc(Alpha component (0-255))doc";

static const char *__doc_GLColor_g = R"doc(Alpha component (0-255))doc";

static const char *__doc_GLColor_operator_add = R"doc(Color addition operator - adds RGB components)doc";

static const char *__doc_GLColor_operator_mul = R"doc(Scalar multiplication operator - scales RGB components by factor)doc";

static const char *__doc_GLColor_r = R"doc(Alpha component (0-255))doc";

static const char *__doc_GLColor_toDouble = R"doc(Convert color components to normalized double array (0.0-1.0))doc";

static const char *__doc_GLColor_toFloat = R"doc(Convert color components to normalized float array (0.0-1.0))doc";

static const char *__doc_GLColor_to_uint = R"doc(Convert color to packed 32-bit unsigned integer)doc";

static const char *__doc_GlobalToLocal = R"doc(convert from global to local coordinates)doc";

static const char *__doc_GlobalToLocalNormal = R"doc(get a normal-like vector from global to local)doc";

static const char *__doc_LocalToGlobal = R"doc(convert from local to global coordinates)doc";

static const char *__doc_LocalToGlobalNormal = R"doc(get a normal-like vector from global to local)doc";

static const char *__doc_Rotate = R"doc(rotate around the center rc)doc";

static const char *__doc_Rotate_2 = R"doc(Rotate angle w around an axis defined by the position vectors a, b.)doc";

static const char *__doc_Scale = R"doc(scale an object)doc";

static const char *__doc_Transform =
R"doc(Class that defines an affine transformation (scale, rotate,
translate). This currently applies the transformation as follows: 1.
scale : the scale is applied in the local coordinate system 2. rotate:
rotation from local to global coordinates 3. translate: translate to a
global position)doc";

static const char *__doc_Transform_Apply = R"doc(apply transformation)doc";

static const char *__doc_Transform_GetPosition = R"doc(get position of object)doc";

static const char *__doc_Transform_GetRotation = R"doc(get orientation)doc";

static const char *__doc_Transform_GetRotationInverse = R"doc(get inverse of rotation)doc";

static const char *__doc_Transform_GetScale = R"doc(get scale of the object)doc";

static const char *__doc_Transform_GlobalToLocal = R"doc(convert from global to local coordinates)doc";

static const char *__doc_Transform_GlobalToLocalNormal = R"doc(get a normal-like vector from global to local)doc";

static const char *__doc_Transform_LocalToGlobal = R"doc(convert from local to global coordinates)doc";

static const char *__doc_Transform_LocalToGlobalNormal = R"doc(get a normal-like vector from global to local)doc";

static const char *__doc_Transform_Reset = R"doc(Reset the transform)doc";

static const char *__doc_Transform_Rotate = R"doc(rotate around the center rc)doc";

static const char *__doc_Transform_Rotate_2 = R"doc(Rotate angle w around an axis defined by the position vectors a, b.)doc";

static const char *__doc_Transform_Scale = R"doc(scale an object)doc";

static const char *__doc_Transform_SetPosition = R"doc(set the position (or translation))doc";

static const char *__doc_Transform_SetRotation = R"doc(set the rotation quaternion)doc";

static const char *__doc_Transform_SetRotation_2 = R"doc(set the rotation vector (uses degrees))doc";

static const char *__doc_Transform_SetRotation_3 =
R"doc(set rotation via Euler angles Tait-Bryan (Z,Y,X) convention (in
degrees))doc";

static const char *__doc_Transform_SetScale = R"doc(set the scale factors)doc";

static const char *__doc_Transform_SetScale_2 = R"doc(set the scale of the object)doc";

static const char *__doc_Transform_Transform = R"doc()doc";

static const char *__doc_Transform_Translate = R"doc(translate the transform)doc";

static const char *__doc_Transform_m_pos = R"doc(scale factors)doc";

static const char *__doc_Transform_m_rot = R"doc(translation (global space))doc";

static const char *__doc_Transform_m_roti = R"doc(rotation)doc";

static const char *__doc_Transform_m_scl = R"doc()doc";

static const char *__doc_Transform_operator_eq = R"doc(comparison)doc";

static const char *__doc_Translate = R"doc(translate the transform)doc";

static const char *__doc_euler2rot = R"doc(Convert euler-angles to a rotation matrix)doc";

static const char *__doc_operator_eq = R"doc(comparison)doc";

static const char *__doc_operator_mul = R"doc(Scalar multiplication operator (scalar * quaternion))doc";

static const char *__doc_quat2euler = R"doc(Extract euler angles from a quaternion)doc";

static const char *__doc_quatd = R"doc(This class implements a quaternion.)doc";

static const char *__doc_quatd_Conjugate = R"doc(Return the conjugate of the quaternion)doc";

static const char *__doc_quatd_DotProduct = R"doc(Calculate dot product with another quaternion)doc";

static const char *__doc_quatd_GetAngle = R"doc(Get the rotation angle in radians)doc";

static const char *__doc_quatd_GetEuler = R"doc(Get XYZ Euler angles from quaternion (roll, pitch, yaw in radians))doc";

static const char *__doc_quatd_GetRotationVector = R"doc(Get the rotation vector (axis scaled by angle))doc";

static const char *__doc_quatd_GetVector = R"doc(Get the normalized rotation axis vector)doc";

static const char *__doc_quatd_Inverse = R"doc(Return the inverse of the quaternion)doc";

static const char *__doc_quatd_MakeUnit = R"doc(Normalize the quaternion to unit length)doc";

static const char *__doc_quatd_Norm = R"doc(Return the norm (squared magnitude) of the quaternion)doc";

static const char *__doc_quatd_RotateVector =
R"doc(Rotate a vector using this quaternion (in-place, for unit quaternions
only))doc";

static const char *__doc_quatd_RotateVectorP = R"doc(Rotate vector using raw pointer arrays)doc";

static const char *__doc_quatd_RotationMatrix = R"doc(Convert a quaternion to a rotation matrix)doc";

static const char *__doc_quatd_SetEuler = R"doc(Set quaternion from XYZ Euler angles (roll, pitch, yaw in radians))doc";

static const char *__doc_quatd_dot = R"doc(Calculate dot product between two quaternions)doc";

static const char *__doc_quatd_lerp = R"doc(Linear interpolation between two quaternions)doc";

static const char *__doc_quatd_operator_add = R"doc(Quaternion addition operator)doc";

static const char *__doc_quatd_operator_div = R"doc(Scalar division operator)doc";

static const char *__doc_quatd_operator_eq = R"doc(Equality comparison operator)doc";

static const char *__doc_quatd_operator_iadd = R"doc(Quaternion addition assignment operator)doc";

static const char *__doc_quatd_operator_idiv = R"doc(Scalar division assignment operator)doc";

static const char *__doc_quatd_operator_imul = R"doc(Quaternion multiplication assignment operator)doc";

static const char *__doc_quatd_operator_isub = R"doc(Quaternion subtraction assignment operator)doc";

static const char *__doc_quatd_operator_mul = R"doc(Quaternion multiplication operator)doc";

static const char *__doc_quatd_operator_mul_2 = R"doc(Scalar multiplication operator)doc";

static const char *__doc_quatd_operator_mul_3 = R"doc(Vector rotation operator (for unit quaternions only))doc";

static const char *__doc_quatd_operator_ne = R"doc(Inequality comparison operator)doc";

static const char *__doc_quatd_operator_sub = R"doc(Unary negation operator)doc";

static const char *__doc_quatd_operator_sub_2 = R"doc(Quaternion subtraction operator)doc";

static const char *__doc_quatd_quatd = R"doc(Default constructor - creates identity quaternion (0, 0, 0, 1))doc";

static const char *__doc_quatd_quatd_2 = R"doc(Constructor from rotation angle and axis vector)doc";

static const char *__doc_quatd_quatd_3 =
R"doc(Constructor from rotation vector (magnitude is angle, direction is
axis))doc";

static const char *__doc_quatd_quatd_4 = R"doc(Constructor from two vectors - creates rotation from v1 to v2)doc";

static const char *__doc_quatd_quatd_5 = R"doc(Constructor from four components)doc";

static const char *__doc_quatd_quatd_6 = R"doc(Constructor from rotation matrix)doc";

static const char *__doc_quatd_slerp = R"doc(Spherical linear interpolation between two quaternions)doc";

static const char *__doc_quatd_w = R"doc(X component of quaternion)doc";

static const char *__doc_quatd_x = R"doc(X component of quaternion)doc";

static const char *__doc_quatd_y = R"doc(X component of quaternion)doc";

static const char *__doc_quatd_z = R"doc(X component of quaternion)doc";

static const char *__doc_rot2euler = R"doc(Convert a rotation matrix to euler angles)doc";

static const char *__doc_to_vec3d = R"doc(Convert vec3f to vec3d)doc";

static const char *__doc_to_vec3f = R"doc(Convert vec3d to vec3f)doc";

static const char *__doc_vec3d = R"doc(3D vector class with double precision components)doc";

static const char *__doc_vec3d_Length = R"doc(Return the length of the vector (FEBio Studio compatibility))doc";

static const char *__doc_vec3d_Normalize = R"doc(Normalize the vector in place (FEBio Studio compatibility))doc";

static const char *__doc_vec3d_Normalized = R"doc(Return a normalized copy of this vector (FEBio Studio compatibility))doc";

static const char *__doc_vec3d_SqrLength = R"doc(Return the squared length of the vector (FEBio Studio compatibility))doc";

static const char *__doc_vec3d_norm = R"doc(Return the length (magnitude) of the vector)doc";

static const char *__doc_vec3d_norm2 = R"doc(Return the squared length of the vector)doc";

static const char *__doc_vec3d_normalized = R"doc(Return a normalized copy of this vector)doc";

static const char *__doc_vec3d_operator = R"doc(Cross product operator)doc";

static const char *__doc_vec3d_operator_add = R"doc(Vector addition operator)doc";

static const char *__doc_vec3d_operator_call = R"doc(Component access operator (non-const))doc";

static const char *__doc_vec3d_operator_call_2 = R"doc(Component access operator (const))doc";

static const char *__doc_vec3d_operator_div = R"doc(Scalar division operator)doc";

static const char *__doc_vec3d_operator_eq = R"doc(Equality comparison operator)doc";

static const char *__doc_vec3d_operator_iadd = R"doc(Vector addition assignment operator)doc";

static const char *__doc_vec3d_operator_idiv = R"doc(Scalar division assignment operator)doc";

static const char *__doc_vec3d_operator_imul = R"doc(Scalar multiplication assignment operator)doc";

static const char *__doc_vec3d_operator_isub = R"doc(Vector subtraction assignment operator)doc";

static const char *__doc_vec3d_operator_mul = R"doc(Scalar multiplication operator)doc";

static const char *__doc_vec3d_operator_mul_2 = R"doc(Dot product operator)doc";

static const char *__doc_vec3d_operator_sub = R"doc(Vector subtraction operator)doc";

static const char *__doc_vec3d_operator_sub_2 = R"doc(Unary negation operator)doc";

static const char *__doc_vec3d_unit = R"doc(Normalize the vector in place and return original length)doc";

static const char *__doc_vec3d_vec3d = R"doc(Default constructor - initializes vector to (0, 0, 0))doc";

static const char *__doc_vec3d_vec3d_2 =
R"doc(Constructor with single value - initializes all components to the same
value)doc";

static const char *__doc_vec3d_vec3d_3 = R"doc(Constructor with three components)doc";

static const char *__doc_vec3d_vec3d_4 = R"doc(Constructor from 2D vector - sets z component to 0)doc";

static const char *__doc_vec3d_x = R"doc(X component of the vector)doc";

static const char *__doc_vec3d_y = R"doc(X component of the vector)doc";

static const char *__doc_vec3d_z = R"doc(X component of the vector)doc";

static const char *__doc_vec3f = R"doc(3D vector class with single precision (float) components)doc";

static const char *__doc_vec3f_Length = R"doc(Return the length (magnitude) of the vector)doc";

static const char *__doc_vec3f_Normalize = R"doc(Normalize the vector in place)doc";

static const char *__doc_vec3f_SqrLength = R"doc(Return the squared length of the vector)doc";

static const char *__doc_vec3f_operator = R"doc(Cross product operator)doc";

static const char *__doc_vec3f_operator_add = R"doc(Vector addition operator)doc";

static const char *__doc_vec3f_operator_div = R"doc(Scalar division operator)doc";

static const char *__doc_vec3f_operator_iadd = R"doc(Vector addition assignment operator)doc";

static const char *__doc_vec3f_operator_idiv = R"doc(Float division assignment operator)doc";

static const char *__doc_vec3f_operator_idiv_2 = R"doc(Integer division assignment operator)doc";

static const char *__doc_vec3f_operator_imul = R"doc(Float multiplication assignment operator)doc";

static const char *__doc_vec3f_operator_isub = R"doc(Vector subtraction assignment operator)doc";

static const char *__doc_vec3f_operator_mul = R"doc(Dot product operator)doc";

static const char *__doc_vec3f_operator_mul_2 = R"doc(Scalar multiplication operator)doc";

static const char *__doc_vec3f_operator_sub = R"doc(Vector subtraction operator)doc";

static const char *__doc_vec3f_operator_sub_2 = R"doc(Unary negation operator)doc";

static const char *__doc_vec3f_vec3f = R"doc(Default constructor - initializes vector to (0, 0, 0))doc";

static const char *__doc_vec3f_vec3f_2 = R"doc(Constructor with three float components)doc";

static const char *__doc_vec3f_x = R"doc(X component of the vector)doc";

static const char *__doc_vec3f_y = R"doc(X component of the vector)doc";

static const char *__doc_vec3f_z = R"doc(X component of the vector)doc";

static const char *__doc_vec3i = R"doc(3D vector class with integer components)doc";

static const char *__doc_vec3i_vec3i = R"doc(Default constructor - initializes vector to (0, 0, 0))doc";

static const char *__doc_vec3i_vec3i_2 = R"doc(Constructor with three integer components)doc";

static const char *__doc_vec3i_x = R"doc(X component of the vector)doc";

static const char *__doc_vec3i_y = R"doc(X component of the vector)doc";

static const char *__doc_vec3i_z = R"doc(X component of the vector)doc";

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

