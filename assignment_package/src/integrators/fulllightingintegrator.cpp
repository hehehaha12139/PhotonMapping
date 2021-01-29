#include "fulllightingintegrator.h"

Color3f FullLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    // Get initial intersection and ray
    Intersection pixelInter = Intersection();
    Color3f throughPut = Color3f(1.0f);
    Color3f finalColor = Color3f(0.0f);
    Vector3f wo = -ray.direction;
    bool specularFlag = false;
    int bouncing = 0;
    Ray curRay = ray;

    while(depth >= 0)
    {
        if(scene.Intersect(curRay, &pixelInter))
        {
            wo = -curRay.direction;
            Color3f reColor = pixelInter.Le(wo);

            // If this is a light
            if(!pixelInter.ProduceBSDF())
            {
                // Correct light
                if(bouncing == 0 || specularFlag)
                {
                    finalColor += throughPut * reColor;
                }
                break;
            }

            // MIS part
            Point2f lightSample = sampler->Get2D();
            Vector3f lightWi = Vector3f(0.0f);
            float lightPdf = 0.0f;
            int lightNum = scene.lights.size();
            int lightIndex = fmin((int)(sampler->Get1D() * lightNum), lightNum - 1);
            Point3f interPoint = Point3f(0.0f);
            Color3f lightSampleColor = scene.lights.at(lightIndex)->Sample_Li(pixelInter, lightSample,
                                                                              &lightWi, &lightPdf, interPoint);
            const std::shared_ptr<Light> light = scene.lights.at(lightIndex);

            // Naive Integrator sample
            Point2f surfaceSample = sampler->Get2D();
            Vector3f surfaceWi = Vector3f(0.0f);
            float surfacePdf = 0.0f;
            Color3f surfaceSampleColor = pixelInter.bsdf->Sample_f(wo, &surfaceWi, surfaceSample, &surfacePdf);

            if(fequal(lightPdf, 0.0f))
            {
                return Color3f(0.0f);
            }

            // Visibility Test
            float visibleFlag = 0.0f;
            Ray testRay = Ray(interPoint, -lightWi);
            Intersection visibleInter = Intersection();
            scene.Intersect(testRay, &visibleInter);
            Point3f visiblePoint = visibleInter.point;
            if(fequal(visiblePoint.x, pixelInter.point.x, 0.01f)
               && fequal(visiblePoint.y, pixelInter.point.y, 0.01f)
               && fequal(visiblePoint.z, pixelInter.point.z, 0.01f))
            {
                visibleFlag = 1.0f;
            }

            // Combine light f and sampled f
            if(!IsBlack(lightSampleColor) || !IsBlack(surfaceSampleColor))
            {
                // Deal with light sampled wi
                Color3f f = pixelInter.bsdf->f(wo, lightWi) * AbsDot(lightWi, pixelInter.normalGeometric);
                if(IsBlack(f))
                {
                    f = pixelInter.bsdf->f(-wo, lightWi) * AbsDot(lightWi, pixelInter.normalGeometric);
                }
                f *= lightSampleColor;



                float surfacePdfByLight = pixelInter.bsdf->Pdf(wo, lightWi);

                if(surfacePdfByLight != 0 && lightPdf != 0)
                {
                    float lightSampleWeight = BalanceHeuristic(1, lightPdf, 1, surfacePdfByLight);
                    reColor += f * visibleFlag * lightSampleWeight / lightPdf;
                }


                // Deal with surface sampled wi
                Intersection surfaceSampleInter;
                float lightPdfBySurface = 0.0f;
                Ray lightInterRay = pixelInter.SpawnRay(surfaceWi);
                Intersection lightInter;
                if(scene.Intersect(lightInterRay, &lightInter))
                {
                    if(lightInter.objectHit->GetAreaLight() == light.get())
                    {
                        Color3f lightColorBySurface = lightInter.Le(-surfaceWi);
                        lightPdfBySurface = light->Pdf_Li(pixelInter, surfaceWi);

                        if(lightPdfBySurface != 0 && surfacePdf != 0)
                        {
                            Color3f g = surfaceSampleColor * lightColorBySurface;
                            float surfaceSampleWeight = BalanceHeuristic(1, surfacePdf, 1, lightPdfBySurface);
                            reColor += g * surfaceSampleWeight / surfacePdf;
                        }
                    }
                }
            }

            reColor *= (float)lightNum;


            // ThroughPut resampling
            Point2f globalSample = sampler->Get2D();
            Vector3f globalWi = Vector3f(0.0f);
            float globalPdf = 0.0f;
            BxDFType globalBSDF;
            Color3f globalSampleColor = pixelInter.bsdf->Sample_f(wo, &globalWi, globalSample, &globalPdf, BSDF_ALL, &globalBSDF);
            specularFlag = false;
            specularFlag = (globalBSDF & BSDF_SPECULAR) != 0;

            if(!specularFlag)
            {
                finalColor += throughPut * reColor;

            }

            if(fequal(globalPdf, 0.0f))
            {
                break;
            }

            // Update throughput
            Color3f curThroughput = globalSampleColor * AbsDot(globalWi, pixelInter.normalGeometric) / globalPdf;
            throughPut *= curThroughput;

            float throughputMax = fmax(throughPut.x, fmax(throughPut.y, throughPut.z));

            if(bouncing > 3)
            {
                //Russian Roulette
                float q = fmax(0.05f, 1 - throughputMax);
                if(sampler->Get1D() < q)
                {
                    break;
                }
                throughPut /= 1 - q;
            }

            // Spawn new ray
            curRay = pixelInter.SpawnRay(glm::normalize(globalWi));

            bouncing++;
            depth--;
        }
        else
        {
            break;
        }
    }

    return finalColor;
}

