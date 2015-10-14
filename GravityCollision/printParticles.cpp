#include <cstdio>
#include "includes/vmath.h"

const size_t MAXLENGTH = 1024*1024*sizeof( vmath::vec4 );
vmath::vec4* points;

int main( int argc, char** argv )
{
	if( argc != 2 )
	{
		fprintf( stderr, "usage: %s <filename>\n", argv[0] );
		return 0;
	}
	
	FILE* inFile = fopen( argv[1], "r" );
	fseek( inFile, 0, SEEK_END );
	size_t fileSize = ftell( inFile );
	if( fileSize > MAXLENGTH )
	{
		fprintf( stderr, "the file is too long\n" );
		return 1;
	}
	size_t fileLen = fileSize / sizeof( vmath::vec4 );
	fseek( inFile, 0, SEEK_SET );
	
	points = new vmath::vec4[MAXLENGTH];
	fread( points, sizeof( vmath::vec4 ), fileLen, inFile );
	
	for( int i = 0; i < fileLen; i++ )
	{
		if( points[i][3] < 1000. )
		{
			printf( "(%f, %f, %f), %f\n", 
					points[i][0], points[i][1], points[i][2], points[i][3] );
		}
		else
		{
			printf( "(%f, %f, %f), %e\n", 
					points[i][0], points[i][1], points[i][2], points[i][3] );
		}
	}
	
	printf( "\nnumber of particles: %d\n", fileLen );
		
	return 0;
}
