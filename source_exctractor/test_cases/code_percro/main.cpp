#include "Cloth.hpp"

#include "viewer.h"
#include <fstream>
#include <string>
#include <sstream>
#include "getopt.h"
#include "statutils.h"

//#define CONSOLE_BUILD

#define HAPTIC_PACKET_SIZE 7
int viennacl_force_platform;

using namespace std;
namespace Eigen
{
int ei_aligned_malloc_size;
}

#ifdef HAS_OPENCL
int selectCLplatformByName(const char * name, cl_platform_id & pid);
#endif


MeanTimer blocks[10];
QTime blockstart;
void pcgStat(bool pre, int level)
{
    if(pre)
    {
        if(level < 0)
        {
            if(blockstart.isNull())
                blockstart.start();
        }
        if(level >= 0)
            blocks[level].pre();
    }
    else
    {
        if(level >= 0)
            blocks[level].post();
        else
        {
            if(blockstart.elapsed() > 2000)
            {
                std::cerr << "PCGStat: " << std::endl;
                for(int i = 0; i < 10; i++)
                {
                    blocks[i].reset();
                    std::cerr << " " << i << " " << blocks[i].mean() << " " << blocks[i].count() << std::endl;
                }
                blockstart.restart();
            }
        }
    }
}

/*
 * Paper Fast Finite Element Solution
 *
 * mass_density rho kg/m^2
 * weft stretch C_1111 N/m Stiramento Trama (Mantes Report is WT)
 * warp stretch C_2222 N/m Stiramento Ordito (Mantes Report is WT)
 * shear modulus C_1212 N/m (rigidità di taglio)
 * transverse contraction C_1122 N/m (???)
 * B_1 Nm weft bend (Mantes Report is B in uNm)
 * B_2 Nm warp bend (Mantes Report is B in uNm)
 * Thickness m
 * compression energy J/m^2
 * friction
 *
 * Mantes Materials: Report Page 54 + 42 e 56 pelli
 * pp 42: density g/m^2
 *
 * LT Linearità della curva carico allungamento
 * WT J/mq = N/m Energia Trazione
 * RT % Resilienza di trazione
 * EMT % Allungamento del provino, a 500gf/cm ovvero 5 N/cm => 5*EMT is weft/warp stretch
 *
 * B uN mq/m = uNm Rigidezza a flessione
 * 2HB mN m/m = J m/m Isteresi di flessione
 *
 * MIU Coefficiente d’attrito
 * MMD Deviazione standard coeff. d’attrito
 * SMD um Rugosità superficiale
 *
 * G N/m deg Rigidezza a taglio a 2.5° (44 mrad) di deformazione
 * 2HG N/m Isteresi di taglio a 0.5° (8.7 mrad) di deformazione
 * 2HG5 N/m Isteresi di taglio a 5° (87 mrad) di deformazione
 *
 * LC Linearità Compressione
 * WC J/mq Energia di deformazione a compressione
 * RC % Resilienza di compressione
 *
 * T mm Spessore (50 N/m^2)
 *
 *
 Missing
 * shear modulus C_1212 N/m (rigidità di taglio)
 * transverse contraction C_1122 N/m (???)

*/
//mass_density, weft_stretch, warp_stretch, shear_modulus (G), transverse_contraction, weft_bend, warp_bend, thickness, compression, friction;
material_t wool =    {"wool",   0.23, 866, 1391, 0.51, 225.7,       0.237e-4, 0.235e-4,    0.005, 0.5,0.5};
material_t wool2 =    {"wool2",   0.23, 1391, 1391, 0.51, 225.7,       0,0,    0.005, 0.5,0.5};
material_t acetate = {"acetate",0.17, 3057, 1534, 1.22, 459.1, 0.055e-4, 0.092e-4, 1.0,0.5};
material_t polyester = {"polyester",0.26, 2400, 3600, 5.23, 600, 0.371e-4, 0.48e-4, 1.0,0.5,0.5};
material_t shear_res = {"shear_res",0.2, 2000, 2000, 1000, 400, 100e-4, 100e-4, 1.0,0.5,0.5};
material_t bend_res = {"bend_res",0.2, 2000, 2000, 5, 400, 100e-4, 100e-4, 1.0,0.5,0.5};

#define TRANSVERSE 225.7

//mass_density, weft_stretch, warp_stretch, shear_modulus (G), transverse_contraction, weft_bend, warp_bend, thickness, compression, friction;
material_t ma_oracle = {"oracle",0.240, 5*2.83,5*4.83,  0.240,TRANSVERSE ,  1.628e-3,1.636e-3, 0.001336,0.851,0.5};
material_t ma_baltic = {"baltic",0.300, 5*33.90,5*20.70,  1.383,TRANSVERSE ,  1.541e-3,2.977e-3, 0.000797,0.310,0.5};
material_t ma_mumbai = {"mumbaiwild",0.055, 6.398,13.826,  1.383,TRANSVERSE ,  2.418e-6, 1.046e-6, 0.000327,0.100,0.5};
material_t ma_relicwild = {"relicwild",0.305, 5*22.89,5*10.81,  0.195,TRANSVERSE ,  14.613e-6,26.896e-6, 0.00115,0.100,0.5};
//material ma_vitello = {"vitello"};
//material ma_suino = {"suino"};

material_t * materials [] = {&wool2,&wool,&acetate,&polyester,&shear_res,&bend_res,&ma_oracle,&ma_baltic,&ma_mumbai,&ma_relicwild,0};
//&ma_vitello,&ma_suino,0};

static int verbose_flag;
static struct option long_options[] =
  {
    {"verbose", no_argument,       &verbose_flag, 1},
    {"state",     required_argument,       0, 's'},
    {"pause",  no_argument,       0, 'p'},
    {"config",  required_argument, 0, 'c'},
    {"define",  required_argument, 0, 'd'},
    {"list",  no_argument, 0, 'l'},
    {"help",  no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };




void help()
{
    std::cout << "Cloth Viewer 2011 ER PT PG @ PERCRO-SSSA\n"
            "Options:\n "
            "\t--verbose (-v)                 shows more stats\n"
            "\t--pause (-p)                   starts paused (press 'p' to continue)\n"
            "\t--config=filename (-c)         load one configuration file (running with no parametrs executes default.cfg)\n"
            "\t--define=name=value (-d)       defines one field\n"
            "\t--list (-l)                    lists fields\n"
            "\t--state=filename (-s)          load a state file (saved by means of 's' key, can be loaded again by 'l')\n"
            "\t--help (-h)                    shows this help\n";
}

Configuration *mconf;
void reloadConfig()
{
    Configuration &  x = *mconf;
}


#if defined(Q_OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
void dbgoutput( const QString& message )
{
   OutputDebugStringW(reinterpret_cast<const WCHAR*>(message.utf16()));
   OutputDebugStringW(L"\n");
}
#endif


int main(int argc, char** argv)
{
    //QDir::addResourceSearchPath(".");


    std::ofstream onf("log");
    char buf[255] = "";
    onf << " PATH:" << getenv("PATH") << std::endl;
    //getcwd(buf,sizeof(buf));
    onf << " CWD:" << buf << std::endl;
    int r;
#ifdef CONSOLE_BUILD
    std::streambuf * pcout = std::cout.rdbuf(onf.rdbuf());
#endif
    std::streambuf * pcerr = std::cerr.rdbuf(onf.rdbuf());
    {
        Configuration conf;
        conf.reset();
        mconf = &conf;
        std::string statefile;
        bool done = true;
        bool paused = false;

        if(argc == 1)
        {
            const char * defname = "default.cfg";
            if(!std::ifstream(defname))
                conf.save(defname);
            else
            {
                done = conf.basicload(defname);
            }
        }
        else
        {
            while (1)
            {
                int option_index = 0;
                int c = getopt_long (argc, argv, "sp:c:d:",long_options, &option_index);
                if(c == -1)
                    break;
                switch(c)
                {
                case 's':
                    if(!statefile.empty())
                    {
                        std::cerr << "Only one state file allowed: " << optarg << std::endl;
                        done = false;
                    }
                    else
                    {
                        statefile = optarg;
                        done = done && conf.basicload(optarg);
                    }
                    break;
                case 'h':
                    help();
                    return  0;
                case 'p':
                    paused = true;
                    break;
                case 'l':
                    std::cout << conf;
                    return 0;
                case 'c':
                    done = done && conf.basicload(optarg);
                    break;
                case 'd':
                    {
                        std::string name = optarg,value;
                        int k = name.find('=');
                        if(k > 0)
                        {
                            value = name.substr(k+1);
                            name = name.substr(0,k);
                        }
                        else
                            value = "1";
                        std::istringstream q(value);
                        done = done && conf.setfield(name,q);
                    }
                    break;
                case 0:
                    std::cerr << "Code Zero" << std::endl;
                    break;
                case '?':
                    std::cerr << "Code " << c << std::endl;
                    break;
                default:
                    std::cerr << "Code " << c << std::endl;
                }
            }
            if (optind < argc)
            {
                for(int i = optind; i < argc; i++)
                    std::cerr << "Unhandled " << argv[i] << std::endl;
            }
        }

        done = done && conf.completeload();
        if(done)
        {
            conf.save("current.cfg");

            if(!statefile.empty())
                conf.allocateOnly = true;
            if(!statefile.empty())
            {
                std::ifstream ins(statefile.c_str(),std::ios::binary);
                // TODO load state
            }

        }
        else
            std::cerr << "Failure in Config loading\n";

    }
#ifdef CONSOLE_BUILD
    std::cout.rdbuf(pcout);
#endif
    std::cerr.rdbuf(pcerr);
    return r;
}

// it was template
int nexthigher(int k) {
        k--;
        for (int i=1; i<sizeof(int)*8; i<<=1)
                k = k | k >> i;
        return k+1;
}


// apply force of 1N to Z only on first iteration
void applyExternalForceFreeSpace(Cloth &c)
{
    if(c.getStep() == 0)
    {
        const Configuration & conf = c.getConfiguration();
        const float amountN = conf.externalForceIntensity;
        Vector3 f = amountN*conf.externalForceDirection;

        // last
        for (int i=0; i < conf.W;i++)
           c.apply_force(conf.H-1, i, f);
    }
    else
        c.clearForces();
}


std::istream & operator >> (std::istream & ins, Eigen::Vector3i & e)
{
    int s1,s2,s3;
    ins >> s1 >> s2 >> s3;
    e = Eigen::Vector3i(s1,s2,s3);
    return ins;
}

std::ostream & operator << (std::ostream & ons, const Eigen::Vector3i & e)
{
    ons << e(0) << ' ' << e(1) << ' ' << e(2);
    return ons;
}

std::istream & operator >> (std::istream & ins, Vector3 & e)
{
    Scalar s1,s2,s3;
    ins >> s1 >> s2 >> s3;
    e = Vector3(s1,s2,s3);
    return ins;
}

std::ostream & operator << (std::ostream & ons, const Vector3 & e)
{
    ons << e(0) << ' ' << e(1) << ' ' << e(2);
    return ons;
}

std::ostream & operator << (std::ostream & ons, const TestMode::Enum & e)
{
    ons << (int)e;
    return ons;
}

std::istream & operator >> (std::istream & ins, TestMode::Enum & e)
{
    int q;
    ins >> q;
    if(ins)
    {
        if(q < 0 || q >= TestMode::TestCount)
            ins.fail();
        else
            e = (TestMode::Enum)q;
    }
    return ins;
}


struct ConfigurationEntry
{
    const char * name;
    const char * type;
    const char * comment;
    const char * unit;
    int writable;
};

std::string sgetenv(const char * name)
{
    const char * cp = getenv(name);
    return cp == 0 ? "" : cp;
}

// useful for diagnostics
ConfigurationEntry configurationTable[] =
{
    #undef DECLARE
    #undef DECLAREX
    #undef DECLARENO
    #define DECLARE(t,n,i,d,u) { #n,#t,d,u,1 },
    #define DECLAREX(t,n,v,i,d,u) { #v,#t,d,u,1 },
    #define DECLARENO(t,n,i,d,u) { #n,#t,d,u,0 },
    #include "selfconfig.h"
    { 0,0,0}
};

void Configuration::iterator::setValue(const std::string &v)
{
    std::istringstream ins(v);
    c.setfield(getName(),ins);
}

std::string Configuration::iterator::getDescription() const
{
    return configurationTable[index].comment;
}

std::string Configuration::iterator::getUnit() const
{
    return configurationTable[index].unit;
}

std::string Configuration::iterator::getType() const
{
    return configurationTable[index].type;
}


std::string Configuration::iterator::getValue() const
{
    std::ostringstream ons;
    c.getfield(getName(),ons);
    ons.flush();
    return ons.str();
}

int Configuration::getFieldCount() const
{
    return sizeof(configurationTable)/sizeof(configurationTable[0])-1;
}

bool Configuration::iterator::isReadOnly() const
{
    return !configurationTable[index].writable;
}


std::string Configuration::iterator::getName() const
{
    return configurationTable[index].name;
}

Configuration::iterator Configuration::end()
{
    return iterator(*this,sizeof(configurationTable)/sizeof(configurationTable[0])-1);
}


void Configuration::reset()
{
#undef DECLARE
#undef DECLAREX
#undef DECLARENO
    #define DECLARE(t,n,i,d,u) n = i;
    #define DECLAREX(t,n,v,i,d,u) n = i;
    #define DECLARENO(t,n,i,d,u)
    #include "selfconfig.h"
    Material = wool2;
    applyExternalForce = 0;

#ifdef DOUBLE_PRECISION
    doublePrecision = true;
#else
    doublePrecision = false;
#endif
#ifdef HAS_OPENMP
    cpuCount = omp_get_max_threads();
#else
    cpuCount = 1;
#endif
    // Processor Architecture
    // /proc/cpuinfo parse and extract:
    //  model name
    //  cpu MHz
    //  cache size
    //  vendor_id
    // count processor number
    // MAC OS X?

    cpuId = sgetenv("PROCESSOR_IDENTIFIER") + " rev:" + sgetenv("PROCESSOR_REVISION");
    if(sgetenv("windir").empty())
    {
        os = "Unix";
        os64 = false;
    }
    else
    {
        os = "Windows";
        os64 = !sgetenv("ProgramW6432").empty();
    }
    build64 = false;
    cpuAffinity = -1;
}

bool Configuration::getfield(const std::string & name, std::ostream &ons) const
{
    if(false)
        return false;
#undef DECLARE
#undef DECLAREX
#undef DECLARENO
#define DECLARE(t,n,i,d,u) else if (name.compare(#n)==0) ons << this->n;
#define DECLAREX(t,n,v,i,d,u) else if (name.compare(#v)==0) ons << this->n;
#define DECLARENO(t,n,i,d,u) else if (name.compare(#n)==0) ons << this->n;
#include "selfconfig.h"
    else
        return false;
    return !ons ? false:true;
}

void *Configuration::getfieldptr(const std::string & name)
{
    if(false)
        return 0;
#undef DECLARE
#undef DECLAREX
#undef DECLARENO
#define DECLARE(t,n,i,d,u) else if (name.compare(#n)==0) return &n;
#define DECLAREX(t,n,v,i,d,u) else if (name.compare(#v)==0) return &n;
#define DECLARENO(t,n,i,d,u) else if (name.compare(#n)==0) return &n;
#include "selfconfig.h"
    else
        return 0;
}

Configuration::iterator Configuration::findField(const std::string & name)
{
    iterator it = begin();
    iterator e = end();
    for(;it != e; ++it)
    {
        if(it.getName() == name)
            return it;
    }
    return e;
}

bool Configuration::setfield(const std::string & name, std::istream &ins)
{
    if(false)
        return false;
#undef DECLARE
#undef DECLAREX
#undef DECLARENO
#define DECLARE(t,n,i,d,u) else if (name.compare(#n)==0) ins >> n;
#define DECLAREX(t,n,v,i,d,u) else if (name.compare(#v)==0) ins >> n;
#define DECLARENO(t,n,i,d,u)
#include "selfconfig.h"
    else
        return false;
    if(name == "material")
        updateMaterial();
    else if(name.size() > 4 && name.substr(0,3) == "mat")
    {
        struct mapping
        {
            const char * name;
            Scalar & to;
            Scalar &from;
        };
        mapping mappings[] =
        {
            {"matdensity",Material.mass_density,matdensity},
            {"matweftstretch",Material.weft_stretch,matweftstretch},
            {"matwarpstretch",Material.warp_stretch,matwarpstretch},
            {"matshear",Material.shear_modulus,matshear},
            {"mattranverse",Material.transverse_contraction,mattranverse},
            {"matweftbend",Material.weft_bend,matweftbend},
            {"matwarpbend",Material.warp_bend,matwarpbend},
            {"matthickness",Material.thickness,matthickness},
        };
        for(int i = 0; i < sizeof(mappings)/sizeof(mappings[0]); i++)
            if(name == mappings[i].name)
            {
                mappings[i].to  = mappings[i].from;
                break;
            }

        if(listener != 0)
        listener->onItemChanged("material");
    }
    else if(name == "hapticprotocol")
    {

        // replace
        switch(hapticprotocol)
        {
        case 1: // GRAB
            hapticdouble = true;
            hapticposindex = Eigen::Vector3i(0,1,2);
            hapticrectimeindex = -1;
            hapticforceindex = Eigen::Vector3i(0,1,2);
            hapticsendtimeindex = 0;
            break;
        case 2: // HANDEXOS
            hapticdouble = false;
            hapticposindex = Eigen::Vector3i(0,1,2);
            haptic2posindex = Eigen::Vector3i(3,4,5);
            hapticrectimeindex = -1;
            hapticforceindex = Eigen::Vector3i(0,1,2);
            haptic2forceindex = Eigen::Vector3i(3,4,5);
            hapticsendtimeindex = -1;
            haptic2 = true;
            break;
        }
    }
    if(!ins && listener)
    {
        listener->onItemChanged(name);
    }

    return !ins ? false:true;
}

#define ENUMENTRY(ns,x) { ns::x, #x},
EnumDescriptor  TestModeDescriptor[]
{
    ENUMENTRY(TestMode,NoTest)
    ENUMENTRY(TestMode,Catenaria)
    ENUMENTRY(TestMode,FreeSpace)
    ENUMENTRY(TestMode,Cylinder)
    ENUMENTRY(TestMode,Swing)
    ENUMENTRY(TestMode,FreeFall)
    ENUMENTRY(TestMode,CenterPick)
    ENUMENTRY(TestMode,CenterPickBall)
    ENUMENTRY(TestMode,BallFall)
    { -1,0}
};


bool Configuration::basicload(const char * fileName)
{
    ifstream input(fileName);
    std::cerr << "Loading "  << fileName << std::endl;
    if(!input)
        return false;
    input >> *this;
    // ops...
    return true;
}

void Configuration::save(const char * name)
{
    std::ofstream onf(name);
    onf << *this;
    onf << std::flush;
}

Configuration::matiterator Configuration::beginmat()
{
    return materials+0;
}

Configuration::matiterator Configuration::endmat()
{
    return materials+sizeof(materials)/sizeof(materials[0]);
}

void Configuration::updateMaterial()
{
    bool found = false;
    for(int i = 0; materials[i]; i++)
        if(material.compare(materials[i]->name)==0)
        {
            Material = *materials[i];
            found = true;

            matdensity = Material.mass_density;
            matweftstretch = Material.weft_stretch;
            matwarpstretch = Material.warp_stretch;
            matweftbend = Material.weft_bend;
            matwarpbend = Material.warp_bend;
            matthickness = Material.thickness;
            matshear = Material.shear_modulus;
            qDebug() << "thickness "<< Material.thickness ;
            break;
        }

    if(!found)
    {
        std::cerr  << "Material not found: " << material << "\n";
    }

}

void Configuration::updateSphereStart()
{
    Vector3 clothCenter(0,0,0);
    if(!gridXZ)
    {
        if(!gridCentered)
        {
            clothCenter(0) = width / 2.0f;
            clothCenter(1) = -height / 2.0f;
        }
    }
    else
    {
        if(!gridCentered)
        {
            clothCenter(0) = width / 2.0f;
            clothCenter(2) = -height / 2.0f;
        }
    }
    clothCenter += gridOffset;

    if(sphereStartPosition[0] == 1E10)
    {
        sphereStartPosition = clothCenter;
        if(gridXZ)
            sphereStartPosition[1] = -sphere2Radius*1.2;
        else
            sphereStartPosition[2] = sphere2Radius*1.2;
    }

    if(sphere2StartPosition[0] == 1E10)
    {
        sphere2StartPosition = clothCenter;
        if(gridXZ)
            sphere2StartPosition[1] = -sphere2Radius*1.2;
        else
            sphere2StartPosition[2] = sphere2Radius*1.2;
    }


    if(hapticoff[0] == 1E10)
    {
        hapticoff[0] = sphereStartPosition[0];
        hapticoff[1] = sphereStartPosition[1];
        hapticoff[2] = sphereStartPosition[2];
    }

    if(haptic)
    {

        //if (InitSendChannel((char*)hapticip.c_str(), hapticoutport, HAPTIC_PACKET_SIZE) != 0)
        //    std::cerr << "Haptic: cannot open send to: " << hapticip << ":" << hapticoutport << " packetsize " << HAPTIC_PACKET_SIZE << std::endl;
        //if (InitRecChannel(hapticinport, HAPTIC_PACKET_SIZE) != 0)
          //  std::cerr << "Haptic: cannot open receive: " << hapticinport << std::endl;
    }
}

void Configuration::updateTestMode()
{
    if(testMode != 0)
    {
        gravity = defGravity;
        sideConstraints = 1;
        gridXZ = false;
        sphereGravity =Vector3(0,0,0);
        sphereMass = 0.0;
    }
    switch(testMode)
    {
        case TestMode::FreeSpace:
            if(sphereCount < 1)
                sphereCount = 1;
            applyExternalForce = applyExternalForceFreeSpace;
            sphereStartPosition[0] = dx*W+sphereRadius*2;
            gravity = 0;
            break;
        case TestMode::Catenaria:
            sphereStartPosition[0] = dx*W+sphereRadius*2;
            if(sphereCount < 1)
                sphereCount = 1;
            break;
        case TestMode::Cylinder:
            // the rest is in another pace
            sphereCount = 1;
            break;
        case TestMode::Swing:
            gridXZ = true;
            sideConstraints = 2;
            break;
        case TestMode::FreeFall:
            gridXZ = true;
            sideConstraints = 0;
            break;
        case TestMode::CenterPick:
            // the rest is done in another place
            gridXZ = true;
            sideConstraints = 0;
            break;
        case TestMode::BallFall:
            gridXZ = true;
            sideConstraints = 15;
            sphereCount = 1;
            sphereRadius = 0.05;
            sphereStartPosition[0] = dx*W/2;
            sphereStartPosition[1] = 10;
            sphereStartPosition[2] = dy*H/2;
            sphereGravity =Vector3(0,-9.1,0);
            sphereMass = 0.1;
            sphereMove = true;
            sphereMoveRealTime = true;

            break;
        case TestMode::CenterPickBall:
            gridXZ = true;
            sideConstraints = 0;
            sphereCount = 1;
            sphereRadius = 0.05;
            sphereStartPosition[0] = dx*W/2;
            sphereStartPosition[1] = -3;
            sphereStartPosition[2] = dy*H/2;
            break;
        default:
          break;
    }
    updateSphereStart();
}

inline void autoparam(int &vertices, Scalar &extent, Scalar &delta)
{
    if(vertices == 0)
    {
        vertices = ceil(extent/delta)+1;
    }
    else if(extent == 0)
    {
        extent = delta*(vertices-1);
    }
    else
    {
        delta = extent/(vertices-1);
    }
}

bool Configuration::completeload()
{
    //gpuVectorSize = GPUVECTORSIZE;
    if(gpuVectorSize != 3 && gpuVectorSize != 4)
        gpuVectorSize = 3;
    cpuVectorSize = VECTORSIZE;

    updateMaterial();

    // width = W*dx
    // cases:
    //  W dx
    //  width W
    //  width dx
    autoparam(W,width,dx);
    autoparam(H,height,dy);


    if((W*H) % 4 != 0)
    {
        std::cerr << "Width x Height has to be multiple of 4\n";
        //return false;
    }

    updateTestMode(); // calls     updateSphereStart();

    std::cerr << "Test Mode is " << testMode << " " << applyExternalForce << std::endl;

    // disable MortonReorder
    switch(vertexOrder)
    {
    case -1: // morton
        if((nexthigher(W) != W || nexthigher(H) != H))
        {
            std::cerr << "Morton Reorder (vertexOrder=-1) requires W and H power of two\n";
            vertexOrder = 1;
        }
        break;
    case 2:
    case 4:
    case 8:
        if((W % vertexOrder) != 0 || (H % vertexOrder) != 0)
        {
            std::cerr << "Vertex Order requires exact power: " << vertexOrder << " switch to classic\n";
            vertexOrder = 1;
        }
        break;
    case 1: break;
    default:
        std::cerr << "Vertex Order not supported: " << vertexOrder << " switch to classic\n";
        vertexOrder = 1;
        break;
    }

#ifndef HAS_OPENCL
    if(gpuMode)
    {
        std::cerr << "No GPU build\n";
        return false;
    }
#else
    if(gpuMode)
    {
        cl_platform_id pid;
        viennacl_force_platform = selectCLplatformByName(openclPlatform.c_str(),pid)+1; // needed
        if(viennacl_force_platform <= 0)
        {
            std::cerr << "OpenCL Platform " << openclPlatform << " not available\n";
            return false;
        }
        doublePrecision = false;

        std::vector<char> xname;
        size_t size =  0;
        clGetPlatformInfo(pid,CL_PLATFORM_NAME,0,0,&size);
        xname.resize(size+1);
        clGetPlatformInfo(pid,CL_PLATFORM_NAME,size,&xname[0],&size);
        openclPlatform = (const char *)&xname[0];


#if defined (__APPLE__) || defined(MACOSX)
   #define GL_SHARING_EXTENSION "cl_APPLE_gl_sharing"
#else
   #define GL_SHARING_EXTENSION "cl_khr_gl_sharing"
#endif

        cl_int SM = 1;
        cl_int wgsize = 0;
        cl_int wisize[3];
        cl_int pref;

        const viennacl::ocl::handle<cl_context> & ctx = viennacl::ocl::current_context().handle();

        size_t ParmDataBytes;
        clGetContextInfo(ctx, CL_CONTEXT_DEVICES, 0, NULL, &ParmDataBytes);
        cl_device_id* GPUDevices = (cl_device_id*)malloc(ParmDataBytes);
        clGetContextInfo(ctx, CL_CONTEXT_DEVICES, ParmDataBytes, GPUDevices, NULL);
        clGetDeviceInfo(GPUDevices[0],CL_DEVICE_MAX_COMPUTE_UNITS,sizeof(cl_int),&SM,0);
        clGetDeviceInfo(GPUDevices[0],CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(cl_int),&wgsize,0);
        clGetDeviceInfo(GPUDevices[0],CL_DEVICE_MAX_WORK_ITEM_SIZES,sizeof(cl_int)*3,wisize,0);
        clGetDeviceInfo(GPUDevices[0],CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,sizeof(cl_int),&pref,0);
        gpuShareGL = 0;

        size_t extensionSize = 0, extensionSize2 = 0;
       clGetDeviceInfo(GPUDevices[0], CL_DEVICE_EXTENSIONS, 0, NULL, &extensionSize );
        if(extensionSize > 0)
        {
            char* extensions = new char[extensionSize];
            clGetDeviceInfo(GPUDevices[0], CL_DEVICE_EXTENSIONS, extensionSize, extensions, &extensionSize2);
            if(extensionSize2 > 0)
            {
                std::string stdDevString(extensions);
                size_t szOldPos = 0;
                size_t szSpacePos = stdDevString.find(' ', szOldPos); // extensions string is space delimited
                while (szSpacePos != stdDevString.npos)
                {
                    if( strcmp(GL_SHARING_EXTENSION, stdDevString.substr(szOldPos, szSpacePos - szOldPos).c_str()) == 0 )
                    {
                        gpuShareGL = 1;
                        break;
                    }
                    do
                    {
                        szOldPos = szSpacePos + 1;
                        szSpacePos = stdDevString.find(' ', szOldPos);
                    }
                    while (szSpacePos == szOldPos);
                }
            }
            delete [] extensions;
        }
        free(GPUDevices);

        gpuCores = SM;
        gpuMaxGroupItems = wgsize;
        gpuMaxItems = wisize[0];
        gpuPreferredVector = pref;

        if(gpuLocalSize > gpuMaxItems)
            gpuLocalSize = gpuMaxItems;

        std::cerr << "GPUVectorSize size is " << gpuVectorSize << std::endl;
    }
#endif
    if(cpuCount < omp_get_max_threads())
        omp_set_num_threads(cpuCount);

    if(collisionMode == 2 && gpuMode)
        collisionMode = 1;
    if(gpuMode && collisionbasedTimeStep)
        collisionbasedTimeStep = false;
    if(profiled && !gpuMode)
    {
        std::cerr << "CPU: profiling not supported in parallelLevel\n";
        parallelLevel = 0;
    }
    if((parallelLevel == 2 || gpuMode) && (pcgAscherPre||pcgAscherInit||pcgAscherThreshold))
    {
        std::cerr << "AscherPre/Init/Bd not supported in Parallel Level 2 or GPU mode\n";
        if(pcgAscherPre)
            pcgAscherPre = 0;
        if(pcgAscherInit)
            pcgAscherInit = 0;
        if(pcgAscherThreshold && !parallelLevel2PCG)
            pcgAscherThreshold = 0;
    }


    if(headless)
    {
        if(stopTime == 0)
        {
            std::cerr << "conf: headless requires stopTime" << std::endl;
            return false;
        }

        if(asyncSimulation)
        {
            std::cerr << "conf: asyncSimulation is meaningless in headless" << std::endl;
            asyncSimulation = false;
        }
        if(stepMode)
        {
            std::cerr << "conf: stepMode is meaningless in headless" << std::endl;
            stepMode = false;
        }
        nographics = true;

    }
    if(friction == -1)
        friction = Material.friction;

    Scalar v = min(dx,dy)*sqrt(Material.mass_density/(Material.warp_stretch*2));
    if(maxTimeStep == -1)
        maxTimeStep = v;
    std::cerr << "Courant check gives: " << v << " maximum dt" << std::endl;
    return true;
}

std::ostream & operator << (std::ostream & onf, const Configuration & conf)
{
    for(int i = 0; configurationTable[i].name; i++)
    {
        if(configurationTable[i].comment[0] !=0)
            onf << '#' << configurationTable[i].comment << '\n';
        onf << configurationTable[i].name << ' ';
        conf.getfield(configurationTable[i].name,onf); // TODO use index
        onf << '\n';
    }
    /*
    onf << "cpuCount " << conf.cpuCount << std::endl;
    onf << "cpuId " << conf.cpuId << std::endl;
    onf << "affinity " << conf.affinity << std::endl;
    onf << "doublePrecision " << conf.doublePrecision << std::endl;
    onf << "os " << conf.os << std::endl;
    onf << "os64 " << conf.os << std::endl;
    onf << "build64 " << conf.build64 << std::endl;
    onf << "gpuCores " << conf.gpuCores << std::endl;
    #define OUT(a,i) onf << a << i << std::endl;
    OUT("gpuMaxItems", conf.gpuMaxItems)
    OUT("gpuMaxGroupItems",conf.gpuMaxGroupItems)
    OUT("gpuPreferredVector",conf.gpuPreferredVector)
    OUT("gpuShareGL",conf.gpuShareGL)
    */
#undef OUT
    return onf;
}

std::istream & operator >> (std::istream & input, Configuration & conf)
{
    string buffer;
    string element;

    while(input)
    {
        buffer[0] = 0;
        getline(input, buffer);
        istringstream line(buffer);
        element.clear();
        line >> element;
        if(element.empty() || element[0] == '#' || element[0] == 0)
            continue;
        if(element[0] == '@')
        {
            std::cerr << ">Sub loading: " << element.substr(1) << std::endl;
            conf.basicload(element.substr(1).c_str());
            std::cerr << ">End Sub\n";
            continue;
        }

        if(element.empty())
            continue;
        else if(element.compare("EOF") == 0)
            break;
        else
        {
            Configuration::iterator it = conf.findField(element);
            if(it != conf.end())
            {
                if(it.isReadOnly())
                    std::cerr << "ReadOnly: " << element << std::endl;
                else
                {
                    if(!conf.setfield(element.c_str(),line))
                                std::cerr << "Configuration Parameter not found: " << element << " in line: " << buffer << std::endl;
                }
            }
            else
            {
                std::cerr  << "NotFound: " << element << " ";
            }
        }
    }
    return input;
}

#ifdef HAS_OPENCL
// name can be: number, "" or name of vendor
int selectCLplatformByName(const char * name, cl_platform_id & pid)
{    
#ifdef HAS_OPENCL
    cl_uint numplats = 0;
    clGetPlatformIDs (0,0, &numplats);
    if(numplats == 0)
        return -1;
    if(name[0] == 0)
        return 0; // first
    std::vector<cl_platform_id> plats(numplats);
    clGetPlatformIDs (numplats, &plats[0], 0);
    int n;
    if(sscanf(name,"%d",&n) == 1)
    {
        if(n <= 0 || n > numplats)
            return -1;
        pid = plats[n-1];
        return n-1;
    }
    int q = strlen(name);
    std::vector<char> xname;
    for(int i = 0; i <numplats; i++)
    {
          size_t size =  0;
          clGetPlatformInfo(plats[i],CL_PLATFORM_NAME,0,0,&size);
          if(size < q)
              continue;
          xname.resize(size+1);
          clGetPlatformInfo(plats[i],CL_PLATFORM_NAME,size,&xname[0],&size);
          std::cerr << "OpenCL Platform: <" << &xname[0] << "> " << i << "/" << numplats << std::endl;
          xname[q] = 0;          
          if(strcmp(&xname[0],name) == 0)
          {
              pid = plats[i];
              return i;
          }
    }
    return -1;
#else
    return -1;
#endif
}
#endif


ConfigVar<Scalar> Configuration::getfloat(const std::string & name)
{
    iterator it = findField(name);
    if(it == end() || it.getName() != "Scalar")
        return ConfigVar<Scalar>((Scalar*)getfieldptr(name));

}

ConfigVar<bool> Configuration::getbool(const std::string & name)
{
    iterator it = findField(name);
    if(it == end() || it.getName() != "bool")
        return ConfigVar<bool>((bool*)getfieldptr(name));

}

ConfigVar<int> Configuration::getint(const std::string & name)
{
    iterator it = findField(name);
    if(it == end() || it.getName() != "int")
        return ConfigVar<int>();
    return ConfigVar<int>((int*)getfieldptr(name));

}


