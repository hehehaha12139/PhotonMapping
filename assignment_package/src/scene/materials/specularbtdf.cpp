#include "specularbTdf.h"

Color3f SpecularBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    // Eta incident and transmitted
    bool entering = wo.z > 0.0f;
    float ei = etaA, et = etaB;
    if(!entering)
    {
        et = etaA;
        ei = etaB;
    }

    // Compute transmitted ray direction
    float sini2 = fmax(0.0f, 1.0f - wo.z * wo.z);
    float eta = ei / et;
    float sint2 = eta * eta * sini2;
    if(sint2 > 1.0f || fequal(sint2, 1.0f))
    {
        *pdf = 1.0f;
        return Color3f(0.0f);
    }
    float cost = sqrtf(fmax(0.0f, 1.0f - sint2));
    if(entering)
        cost = -cost;
    float sintOverSini = eta;
    *wi = Vector3f(sintOverSini * -wo.x, sintOverSini * -wo.y, cost);

    *pdf = 1.0f;
    Color3f F = fresnel->Evaluate(wo.z);
    return (et * et) / (ei * ei) * (Color3f(1.0f) - F) * T / fabsf(wi->z);
}
