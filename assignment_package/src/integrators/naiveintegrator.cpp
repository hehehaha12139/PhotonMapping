#include "naiveintegrator.h"
#include <string>
#include <sstream>

Color3f NaiveIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
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
        if(!pixelInter.ProduceBSDF() || depth == 0)
        {
            return reColor;
        }

        // Initialize sample_f function parameter
        Point2f sample = sampler->Get2D();
        Vector3f wi = Vector3f(0.0f);
        float pdf = 1.0f;

        // Get Sampled Color
        Color3f sampleColor = pixelInter.bsdf->Sample_f(wo, &wi, sample, &pdf);
        if(fequal(pdf, 0.0f))
        {
            return Color3f(0.0f);
        }

        Color3f scatteredTerm = sampleColor * AbsDot(wi, pixelInter.normalGeometric) / pdf;

        Ray newRay = pixelInter.SpawnRay(glm::normalize(wi));

        // Recursively invoke Li
        scatteredTerm *= Li(newRay, scene, sampler, depth - 1);
        reColor += scatteredTerm;
        return reColor;
    }
    else
    {
        return Color3f(0.0f);
    }
}
