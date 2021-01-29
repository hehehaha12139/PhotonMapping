#include "specularreflection.h"

SpecularReflection::SpecularReflection(const Color3f &r)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)), R(r), fresnel(Color3f(2.42f), Color3f(1.0f)) {}

Color3f SpecularReflection::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.0f);
}

Color3f SpecularReflection::Sample_f(const Vector3f &wo, Vector3f *wi,
                                     const Point2f &sample, Float *pdf,
                                     BxDFType *sampledType) const
{
    *wi = glm::vec3(-wo.x, -wo.y, -wo.z);
    *pdf = Pdf(wo, *wi);
    Color3f result = fresnel.evaluate(wo.z) * R / fabsf(wi->z);
    return result;
}

float SpecularReflection::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 1.0f;
}
