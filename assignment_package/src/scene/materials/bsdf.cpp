#include "bsdf.h"
#include <warpfunctions.h>
#include "specularreflection.h"
#include "specularTransmission.h"

BSDF::BSDF(const Intersection& isect, float eta /*= 1*/)
//TODO: Properly set worldToTangent and tangentToWorld
    : worldToTangent(/*COMPUTE ME*/),
      tangentToWorld(/*COMPUTE ME*/),
      normal(isect.normalGeometric),
      eta(eta),
      numBxDFs(0),
      bxdfs{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
{
     this->UpdateTangentSpaceMatrices(glm::normalize(isect.normalGeometric),
                                      isect.tangent,
                                      isect.bitangent);
}


void BSDF::UpdateTangentSpaceMatrices(const Normal3f& n, const Vector3f& t, const Vector3f b)
{
    //TODO: Update worldToTangent and tangentToWorld based on the normal, tangent, and bitangent
    tangentToWorld = glm::mat3(t, b, n);
    worldToTangent = glm::transpose(tangentToWorld);
}


//
Color3f BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags /*= BSDF_ALL*/) const
{
    //TODO
    // Transform light direction to tangent space
    Vector3f woWTangent = worldToTangent * woW;
    Vector3f wiWTangent = worldToTangent * wiW;
    Color3f reColor = Color3f(0.0f);

    // Traverse BxDFs to get color sum
    for(int i = 0; i < numBxDFs; ++i)
    {
        if(bxdfs[i]->MatchesFlags(flags))
        {
            reColor += bxdfs[i]->f(woWTangent, wiWTangent);
        }
    }

    return reColor;
}

// Use the input random number _xi_ to select
// one of our BxDFs that matches the _type_ flags.

// After selecting our random BxDF, rewrite the first uniform
// random number contained within _xi_ to another number within
// [0, 1) so that we don't bias the _wi_ sample generated from
// BxDF::Sample_f.

// Convert woW and wiW into tangent space and pass them to
// the chosen BxDF's Sample_f (along with pdf).
// Store the color returned by BxDF::Sample_f and convert
// the _wi_ obtained from this function back into world space.

// Iterate over all BxDFs that we DID NOT select above (so, all
// but the one sampled BxDF) and add their PDFs to the PDF we obtained
// from BxDF::Sample_f, then average them all together.

// Finally, iterate over all BxDFs and sum together the results of their
// f() for the chosen wo and wi, then return that sum.

Color3f BSDF::Sample_f(const Vector3f &woW, Vector3f *wiW, const Point2f &xi,
                       float *pdf, BxDFType type, BxDFType *sampledType) const
{
    //TODO
    // Select random bxdf by xi
    int num = BxDFsMatchingFlags(type);
    int bxdfIndex = floor(xi[0] * num);
    int count = 0;
    BxDF *selected = nullptr;
    for(int i = 0; i < numBxDFs; i++)
    {
        if(bxdfs[i]->MatchesFlags(type))
        {
            if(count == bxdfIndex)
            {
                selected = bxdfs[i];
                break;
            }
            count++;
        }
    }

    if(sampledType)
    {
        *sampledType = selected->type;
    }


    // Update new rand for xi
    float newRand = (float)rand() / (float)RAND_MAX;

    // Convert world coordinate to local
    Vector3f woWTangent = worldToTangent * woW;
    Vector3f wiWTangent = worldToTangent * (*wiW);

    *pdf = 0.0f;

    Color3f reColor = Color3f(0.0f, 0.0f, 0.0f);
    reColor = selected->Sample_f(woWTangent, &wiWTangent, Point2f(newRand, xi[1]), pdf, sampledType);

    // Given direction's probablity equals to zero, return
    if(*pdf == 0)
    {
        if(sampledType)
        {
            *sampledType = BxDFType(0);
        }
        return Color3f(0.0f);
    }

    // Get new input direction
    *wiW = tangentToWorld * wiWTangent;

    float avgPdf = *pdf;
    *pdf /= num;
    count = 0;

    // Skip when given bsdf contains specular material
    if(!(selected->type & BSDF_SPECULAR))
    {
        // Compute pdf
        for(int i = 0; i < numBxDFs; i++)
        {
            if(bxdfs[i]->MatchesFlags(type))
            {
                if(count == bxdfIndex)
                {
                    count++;
                    continue;
                }
                avgPdf += bxdfs[i]->Pdf(woWTangent, wiWTangent);
            }
        }

        avgPdf = avgPdf / num;
        *pdf = avgPdf;

        reColor = Vector3f(0.0f);
        // Compute all color
        for(int i = 0; i < numBxDFs; i++)
        {
            if(bxdfs[i]->MatchesFlags(type))
            {
                if(typeid(*(bxdfs[i])) == typeid(SpecularReflection)
                        || typeid(*(bxdfs[i])) == typeid(SpecularTransmission))
                {
                    Vector3f newWi = wiWTangent;
                    float newPdf = *pdf;
                    reColor += bxdfs[i]->Sample_f(woWTangent, &newWi,
                                                  Point2f(newRand, xi[1]), &newPdf,
                                                  sampledType);
                }
                reColor += bxdfs[i]->f(woWTangent, wiWTangent);
            }
        }
    }


    return reColor;
}


float BSDF::Pdf(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const
{
    //TODO
    Vector3f woWTangent = worldToTangent * woW;
    Vector3f wiWTangent = worldToTangent * wiW;
    float pdf = 0.0f;

    // Sum the pdf from bxdfs
    for(int i = 0; i < numBxDFs; i++)
    {
        if(bxdfs[i]->MatchesFlags(flags))
        {
            pdf = bxdfs[i]->Pdf(woWTangent, wiWTangent);
        }
    }
    return pdf / BxDFsMatchingFlags(flags);
}

Color3f BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi,
                       Float *pdf, BxDFType *sampledType) const
{
    //TODO
    *wi = WarpFunctions::squareToHemisphereUniform(xi);
    if(wo.z < 0.0f)
    {
        *wi *= -1.0f;
    }
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

// The PDF for uniform hemisphere sampling
float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return SameHemisphere(wo, wi) ? Inv2Pi : 0;
}

BSDF::~BSDF()
{
    for(int i = 0; i < numBxDFs; i++)
    {
        delete bxdfs[i];
    }
}
