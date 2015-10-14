/*
 * particle simulator header
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

using namespace std;

#include "includes/vermilion.h"
#include "includes/vgl.h"
#include "includes/vapp.h"
#include "includes/vutils.h"
#include "includes/vbm.h"
#include "includes/vmath.h"
#include <omp.h>

#include "params.h"

#define check_gl_error() _check_gl_error(__FILE__,__LINE__)

#define ESC 27
#define TAB 9

// constants:

const double G         		= 6.67300e-11;	// m^3 / ( kg s^2 )
const double EARTH_MASS		= 5.9742e24;	// kg
const double EARTH_DIAMETER	= 12756000.32;	// meters
const float TIMESTEP_FACT = 0.0001f;

const int LEFT = { 4 };
const int MIDDLE = { 2 };
const int RIGHT = { 1 };

const float TRANSFACT = 0.5;
const float ANGFACT = 0.75f;
const float SCALEFACT = 0.0002f;
const float MINSCALE = 0.001f;

GLuint posBuf;
GLuint colBuf;

vmath::vec4* positions;
vmath::vec4* velocities;
vmath::vec4* forces;
vmath::vec4* newPos;
vmath::vec4* newVel;

size_t particleCount;

float aspect_ratio;
float xTrans, yTrans, zTrans;
float xRot, yRot;
float scaleAmt;
int mouseX, mouseY;
int activeButton;
char activeKey;

// program functions forward declarations
static vmath::vec3 random_vector( float minmag, float maxmag);
void _check_gl_error( const char *file, int line );
static inline float random_float();
float Ranf( float low, float high );
int Ranf( int ilow, int ihigh );

float GetDistanceSquared( vmath::vec4 *bi, vmath::vec4 *bj );
float GetUnitVector( vmath::vec4 *from, vmath::vec4 *to, float *ux, float *uy, float *uz );
void simulate( float );
void mouseButton(int, int, int, int);
void mouseMove( int x, int y );
void keyboard( unsigned char c, int x, int y );
void writeToFile();

BEGIN_APP_DECLARATION(ParticleSimulator)

// Override functions from base class
virtual void Initialize(const char * title);
virtual void Display(bool auto_redraw);
virtual void Finalize(void);
virtual void Reshape(int width, int height);

END_APP_DECLARATION()
