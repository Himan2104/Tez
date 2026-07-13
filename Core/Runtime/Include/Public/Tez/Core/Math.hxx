#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

namespace Tez
{

using Vec2f = glm::vec2;
using Vec3f = glm::vec3;
using Vec4f = glm::vec4;

using Vec2i = glm::ivec2;
using Vec3i = glm::ivec3;
using Vec4i = glm::ivec4;

using Vec2u = glm::uvec2;
using Vec3u = glm::uvec3;
using Vec4u = glm::uvec4;

using Vec2d = glm::dvec2;
using Vec3d = glm::dvec3;
using Vec4d = glm::dvec4;

using Vec2b = glm::bvec2;
using Vec3b = glm::bvec3;
using Vec4b = glm::bvec4;

using Mat2f = glm::mat2;
using Mat3f = glm::mat3;
using Mat4f = glm::mat4;
using Mat2d = glm::dmat2;
using Mat3d = glm::dmat3;
using Mat4d = glm::dmat4;

using Mat2x3 = glm::mat2x3;
using Mat3x2 = glm::mat3x2;
using Mat2x4 = glm::mat2x4;
using Mat4x2 = glm::mat4x2;
using Mat3x4 = glm::mat3x4;
using Mat4x3 = glm::mat4x3;

using Quatf = glm::quat;
using Quatd = glm::dquat;

#define TEZ_WRAP_GLM(aliasName, funcName)                       \
    template <typename... Args>                                 \
    constexpr auto aliasName(Args&&... args) noexcept(          \
        noexcept(glm::funcName(std::forward<Args>(args)...)))   \
        -> decltype(glm::funcName(std::forward<Args>(args)...)) \
    { return glm::funcName(std::forward<Args>(args)...); }

TEZ_WRAP_GLM(Radians, radians)
TEZ_WRAP_GLM(Degrees, degrees)
TEZ_WRAP_GLM(Sin, sin)
TEZ_WRAP_GLM(Cos, cos)
TEZ_WRAP_GLM(Tan, tan)
TEZ_WRAP_GLM(Asin, asin)
TEZ_WRAP_GLM(Acos, acos)
TEZ_WRAP_GLM(Atan, atan)
TEZ_WRAP_GLM(Sinh, sinh)
TEZ_WRAP_GLM(Cosh, cosh)
TEZ_WRAP_GLM(Tanh, tanh)

TEZ_WRAP_GLM(Pow, pow)
TEZ_WRAP_GLM(Exp, exp)
TEZ_WRAP_GLM(Log, log)
TEZ_WRAP_GLM(Exp2, exp2)
TEZ_WRAP_GLM(Log2, log2)
TEZ_WRAP_GLM(Sqrt, sqrt)
TEZ_WRAP_GLM(InverseSqrt, inversesqrt)

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

TEZ_WRAP_GLM(Length, length)
TEZ_WRAP_GLM(Distance, distance)
TEZ_WRAP_GLM(Dot, dot)
TEZ_WRAP_GLM(Cross, cross)
TEZ_WRAP_GLM(Normalize, normalize)
TEZ_WRAP_GLM(FaceForward, faceforward)
TEZ_WRAP_GLM(Reflect, reflect)
TEZ_WRAP_GLM(Refract, refract)

TEZ_WRAP_GLM(MatrixCompMult, matrixCompMult)
TEZ_WRAP_GLM(OuterProduct, outerProduct)
TEZ_WRAP_GLM(Transpose, transpose)
TEZ_WRAP_GLM(Determinant, determinant)
TEZ_WRAP_GLM(Inverse, inverse)

TEZ_WRAP_GLM(LessAn, lessThan)
TEZ_WRAP_GLM(LessThanEqual, lessThanEqual)
TEZ_WRAP_GLM(GreaterThan, greaterThan)
TEZ_WRAP_GLM(GreaterThanEqual, greaterThanEqual)
TEZ_WRAP_GLM(Equal, equal)
TEZ_WRAP_GLM(NotEqual, notEqual)
TEZ_WRAP_GLM(Any, any)
TEZ_WRAP_GLM(All, all)
TEZ_WRAP_GLM(Not, not_)

TEZ_WRAP_GLM(Translate, translate)
TEZ_WRAP_GLM(Rotate, rotate)
TEZ_WRAP_GLM(Scale, scale)
TEZ_WRAP_GLM(Ortho, ortho)
TEZ_WRAP_GLM(Perspective, perspective)
TEZ_WRAP_GLM(LookAt, lookAt)

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
