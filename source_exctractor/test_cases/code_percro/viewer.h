#ifndef VIEWER_H
#define VIEWER_H
#pragma once

#include "types.hpp"
/**
 * Material Specifier describing the common scalar properties obtained from external reference
 * mass_density rho kg/m^2
 * weft stretch C_1111 N/m
 * warp stretch C_2222 N/m
 * shear modulus C_1212 N/m
 * transverse contraction C_1122 N/m
 * B_1 Nm waft bend
 * B_2 Nm warp bend
 * Thickness m
 * compression energy J/m^2
 * friction
 */
struct material_t
{
	const char * name;

    Scalar mass_density, weft_stretch, warp_stretch, shear_modulus, transverse_contraction, weft_bend, warp_bend, thickness, compression, friction;
};

extern material_t wool, acetate, polyester, shear_res, bend_res;

struct EnumDescriptor
{
    int value;
    const char * name;
};

namespace TestMode
{
    enum Enum { NoTest, Catenaria, FreeSpace, Cylinder, Swing, FreeFall, CenterPick, BallFall, CenterPickBall, TestCount };
}
extern EnumDescriptor  TestModeDescriptor[];

class Cloth;

template <class T>
class ConfigVar
{
public:
    ConfigVar(T * p = 0): pv(p) {}

    bool isvalid() const { return pv != 0;}

    ConfigVar & operator = (T f) { *pv = f; }

    operator T() { return *pv; }

    T * pv;
};


struct Configuration
{
    struct Listener
    {
        virtual void onItemChanged(const std::string & name) = 0;
    };

    Listener *listener;

    Configuration() : allocateOnly(false),listener(0) {}
    std::string filename;
    //Scalar damping;
    //Scalar tollerance;
    //Scalar sphereRadius;
    //int numIterations;
    material_t Material;
    //int W,H;
    //Scalar dx,dy;
    //bool gpuMode;
    //bool mortonReorder;
    //int parallelLevel;
    //int frameSkip;
    //bool collisionbasedTimeStep;
    //bool wireframe;
    //bool nographics;
    //bool collisionByTriangle;
    //bool gridLayout;
    //bool haptic; //HI attached?
    //Scalar stopTime;
    //bool logging; // full logging, not implemented
    //TestMode::Enum testMode;
    //Scalar gravity,defGravity;
    void (*applyExternalForce)(Cloth &);
    //Scalar sphereStartPosition[3];
    //int hapticinport,hapticoutport;
    //std::string hapticip;
    //Scalar hapticoff[3];
    //Scalar hapticscale;
    //Scalar forceScale;
    //Scalar forceScaleGfx;

#undef DECLARE
#undef DECLAREX
#undef DECLARENO
#define DECLARE(t,n,i,de,u) t n;
#define DECLAREX(t,n,v,i,d,u) t n;
#define DECLARENO(t,n,i,de,u) t n;

#include "selfconfig.h"



    void reset();
    bool basicload(const char * name);
    bool setfield(const std::string & name, std::istream & ins);
    bool getfield(const std::string & name, std::ostream & ons) const;
    void * getfieldptr(const std::string & name) ;
    void save(const char * name);
    bool completeload();
    void updateMaterial();
    void updateTestMode();
    class iterator
    {
    public:
        iterator(Configuration & ac, int aindex) : c(ac), index(aindex) {}
        iterator & operator ++ () { index++; return *this; }
        iterator operator ++ (int) { iterator r(*this); index++; return r; }
        void setValue(const std::string &);
        std::string getValue() const;
        std::string getDescription() const;
        std::string getUnit() const;
        std::string getName() const;
        std::string getType() const;
        bool isReadOnly() const;
        bool operator == (const iterator & it) const { return index == it.index; }
        bool operator != (const iterator & it) const { return index != it.index; }

        Configuration & c;
        int index;
    };

    ConfigVar<Scalar> getfloat(const std::string & name);

    ConfigVar<bool> getbool(const std::string & name);

    ConfigVar<int> getint(const std::string & name);

    iterator findField(const std::string & name);
    iterator begin() { return iterator(*this, 0); }
    iterator end();
    int getFieldCount() const;

    // Build Config
    // DOUBLE_PRECISION
    // QSysInfo

    typedef material_t ** matiterator;
    matiterator beginmat();
    matiterator endmat();
    void updateSphereStart();


public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};


std::ostream & operator << (std::ostream & ons, const Configuration & conf);
std::istream & operator >> (std::istream & ins, Configuration & conf);

void LoadConfigFile(const char* fileName, Configuration &conf);
void DefaultConfig(Configuration & conf);


#endif // VIEWER_H
