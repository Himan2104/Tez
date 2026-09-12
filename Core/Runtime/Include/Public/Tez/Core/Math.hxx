#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

///
/// @brief Header re-exposing engine-friendly, PascalCase aliases and wrappers over glm's API,
///        so engine code reads in the project's naming conventions instead of glm's.
///
namespace Tez
{

///
/// @brief 2-component single-precision float vector.
///
using Vec2f = glm::vec2;
///
/// @brief 3-component single-precision float vector.
///
using Vec3f = glm::vec3;
///
/// @brief 4-component single-precision float vector.
///
using Vec4f = glm::vec4;

///
/// @brief 2-component signed integer vector.
///
using Vec2i = glm::ivec2;
///
/// @brief 3-component signed integer vector.
///
using Vec3i = glm::ivec3;
///
/// @brief 4-component signed integer vector.
///
using Vec4i = glm::ivec4;

///
/// @brief 2-component unsigned integer vector.
///
using Vec2u = glm::uvec2;
///
/// @brief 3-component unsigned integer vector.
///
using Vec3u = glm::uvec3;
///
/// @brief 4-component unsigned integer vector.
///
using Vec4u = glm::uvec4;

///
/// @brief 2-component double-precision float vector.
///
using Vec2d = glm::dvec2;
///
/// @brief 3-component double-precision float vector.
///
using Vec3d = glm::dvec3;
///
/// @brief 4-component double-precision float vector.
///
using Vec4d = glm::dvec4;

///
/// @brief 2-component boolean vector.
///
using Vec2b = glm::bvec2;
///
/// @brief 3-component boolean vector.
///
using Vec3b = glm::bvec3;
///
/// @brief 4-component boolean vector.
///
using Vec4b = glm::bvec4;

///
/// @brief 2x2 single-precision float matrix.
///
using Mat2f = glm::mat2;
///
/// @brief 3x3 single-precision float matrix.
///
using Mat3f = glm::mat3;
///
/// @brief 4x4 single-precision float matrix.
///
using Mat4f = glm::mat4;
///
/// @brief 2x2 double-precision float matrix.
///
using Mat2d = glm::dmat2;
///
/// @brief 3x3 double-precision float matrix.
///
using Mat3d = glm::dmat3;
///
/// @brief 4x4 double-precision float matrix.
///
using Mat4d = glm::dmat4;

///
/// @brief 2x3 single-precision float matrix.
///
using Mat2x3 = glm::mat2x3;
///
/// @brief 3x2 single-precision float matrix.
///
using Mat3x2 = glm::mat3x2;
///
/// @brief 2x4 single-precision float matrix.
///
using Mat2x4 = glm::mat2x4;
///
/// @brief 4x2 single-precision float matrix.
///
using Mat4x2 = glm::mat4x2;
///
/// @brief 3x4 single-precision float matrix.
///
using Mat3x4 = glm::mat3x4;
///
/// @brief 4x3 single-precision float matrix.
///
using Mat4x3 = glm::mat4x3;

///
/// @brief Single-precision quaternion (rotation).
///
using Quatf = glm::quat;
///
/// @brief Double-precision quaternion (rotation).
///
using Quatd = glm::dquat;

///
/// @brief Declares a PascalCase inline wrapper around a glm free function, forwarding all
///        arguments by perfect forwarding and mirroring the callee's return type and
///        noexcept specification.
/// @param aliasName The PascalCase name to expose (e.g. Degrees).
/// @param funcName The glm function being wrapped (e.g. degrees).
///
#define TEZ_WRAP_GLM(aliasName, funcName)                       \
    template <typename... Args>                                 \
    constexpr auto aliasName(Args&&... args) noexcept(          \
        noexcept(glm::funcName(std::forward<Args>(args)...)))   \
        -> decltype(glm::funcName(std::forward<Args>(args)...)) \
    { return glm::funcName(std::forward<Args>(args)...); }

// Angle/unit conversion
TEZ_WRAP_GLM(Radians, radians)
TEZ_WRAP_GLM(Degrees, degrees)

// Trigonometric
TEZ_WRAP_GLM(Sin, sin)
TEZ_WRAP_GLM(Cos, cos)
TEZ_WRAP_GLM(Tan, tan)
TEZ_WRAP_GLM(Asin, asin)
TEZ_WRAP_GLM(Acos, acos)
TEZ_WRAP_GLM(Atan, atan)
TEZ_WRAP_GLM(Sinh, sinh)
TEZ_WRAP_GLM(Cosh, cosh)
TEZ_WRAP_GLM(Tanh, tanh)

// Exponential / logarithmic
TEZ_WRAP_GLM(Pow, pow)
TEZ_WRAP_GLM(Exp, exp)
TEZ_WRAP_GLM(Log, log)
TEZ_WRAP_GLM(Exp2, exp2)
TEZ_WRAP_GLM(Log2, log2)
TEZ_WRAP_GLM(Sqrt, sqrt)
TEZ_WRAP_GLM(InverseSqrt, inversesqrt)

// Component-wise arithmetic
TEZ_WRAP_GLM(Abs, abs)
TEZ_WRAP_GLM(Sign, sign)
TEZ_WRAP_GLM(Floor, floor)
TEZ_WRAP_GLM(Trunc, trunc)
TEZ_WRAP_GLM(Round, round)
TEZ_WRAP_GLM(Ceil, ceil)
TEZ_WRAP_GLM(Fract, fract)
TEZ_WRAP_GLM(Mod, mod)
TEZ_WRAP_GLM(Min, min)
TEZ_WRAP_GLM(Max, max)
TEZ_WRAP_GLM(Clamp, clamp)
TEZ_WRAP_GLM(Mix, mix)
TEZ_WRAP_GLM(Step, step)
TEZ_WRAP_GLM(SmoothStep, smoothstep)
TEZ_WRAP_GLM(IsNan, isnan)
TEZ_WRAP_GLM(IsInf, isinf)

// Vector relationships
TEZ_WRAP_GLM(Length, length)
TEZ_WRAP_GLM(Distance, distance)
TEZ_WRAP_GLM(Dot, dot)
TEZ_WRAP_GLM(Cross, cross)
TEZ_WRAP_GLM(Normalize, normalize)
TEZ_WRAP_GLM(FaceForward, faceforward)
TEZ_WRAP_GLM(Reflect, reflect)
TEZ_WRAP_GLM(Refract, refract)

// Matrix algebra
TEZ_WRAP_GLM(MatrixCompMult, matrixCompMult)
TEZ_WRAP_GLM(OuterProduct, outerProduct)
TEZ_WRAP_GLM(Transpose, transpose)
TEZ_WRAP_GLM(Determinant, determinant)
TEZ_WRAP_GLM(Inverse, inverse)

// Component-wise comparisons
TEZ_WRAP_GLM(LessAn, lessThan)
TEZ_WRAP_GLM(LessThanEqual, lessThanEqual)
TEZ_WRAP_GLM(GreaterThan, greaterThan)
TEZ_WRAP_GLM(GreaterThanEqual, greaterThanEqual)
TEZ_WRAP_GLM(Equal, equal)
TEZ_WRAP_GLM(NotEqual, notEqual)
TEZ_WRAP_GLM(Any, any)
TEZ_WRAP_GLM(All, all)
TEZ_WRAP_GLM(Not, not_)

// Matrix transformations
TEZ_WRAP_GLM(Translate, translate)
TEZ_WRAP_GLM(Rotate, rotate)
TEZ_WRAP_GLM(Scale, scale)
TEZ_WRAP_GLM(Ortho, ortho)
TEZ_WRAP_GLM(Perspective, perspective)
TEZ_WRAP_GLM(LookAt, lookAt)

// Quaternion utilities
TEZ_WRAP_GLM(AngleAxis, angleAxis)
TEZ_WRAP_GLM(Slerp, slerp)
TEZ_WRAP_GLM(Conjugate, conjugate)
TEZ_WRAP_GLM(Pitch, pitch)
TEZ_WRAP_GLM(Yaw, yaw)
TEZ_WRAP_GLM(Roll, roll)
TEZ_WRAP_GLM(EulerAngles, eulerAngles)
TEZ_WRAP_GLM(Mat3Cast, mat3_cast)
TEZ_WRAP_GLM(Mat4Cast, mat4_cast)
TEZ_WRAP_GLM(QuatCast, quat_cast)

// Raw access and construction helpers
TEZ_WRAP_GLM(ValuePtr, value_ptr)
TEZ_WRAP_GLM(MakeVec2, make_vec2)
TEZ_WRAP_GLM(MakeVec3, make_vec3)
TEZ_WRAP_GLM(MakeVec4, make_vec4)
TEZ_WRAP_GLM(MakeMat3, make_mat3)
TEZ_WRAP_GLM(MakeMat4, make_mat4)
TEZ_WRAP_GLM(MakeQuat, make_quat)

// Clean up the macro so it doesn't leak out of the header
#undef TEZ_WRAP_GLM

} // namespace Tez
