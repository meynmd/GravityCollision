/*
 * params.h
 *
 * particle system parameters
 */

#ifndef PARTICLE_PARAMS
#define PARTICLE_PARAMS
#define NUMT 8

//#define PARTICLE_GROUP_SIZE 128
//#define PARTICLE_GROUP_COUNT 20
//#define PARTICLE_COUNT PARTICLE_GROUP_SIZE * PARTICLE_GROUP_COUNT

const char* VSHADER_FILE = "vShader.txt";
const char* FSHADER_FILE = "fShader.txt";

// particle parameters:
unsigned int PARTICLE_COUNT;

const float XCENTER1 = { 100.f };
const float YCENTER1 = { 100.f };
const float ZCENTER1 = { 0.f };

const float XCENTER2 = { -100.f };
const float YCENTER2 = { -100.f };
const float ZCENTER2 = { 0.f };

const float XCENTER3 = { -100.f };
const float YCENTER3 = { 200.f };
const float ZCENTER3 = { 0.f };

const float RMAX = { 100. };
const float LOCAL_RMIN = { 10. };
const float LOCAL_RMAX = { 25. };

const int MAXLOCALPARTICLES = { 128 };

const float VMIN = { -50. };
const float VMAX = { 50. };


#endif
