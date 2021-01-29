#pragma once
#include <la.h>
#include <globals.h>

class Photon
{
public:
    Point3f pos;
    Color3f color;
    Vector3f wi;

    Photon():
        pos(Point3f(0.0f)), color(Color3f(0.0f)), wi(Vector3f(0.0f)) {}

    Photon(Point3f p, Color3f c, Vector3f w)
        : pos(p), color(c), wi(w)
    {}
};
