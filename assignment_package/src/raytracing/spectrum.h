#pragma once
#ifndef SPECTRUM_H
#define SPECTRUM_H
#include "globals.h"
#include <vector>

class Spectrum
{
public:
    Spectrum(int _nSamples, float v = 0.0f);
    // Calculation
    Spectrum& operator+=(const Spectrum &s2);
    Spectrum operator+(const Spectrum &s2) const;
    Spectrum& operator-=(const Spectrum &s2);
    Spectrum operator-(const Spectrum &s2) const;
    Spectrum& operator*=(const Spectrum &s2);
    Spectrum operator*(const Spectrum &s2) const;
    Spectrum& operator/=(const Spectrum &s2);
    Spectrum operator/(const Spectrum &s2) const;

    // Test if it is black
    bool IsBlack() const;

    // Friend functions
    friend Spectrum Sqrt(const Spectrum &s);
    friend Spectrum operator*(const float coef, const Spectrum &s);
    friend Spectrum operator/(const float coef, const Spectrum &s);

    // Lerp two spectrum
    inline Spectrum Lerp(float t, const Spectrum &s1, Spectrum &s2);

    // Clamp a spectrum
    Spectrum Clamp(float low = 0, float high = FLT_MAX);

    // Have nan?
    bool HasNaNs() const;

    int nSamples;
    std::vector<float> c;
};

// Some const value for sampled spectrum
static const int sampledLambdaStart = 400;
static const int sampledLambdaEnd = 700;
static const int nSpectralSamples = 30;
static const int nCIESamples = 471;
extern const float CIE_X[nCIESamples];
extern const float CIE_Y[nCIESamples];
extern const float CIE_Z[nCIESamples];
extern const float CIE_lambda[nCIESamples];

inline void XYZToRGB(const Vector3f &xyz, Color3f &rgb)
{
    rgb.x = 3.240479f * xyz.x - 1.537150f * xyz.y - 0.498535f * xyz.z;
    rgb.y = -0.969256f * xyz.x + 1.875991f * xyz.y + 0.041556f * xyz.z;
    rgb.z = 0.055648f * xyz.x - 0.204043f * xyz.y + 1.057311f * xyz.z;
}

inline void RGBToXYZ(const Color3f &rgb, Vector3f &xyz)
{
   xyz.x = 0.412453f * rgb.x + 0.357580f * rgb.y + 0.180423f * rgb.z;
   xyz.y = 0.212671f * rgb.x + 0.715160f * rgb.y + 0.072169f * rgb.z;
   xyz.z = 0.019334f * rgb.x + 0.119193f * rgb.y + 0.950227f * rgb.z;
}


class SampledSpectrum: public Spectrum
{
public:
    SampledSpectrum(float v = 0.0f);
    static SampledSpectrum FromSampled(const float *lambda, const float *v, int n);
    static void Init()
    {
        for(int i = 0; i < nSpectralSamples; ++i)
        {
            float t0 = float(i) / float(nSpectralSamples);
            float t1 = float(i + 1) / float(nSpectralSamples);
            float wl0 = (1.0f - t0) * sampledLambdaStart + t0 * sampledLambdaEnd;
            float wl1 = (1.0f - t1) * sampledLambdaStart + t1 * sampledLambdaEnd;
            X.c.at(i) = AverageSpectrumSamples(CIE_lambda, CIE_X, nCIESamples, wl0, wl1);
            Y.c.at(i) = AverageSpectrumSamples(CIE_lambda, CIE_Y, nCIESamples, wl0, wl1);
            Z.c.at(i) = AverageSpectrumSamples(CIE_lambda, CIE_Z, nCIESamples, wl0, wl1);
            yint += Y.c.at(i);
        }
    }

    void ToXYZ(Vector3f &xyz) const
    {
        xyz.x = 0.0f;
        xyz.y = 0.0f;
        xyz.z = 0.0f;
        for(int i = 0; i < nSpectralSamples; ++i)
        {
            xyz.x += X.c.at(i) * c.at(i);
            xyz.y += Y.c.at(i) * c.at(i);
            xyz.z += Z.c.at(i) * c.at(i);
        }
        xyz.x /= yint;
        xyz.y /= yint;
        xyz.z /= yint;
    }

    void ToRGB(Color3f &rgb) const
    {
        Vector3f xyz;
        ToXYZ(xyz);
        XYZToRGB(xyz, rgb);
    }

    float y() const
    {
        float yy = 0.0f;
        for(int i = 0; i < nSpectralSamples; i++)
        {
            yy += Y.c.at(i) * c.at(i);
        }
        return yy / yint;
    }
private:
    static bool SpectrumSampledSorted(const float *lambda, const float *v, int n);
    static void SortSpectrumSamples(std::vector<float> &lambda, std::vector<float> &v, int n);
    static void quickSortLambda(std::vector<float> &lambda, std::vector<float> &v, int l, int r);
    static float AverageSpectrumSamples(const float *lambda, const float *vals, int n,
                                        float lambdaStart, float lambdaEnd);
    static SampledSpectrum X, Y, Z;
    static float yint;
};

class RGBSpectrum : public Spectrum
{

};

void swap(std::vector<float> &vec, int ori, int dest);

#endif // SPECTRUM_H
