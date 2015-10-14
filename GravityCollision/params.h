/*
 * params.h
 *
 * particle system parameters
 */

#ifndef PARTICLE_PARAMS
#define PARTICLE_PARAMS
#define NUMT 8

const char* VSHADER_FILE = "vShader.txt";
const char* FSHADER_FILE = "fShader.txt";

unsigned int PARTICLE_COUNT;

// particle parameters:

const float TIMESTEP_FACT = 0.0001f;

#endif
