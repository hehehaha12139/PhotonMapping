#include "orennayarbrdf.h"
#include <warpfunctions.h>

OrenNayarBRDF::OrenNayarBRDF(const Color3f &R, float sig)
    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R)
{
    float sigma = glm::radians(sig);
    float sigma2 = sigma * sigma;
    A = 1.0f - (sigma2 / (2.0f * (sigma2 + 0.33f)));
    B = 0.45f * sigma2 / (sigma2 + 0.09f);
}

Color3f OrenNayarBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    float sinThetai = sqrt(fmax(0.0f, 1.0f - wi.z * wi.z));
    float sinThetao = sqrt(fmax(0.0f, 1.0f - wo.z * wo.z));
    float maxCos = 0.0f;
    if(sinThetai > 1e-4 && sinThetao > 1e-4)
    {
        float sinPhii = glm::clamp(wi.y / sinThetai, -1.0f, 1.0f);
        float cosPhii = glm::clamp(wi.x / sinThetai, -1.0f, 1.0f);
        float sinPhio = glm::clamp(wo.y / sinThetao, -1.0f, 1.0f);
        float cosPhio = glm::clamp(wo.x / sinThetao, -1.0f, 1.0f);
        float dcos = cosPhii * cosPhio + sinPhii * sinPhio;
        maxCos = fmax(0.0f, dcos);
    }
    float sinAlpha, tanBeta;
    if(fabs(wi.z) > fabs(wo.z))
    {
        sinAlpha = sinThetao;
        tanBeta = sinThetai / fabs(wi.z);
    }
    else
    {
        sinAlpha = sinThetai;
        tanBeta = sinThetao / fabs(wo.z);
    }
    return R * InvPi * (A + B * maxCos * sinAlpha * tanBeta);
}

Color3f OrenNayarBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                                Float *pdf, BxDFType *sampledType) const
{
    //TODO
    *wi = WarpFunctions::squareToHemisphereCosine(u);

    if(wo.z < 0)
    {
        wi->z *= -1.0f;
    }
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

float OrenNayarBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    float pdf = WarpFunctions::squareToHemisphereCosinePDF(wi);
    return SameHemisphere(wo, wi) ? pdf : 0;
}
