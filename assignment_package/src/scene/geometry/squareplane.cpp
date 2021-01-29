#include "squareplane.h"
#include "warpfunctions.h"

float SquarePlane::Area() const
{
    //TODO
    Vector3f scale = this->transform.getScale();
    return scale.x * scale.y;
}

bool SquarePlane::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the square
    if(t > 0 && P.x >= -0.5f && P.x <= 0.5f && P.y >= -0.5f && P.y <= 0.5f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void SquarePlane::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
        CoordinateSystem(*nor, tan, bit);
}


Point2f SquarePlane::GetUVCoordinates(const Point3f &point) const
{
    return Point2f(point.x + 0.5f, point.y + 0.5f);
}

Intersection SquarePlane::Sample(const Point2f &xi, Float *pdf) const
{
    // Shift sample point to squarePlane
    float x = (xi.x - 0.5f);
    float y = (xi.y - 0.5f);

    Intersection result;

    // Get sampled point
    Point3f samplePoint = glm::vec3(this->transform.T() * glm::vec4(x, y, 0.0f, 1.0f));

    result.point = samplePoint;

    result.normalGeometric = glm::normalize(transform.invTransT() * Normal3f(0,0,1));


    *pdf = 1 / Area();

    return result;
}

Vector3f SquarePlane::SampleDirection(const Point2f &xd, float *pdf) const
{
    Point3f wi = WarpFunctions::squareToHemisphereCosine(xd);
    *pdf = WarpFunctions::squareToHemisphereUniformPDF(wi);
    Vector3f wiW = glm::normalize(glm::vec3(transform.T() * glm::vec4(wi, 0.0f)));
    return wiW;
}

