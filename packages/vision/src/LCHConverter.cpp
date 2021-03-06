/*
 * Copyright (C) 2010 Robotics at Maryland
 * Copyright (C) 2010 Jonathan Wonders
 * All rights reserved.
 *
 * Author: Jonathan Wonders <jwonders@umd.edu>
 * File:  packages/vision/src/Convert.cpp
 */

// STD Includes
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

// Project Includes
#include "math/include/Math.h"
#include "math/include/Vector3.h"
#include "vision/include/LCHConverter.h"

namespace ram {
namespace vision {

bool LCHConverter::lookupInit = false;

unsigned char LCHConverter::rgb2lchLookup[256][256][256][3] = {{{{0}}}};

// gamma correction factor
static double gamma = 2.2; // sRGB
    
// sRGB transform matrix
static math::Matrix3 rgb2xyzTransform = 
    math::Matrix3(0.4124564, 0.3575761, 0.1804375,
                  0.2126729, 0.7151522, 0.0721750,
                  0.0193339, 0.1191920, 0.9503041);

// NTSC RGB
//static math::Matrix3 rgb2xyzTransform =
//    math::Matrix3(0.6068909,  0.1735011,  0.2003480,
//                  0.2989164,  0.5865990,  0.1144845,
//                  -0.0000000,  0.0660957,  1.1162243);


/**
 * We are currently using values from a 2 degree observer
 * X, Y, Z White Point tristimulus values can be calculated
 * from x, y chromaticity values by setting Y = 1, then X = x/y
 * and Z = (1 - x - y) / y
 */

// D65 illuminant
static double Y = 1;
static double x = 0.31271;
static double y = 0.32902;

// D75 illuminant
// x = 0.29902;
// y = 0.31485;

// C illuminant
//x = 0.31006;
// y = 0.31616;

// calculate xyz tristimulus values
static double X_ref = (x * Y) / y;
static double Y_ref = Y;
static double Z_ref = (Y * (1 - x - y) )/ y;

static double eps = 0.008856; // CIE Standard
static double kappa = 903.3;  // CIE Standard

static double refDenom = (X_ref + (15 * Y_ref) + (3 * Z_ref));
static double u_prime_ref = (4 * X_ref) / refDenom;
static double v_prime_ref = (9 * Y_ref) / refDenom;

void LCHConverter::rgb2xyz(double *r2x, double *g2y, double *b2z)
{
    math::Vector3 rgb(*r2x, *g2y, *b2z);
    math::Vector3 xyz = rgb2xyzTransform * rgb;
    *r2x = xyz[0]; // X
    *g2y = xyz[1]; // Y
    *b2z = xyz[2]; // Z
}

void LCHConverter::xyz2lab(double *x2l, double *y2a, double *z2b)
{
    double X = *x2l / X_ref;
    double Y = *y2a / Y_ref;
    double Z = *z2b / Z_ref;

    double fx = pow(X, 1/3);
    if(X < 0.008856)
        fx = X*(841/108) + (4/29);

    double fy = pow(Y, 1/3);
    if(Y < 0.008856)
        fy = Y*(841/108) + (4/29);

    double fz = pow(Z, 1/3);
    if(Z < 0.008856)
        fz = Z*(841/108) + (4/29);

    *x2l = 116 * fy - 16;   // L*
    *y2a = 500 * (fx - fy); // a*
    *z2b = 200 * (fy - fz); // b*
}

void LCHConverter::xyz2luv(double *x2l, double *y2u, double *z2v)
{
    double X = *x2l, Y = *y2u, Z = *z2v;

    double pxDenom = (X + (15 * Y) + (3 * Z));
    double u_prime = (4 * X) / pxDenom;
    double v_prime = (9 * Y) / pxDenom;

    double yr = Y / Y_ref;

    double L = 0, u = 0, v = 0;

    if(yr > eps)
        L = 116 * pow(yr, .3333) - 16;
    else
        L = kappa * yr;

    u = 13 * L * (u_prime - u_prime_ref);
    v = 13 * L * (v_prime - v_prime_ref);

    *x2l = L*2.55;
    *y2u = u;
    *z2v = v;
}

void LCHConverter::lab2lch_ab(double *l2l, double *a2c, double *b2h)
{
    double a = *a2c;
    double b = *b2h;

    // calculate the chrominance
    double c = sqrt(a*a + b*b);
    // calculate the hue
    double h = atan2(b, a) / math::Math::PI;

    while(h < 0)
        h += 2;
    while(h > 2)
        h -= 2;

    // scale to (0, 255)
    h *= 127.5;

    *a2c = c;
    *b2h = h;
}

void LCHConverter::luv2lch_uv(double *l2l, double *u2c, double *v2h)
{
    double u = *u2c;
    double v = *v2h;

    // calculate the chrominance
    double c = sqrt(u*u + v*v);
    // calculate the hue
    double h = atan2(v, u) / math::Math::PI;

    while(h < 0)
        h += 2;
    while(h > 2)
        h -= 2;

    // scale to (0, 255)
    h *= 127.5;

    *u2c = c;
    *v2h = h;
}

void LCHConverter::invGammaCorrection(double *ch1, double *ch2, double *ch3)
{
    // this is a simplified gamma correction if the RGB system is sRGB
    // this takes in values between [0, 1]
        *ch1 = pow(*ch1, gamma);
    *ch2 = pow(*ch2, gamma);
    *ch3 = pow(*ch3, gamma);
}

void LCHConverter::convert(vision::Image* image)
{
    assert(image->getPixelFormat() == Image::PF_RGB_8 && "Incorrect Pixel Format");

    bool lookupTableAvailable = true;
    if(!lookupInit)
        lookupTableAvailable = loadLookupTable();
    
    unsigned char *data = (unsigned char *) image->getData();

    unsigned int numpixels = image->getWidth() * image->getHeight();
    for(unsigned int pix = 0; pix < numpixels; pix++)
    {
        unsigned char ch1 = data[0];
        unsigned char ch2 = data[1];
        unsigned char ch3 = data[2];

        if (lookupTableAvailable) {
            unsigned char *tablePos = rgb2lchLookup[ch1][ch2][ch3];
            ch1 = tablePos[0];
            ch2 = tablePos[1];
            ch3 = tablePos[2];
        } else {
            convertPixel(ch1, ch2, ch3);
        }

        data[0] = ch1;
        data[1] = ch2;
        data[2] = ch3;

        data += 3;
    }
}

void LCHConverter::convertPixel(unsigned char &r,
                                unsigned char &g,
                                unsigned char &b)
{
    double ch1 = (double) r / 255;
    double ch2 = (double) g / 255;
    double ch3 = (double) b / 255;

    invGammaCorrection(&ch1, &ch2, &ch3);
    rgb2xyz(&ch1, &ch2, &ch3);
    xyz2luv(&ch1, &ch2, &ch3);
    luv2lch_uv(&ch1, &ch2, &ch3);

    r = ch1;
    g = ch2;
    b = ch3;
}

void LCHConverter::createLookupTable(bool verbose)
{
    static unsigned char lookup[256][256][256][3];
    
    int counter = 0;
    int size = 256 * 256 * 256;
    for(int c1 = 0; c1 < 256; c1++){
        for(int c2 = 0; c2 < 256; c2++){
            for(int c3 = 0; c3 < 256; c3++){
                unsigned char ch1 = c1, ch2 = c2, ch3 = c3;
                convertPixel(ch1, ch2, ch3);
                                
                lookup[c1][c2][c3][0] = ch1;
                lookup[c1][c2][c3][1] = ch2;
                lookup[c1][c2][c3][2] = ch3;
            }
            if (verbose)
                std::cout << "\r" << 256*counter << " / " << size;
            counter++;
        }
    }
    if (verbose)
        std::cout << std::endl;
    saveLookupTable((char*)&(lookup[0][0][0][0]));
}

void LCHConverter::saveLookupTable(const char *data)
{
    std::ofstream lookupFile;
    std::string baseDir(getenv("RAM_SVN_DIR"));
    lookupFile.open((baseDir + "/rgb2luvLookup.bin").c_str(),
                    std::ios::out | std::ios::binary);
    if(lookupFile.is_open()){   
        lookupFile.write(data, 256*256*256*3);
    } else {
        std::cerr << "Error opening file for output." << std::endl;
    }
}

bool LCHConverter::loadLookupTable()
{
    std::ifstream lookupFile;
    char *data = (char *) &rgb2lchLookup[0][0][0][0];
    std::string baseDir(getenv("RAM_SVN_DIR"));
    lookupFile.open((baseDir + "/rgb2luvLookup.bin").c_str(),
                    std::ios::in | std::ios::binary);
    
    if (lookupFile.is_open()) {
        lookupFile.seekg(0, std::ios::beg);
        lookupFile.read(data, 256*256*256*3);
        lookupInit = true;
        return true;
    } else {
        return false;
    }
}

} // namespace vision
} // namespace ram
