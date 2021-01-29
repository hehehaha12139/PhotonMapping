#pragma once
#include <scene/geometry/shape.h>
#include "warpfunctions.h"
#include "samplers/sampler.h"
#include "scene/photon.h"

//A SquarePlane is assumed to have a radius of 1 and a center of <0,0,0>.
//These attributes can be altered by applying a transformation matrix to the SquarePlane.
class SquarePlane : public Shape
{
public:
    virtual bool Intersect(const Ray &ray, Intersection *isect) const;
    virtual Point2f GetUVCoordinates(const Point3f &point) const;
    virtual void ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const;

    // Since ellipsoids should technically be a different
    // class of Shape than SquarePlane, you may assume that a SquarePlane
    // has been scaled uniformly when computing its surface area.
    virtual float Area() const;

    // Sample a point on the surface of the shape and return the PDF with
    // respect to area on the surface.
    virtual Intersection Sample(const Point2f &xi, Float *pdf) const;

    void create();

    Vector3f SampleDirection(const Point2f &xd, float *pdf) const;
    virtual void Sample_Photon(Photon &inPhoton, std::shared_ptr<Sampler> sampler, float &pdf) const
    {
        // TODO: is direction transformation correct? do we need normalize?
        Point2f xi = sampler->Get2D();
        Point2f xd = sampler->Get2D();
        Vector3f wi = WarpFunctions::squareToHemisphereCosine(xd);
        pdf = WarpFunctions::squareToHemisphereCosinePDF(wi);
        glm::vec4 localPoint = glm::vec4(xi[0]-0.5f, xi[1]-0.5f, 0.f, 1.f);
        Point3f worldPoint = glm::vec3(transform.T() * localPoint);
        Vector3f worldDir = glm::normalize(glm::vec3(transform.T() * glm::vec4(wi, 0.f)));
        inPhoton.pos = worldPoint;
        inPhoton.wi = worldDir;
    }
};
