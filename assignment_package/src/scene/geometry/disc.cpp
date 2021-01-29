#include "disc.h"
#include "warpfunctions.h"

float Disc::Area() const
{
    //TODO
    Vector3f scale = transform.getScale();
    return Pi * 0.5f * scale.x * 0.5f * scale.y;
}

bool Disc::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the disc (not bothering to take the sqrt of the dist b/c we know the radius)
    float dist2 = (P.x * P.x + P.y * P.y);
    if(t > 0 && dist2 <= 1.f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void Disc::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * glm::normalize(P));
    CoordinateSystem(*nor, tan, bit);
}


Point2f Disc::GetUVCoordinates(const Point3f &point) const
{
    return glm::vec2((point.x + 1)/2.f, (point.y + 1)/2.f);
}

Intersection Disc::Sample(const Point2f &xi, Float *pdf) const
{
    // Shift sample point to squarePlane
    Point3f discSample = WarpFunctions::squareToDiskConcentric(xi);
    float x = discSample.x / 2.0f;
    float y = discSample.y / 2.0f;

    Intersection result;

    // Get sampled point
    Point3f samplePoint = glm::vec3(this->transform.T() * glm::vec4(x, y, 0.0f, 1.0f));

    result.point = samplePoint;

    result.normalGeometric = glm::normalize(transform.invTransT() * Normal3f(0,0,1));


    *pdf = 1 / Area();

    return result;
}
