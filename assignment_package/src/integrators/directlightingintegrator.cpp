#include "directlightingintegrator.h"

Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    //TODO
    Intersection pixelInter = Intersection();
    Color3f reColor = Color3f(0.0f);
    Vector3f wo = -ray.direction;
    if(scene.Intersect(ray, &pixelInter))
    {
        // Emission term of LTE
        reColor = pixelInter.Le(wo);

        // Intergration Term
        // Produce Intersection Point's BSDF
        // If this is a light
        if(!pixelInter.ProduceBSDF())
        {
            return pixelInter.objectHit->areaLight->L(pixelInter, ray.direction * -1.0f);
        }

        // Choose a light to smaple
        Point2f sample = sampler->Get2D();
        Vector3f wi = Vector3f(0.0f);
        float pdf = 1.0f;
        int lightNum = scene.lights.size();
        int lightIndex = fmin((int)(sampler->Get1D() * lightNum), lightNum - 1);
        Point3f interPoint = Point3f(0.0f);
        Color3f sampleColor = scene.lights.at(lightIndex)->Sample_Li(pixelInter, sample, &wi, &pdf, interPoint) * (float)lightNum;

        if(fequal(pdf, 0.0f))
        {
            return Color3f(0.0f);
        }

        // Visibility Test
        float visibleFlag = 0.0f;
        Ray testRay = Ray(interPoint, -wi);
        Intersection visibleInter = Intersection();
        scene.Intersect(testRay, &visibleInter);
        Point3f visiblePoint = visibleInter.point;
        if(fequal(visiblePoint.x, pixelInter.point.x, 0.01f)
           && fequal(visiblePoint.y, pixelInter.point.y, 0.01f)
           && fequal(visiblePoint.z, pixelInter.point.z, 0.01f))
        {
            visibleFlag = 1.0f;
        }

        if(!IsBlack(sampleColor))
        {
            // Recursively invoke Li
            Color3f f = pixelInter.bsdf->f(wo, wi) * AbsDot(wi, pixelInter.normalGeometric);
            if(IsBlack(f))
            {
                f = pixelInter.bsdf->f(-wo, wi) * AbsDot(wi, pixelInter.normalGeometric);
            }
            sampleColor *= f;
            reColor += sampleColor * visibleFlag / pdf;
            return reColor;
        }

        return reColor;
    }
    else
    {
        for(int i = 0; i < scene.lights.size(); i++)
        {
            reColor += scene.lights.at(i)->Le(ray);
        }
        return reColor;
    }
}
