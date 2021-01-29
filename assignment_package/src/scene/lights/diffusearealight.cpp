#include "diffusearealight.h"
#include "raytracing/spectrum.h"

Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    //TODO
    if(twoSided)
    {
        return emittedLight;
    }
    else if(glm::dot(isect.normalGeometric, w) > 0)
    {
        return emittedLight;
    }
    else
    {
        return Color3f(0.f);
    }
}

Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi, Vector3f *wi, Float *pdf, Point3f &interPoint) const
{
    // Get sample from light shape
    Intersection shapeInter = shape->Sample(ref, xi, pdf);
    if(*pdf == 0)
    {
        return Color3f(0.0f);
    }

    Point3f refPoint = ref.point;
    Point3f testPoint = shapeInter.point;
    if(fequal(refPoint.x, testPoint.x)
       && fequal(refPoint.y, testPoint.y)
       && fequal(refPoint.z, testPoint.z))
    {
        return Color3f(0.0f);
    }
    interPoint = shapeInter.point;

    *wi = glm::normalize(shapeInter.point - ref.point);
    return L(shapeInter, -*wi);
}

float DiffuseAreaLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    Intersection shapeInter;
    Ray shapeRay = ref.SpawnRay(wi);

    if(this->shape->Intersect(shapeRay, &shapeInter))
    {
        // If detect light, return corresponding pdf
        float pdf = 0.0f;
        Normal3f nor = shapeInter.normalGeometric;
        Point3f interPoint = shapeInter.point;
        pdf = 1 / this->shape->Area();
        pdf *= glm::length2(interPoint - ref.point) / AbsDot(nor, -wi);
        return pdf;
    }
    else
    {
        // If cant detect light, return 0.0f
        return 0.0f;
    }
}

Color3f DiffuseAreaLight::Power() const
{
    return float(twoSided ? 2 : 1) * emittedLight * area * Pi;
}

Intersection DiffuseAreaLight::Sample_Le(Photon* photon, int numPhotons, const Point2f &xi, const Point2f &xd, float *pdf) const
{
    // Sample Origin
    float photonPdf = 0.0f;
    Intersection interOri = shape->Sample(xi, &photonPdf);
    *pdf = photonPdf;
    Vector3f dir = shape->SampleDirection(xd, &photonPdf);
    *pdf *= photonPdf;
    photon->pos = interOri.point;
    photon->wi = dir;
    photon->color = emittedLight * area / (numPhotons * photonPdf);
    return interOri;
}
