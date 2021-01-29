#include "specularTransmission.h"

SpecularTransmission::SpecularTransmission(const Color3f &t, float ei, float et)
    : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)), fresnel(ei, et),
      T(t), etai(ei), etat(et) {}

Color3f SpecularTransmission::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.0f);
}

Color3f SpecularTransmission::Sample_f(const Vector3f &wo, Vector3f *wi,
                                       const Point2f &sample, Float *pdf,
                                       BxDFType *sampledType) const
{
    // Eta incident and transmitted
    bool entering = wo.z > 0.0f;
    float ei = etai, et = etat;
    if(!entering)
    {
        et = etai;
        ei = etat;
    }

    // Compute transmitted ray direction
    float sini2 = fmax(0.0f, 1.0f - wo.z * wo.z);
    float eta = ei / et;
    float sint2 = eta * eta * sini2;
    if(sint2 > 1.0f || fequal(sint2, 0.0f))
    {
        return Color3f(0.0f);
    }
    float cost = sqrtf(fmax(0.0f, 1.0f - sint2));
    if(entering)
        cost = -cost;
    float sintOverSini = eta;
    *wi = Vector3f(sintOverSini * -wo.x, sintOverSini * -wo.y, cost);

    *pdf = Pdf(wo, *wi);
    Color3f F = fresnel.evaluate(wo.z);
    return (et * et) / (ei * ei) * (Color3f(1.0f) - F) * T / fabsf(wi->z);
}

float SpecularTransmission::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 1.0f;
}
