//---------------------------------------------------------------------------

#pragma hdrstop

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "sh_crc.h"
#include "app_header.h"
#include "flash_layout.h"


//---------------------------------------------------------------------------

#pragma argsused
int main(int argc, char* argv[])
{
  FILE* file;
  dword size = 0;
  byte buffer[260];
  int len;
  dword crc;
  app_header_t* app_hdr  = (app_header_t*) buffer;
  Sh_Timestamp_t ts;
//SYSTEMTIME st;


  // Check command line
  if( argc < 2 )
  {
    printf( "Please add filename\n" );
    return 1;
  }

  // Open file
  if( ( file=fopen( argv[1], "r+b" ) ) == NULL )
  {
    printf( "Can't open %s\n", argv[1] );
    return 2;
  }

  // Go to start of application
  if( fseek( file, APP_HEADER_SIZE, SEEK_SET ) != 0 )
  {
    printf( "File error (fseek)\n" );
    fclose( file );
    return 3;
  }

  // Read data and canculate crc
  size = 0;
  sh_CRC32_start();

  while( ( len = fread( buffer, 1, 256, file ) ) != 0 )
  {
    crc   = sh_CRC_next_data( buffer, len );
    size += len;
  }

  // Go to begin of file
  clearerr(file);
  if( fseek( file, 0, SEEK_SET ) != 0 )
  {
    printf( "File error (fseek)\n" );
    fclose( file );
    return 4;
  }

  // Build header structure
  memset( buffer, 0x00, sizeof(buffer) );

  ts.time = time(NULL);
  sync_timestamp_time( &ts );

  app_hdr->magic    = HDR_MAGIC_VALUE;
  app_hdr->valid    = 1;
  app_hdr->size     = size;
  app_hdr->crc      = crc;
  app_hdr->version  = ts.days;

  // Write header
  len = fwrite( buffer, 1, sizeof(app_header_t), file );
  if (len != sizeof(app_header_t))
  {
    printf("File error (fwrite)\n");
    fclose(file);
    return 5;
  }

  // Finish
  fclose( file );

  return 0;
}
//---------------------------------------------------------------------------


