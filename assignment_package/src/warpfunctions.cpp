#define _USE_MATH_DEFINES
#include "warpfunctions.h"
#include <math.h>
#include "globals.h"

glm::vec3 WarpFunctions::squareToDiskUniform(const glm::vec2 &sample)
{
    //TODO
    float radius = sqrt(sample.x);
    float angle = 2 * Pi * sample.y;
    return glm::vec3(radius * cos(angle), radius * sin(angle), 0.0f);
}

glm::vec3 WarpFunctions::squareToDiskConcentric(const glm::vec2 &sample)
{
    //TODO
    float a = 2 * sample.x - 1;
    float b = 2 * sample.y - 1;
    float fi, r, u, v;

    if(a > -b)
    {
        if(a > b)
        {
            // Region 1, |a| > |b|
            r = a;
            fi = (Pi / 4) * (b / a);
        }
        else
        {
            // Region 2, |b| > |a|
            r = b;
            fi = (Pi / 4) * (2 - (a / b));
        }
    }
    else
    {
        if(a < b)
        {
            // Region 3, |a| >= |b|, a != 0
            r = -a;
            fi = (Pi / 4) * (4 + b / a);
        }
        else
        {
            // Region 4, |a| <= |b|
            r = -b;
            if(b != 0)
            {
                fi = (Pi / 4) * (6 - (a / b));
            }
            else
            {
                fi = 0;
            }
        }
    }
    u = r * cos(fi);
    v = r * sin(fi);
    return glm::vec3(u, v, 0.0f);
}

float WarpFunctions::squareToDiskPDF(const glm::vec3 &sample)
{
    //TODO
    return 1 / Pi;
}

glm::vec3 WarpFunctions::squareToSphereUniform(const glm::vec2 &sample)
{
    //TODO
    float fi = 2 * Pi * sample.y;
    float z = 1.0f - 2.0f * sample.x;
    float r = sqrt(fmax(0.0f, 1.0f - pow(z, 2.0f)));
    float x = r * cos(fi);
    float y = r * sin(fi);
    return glm::vec3(x, y, z);
}

float WarpFunctions::squareToSphereUniformPDF(const glm::vec3 &sample)
{
    //TODO
    return 1 / (4 * Pi);
}

glm::vec3 WarpFunctions::squareToSphereCapUniform(const glm::vec2 &sample, float thetaMin)
{
    //TODO
    float fi = 2 * Pi * sample.y;
    float z = 0;
    if(thetaMin > 90)
    {
        thetaMin = 180 - thetaMin;
        z = sample.x * (2 * thetaMin / 180.0f) - 1;
    }
    else
    {
        z = 1 - sample.x * (2 * thetaMin / 180.0f);
    }
    float r = sqrt(fmax(0.0f, 1.0f - pow(z, 2.0f)));
    float x = r * cos(fi);
    float y = r * sin(fi);
    return glm::vec3(x, y, z);
}

float WarpFunctions::squareToSphereCapUniformPDF(const glm::vec3 &sample, float thetaMin)
{
    // TODO
    float theta = Pi * (1.0f - thetaMin / 180.0f);
    return 1.0f / (2 * Pi * (1.0f - glm::cos(theta)));
}

glm::vec3 WarpFunctions::squareToHemisphereUniform(const glm::vec2 &sample)
{
    //TODO
    float fi = 2 * Pi * sample.y;
    float z = sample.x;
    float r = sqrt(fmax(0.0f, 1.0f - pow(z, 2.0f)));
    float x = r * cos(fi);
    float y = r * sin(fi);
    return glm::vec3(x, y, z);
}

float WarpFunctions::squareToHemisphereUniformPDF(const glm::vec3 &sample)
{
    //TODO
    return 1 / (2 * Pi);
}

glm::vec3 WarpFunctions::squareToHemisphereCosine(const glm::vec2 &sample)
{
    //TODO
    glm::vec3 diskSample = squareToDiskConcentric(sample);
    float z = sqrt(1 - pow(diskSample.x, 2) - pow(diskSample.y, 2));
    return glm::vec3(diskSample.x, diskSample.y, z);
}

float WarpFunctions::squareToHemisphereCosinePDF(const glm::vec3 &sample)
{
    //TODO
    float length=sqrt(pow(sample.x, 2.0f) + pow(sample.y, 2.0f) + pow(sample.z, 2.0f));
    if(length != 0)
    {
        float cosine = fabs(sample.z) / length;
        return cosine / Pi;
    }
    else
    {
        return 0;
    }

}
