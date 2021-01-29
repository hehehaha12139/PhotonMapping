#ifndef SPECULARTRANSMISSION_H
#define SPECULARTRANSMISSION_H

#include "fresnel.h"
#include "bsdf.h"

class SpecularTransmission: public BxDF
{
public:
    SpecularTransmission(const Color3f &t, float ei, float et);
    Color3f f(const Vector3f &wo, const Vector3f &wi) const;
    virtual Color3f Sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &sample, Float *pdf,
                              BxDFType *sampledType = nullptr) const;
    virtual float Pdf(const Vector3f &wo, const Vector3f &wi) const;
private:
    Color3f T;
    float etai, etat;
    FresnelDielectric fresnel;
};

#endif // SPECULARTRANSMISSION_H
