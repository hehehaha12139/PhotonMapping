#pragma once
#ifndef ORENNAYARBRDF_H
#define ORENNAYARBRDF_H

#include "bsdf.h"
class OrenNayarBRDF: public BxDF
{
public:
    // Constructor
    OrenNayarBRDF(const Color3f &R, float sig);

    // BRDF function
    Color3f f(const Vector3f &wo, const Vector3f &wi) const;

    // Sample function
    virtual Color3f Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                             Float *pdf, BxDFType *sampledType) const;

    virtual float Pdf(const Vector3f &wo, const Vector3f &wi) const;
private:
    const Color3f R;    // The energy scattering coefficient of this BRDF (i.e. its color)
    float A, B;         // OrenNayar brdf coefficients
};

#endif // ORENNAYARBRDF_H
