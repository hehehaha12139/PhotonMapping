#include "photonmapper.h"

PhotonMapper::PhotonMapper(int numPhotons, std::vector<Photon> *photons, std::vector<Photon>* caustics, Scene *s, std::shared_ptr<Sampler> sampler, int recursionLimit)
    : Integrator(Bounds2i(Point2i(0,0), Point2i(0,0)), s, sampler, recursionLimit), preprocessing(true), numPhotons(numPhotons), photons(photons), causticPhotons(caustics)
{
    photons = photons;
}

PhotonMapper::PhotonMapper(Bounds2i bounds, Scene *s, std::shared_ptr<Sampler> sampler, int recursionLimit, float searchRadius, KDTree* photonTree, KDTree* causticTree)
    : Integrator(bounds, s, sampler, recursionLimit), preprocessing(false), numPhotons(0), photons(nullptr),
      searchRadius(searchRadius), photonTree(photonTree), causticTree(causticTree)
{}

void PhotonMapper::Render()
{
    // PhotonMapper's Render() function has
    // two modes: when preprocessing, it traces
    // a collection of photons through the scene
    // and stores them in the given k-d tree.
    // If not preprocessing, it runs like a regular Integrator
    // and invokes Li().
    if(preprocessing)
    {
        // TODO
        // Determine how many photons to assign to each light source
        // given numPhotons and the intensity of each light.
        // Shoot a number of photons equal to numPhotons from
        // the lights, bouncing them through the scene and pushing
        // back the result of each bounce to the photons vector
        // stored in the PhotonMapper.
        // First pass: Emit photons from light
        float totalLightIntensity = 0.0f;
        for(int i = 0; i < scene->lights.size(); i++)
        {
            glm::vec3 lightPower = scene->lights.at(i)->Power();
            float colorPower = (0.2126f * lightPower.r + 0.7152f * lightPower.g + 0.0722f * lightPower.b);

            totalLightIntensity += colorPower;
        }

        int photonRemain = numPhotons;

        for(int i = 0; i < scene->lights.size(); i++)
        {
            // Sample photon number according to light intensity
            Light* curLight = scene->lights.at(i).get();
            int lightPhotonNum = 0;
            if(i == scene->lights.size() - 1)
            {
                lightPhotonNum = photonRemain;
            }
            else
            {
                glm::vec3 lightPower = scene->lights.at(i)->Power();
                float colorPower = (0.2126f * lightPower.r + 0.7152f * lightPower.g + 0.0722f * lightPower.b);
                lightPhotonNum = floor(colorPower * float(numPhotons) / totalLightIntensity);
                photonRemain -= lightPhotonNum;
            }
            lightPhotonNum = numPhotons / scene->lights.size();
            //lightPhotonNum = numPhotons / scene->lights.size();
            for(int j = 0; j < lightPhotonNum; j++)
            {
                // Sample each photon for light
                int totalPhotons = 8 * lightPhotonNum;
                Photon curPhoton = Photon();
                Point2f xi = sampler->Get2D();
                Point2f xd = sampler->Get2D();
                float photonPdf = 0.0f;
                Intersection lightIsect = curLight->Sample_Le(&curPhoton, totalPhotons, xi, xd, &photonPdf);
                Ray photonRay = Ray(curPhoton.pos, curPhoton.wi);
                Color3f throughput =  Color3f(1.0f);
                throughput /= (float) totalPhotons;
                bool isCaustic = false;
                for(int j = 0; j < recursionLimit; j++)
                {
                    // Emit photons
                    Vector3f woW = -photonRay.direction;
                    Intersection photonInter;
                    if(scene->Intersect(photonRay, &photonInter))
                    {
                        // Sample photon scattering ray
                        Point2f wiSample = sampler->Get2D();
                        Vector3f wiW = Vector3f(0.0f);
                        float pdf = 0.0f;
                        BxDFType wiBSDF;
                        if(!photonInter.ProduceBSDF())
                        {
                            continue;
                        }
                        Color3f photonColor = photonInter.bsdf->Sample_f(woW, &wiW, wiSample,
                                                                               &pdf, BSDF_ALL, &wiBSDF);
                        bool specularFlag = wiBSDF & BSDF_SPECULAR;
                        //bool transmissiveFlag = wiBSDF & BSDF_TRANSMISSION;
                        wiW = glm::normalize(wiW);


                        if(IsBlack(photonColor) || pdf == 0.0f)
                            break;

                        curPhoton.pos = photonInter.point;

                        if(!isCaustic && j > 0)
                        {
                            photons->push_back(curPhoton);
                        }
                        else if(isCaustic && j > 0)
                        {
                            causticPhotons->push_back(curPhoton);
                        }

                        if(specularFlag)
                        {
                            isCaustic = true;
                        }
                        else if(isCaustic && (!specularFlag))
                        {
                            isCaustic = false;
                        }


                        curPhoton.wi = wiW;
                        curPhoton.color *= photonColor * AbsDot(wiW, photonInter.normalGeometric) / pdf;

                        throughput *= photonColor * AbsDot(wiW, photonInter.normalGeometric) / pdf;
                        photonRay = photonInter.SpawnRay(wiW);

                        // Russian roulette
                        float throughputMax = fmax(throughput.x, fmax(throughput.y, throughput.z));

                        if(j > 3)
                        {
                            //Russian Roulette
                            float q = fmax(0.05f, 1 - throughputMax);
                            if(sampler->Get1D() < q)
                            {
                                break;
                            }
                        }

                    }
                }
            }
        }
    }
    else
    {
        Integrator::Render(); // Invokes Li for each ray from a pixel
    }
}

Color3f PhotonMapper::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    try {
        // Get initial intersection and ray
        Intersection pixelInter = Intersection();
        Color3f throughPut = Color3f(1.0f);
        Color3f finalColor = Color3f(0.0f);
        Vector3f wo = -ray.direction;
        bool specularFlag = false;
        int bouncing = 0;
        Ray curRay = ray;


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
                return finalColor;
            }

            int count = 0;
//            // Global Illumination by Photon
            bool pixelDiffuse = pixelInter.bsdf->BxDFsMatchingFlags(BSDF_DIFFUSE);
            while(true)
            {
                if(!pixelInter.ProduceBSDF() || count >= 5)
                {
                    return reColor;
                }


                Vector3f wi = Vector3f(0.0f);
                Point2f xi = sampler->Get2D();
                float pdf = 0.0f;
                BxDFType wiBSDF;
                Color3f findDiffuseTemp = pixelInter.bsdf->Sample_f(wo, &wi, xi,
                                                                    &pdf, BSDF_ALL, &wiBSDF);
                pixelDiffuse = wiBSDF & BSDF_DIFFUSE;
                if(pixelDiffuse)
                {
                    break;
                }
                Ray testRay = pixelInter.SpawnRay(glm::normalize(wi));
                if(!scene.Intersect(testRay, &pixelInter))
                    return reColor;
                count++;
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

            //reColor *= (float)lightNum;



            float photonRange = Pi * searchRadius * searchRadius;
            std::vector<Photon> globalPhotons = photonTree->particlesInSphere(pixelInter.point, searchRadius);
            Color3f globalColor = Color3f(0.0f);
            int photonCount = 0;
            for(int i = 0; i < globalPhotons.size(); i++)
            {
                glm::vec3 photonWi = globalPhotons.at(i).wi;
                float pdf = pixelInter.bsdf->Pdf(wo, -photonWi);
                Color3f curColor = pixelInter.bsdf->f(wo, -photonWi);
                if(fequal(pdf, 0.0f))
                    continue;
                float dis = glm::length(globalPhotons.at(i).pos - pixelInter.point);
                float fallOffRatio = dis / searchRadius;

                float diff = 0.5f;

                float fallOffValue = expf(-0.5f * powf((fallOffRatio - 0.0f) / diff, 2.0f)) / (diff * sqrtf(2.0f * Pi));

                if(AbsDot(pixelInter.normalGeometric, globalPhotons.at(i).pos - pixelInter.point) < 0.0001)
                {
                    globalColor += curColor * globalPhotons.at(i).color * AbsDot(pixelInter.normalGeometric, photonWi) / pdf;
                }

            }

            reColor += globalColor / (4.0f * photonRange);


            if(!causticTree->isEmpty)
            {
                // Caustic Illumination by Photon
                std::vector<Photon> causticPhotons = causticTree->particlesInSphere(pixelInter.point, searchRadius);
                Color3f causticColor = Color3f(0.0f);

                for(int i = 0; i < causticPhotons.size(); i++)
                {
                    glm::vec3 photonWi = causticPhotons.at(i).wi;
                    float pdf = pixelInter.bsdf->Pdf(wo, -photonWi);
                    Color3f curColor = pixelInter.bsdf->f(wo, -photonWi);
                    if(fequal(pdf, 0.0f))
                        continue;
                    float dis = glm::length(causticPhotons.at(i).pos - pixelInter.point);
                    float fallOffRatio = dis / searchRadius;

                    float diff = 0.8f;

                    float fallOffValue = expf(-0.5f * powf((fallOffRatio - 0.0f) / diff, 2.0f)) / (diff * sqrtf(2.0f * Pi));


                    if(AbsDot(pixelInter.normalGeometric, causticPhotons.at(i).pos - pixelInter.point) < 0.0001)
                    {
                        causticColor += curColor * causticPhotons.at(i).color * AbsDot(pixelInter.normalGeometric, photonWi) * fallOffValue / pdf;
                        //causticColor += Color3f(1.0f);
                    }



                }

                finalColor += causticColor / (4.0f * photonRange);
            }



            finalColor += reColor;
        }

        return finalColor;
    }
    catch (const std::bad_alloc&)
    {

    }

}
