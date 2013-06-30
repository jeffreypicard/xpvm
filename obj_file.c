/*
 * obj_file.c
 *
 * Functions to handle the object files for the XPVM.
 *
 * Author: Jeffrey Picard
 */
#include <stdlib.h>
#include <stdio.h>
#include "xpvm.h"

/*
 * load_object_file
 *
 * Loads an object file for the XPVM
 *
 *  only one object file may be loaded at a time
 *  the function returns 1 if successful and 0 otherwise
 *  if 0 is returned then an error number is returned through the second
 *    parameter if it is not NULL
 *  the following error numbers are supported:
 *    -1: file not found
 *    -2: file contains outsymbols
 *    -3: file is not a valid object file
 */
int32_t load_object_file( char *filename, int32_t *errorNumber, uint32_t* block_cnt, uint64_t **block_ptr )
{
  FILE *fp;
  //uint64_t obj_len = 0;
  const uint32_t MAGIC = 0x31303636;
  uint32_t magic = 0;
  int i = 0;
  // FIXME
  //atexit( cleanup );

  //CIO = 0;
  //CIB = 0;

  /* Open the file */
  fp = fopen( filename, "r" );
  if( NULL == fp )
  {
    *errorNumber = -1;
    return 0;
  }

  /* read headers */
  if( !read_word( fp, (uint32_t*) &magic ) )
  {
    *errorNumber = -3;
    return 0;
  }
#if DEBUG_XPVM
  fprintf( stderr, "magic: %x\n", magic );
#endif
  if( MAGIC != magic )
  {
    *errorNumber = -3;
    return 0;
  }
  if( !read_word( fp, (uint32_t *) block_cnt ) )
  {
    *errorNumber = -3;
    return 0;
  }

  if(!((*block_ptr) = calloc(*block_cnt, sizeof(uint64_t))))
    EXIT_WITH_ERROR("Error: malloc failed in load_object_file");

#if DEBUG_XPVM
  fprintf(stderr, "block_cnt: %d\n", *block_cnt );
#endif

  /* Read the blocks into memory */
  for( i = 0; i < *block_cnt; i++ )
  {
    if (!read_block(fp, i, *block_ptr))
    {
      *errorNumber = -3;
      return 0;
    }
  }

  fclose( fp );

  return 1;
}

/*
 * read_block
 *
 * Takes a file pointer to the object code
 * and the number of the block which needs to
 * be read in. Reads in all the information
 * and stores it into the already allocated
 * array of blocks stored at register BLOCK_REG.
 */
int read_block( FILE *fp, int block_num, uint64_t *block_ptr )
{
#if DEBUG_XPVM
  fprintf( stderr, "------- Reading block %d from object file. -------\n", 
                   block_num );
#endif
  char name[256];
  uint32_t length               = 0;
  uint32_t frame_size           = 0;
  uint32_t num_except_handlers  = 0;
  uint32_t num_native_refs      = 0;
  uint32_t num_outsymbol_refs   = 0;
  uint32_t length_aux_data      = 0;
  uint64_t annots               = 0;
  uint64_t owner                = 0;
  uint8_t  *b_data              = 0;
  uint8_t  temp                 = 0;

  int i = 0, j = 0;
  /* Read name string from block */
  while( (name[i] = fgetc( fp )) && ++i < MAX_NAME_LEN );
  /* Name too long */
  if( MAX_NAME_LEN == i )
    return 0;

#if DEBUG_XPVM
  fprintf( stderr, "name: %s\n", name );
#endif

  /* Read trait annotations */
  READ_INT64_LITTLE_ENDIAN( annots, fp );

#if DEBUG_XPVM
  fprintf( stderr, "annots: %" PRIx64 "\n", annots );
#endif

  /* Read frame size */
  READ_INT32_LITTLE_ENDIAN( frame_size, fp );

#if DEBUG_XPVM
  fprintf( stderr, "frame_size: %08x\n", frame_size );
#endif

  /* Read contents length */
  READ_INT32_LITTLE_ENDIAN( length, fp );

#if DEBUG_XPVM
  fprintf( stderr, "length: %08x\n", length );
#endif

  /* Allocate contents */
  b_data = calloc( length + BLOCK_HEADER_LENGTH, sizeof(uint8_t) );
  block_ptr[block_num] = (uint64_t) CAST_INT (b_data + BLOCK_HEADER_LENGTH);
  if( !b_data && length )
  {
    fprintf( stderr, "Error: malloc failed in read_block\n");
    exit(-1);
  }

  /* Read contents */
  uint8_t *b_data_read = b_data + BLOCK_HEADER_LENGTH;
  for( i = 0; i < length; i++ )
  {
    b_data_read[i] = fgetc( fp );
#if DEBUG_XPVM
    fprintf( stderr, "%02x\n", b_data_read[i] );
#endif
  }

  /* Read number of exception handlers */
  READ_INT32_LITTLE_ENDIAN( num_except_handlers, fp );

#if DEBUG_XPVM
  fprintf( stderr, "num_except_handlers: %08x\n", num_except_handlers );
#endif

  uint8_t *except_data = calloc( 4 + 12*num_except_handlers, sizeof(uint8_t));
  *((uint32_t*)except_data) = num_except_handlers;

  j = 4;
  for( i = 0; i < num_except_handlers; i++ )
  {
    READ_INT32_LITTLE_ENDIAN( *((uint32_t*)(except_data+j)), fp );
    j += 4;
    READ_INT32_LITTLE_ENDIAN( *((uint32_t*)(except_data+j)), fp );
    j += 4;
    READ_INT32_LITTLE_ENDIAN( *((uint32_t*)(except_data+j)), fp );
    j += 4;
  }

  /* Read number of outsymbol references */
  READ_INT32_LITTLE_ENDIAN( num_outsymbol_refs, fp );

#if DEBUG_XPVM
  fprintf( stderr, "num_outsymbol_refs: %08x\n", num_outsymbol_refs );
#endif

  for( i = 0; i < num_outsymbol_refs; i++ )
  {
    /* Read until null */
    while( fgetc(fp) );
    READ_INT32_LITTLE_ENDIAN( temp, fp );
  }

  /* Read number of native function references */
  READ_INT32_LITTLE_ENDIAN( num_native_refs, fp );

#if DEBUG_XPVM
  fprintf( stderr, "num_outsymbol_refs: %08x\n", num_outsymbol_refs );
#endif

  for( i = 0; i < num_outsymbol_refs; i++ )
  {
    /* Read until null */
    while( fgetc(fp) );
    READ_INT32_LITTLE_ENDIAN( temp, fp );
  }

  /* Read length of auxiliary data */
  READ_INT32_LITTLE_ENDIAN( length_aux_data, fp );

#if DEBUG_XPVM
  fprintf( stderr, "length_aux_data: %08x\n", length_aux_data );
#endif

  /* Allocate auxiliary data */
  
  /*
  aux_data = calloc( length_aux_data, sizeof(char) );
  if( aux_data && length_aux_data )
  {
    fprintf( stderr, "Error: malloc failed in read_block\n");
    exit(-1);
  }*/

  /* Read auxiliary data */
#if DEBUG_XPVM
    fprintf( stderr, "Skipping aux data.\n");
#endif
  for( i = 0; i < length_aux_data; i++ )
  {
#if DEBUG_XPVM
    temp = fgetc( fp );
    fprintf( stderr, "%02x\n", temp );
#else
    fgetc(fp);
#endif
  }
  BLOCK_OWNER( b_data_read )            = owner;
  BLOCK_ANNOTS( b_data_read )           = annots;
  BLOCK_AUX_LENGTH( b_data_read )       = length_aux_data;
  BLOCK_OUT_SYM_REFS( b_data_read )     = num_outsymbol_refs;
  BLOCK_EXCEPT_HANDLERS( b_data_read )  = CAST_INT except_data;
  BLOCK_NATIVE_REFS( b_data_read )      = num_native_refs;
  BLOCK_FRAME_SIZE( b_data_read )       = frame_size;
  BLOCK_LENGTH( b_data_read )           = length;

  /*test_block_macros( b_data_read );*/


  /* TODO Add this block to the VM memory. */

#if DEBUG_XPVM
  fprintf( stderr, "------- Block %d (%p)successfully "
                   "read from object file. -------\n", 
                   block_num, b_data_read );
#endif

  return 1;  
}

/*
 * read_word
 * op
 *
 * Reads a word (32 bits) from the object file and
 * returns it through the second parameter.
 * returns 0 on EOF.
 */
int read_word(FILE *fp, uint32_t *out_word)
{
  int c1 = getc(fp);
  int c2 = getc(fp);
  int c3 = getc(fp);
  int c4 = getc(fp);

  if (c4 == EOF)
  {
    return 0;
  }
  *out_word = ((c1 << 24) | (c2 << 16) | (c3 << 8) | c4);
  return 1;
}

/*
 * verify_obj_format
 *
 * Gets the length of the object file in bytes.
 * Takes a file pointer, a 32 bit block count and a
 * pointer to a 64 bit int for returning the length in
 * bytes.
 * Returns 0 on success, nonzero otherwise.
 */
int verify_obj_format(char *file_name, uint64_t *obj_len)
{
  FILE *fp = NULL;
  uint32_t cur_length = 0, magic = 0, block_cnt = 0;
  const uint32_t MAGIC = 0x31303636;
  int i = 0;
  char *c = 0;

#if DEBUG_XPVM
  fprintf( stderr, "In verify_obj_format\n");
#endif

  /*fp_pos = calloc( 1, sizeof(fpos_t) );*/
  fp = fopen( file_name, "r" );
  MALLOC_CHECK( fp, "Error: fopen failed in verify_obj_format\n");

  /* read headers */
  if( !read_word( fp, (uint32_t*) &magic ) )
    return -3;
#if DEBUG_XPVM
  fprintf( stderr, "magic: %x\n", magic );
#endif
  if( MAGIC != magic )
    return -3;
  if( !read_word( fp, (uint32_t *) &block_cnt ) )
    return -3;

#if DEBUG_XPVM
  fprintf( stderr, "block_cnt: %d\n", block_cnt );
#endif

  /* Save the current position in the file. 
  if( 0 > fgetpos( fp, fp_pos ) )
    return 0;*/

#if DEBUG_XPVM
  fprintf( stderr, "After intial read\n");
#endif

  *obj_len = 0;
  for( i = 0; i < block_cnt; i++ )
  {
    /* skip name */
    fread( &c, 1, 1, fp );
    while( c ) fread( &c, 1, 1, fp );

    /* Skip annotations */
    if( !read_word( fp, &cur_length ) )
      return 0;
    if( !read_word( fp, &cur_length ) )
      return 0;
    /* skip frame size */
    if( !read_word( fp, &cur_length ) )
      return 0;

    /* Get the current block size */
    if( !read_word( fp, &cur_length ) )
      return 0;
#if DEBUG_XPVM
    fprintf( stderr, "cur_length: %d\n", cur_length );    
#endif
    if( 0 > fseek( fp, cur_length-1, SEEK_CUR ) )
      return 0;
    *obj_len += cur_length;
  }

#if DEBUG_XPVM
  fprintf( stderr, "After obj_len calculation\n");
#endif

  /* Return the file reading to its original position 
  if( 0 > fsetpos( fp, fp_pos ) )
    return 0;

  free( fp_pos );*/

#if DEBUG_XPVM
  fprintf( stderr, "Leaving getObjLength\n");
#endif

  fclose(fp);
  return 0;
}


