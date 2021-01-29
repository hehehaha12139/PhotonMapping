#include "lambertbrdf.h"
#include <warpfunctions.h>

Color3f LambertBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    return R * InvPi;
}

Color3f LambertBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                        Float *pdf, BxDFType *sampledType) const
{
    //TODO
    *wi = WarpFunctions::squareToHemisphereCosine(sample);

    if(wo.z < 0)
    {
        wi->z *= -1.0f;
    }
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

float LambertBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    float pdf = WarpFunctions::squareToHemisphereCosinePDF(wi);
    return SameHemisphere(wo, wi) ? pdf : 0;
}
