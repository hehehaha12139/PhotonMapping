#include "shape.h"
#include <QDateTime>

pcg32 Shape::colorRNG = pcg32(QDateTime::currentMSecsSinceEpoch());


void Shape::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t;
}

Intersection Shape::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    //TODO
    Intersection result = Sample(xi, pdf);
    Vector3f wi = ref.point - result.point;

    if(fequal(glm::length(wi), 0.0f))
    {
        // If intersetcion is light itself
        *pdf = 0;
    }
    else
    {
        // Change pdf from area based to solid angle based
        wi = glm::normalize(wi);
        *pdf *= glm::length2(ref.point - result.point) / AbsDot(result.normalGeometric, wi);

       if (std::isinf(*pdf))
       {
           *pdf = 0.f;
       }
    }
    return result;
}
