/* translate particles by (x, y, z, w) */

#include <cstdio>
#include "includes/vmath.h"
#include <string>
#include <cstdlib>
using namespace std;


const size_t MAXLENGTH = 1024*1024*sizeof( vmath::vec4 );
vmath::vec4* points;

int main( int argc, char** argv )
{
	if( argc != 6 )
	{
		fprintf( stderr, "usage: %s <filename> <x> <y> <z> <w>\n", argv[0] );
		return 0;
	}
	
	string tFilename(argv[1]);
	tFilename += ".new";
	
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
	
	int x = atoi( argv[2] );
	int y = atoi( argv[3] );
	int z = atoi( argv[4] );
	int w = atoi( argv[5] );
	
	for( int i = 0; i < fileLen; i++ )
	{
		points[i][0] += (float)(x);
		points[i][1] += (float)(y);
		points[i][2] += (float)(z);
		points[i][3] += (float)(w);				
	}
	
	FILE* tFile = fopen( tFilename.c_str(), "w" );
	fwrite(points, sizeof( vmath::vec4 ), fileLen, tFile );
	fclose(tFile);
	
	return 0;
}
