#pragma once
#ifndef REFLECTIVEMATERIAL_H
#define REFLECTIVEMATERIAL_H
#include "material.h"

class ReflectiveMaterial : public Material
{
public:
    ReflectiveMaterial(const Color3f &Kd, float Eta, float Kk,
                  const std::shared_ptr<QImage> &textureMap,
                  const std::shared_ptr<QImage> &normalMap)
        : kd(Kd), eta(Eta), kk(Kk),
          textureMap(textureMap), normalMap(normalMap)
    {}

    void ProduceBSDF(Intersection *isect) const;


private:
    Color3f kd;                     // Metal color
    float eta, kk;                  // Reflective coefficient

    std::shared_ptr<QImage> textureMap; // The color obtained from this (assuming it is non-null) is multiplied with the base material color.
    std::shared_ptr<QImage> normalMap;
};


#endif // REFLECTIVEMATERIAL_H
