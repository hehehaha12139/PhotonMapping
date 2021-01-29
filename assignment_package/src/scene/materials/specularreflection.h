#ifndef SPECULARREFLECTION_H
#define SPECULARREFLECTION_H

#include "fresnel.h"
#include "bsdf.h"

class SpecularReflection: public BxDF
{
public:
    SpecularReflection(const Color3f &r);
    Color3f f(const Vector3f &wo, const Vector3f &wi) const;
    virtual Color3f Sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &sample, Float *pdf,
                              BxDFType *sampledType = nullptr) const;
    virtual float Pdf(const Vector3f &wo, const Vector3f &wi) const;
private:
    const Color3f R;
    //FresnelConductor fresnel;
};

#endif // SPECULARREFLECTION_H
