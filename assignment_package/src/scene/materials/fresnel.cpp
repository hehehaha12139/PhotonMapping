#include "fresnel.h"

Color3f FresnelDielectric::Evaluate(float cosThetaI) const
{
    //TODO
    cosThetaI = glm::clamp(cosThetaI, -1.0f, 1.0f);
    // Compute indices of refraction for dielectric
    bool entering = cosThetaI > 0;
    float ei = etaI, et = etaT;
    if(!entering)
    {
        et = etaI;
        ei = etaT;
    }
    // Snell's law
    float sint = ei / et * sqrtf(fmax(0.0f, 1.0f - cosThetaI * cosThetaI));

    // Greater than Critial Angle
    if(sint > 1.0f)
    {
        return Color3f(1.0f);
    }
    else
    {
        float cost = sqrtf(fmax(0.0f, 1.0f - sint * sint));
        // FrDiel function
        float cosi = fabsf(cosThetaI);
        Color3f etat = Color3f(et);
        Color3f etai = Color3f(ei);
        Color3f Rparl = ((etat * cosi) - (etai * cost))
                        / ((etat * cosi) + (etai * cost));
        Color3f Rperp = ((etai * cosi) - (etat * cost))
                        / ((etai * cosi) + (etat * cost));

        return (Rparl * Rparl + Rperp * Rperp) / 2.0f;
    }
}


FresnelConductor::FresnelConductor(const Color3f &e, const Color3f &kk)
    : eta(e), k(kk) {}

Color3f FresnelConductor::frCond(float cosi, const Color3f &eta, const Color3f &k) const
{
    Color3f tmp = (eta * eta + k * k) * cosi * cosi;
    Color3f Rparl2 = (tmp - (2.0f * eta * cosi) + glm::vec3(1.0f))
                     / (tmp + (2.0f * eta * cosi) + glm::vec3(1.0f));
    Color3f tmp_f = eta * eta + k * k;
    Color3f Rperp2 = (tmp_f - (2.0f * eta * cosi) + cosi * cosi)
                     / (tmp_f + (2.0f * eta * cosi) + cosi * cosi);
    Color3f result = (Rparl2 + Rperp2) / 2.0f;
    return result;
}

Color3f FresnelConductor::Evaluate(float cosi) const
{
    return frCond(fabsf(cosi), eta, k);
}
