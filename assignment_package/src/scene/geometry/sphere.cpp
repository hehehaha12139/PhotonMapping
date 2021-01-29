#include "sphere.h"
#include <iostream>
#include <math.h>
#include "warpfunctions.h"

float Sphere::Area() const
{
    //TODO later
    Vector3f scale = this->transform.getScale();
    return 4 * Pi * pow(1.0f * scale.x, 2.0f);
}

void Sphere::ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const
{
    *nor = glm::normalize(transform.invTransT() * glm::normalize(P));
    CoordinateSystem(*nor, tan, bit);
}

bool Sphere::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    float A = pow(r_loc.direction.x, 2.f) + pow(r_loc.direction.y, 2.f) + pow(r_loc.direction.z, 2.f);
    float B = 2*(r_loc.direction.x*r_loc.origin.x + r_loc.direction.y * r_loc.origin.y + r_loc.direction.z * r_loc.origin.z);
    float C = pow(r_loc.origin.x, 2.f) + pow(r_loc.origin.y, 2.f) + pow(r_loc.origin.z, 2.f) - 1.f;//Radius is 1.f
    float discriminant = B*B - 4*A*C;
    //If the discriminant is negative, then there is no real root
    if(discriminant < 0){
        return false;
    }
    float t = (-B - sqrt(discriminant))/(2*A);
    if(t < 0)
    {
        t = (-B + sqrt(discriminant))/(2*A);
    }
    if(t >= 0)
    {
        Point3f P = glm::vec3(r_loc.origin + t*r_loc.direction);
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

Point2f Sphere::GetUVCoordinates(const Point3f &point) const
{
    Point3f p = glm::normalize(point);
    float phi = atan2f(p.z, p.x);
    if(phi < 0)
    {
        phi += TwoPi;
    }
    float theta = glm::acos(p.y);
    return Point2f(1 - phi/TwoPi, 1 - theta / Pi);
}

// We don't use this Sample function here
Intersection Sphere::Sample(const Point2f &xi, Float *pdf) const
{}

Intersection Sphere::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    //TODO
    //Intersection result = Sample(xi, pdf);
    Point3f spherePos = this->transform.position();
    Vector3f sampleDir = glm::normalize(ref.point - spherePos);
    Point3f sphereSample = WarpFunctions::squareToHemisphereCosine(xi);

    Vector3f tangent = Vector3f(0.0f);
    Vector3f biTangent = Vector3f(0.0f);
    CoordinateSystem(sampleDir, &tangent, &biTangent);
    glm::mat3 sampleRot = glm::mat3(tangent, biTangent, sampleDir);

    // Rot to Facing Direction
    sphereSample = sampleRot * sphereSample;
    Normal3f nor = sphereSample;
    Intersection result;

    // Transform to World Space
    sphereSample = glm::vec3(this->transform.T() * glm::vec4(sphereSample, 1.0f));
    result.point = sphereSample;
    result.normalGeometric = glm::normalize(transform.invTransT() * nor);

    Vector3f wi = ref.point - result.point;

    if(fequal(glm::length(wi), 0.0f))
    {
        *pdf = 0;
    }
    else
    {
        // Change pdf from area based to direction based
        wi = glm::normalize(wi);
        *pdf = 1 / Area();
        *pdf *= glm::length2(ref.point - result.point) / AbsDot(result.normalGeometric, wi);

       if (std::isinf(*pdf))
       {
           *pdf = 0.f;
       }
    }
    return result;
}
