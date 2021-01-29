#pragma once
#include <globals.h>
#include <scene/transform.h>
#include <raytracing/intersection.h>
#include <scene/photon.h>

class Intersection;

class Light
{
  public:
    virtual ~Light(){}
    Light(Transform t)
        : transform(t), name()
    {}

    // Returns the light emitted along a ray that does
    // not hit anything within the scene bounds.
    // Necessary if we want to support things like environment
    // maps, or other sources of light with infinite area.
    // The default implementation for general lights returns
    // no energy at all.
    virtual Color3f Le(const Ray &r) const;

    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                                                Vector3f *wi, Float *pdf, Point3f &interPoint) const = 0;
    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const = 0;

    virtual Color3f Power() const = 0;

    virtual Intersection Sample_Le(Photon* photon, int numPhotons, const Point2f &xi, const Point2f &xd, float *pdf) const = 0;

    QString name; // For debugging

    const Transform transform;
};

class AreaLight : public Light
{
public:
    AreaLight(const Transform &t) : Light(t){}
    // Returns the light emitted from a point on the light's surface _isect_
    // along the direction _w_, which is leaving the surface.
    virtual Color3f L(const Intersection &isect, const Vector3f &w) const = 0;
    Color3f Power() const = 0;
    Intersection Sample_Le(Photon* photon, int numPhotons, const Point2f &xi,
                      const Point2f &xd, float *pdf) const = 0;
};

class PointLight: public Light
{
public:
    PointLight(const Transform &t, const Color3f& Le):
        Light(t), emittedLight(Le),
        pLight(t.position()){}

    Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                                                   Vector3f *wi, Float *pdf, Point3f &interPoint) const;

    float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    Color3f Le(const Ray &r) const;

    Color3f Power() const;
    Intersection Sample_Le(Photon* photon, int numPhotons, const Point2f &xi,
                      const Point2f &xd, float *pdf) const;
public:
    Color3f emittedLight;
    Point3f pLight;
};

class SpotLight: public Light
{
public:
    SpotLight(const Transform &t, const Color3f &Le,
                     Float totalWidth, Float falloffStart):
        Light(t), emittedLight(Le),
        pLight(t.position()),
        cosTotalWidth(std::cos(glm::radians(totalWidth))),
        cosFallOffStart(std::cos(glm::radians(falloffStart))) {};
    float fallOff(const Vector3f &w) const;
    Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                      Vector3f *wi, Float *pdf, Point3f &interPoint) const override;

    float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    Color3f Le(const Ray &r) const;
    Color3f Power() const;
    Intersection Sample_Le(Photon* photon, int numPhotons, const Point2f &xi,
                      const Point2f &xd, float *pdf) const;
public:
    Color3f emittedLight;
    Point3f pLight;
    float cosTotalWidth, cosFallOffStart;
};

class DistantLight: public Light
{
public:
    DistantLight(const Transform &t, const Color3f &Le,
                 const Vector3f &w, const Point3f& worldCenter, const float &worldRadius):
        Light(t), emittedLight(Le),
        worldCenter(worldCenter), worldRadius(worldRadius),
        wLight(glm::normalize(t.invT() * glm::vec4(w, 0.0f))) {}

    Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                      Vector3f *wi, Float *pdf, Point3f &interPoint) const override;
    float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    Color3f Le(const Ray &r) const;
    Color3f Power() const;
    Intersection Sample_Le(Photon* photon, int numPhotons, const Point2f &xi,
                      const Point2f &xd, float *pdf) const;
public:
    const Color3f emittedLight;
    const Vector3f wLight;
    Point3f worldCenter;
    float worldRadius;
};

class ProjectionLight: public Light
{
public:
    ProjectionLight(const Transform &t, const Color3f &Le,
                    const std::string &texName, float fov);

    Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                      Vector3f *wi, Float *pdf, Point3f &interPoint) const;

    Color3f Projection(const Vector3f &w) const;

    float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    Color3f Le(const Ray &r) const;

    Color3f Power() const;

    Intersection Sample_Le(Photon* photon, int numPhotons, const Point2f &xi,
                      const Point2f &xd, float *pdf) const;
public:
    std::shared_ptr<QImage> projectionMap;
    const Point3f pLight;
    const Color3f emittedLight;
    Matrix4x4 lightProjection;
    Point2f minPoint;
    Point2f maxPoint;
    float cosTotalWidth;
    float n;
    float f;
};
