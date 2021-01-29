#include "specularbrdf.h"

Color3f SpecularBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    //TODO!
    // Get totally inverse reflection direction
    *wi = glm::vec3(-wo.x, -wo.y, wo.z);
    *pdf = 1.0f;
    Color3f result = fresnel->Evaluate(wo.z) * R / fabsf(wi->z);
    return result;
}