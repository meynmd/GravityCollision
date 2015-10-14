/*
 * particle gravity simulation
 *
 * by M. Meyn
 *
 * using app framework from OpenGL Programming Guide, Eighth Edition
 * by Dave Shreiner, Graham Sellers, John Kessenich, Bill Licea-Kane
 *
 * physics simulation by J. Parallel
 *
 * This is a graphical adaptation of an N-body problem from Prof. Mike Bailey's class on Parallel
 * Programming at Oregon State University. That problem in turn was inspired by the "colliding
 * galaxies" scene from the movie "Cosmic Voyage."
 */

#include "particles.h"
#include "params.h"

#define STRINGIZE(a) #a


DEFINE_APP( ParticleSimulator, "Compute Shader Particle System" )

GLuint renderShader;
GLuint render_vao;

void ParticleSimulator::Initialize( const char * title )
{
#ifndef _OPENMP
	fprintf( stderr, "OpenMP is not available\n" );
	return;
#endif
	omp_set_num_threads( NUMT );
	int numProcessors = omp_get_num_procs();
	fprintf( stderr, "N-body Gravity Simulation\nHave %d threads available. Using %d.\n\n", numProcessors, NUMT );
	fprintf( stderr, "Left mouse button: Rotate\nRight mouse button: Scale\n\n");
	fprintf( stderr, "Esc to exit\n4 / 6 to rotate 90 degrees about Y axis\n2 / 8 to rotate 90 degrees about X axis\n" );
	fprintf( stderr, "5 to reset rotation\nTab to reset view\n\n" );
	
	// read the initial particle positions and velocities
	size_t numPart2 = 0, numPart1 = 0;
	
	FILE* pFile1 = fopen( "points/in1.pos", "r" );
	FILE* vFile1 = fopen( "points/in1.vel", "r" );
	FILE* pFile2 = fopen( "points/in2.pos", "r" );
	FILE* vFile2 = fopen( "points/in2.vel", "r" );
	
	fseek( pFile1, 0, SEEK_END );
	numPart1 = ftell( pFile1 ) / sizeof( vmath::vec4 );
	fseek( pFile2, 0, SEEK_END );
	numPart2 = ftell( pFile2 ) / sizeof( vmath::vec4 );
	particleCount = numPart1 + numPart2;
	fseek( pFile1, 0, SEEK_SET );
	fseek( pFile2, 0, SEEK_SET );
	
	positions = new vmath::vec4[particleCount];
	velocities = new vmath::vec4[particleCount];
	forces = new vmath::vec4[particleCount];
	newPos = new vmath::vec4[particleCount];
	newVel = new vmath::vec4[particleCount];

	if( fread( positions, sizeof(vmath::vec4), numPart1, pFile1 ) != numPart1 )
	{
		fprintf( stderr, "input file 1: did not read the expected number of positions\n" );
		exit( 1 );
	}
	if( fread( velocities, sizeof(vmath::vec4), numPart1, vFile1 ) != numPart1 )
	{
		fprintf( stderr, "input file 1: did not read the expected number of velocities\n" );
		exit( 1 );
	}
	if( fread(positions + numPart1, sizeof(vmath::vec4), numPart2, pFile2) != numPart2 )
	{
		fprintf( stderr, "input file 2: did not read the expected number of positions\n" );
		exit( 1 );
	}
	if( fread(velocities + numPart1, sizeof(vmath::vec4), numPart2, vFile2) != numPart2 )
	{
		fprintf( stderr, "input file 2: did not read the expected number of velocities\n" );
		exit( 1 );
	}
		
	fclose(pFile1);
	fclose(pFile2);
	fclose(vFile1);
	fclose(vFile2);
	
	base::Initialize( title );

	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouseButton );
	glutMotionFunc( mouseMove );
	xTrans = 0.f;
	yTrans = 0.f;
	zTrans = -500.f;
	xRot = 0.f;
	yRot = 0.f;
	scaleAmt = 0.25f;

	// render shaders adapted from OpenGL Programming Guide
	static const char v_shader_source[] =
			"#version 430 core\n"
			"\n"
			"in vec4 vert;\n"
			"\n"
			"uniform mat4 mvp;\n"
			"\n"
			"out float intensity;\n"
			"\n"
			"void main(void)\n"
			"{\n"
			"    intensity = vert.w;\n"
			"    gl_Position = mvp * vec4(vert.xyz, 1.0);\n"
			"}\n";

	static const char f_shader_source[] =
			"#version 430 core\n"
			"\n"
			"layout (location = 0) out vec4 color;\n"
			"\n"
			"in float intensity;\n"
			"\n"
			"void main(void)\n"
			"{\n"
			"    color = mix(vec4(0.0f, 0.2f, 0.5f, 1.0f), vec4(0.3f, 0.05f, 0.0f, 1.0f), intensity);\n"
			"}\n";

	// set up render shaders
	renderShader = glCreateProgram();
	vglAttachShaderSource( renderShader, GL_VERTEX_SHADER, v_shader_source );
	vglAttachShaderSource( renderShader, GL_FRAGMENT_SHADER, f_shader_source );
	glLinkProgram( renderShader );

	// save the openGL compiler log
	char log[8192];
	GLsizei logSize;
	glGetProgramInfoLog(renderShader, (GLsizei)8191, &logSize, log);
	FILE* logFile = fopen("shaderlog.txt", "w");
	fwrite(log, 1, (size_t)logSize, logFile);

	// positions: (x, y, z, color)
	// velocities: (x, y, z, mass)

	for( int i = 0; i < numPart1; i++ )
	{
		positions[i][0] += 1000;

		// choose color based on mass
		positions[i][3] = 1 - (1.5e15f - velocities[i][3]) / (2.5e15f - 1.5e14f);
	}

	for( int i = numPart1; i < particleCount; i++ )
	{
		positions[i][0] += 1000;
		
		// choose color based on mass
		positions[i][3] = (2e15f - velocities[i][3]) / (2.5e15f - 1.5e14f);
	}

	for( int i = numPart1; i < particleCount; i++ )
	{
		velocities[i][0] -= 475.f;
		velocities[i][1] -= 50.f;
	}

	glGenBuffers( 1, &posBuf );
	glGenBuffers( 1, &colBuf );

	glGenVertexArrays( 1, &render_vao );
	glBindVertexArray( render_vao );

	glBindBuffer( GL_ARRAY_BUFFER, posBuf );
	glBufferData( GL_ARRAY_BUFFER, particleCount * sizeof(vmath::vec4), positions,
			GL_STREAM_DRAW );

	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, NULL );
	glEnableVertexAttribArray( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	check_gl_error();
}

/*
 * update and draw the particles
 */
void ParticleSimulator::Display( bool auto_redraw )
{
	static const GLuint start_ticks = 0;
	GLuint current_ticks = (int) ( 1000 * omp_get_wtime() );
	static GLuint last_ticks = current_ticks;
	float time = ( ( start_ticks - current_ticks ) & 0xFFFFF ) / float( 0xFFFFF );
	float delta_time = (float) ( current_ticks - last_ticks ) * TIMESTEP_FACT;

	// If dt is too large, the system could explode, so cap it to some maximum allowed value
	if (delta_time >= 2.0f)
	{
		delta_time = 2.0f;
	}

	// call the physics simulation
	simulate(delta_time);

	// draw the particles
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	glBufferData( GL_ARRAY_BUFFER, particleCount * sizeof(vmath::vec4), positions, GL_STREAM_COPY );
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	vmath::mat4 mvp = vmath::perspective( 45.0f, aspect_ratio, 0.1f, 1000.0f )
			* vmath::translate( xTrans, yTrans, zTrans )
			* vmath::scale( scaleAmt )
			* vmath::rotate( xRot, vmath::vec3( 1.f, 0.f, 0.f ) )
			* vmath::rotate( yRot, vmath::vec3( 0.f, 1.f, 0.f ) );

	// Clear, select the rendering program and draw a full screen quad
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST );

	glUseProgram(renderShader);

	glUniformMatrix4fv( 0, 1, GL_FALSE, mvp );
	glBindVertexArray( posBuf );

	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE );
	glPointSize( 3.0f );
	glDrawArrays( GL_POINTS, 0, particleCount );

	last_ticks = current_ticks;

	base::Display();
}

void ParticleSimulator::Finalize( void )
{
	return;
}

void ParticleSimulator::Reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
	aspect_ratio = (float) width / (float) height;
}

static inline float random_float()
{
	float res;
	unsigned int tmp;
	static unsigned int seed = 0xFFFF0C59;

	seed *= 16807;

	tmp = seed ^ ( seed >> 4 ) ^ ( seed << 15 );

	*( (unsigned int *) &res ) = ( tmp >> 9 ) | 0x3F800000;

	return ( res - 1.0f );
}

static vmath::vec3 random_vector( float minmag = 0.0f, float maxmag = 1.0f )
{
	vmath::vec3 randomvec( random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f,
			random_float() * 2.0f - 1.0f );
	randomvec = normalize( randomvec );
	randomvec *= ( random_float() * ( maxmag - minmag ) + minmag );

	return randomvec;
}

void _check_gl_error( const char *file, int line )
{
	GLenum err( glGetError() );

	while ( err != GL_NO_ERROR )
	{
		string error;

		switch ( err ) {
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		cerr << "GL_" << error.c_str() << " - " << file << ":" << line << endl;
		err = glGetError();
	}
}

void keyboard( unsigned char c, int x, int y )
{
	switch( c )
	{
	case '2':
		xRot = -90.;
		break;
	case '4':
		yRot = 90.;
		break;
	case '6':
		yRot = -90.;
		break;
	case '8':
		xRot = 90.;
		break;
	case '5':
		xRot = 0.;
		yRot = 0.;
		break;
		
	case ESC:
		exit(0);
		break;
		
	case TAB:
		xRot = 0.f;
		yRot = 0.f;
		xTrans = 0.f;
		yTrans = 0.f;
		zTrans = -500.f;
		scaleAmt = 1.f;
		break;

	case 'w':
		writeToFile();
		break;

	default:
		break;
	};
}

void mouseButton( int button, int state, int x, int y )
{
	int b; // LEFT, MIDDLE, or RIGHT

	switch ( button ) {
	case GLUT_LEFT_BUTTON:
		b = LEFT;
		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;
		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;
		break;

	default:
		b = 0;
		fprintf( stderr, "Unknown mouse button: %d\n", button );
		break;
	}

	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		mouseX = x;
		mouseY = y;
		activeButton |= b; // set the proper bit
	} else
	{
		activeButton &= ~b; // clear the proper bit
	}
}

void mouseMove( int x, int y )
{
	int dx = x - mouseX; // change in mouse coords
	int dy = y - mouseY;

	if (activeButton == LEFT)
	{
		xRot += ( ANGFACT * dy );
		yRot += ( ANGFACT * dx );
	}

	if (activeButton == RIGHT)
	{
		scaleAmt += SCALEFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:
		if (scaleAmt < MINSCALE)
			scaleAmt = MINSCALE;

	//	fprintf( stderr, "Scale: %f\n", scaleAmt );
	}

	if( activeButton == 5 )
	{
		xTrans += TRANSFACT * dx;
		yTrans -= TRANSFACT * dy;
	}

	mouseX = x; // new current position
	mouseY = y;
}

/*
 * physics simulation by Joe Parallel, Oregon State University
 */
void simulate( float dt )
{
#pragma omp parallel for default(none) shared(particleCount, positions, velocities, forces, newPos, newVel, dt) schedule(dynamic)
	for (int i = 0; i < particleCount; i++)
	{
		float fx = 0.;
		float fy = 0.;
		float fz = 0.;
		vmath::vec4 *p_i = &positions[i];
		vmath::vec4 *v_i = &velocities[i];
		vmath::vec4 *f_i = &forces[i];

		for (int j = 0; j < particleCount; j++)
		{
			if (j == i)
				continue;

			vmath::vec4 *p_j = &positions[j];
			vmath::vec4 *v_j = &velocities[j];
			vmath::vec4 *f_j = &forces[j];

			float rsqd = GetDistanceSquared( p_i, p_j );

			if (rsqd > 0.)
			{
				float f = G * v_i->operator[](3) * v_j->operator[](3) / rsqd;

				float ux, uy, uz;
				GetUnitVector( p_i, p_j, &ux, &uy, &uz );

				fx += f * ux;
				fy += f * uy;
				fz += f * uz;
			}
		}

		float ax = fx / velocities[i][3];
		float ay = fy / velocities[i][3];
		float az = fz / velocities[i][3];

		newPos[i][0] = positions[i][0] + velocities[i][0] * dt + 0.5 * ax * dt * dt;
		newPos[i][1] = positions[i][1] + velocities[i][1] * dt + 0.5 * ay * dt * dt;
		newPos[i][2] = positions[i][2] + velocities[i][2] * dt + 0.5 * az * dt * dt;

		newVel[i][0] = velocities[i][0] + ax * dt;
		newVel[i][1] = velocities[i][1] + ay * dt;
		newVel[i][2] = velocities[i][2] + az * dt;
	}

	// setup the state for the next animation step:
	for (int i = 0; i < particleCount; i++)
	{
		positions[i][0] = newPos[i][0];
		positions[i][1] = newPos[i][1];
		positions[i][2] = newPos[i][2];
		velocities[i][0] = newVel[i][0];
		velocities[i][1] = newVel[i][1];
		velocities[i][2] = newVel[i][2];
	}
	return;
}

void writeToFile()
{
	FILE* pointsOut = fopen( "out.pos", "w" );
	fwrite(positions, sizeof( vmath::vec4 ), particleCount, pointsOut );
	FILE* velOut = fopen( "out.vel", "w" );
	fwrite(velocities, sizeof( vmath::vec4 ), particleCount, velOut );
	fclose(pointsOut);
	fclose(velOut);
}

float GetDistanceSquared( vmath::vec4 *bi, vmath::vec4 *bj )
{
	float dx = (*bi)[0] - (*bj)[0];
	float dy = (*bi)[1] - (*bj)[1];
	float dz = (*bi)[2] - (*bj)[2];
	return dx * dx + dy * dy + dz * dz;
}

float GetUnitVector( vmath::vec4 *from, vmath::vec4 *to, float *ux, float *uy, float *uz )
{
	float dx = (*to)[0] - (*from)[0];
	float dy = (*to)[1] - (*from)[1];
	float dz = (*to)[2] - (*from)[2];

	float d = sqrt( dx * dx + dy * dy + dz * dz );
	if (d > 0.)
	{
		dx /= d;
		dy /= d;
		dz /= d;
	}

	*ux = dx;
	*uy = dy;
	*uz = dz;

	return d;
}

float Ranf( float low, float high )
{
	float r = (float) rand(); // 0 - RAND_MAX

	return ( low + r * ( high - low ) / (float) RAND_MAX );
}

int Ranf( int ilow, int ihigh )
{
	float low = (float) ilow;
	float high = (float) ihigh + 0.9999f;

	return (int) ( Ranf( low, high ) );
}
