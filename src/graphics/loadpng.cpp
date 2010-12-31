#include <cstdio>
#include <cstdlib>
#ifdef __APPLE__
#include "png.h"
#else
#include <png.h>
#endif

int pngLoad(const char *file, unsigned long *pwidth, unsigned long *pheight, char **image_data_ptr, bool& alpha)
{
	
	FILE         *infile;         /* PNG file pointer */
	png_structp   png_ptr;        /* internally used by libpng */
	png_infop     info_ptr;       /* user requested transforms */
	
	char         *image_data;      /* raw png image data */
	char         sig[8];           /* PNG signature array */
	/*char         **row_pointers;   */
	
	int           bit_depth;
	int           color_type;
	
	png_uint_32 width;            /* PNG image width in pixels */
	png_uint_32 height;           /* PNG image height in pixels */
	png_uint_32 rowbytes;         /* raw bytes at row n in image */
	
	image_data = NULL;
	unsigned long i;
	png_bytepp row_pointers = NULL;
	
	/* Open the file. */
	infile = fopen(file, "rb");
	if(!infile)
	{
		return 0;
	}
	
	
	/*
	*           13.3 readpng_init()
	*/
	
	/* Check for the 8-byte signature */
	size_t ret = fread(sig, 1, 8, infile);
	if(ret != 8) // || !png_check_sig((unsigned char *) sig, 8)) // TODO: hahahahahahahahahahahahahahahaha :G
	{
		fclose(infile);
		return 0;
	}
	
	/* 
	* Set up the PNG structs 
	*/
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fclose(infile);
		return 4;    /* out of memory */
	}
	
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
		fclose(infile);
		return 4;    /* out of memory */
	}
	
	
	/*
	* block to handle libpng errors, 
	* then check whether the PNG file had a bKGD chunk
	*/
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(infile);
		return 0;
	}
	
	/* 
	* takes our file stream pointer (infile) and 
	* stores it in the png_ptr struct for later use.
	*/
	/* png_ptr->io_ptr = (png_voidp)infile;*/
	png_init_io(png_ptr, infile);
	
	/*
	* lets libpng know that we already checked the 8 
	* signature bytes, so it should not expect to find 
	* them at the current file pointer location
	*/
	png_set_sig_bytes(png_ptr, 8);
	
	/* Read the image info.*/
	
	/*
	* reads and processes not only the PNG file's IHDR chunk 
	* but also any other chunks up to the first IDAT 
	* (i.e., everything before the image data).
	*/
	
	/* read all the info up to the image data  */
	png_read_info(png_ptr, info_ptr);
	
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
	
	*pwidth = width;
	*pheight = height;
	
	/* Set up some transforms. */
	if (color_type & PNG_COLOR_MASK_ALPHA)
		alpha = true;
	else
		alpha = false;
	
	if (bit_depth > 8)
	{
		png_set_strip_16(png_ptr);
	}
	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}
	
	/* Update the png info struct.*/
	png_read_update_info(png_ptr, info_ptr);
	
	/* Rowsize in bytes. */
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	
	/* Allocate the image_data buffer. */
	image_data = new char[rowbytes * height];
	row_pointers = new png_bytep[height];
	
	/* set the individual row_pointers to point at the correct offsets */
	for (i = 0;  i < height;  ++i)
		row_pointers[height - 1 - i] = (png_byte*)image_data + i*rowbytes;
	
	
	/* now we can go ahead and just read the whole image */
	png_read_image(png_ptr, row_pointers);
	
	/* and we're done!  (png_read_end() can be omitted if no processing of
	* post-IDAT text/time/etc. is desired) */
	
	/* Clean up. */
	delete[] row_pointers;
	
	/* Clean up. */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(infile);
	
	*image_data_ptr = image_data;
	
	return 1;
}

void pngUnload(char* data)
{
	delete[] data;
}

