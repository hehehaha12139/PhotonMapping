#include "light.h"
#include "warpfunctions.h"

Color3f Light::Le(const Ray &r) const
{
    return Color3f(0.f);
}

Color3f PointLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                              Vector3f *wi, Float *pdf, Point3f &interPoint) const
{
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.0f;
    interPoint = pLight;
    return emittedLight / glm::length2(pLight - ref.point);
}

float PointLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return 0.0f;
}

Color3f PointLight::Le(const Ray &r) const
{
    return Color3f(0.0f);
}

Color3f PointLight::Power() const
{
    return 4 * Pi * emittedLight;
}

Intersection PointLight::Sample_Le(Photon* photon, int numPhotons, const Point2f &xi,
                              const Point2f &xd, float *pdf) const
{
    Point3f ori = this->pLight;
    Point3f wi = WarpFunctions::squareToHemisphereUniform(xd);
    *pdf = WarpFunctions::squareToHemisphereUniformPDF(wi);
    Vector3f wiW = glm::normalize(glm::vec3(transform.T() * glm::vec4(wi, 0.0f)));
    photon->pos = ori;
    photon->wi = wiW;
    photon->color = emittedLight / (numPhotons * *pdf);
    return Intersection();
}

Color3f SpotLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                             Vector3f *wi, Float *pdf, Point3f &interPoint) const
{
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.f;
    interPoint = pLight;

    return emittedLight * fallOff(-*wi) / glm::length2(pLight - ref.point);
}

float SpotLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return 0.0f;
}

Color3f SpotLight::Le(const Ray &r) const
{
    return Color3f(0.0f);
}

float SpotLight::fallOff(const Vector3f &w) const
{
    Vector3f wl = Vector3f(glm::normalize(transform.invT() * glm::vec4(w, 0.0f)));
    float cosTheta = wl.z;
    if(cosTheta < cosTotalWidth) return 0;
    if(cosTheta > cosFallOffStart) return 1;
    float delta = (cosTheta - cosTotalWidth) / (cosFallOffStart - cosTotalWidth);
    return powf(delta, 4.0f);
}

Color3f SpotLight::Power() const
{
    return emittedLight * 2.0f * Pi * (1 - .5f * (cosFallOffStart + cosTotalWidth));
}

Intersection SpotLight::Sample_Le(Photon* photon, int numPhotons, const Point2f &xi,
                                     const Point2f &xd, float *pdf) const
{
    Point3f ori = this->pLight;
    Point3f wi = WarpFunctions::squareToHemisphereUniform(xd);
    *pdf = WarpFunctions::squareToHemisphereUniformPDF(wi);
    Vector3f wiW = glm::normalize(glm::vec3(transform.T() * glm::vec4(wi, 0.0f)));
    photon->pos = ori;
    photon->wi = wiW;
    photon->color = emittedLight * fallOff(wiW) / (numPhotons * *pdf);
    return Intersection();
}

Color3f DistantLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                Vector3f *wi, Float *pdf, Point3f &interPoint) const
{
    *wi = wLight;
    *pdf = 1;
    Point3f pOutSide = ref.point + wLight * (2 * worldRadius);
    interPoint = pOutSide;
    return emittedLight /  glm::length2(this->transform.position() - ref.point);
}

float DistantLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return 0.0f;
}

Color3f DistantLight::Le(const Ray &r) const
{
    return Color3f(0.0f, 0.0f, 0.0f);
}

Color3f DistantLight::Power() const
{
    return emittedLight * worldRadius * worldRadius * Pi;
}

Intersection DistantLight::Sample_Le(Photon* photon, int numPhotons, const Point2f &xi,
                                const Point2f &xd, float *pdf) const {}

ProjectionLight::ProjectionLight(const Transform &t, const Color3f &Le,
                const std::string &texName, float fov):
    Light(t), emittedLight(Le), pLight(t.position())
{
    QString mapFilePath = QString::fromStdString(texName);
    projectionMap = std::make_shared<QImage>(mapFilePath);
    float aspect = (float)projectionMap->width() / (float)projectionMap->height();
    if(aspect > 1.0f)
    {
        minPoint = Point2f(-aspect, -1.0f);
        maxPoint = Point2f(aspect, 1.0f);
    }
    else
    {
        minPoint = Point2f(-1.0f, -1.0f / aspect);
        maxPoint = Point2f(1.0f, 1.0 / aspect);
    }
    n = 1e-1f;
    f = 1e3f;
    float invTanAng = 1 / std::tan(glm::radians(fov) / 2);
    Matrix4x4 persp = Matrix4x4(1, 0,           0,              0,
                                0, 1,           0,              0,
                                0, 0, f / (f - n), -f*n / (f - n),
                                0, 0,           1,              0);
    Matrix4x4 scale = Matrix4x4(1/invTanAng, 0,           0,              0,
                                0          , 1/invTanAng, 0,              0,
                                0          , 0          , 1,              0,
                                0          , 0          , 0,              1);

    lightProjection = scale * persp;

    float opposite = tan(glm::radians(fov) / 2.0f);
    float tanDiag = opposite * std::sqrt(1.0f + 1.0f /(aspect * aspect));
    cosTotalWidth = std::cos(std::atan(tanDiag));
}

Color3f ProjectionLight::Projection(const Vector3f &w) const
{
    Vector3f wl = Vector3f(transform.invT() * glm::vec4(w, 0.0f));

    if(wl.z < n) return Color3f(0.0f);

    Point3f p = Point3f(lightProjection * glm::vec4(wl.x, wl.y, wl.z, 1.0f));
    if(!(p.x > minPoint.x && p.x < maxPoint.x && p.y > minPoint.y && p.y < maxPoint.y))
    {
        return Color3f(0.0f);
    }
    if(!projectionMap)
        return Color3f(1.0f);

    Vector2f o = Point2f(p) - minPoint;
    if (maxPoint.x > minPoint.x) o.x /= maxPoint.x - minPoint.x;
    if (maxPoint.y > minPoint.y) o.y /= maxPoint.y - minPoint.y;

    Point2f st = Point2f(o);
    st = Point2f(st.x * projectionMap->width(), st.y * projectionMap->height());
    QColor re = projectionMap->pixel(st.x, st.y);
    return Color3f(re.red(), re.green(), re.blue());
}

Color3f ProjectionLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                             Vector3f *wi, Float *pdf, Point3f &interPoint) const
{
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.f;
    interPoint = pLight;
    Color3f proj = Projection(-*wi);
    return emittedLight * proj / glm::length2(pLight - ref.point);
}

float ProjectionLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return 0.0f;
}

Color3f ProjectionLight::Le(const Ray &r) const
{
    return Color3f(0.0f);
}

Color3f ProjectionLight::Power() const
{
    return Color3f(0.0f);
}

Intersection ProjectionLight::Sample_Le(Photon *photon, int numPhotons,
                                   const Point2f &xi, const Point2f &xd, float *pdf) const
{}



