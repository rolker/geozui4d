// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0


/*
 *  Sv_Raster
 * 
 *  Defines a class for the loading and saving of raster images
 *  in various formats, including PPM, GIF, JPEG, TIFF, IrisRGB.
 * 
 *  Author: Dan Fleet
 *          Mark Paton  - various additions & mods.
 * 
 *  NOTES:
 * 
 *  Currently, saving is only supported for JPEG and TIFF.
 * 
 *  PPM4 loading is not fully implemented and will fail
 *  with a warning message.
 * 
 *  B/W Iris imagery files are not implemented,  only true
 *  RGB/RGBA Iris files are supported.
 */

#include<gz4d/glo.h>

/*#ifdef WIN32
#include "Togl/togl.h" //get what we need included for glu.h to load right.
#endif*/
#include <GL/glu.h>
#undef EXTERN
#include <string.h>

#define SV_RASTER_JPEGSUP
//#ifndef WIN32
#define SV_RASTER_TIFFSUP
//#endif
#include "widgets/Sv_Raster.h"

// Some of these define stub functions for parts which have
// been optioned out, or the actual work functions
// for parts which have been optioned in.

/* =================================  JPEG  ================================ */

#ifndef SV_RASTER_JPEGSUP
GLint Sv_Raster::_ReadJPEG(const char *fileName)
{
    cerr << "Unable to load: " << fileName << " as JPEG file.\n"
	 << "JPEG support has been compiled out of the Sv_Raster library.\n";
	 
    return (-1);
}

GLint Sv_Raster::_WriteJPEG(FILE *out)
{
    cerr << "Sv_Raster::_WriteJPEG() - JPEG support has been compiled out of the Sv_Raster library.\n";
    fclose(out);
    return (-1);
}

#else
// IJG's JPEG software library.

extern "C" {
#ifdef WIN32
#define XMD_H //random gunk from jmorecfg!
#endif
#include "jpeglib.h" // in /usr/local/include/
#include "jerror.h"
}

using namespace std;

// Find the size of a jpeg
GLint Sv_Raster::_sizeJPEG(const char *fileName,GLint &width,GLint &height)
{
    struct jpeg_decompress_struct cinfo; // uses libjpeg.a!!
    struct jpeg_error_mgr jerr;
    
    FILE *in;
    
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }
    
   
    cinfo.err = jpeg_std_error(&jerr);
    
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, in);
    (void) jpeg_read_header(&cinfo, TRUE);

    
    width = cinfo.image_width;
    height = cinfo.image_height;
    
    fclose(in);
    
    return (0); // Sucess!    
    
}

// Read a JPEG/JFIF file into the _Image array.
// Note: reads as GL_RGBA data.  Pads the alpha
// component as 255 (1.0f)
// NOTE: Uses libjpeg.  Link with -ljpeg.
GLint Sv_Raster::_ReadJPEG(const char *fileName)
{
    struct jpeg_decompress_struct cinfo; // uses libjpeg.a!!
    struct jpeg_error_mgr jerr;
    
    int row_stride;
    JSAMPARRAY buffer; // row buffer.
    int x, y, q;
    GLubyte *tmp;
    FILE *in;
    
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }
    
    
    // create decompress object, and specify input file.
    // (note - libjpeg.a is a C-based library, so this is
    // not C++ objects, here.)
    
    cinfo.err = jpeg_std_error(&jerr);
    
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, in);
    
    (void) jpeg_read_header(&cinfo, TRUE);

    (void) jpeg_start_decompress(&cinfo);
    
       
    // SPECIAL NOTE - Colour jpeg components have 3 components
    // per pixel (RGB data).  Greyscale only have ONE component/pixel.
    // As texture images have 4, we must correctly duplicate
    // (well, triplicate -- RGB) the greyscale value.
    
    row_stride = cinfo.output_width*cinfo.output_components;
    
    buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
    
    y=0;
    
    Destroy(); // destroy old image, if any.
    
    _Width = cinfo.output_width;
    _Height = cinfo.output_height;

    // create image memory, GL_RGBA (4 components/pixel)
    
    _AllocateNewImage(); 
    
    q=0;
    
    while ((y=cinfo.output_scanline) < (int)cinfo.output_height)
    {
		// Read one scan line at a time.
		if ((q=jpeg_read_scanlines(&cinfo, buffer, 1)) != 1)
	    	cerr << "Sv_Raster::_ReadJPEG() - WARN! " << q << " scanlines read, expected 1!\n"; 

		// Store the scan line into the texture data memory.

		y = (_Height-1) - y;


		if (cinfo.output_components == 1)
		{

	    	tmp = (GLubyte *) &buffer[0][0];


	    	for (x=0; x<_Width; x++)
	    	{		
				_Image[(y*_Width*4)+(x*4)] =  *tmp;
				_Image[(y*_Width*4)+(x*4)+1] = *tmp;
				_Image[(y*_Width*4)+(x*4)+2] = *tmp;
				_Image[(y*_Width*4)+(x*4)+3] = 255; // Alpha value.

				tmp++;
	    	}	           
		}
		else // 3 component
		{

	    	tmp = (GLubyte *) &buffer[0][0];

	    	for (x=0, q=0; x<(_Width*3); x+=3, q++)
	    	{

				_Image[(y*_Width*4)+(q*4)] = *tmp;
				_Image[(y*_Width*4)+(q*4)+1] = *(tmp+1);
				_Image[(y*_Width*4)+(q*4)+2] = *(tmp+2);
				_Image[(y*_Width*4)+(q*4)+3] = 255; // Alpha value.

				tmp += 3;
	    	}	              
		}
    	}
    
    // DONE decompressing.
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

    fclose(in);
    
    return (0); // Sucess!    
    
}

GLint Sv_Raster::_WriteJPEG(FILE *outfile)
{
    struct jpeg_compress_struct cinfo;

    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
 //   int row_stride;		/* physical row width in image buffer */
    GLubyte *imageLine;
    int x, y;
    
    if (!ImageLoaded())
    {
		cerr << "Sv_Raster::_WriteJPEG() - Error: No image to save!\n";
		return (-1);
    }
    
    cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = _Width; 	/* image width and height, in pixels */
    cinfo.image_height = _Height;
    cinfo.input_components = 3;		/* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 80, TRUE );
    jpeg_start_compress(&cinfo, TRUE);

    imageLine = new GLubyte [_Width*3];  // 3 component output.
//    row_stride = _Width * 3;
    
    for (y=_Height-1; y>=0; y--)
    {
		for (x=0; x<_Width; x++)
		{
	    	imageLine[x*3] = _Image[y*_Width*4+x*4];
	    	imageLine[x*3+1] = _Image[y*_Width*4+x*4+1];
	    	imageLine[x*3+2] = _Image[y*_Width*4+x*4+2];
	    	// We don't do the alpha value.
		}

		row_pointer[0] = imageLine;
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	
    }
    
    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
    return (0);
}

#endif // SV_RASTER_JPEGSUP

/* ===============================  END JPEG  ============================== */

/* ===============================  TIFF  ================================== */

#ifndef SV_RASTER_TIFFSUP
GLint Sv_Raster::_ReadTIFF(const char *fileName)
{
    cerr << "Unable to load: `" << fileName << "' as TIFF file.\n"
	 << "TIFF support has been compiled out of the Sv_Raster library.\n";
	 
    return (-1);
}

GLint Sv_Raster::_WriteTIFF( const char* fileName )
{
    cerr << "Sv_Raster::_WriteTIFF() - TIFF support has been "
         << "compiled out of the Sv_Raster library." << endl;
    return (-1);
}

#else

// SGI's public libtiff library.

#include "tiffio.h" // in /usr/local/include/

// rja - Jan 25, 2002   Adding geotiff support
//#include "xtiffio.h"
//#include "geotiffio.h"


GLint Sv_Raster::_sizeTIFF(const char *fileName,GLint &width,GLint &height)
{
	TIFF *tif = TIFFOpen(fileName,"rb");

    if (!tif)
    {
        cerr << "Sv_Raster::_sizeTIFF() - Error reading TIFF file.\n";
        return (-1);
    }
    
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	
	TIFFClose(tif);
	
	return 0;
}

// Reads a TIFF image from the specified file.  Note that
// this code uses libtiff, and must be linked -ltiff.
// Also node that libtiff wants to open the file itself,
// so we pass it the filename, rather than a file pointer.

GLint Sv_Raster::_ReadTIFF( const char *infile )
{
//    GTIF *gtif=(GTIF*)0;
    
//    TIFF *tif = XTIFFOpen(infile,"rb");
	TIFF *tif = TIFFOpen(infile,"rb");
    uint32 *tmp;
    GLuint w=0,h=0;

    if (!tif)
    {
        cerr << "Sv_Raster::_ReadTIFF() - Error reading TIFF file.\n";
        return (-1);
    }
    
// 	gtif = GTIFNew(tif); 
//    if(gtif)
//    {
//    }
 
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    tmp = (uint32 *)_TIFFmalloc(w*h*4);

    if (!tmp)
    {
        cerr << "Sv_Raster::_ReadTIFF() - OUT OF MEMORY!\n";
        return (-1);
    }

    if (TIFFReadRGBAImage(tif, w, h, tmp, 0))
    {
        Destroy(); // destroy current raster image, if any.

        // TIFFReadRGBAImage() stores its data in Iris GL (ABGR) format.
        // We use the LoadFromMemory() with the SVR_ABGR tag to store
        // the image in _Image, autoswapping to RGBA format along the way.

        TIFFClose(tif);

//#ifndef WIN32
        if (LoadFromMemory(tmp,w,h,SVR_ABGR))
//#else
//	if (LoadFromMemory(tmp,w,h,SVR_RGBA))
//#endif
        {
            cerr << "Sv_Raster::_ReadTIFF() - Error building _Image from swapped data.\n";
            _TIFFfree(tmp);

            return (-1);
        }

        _TIFFfree(tmp);
        return (0);
    }
    else
    {
        cerr << "Sv_Raster::_ReadTIFF() - Error in TIFFReadRGBAImage().\n";
        _TIFFfree(tmp);
        return (-1);
    }
}

GLint Sv_Raster::_WriteTIFF(const char *name)
{
    TIFF *tif;

    if (!ImageLoaded())
    {
        cerr << "Sv_Raster::_WriteTIFF() - Error: No image to save!\n";
        return (-1);
    }

    tif = TIFFOpen(name,"wb");

    if (!tif)
    {
        cerr << "Sv_Raster::_WriteTIFF() - Error opening file.\n";
        return (-1);
    }

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, _Width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, _Height);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3); // R,G,B
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    TIFFSetField(tif, TIFFTAG_PREDICTOR, 2); // LZW compression mode.
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT);

    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);

    // store the image to the file as strips:

    tsize_t stripsize = TIFFStripSize(tif);
    unsigned char *strip = ( unsigned char *)_TIFFmalloc(stripsize);

    register int x,y;

    for (y=0; y < _Height; y++)
    {
        register  unsigned char *pp = strip;

        for (x=0; x<_Width; x++)
        {
            pp[0] = _Image[y*_Width*4+x*4];     // Red
            pp[1] = _Image[y*_Width*4+x*4+1];   // Green
            pp[2] = _Image[y*_Width*4+x*4+2];   // Blue
            pp += 3;
        }

        if (TIFFWriteEncodedStrip(tif, TIFFComputeStrip(tif, y, 0), strip, stripsize) < 0)
            break;
    }
    _TIFFfree(strip);

    TIFFClose(tif); // Done!

    return (0);
}

#endif // SV_RASTER_TIFFSUP

/* ============================  END TIFF  ================================= */

/* ============================    GIF    ================================== */

#include <math.h>

// GIF decoding.  Note that this uses the patented LZW compression
// algorithm.  This algorthim also applies a small amount of gamma
// correction to the colours when converting them to a 24 bit palette.
//
// This is a fairly robust decoder, it supports GIF89a, and will
// decode the first image that it encounters in the GIF stream.
// Note, however, that it does not perform text annotations,
// multiple image readings, or other things supported by the format.
// It merely processes the stream until a complete image is read,
// then stops.
//
// ***** NOTE NOTE NOTE NOTE *****
// The LZW Algorithm used in GIF is owned by Unisys.  It must be licensed
// for use in commercial products.  Contact Unisys or see their web page
// for more information.


#ifndef SV_RASTER_GIFSUP
GLint Sv_Raster::_ReadGIF(const char *fileName)
{
    cerr << "Unable to load: `" << fileName << "' as GIF file.\n"
	 << "GIF support has been compiled out of the Sv_Raster library.\n";
	 
    return (-1);
}


GLint Sv_Raster::_ReadGIFLZW ( FILE *in, GLboolean interlaced,
                                GLint ti, CLUT *ColourTable,
                                GLint TableSize )
{
    cerr << "Unable to decode GIF file.\n"
	 << "GIF support has been compiled out of the Sv_Raster library.\n";
	 
    return (-1);
}

GLint Sv_Raster::_WriteGIF(FILE *out)
{
    cerr << "Sv_Raster::_WriteGIF() - GIF support has been compiled out of the Sv_Raster library.\n";
    fclose(out);
    return (-1);
}

#else

GLint Sv_Raster::_sizeGIF(const char *fileName,GLint &width,GLint &height)
{
	cerr << "Sv_Raster::_sizeGIF not yet implemented!\n";
	return -1;
}


GLint Sv_Raster::_ReadGIF(const char *fileName)
{
    GLubyte inBuffer[4096];  // Should be much more than plenty.
    int n;
    CLUT GlobalTable[256];
    CLUT LocalTable[256];
    GLint GlobalTableSize = -1;
    GLint LocalTableSize = -1;
    GLboolean UseLocal = GL_FALSE;
    GLint TransparentIndex = -1;
    GLint BitsPerComponent = -1;
    GLint i;
    GLboolean done = GL_FALSE;
    GLboolean interlaced = GL_FALSE; // Set if image is interlaced.
    FILE *in;
   int tmpcnt=0;
   
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }

    // Read in the header.  We ignore this,
    // as it should have already been parsed in
    // the _ParseHeader call.
    rewind(in);
    
    n = fread(inBuffer, 1, 6, in);
    if (n < 6)
    {
        cerr << SVR_GIF_ERROR;
        fclose(in);
        return(-1); // error
    }
    
    // Read in the logical screen descriptor block.
    // This describes the screen mode the image
    // was originally intended to be viewed in.
    // We ignore most of this block, as it does _not_
    // describe the image itself.  However,
    // One field is used to determine if the GIF data
    // stream has a global colour table.  If so, we
    // read it in.

    n = fread(inBuffer, 1, 7, in);

    if (n < 7)
    {
        cerr << SVR_GIF_ERROR;
        fclose(in);
        return(-1); // error
    }

    // inBuffer[4] now contains the flags field that we are interested in.
    if (inBuffer[4] & (GLubyte(128))) // is a global colour table present?
        GlobalTableSize = 1 << ((inBuffer[4] & (GLubyte(7)))+1); // table size

    BitsPerComponent = ((inBuffer[4] & (GLubyte(112))) >> 4)+1;

    // If there is a global colour table, then the next block is
    // the entries for that table.

    if (GlobalTableSize > 0)
    {
        n = fread(inBuffer,1,3*GlobalTableSize, in);

        if (n < 3*GlobalTableSize)
        {
            cerr << SVR_GIF_ERROR;
            fclose(in);
            return(-1); // error
        }

        for (i=0; i<GlobalTableSize; i++)
        {
            GlobalTable[i].red = (GLubyte)   inBuffer[(i*3)];
            GlobalTable[i].green = (GLubyte) inBuffer[(i*3)+1];
            GlobalTable[i].blue = (GLubyte)  inBuffer[(i*3)+2];

        }

    }

    // Now we must read blocks as they appear.

    while (!done)
    {
        n = fread(inBuffer,1,1,in); // read a byte from the file
	
	if (!n)
	{
	    cerr << "Sv_Raster::_ReadGIF() - Unexpected EOF.\n";
	    fclose(in);
	    return (-1);
	}
	
        switch ((int)inBuffer[0])
        {
            case 0x00:
            break; // Kludge for broken GIF files which write null data blocks
                   // to the stream.

            case 0x2C:      // Image separator.  Indicates an image is forthcoming.
             n = fread(inBuffer,1,9,in);
		if (n<9)
		{
		    cerr << "Sv_Raster::_ReadGIF() - Unexpected EOF reading image block.\n";
		    fclose(in);
		    return (-1);
		}

			
                Destroy();
		
		    // W/H are coded in little endian order
		    // within the GIF file.
		    
		    _Width = ((GLuint)(inBuffer[5]) << 8) | ((GLuint)(inBuffer[4]));
			_Height = ((GLuint)(inBuffer[7]) << 8) | ((GLuint)(inBuffer[6]));
		
         if (inBuffer[8] & (GLubyte)(128))
        {
            // Image has a local colour table
            UseLocal = GL_TRUE;
            LocalTableSize = 1 << ((inBuffer[8] & (GLubyte)7)+1);

            // Read in local colour table.
            n = fread(inBuffer,1,3*LocalTableSize, in);

            if (n < 3*LocalTableSize)
            {
                cerr << SVR_GIF_ERROR;
                fclose(in);
                return(-1); // error
            }

            for (i=0; i<LocalTableSize; i++)
            {
                LocalTable[i].red = (GLubyte)   inBuffer[(i*3)];
                LocalTable[i].green = (GLubyte) inBuffer[(i*3)+1];
                LocalTable[i].blue = (GLubyte)  inBuffer[(i*3)+2];
            }
       }

           if (inBuffer[8] & (GLubyte)(64))
	       {
		    interlaced = GL_TRUE;
	       }
		
                // We are now ready to read in the image data.
                // This works as follows:

                _AllocateNewImage();

                // Reads the GIF data and decompesses it, placing
                // it into _Image, setting the alpha
                // value to 255.  Returns zero on sucess,
                // non-zero on an error.
                // Supports transparent GIFs by setting
                // the Alpha value for any index which == TransparentIndex
                // (if TransparentIndex is >= 0)
                // to 0.  Note that this will only have an effect if
                // the texture is applied in a mode where the alpha
                // values are meaningful.

                if (_ReadGIFLZW(in, interlaced, TransparentIndex,
                                (UseLocal ? LocalTable : GlobalTable),
                                (UseLocal ? LocalTableSize : GlobalTableSize)))
                               
                {
                    Destroy();
                    fclose(in);
                    return (-1);
                }

                // Sucessfully read in an image.
                return (0);
 
            case 0x21:  // Extension blocks.
                // read in the next byte.
                n = fread(inBuffer, 1, 1, in);

                if (!n)
                {
                    cerr << SVR_GIF_ERROR;
                    fclose(in);
                    return (-1);
                }

 		     
                switch ((int)inBuffer[0])
                {
                    case 0xF9:  // Graphic Control Extension Block

                        // We ignore most of the stuff this block contains.
                        // It is used in streams (i.e. Compuserve)
                        // to indicate awaiting for user input, delays, etc.
                        // this is irrelavent when simply decoding an image
                        // from a file.
                        // This block does contain the identifier for
                        // the transparent colour index, if any.

                        // read in the block length
                        n = fread(inBuffer, 1, 1, in);

                        if (!n)
                        {
                            cerr << SVR_GIF_ERROR;
                            fclose(in);
                            return (-1);
                        }

                        // Read in block based on block length.
                        // (plus terminating null byte, hence the +1)

                        tmpcnt = inBuffer[0]+1;

                        n = fread(inBuffer, 1, tmpcnt, in);

                        if (n < tmpcnt)
                        {
                            cerr << SVR_GIF_ERROR;
                            fclose(in);
                            return (-1);
                        }

                        // Check to see if a transparent index is given.

                        if (inBuffer[1] & (GLubyte)(1))
                            TransparentIndex = (GLint)(inBuffer[4]);

                    break;

                    case 0xFE:  // Comment Extension Blocks.
                        // We ignore these.

                        // Read in all the data blocks containing
                        // the comment information.
                        // Simply ignore it.

                        n = fread(inBuffer,1,1,in);

                        if (!n)
                        {
                            cerr << SVR_GIF_ERROR;
                            fclose(in);
                            return (-1);
                        }

                        while (inBuffer[0])
                        {
                            tmpcnt = inBuffer[0];
                            n = fread(inBuffer, 1, tmpcnt, in);
                            if (n < tmpcnt)
                            {
                                cerr << SVR_GIF_ERROR;
                                fclose(in);
                                return(-1);
                            }
			    inBuffer[tmpcnt] = '\0';
			    
                            n = fread(inBuffer,1,1,in);
                            if (!n)
                            {
                                cerr << SVR_GIF_ERROR;
                                fclose(in);
                                return(-1);
                            }
                        }
                    break;

                    case 0xFF:      // Application Extension Block

                        // Blocks placed in by certain applications.
                        // We ignore these - we only want image data.
			// Drop through... (the code for ignoring
			// these blocks is below.)
			
                    case 0x01: // Plain Text Extension Block

                        // This is a GIF 89a extension in which
                        // the decoder is supposed to render text
                        // (contained in following data blocks)
                        // into the screen at the locations
                        // specified in the block.
                        // This is moreso intended for using the
                        // GIF data stream as a graphical interface to
                        // a system, and so is unsupported here.
                        // Not certain exactly how many GIF images
                        // really use it, anyway.  It's intended for true 
			// GIF streams, such as used in Compuserve's
			// graphical interface programs.

                        // Get the block size.

                        n = fread(inBuffer,1,1,in);

                        if (!n)
                        {
                            cerr << SVR_GIF_ERROR;
                            fclose(in);
                            return (-1);
                        }

                        // Block size

                        tmpcnt = inBuffer[0];

                        // Read block.
                        n = fread(inBuffer,1,tmpcnt,in);

                        if (n < tmpcnt)
                        {
                            cerr << SVR_GIF_ERROR;
                            fclose(in);
                            return (-1);
                        }

                        // Now, read the data blocks until
                        // they are exhausted (i.e. until null block
                        // is read in).

                        n = fread(inBuffer,1,1,in);

                        if (!n)
                        {
                            cerr << SVR_GIF_ERROR;
                            fclose(in);
                            return (-1);
                        }

                        while (inBuffer[0])
                        {
                            tmpcnt = inBuffer[0];
                            n = fread(inBuffer, 1, inBuffer[0], in);
                            if (n < tmpcnt)
                            {
                                cerr << SVR_GIF_ERROR;
                                fclose(in);
                                return(-1);
                            }

                            n = fread(inBuffer,1,1,in);
                            if (!n)
                            {
                                cerr << SVR_GIF_ERROR;
                                fclose(in);
                                return(-1);
                            }
                        }
                    break;

                    default:
                        cerr << SVR_GIF_ERROR;
                        cerr << "Illegal/Unknown/Unsupported Extension Block found.\n";
                        cerr << "Aborting.\n";
                        fclose(in);
                        return (-1);
                }

            break;

            case 0x3B: // Data stream trailer.  Shouldn't reach this
                           // before an image is read in.  However, a valid
                           // GIF file does not need to have an image.

                    if (!_Image)
                    {
                        cerr << SVR_GIF_ERROR;
                        return (-1);
                    }
                    else
                        done = GL_TRUE;
            break;

            default:
                    cerr << "Unknown/Illegal/Unsupported Control block found.\n";
                    cerr << "Aborting.\n";
                    cerr << SVR_GIF_ERROR;
                    fclose(in);
                    return (-1);
 
        } // switch()

    } // while (!done)

    fclose (in);

    if (_Image)
        return (0);
    else
        return (-1);
}

// This function acually performs the LZW decompression of the GIF data,
// once that data has been reached in the stream.

// Uses gif decompression code from gif2tiff.c in the libtiff distribution.
// *** NOTE ***
// The LZW Compression/Decompression algorithm is owned by Unisys.
// Any commercial or for-profit use requires the purchase of a license.
// (And royalties paid to Unisys)

GLint Sv_Raster::_ReadGIFLZW ( FILE *in, GLboolean interlaced,
                                GLint ti, CLUT *ColourTable,
                                GLint TableSize )
{
    GLubyte inBuffer[1024]; // Plenty.
    int n;

    // Special decompression codes
    GLint clear,eoi,avail,oldcode;
    GLint datasize,codesize,codemask;

    register GLint code,count;
    register GLuint datum = 0;
    register GLint bits = 0;
    register GLubyte *ch;

    GLint prefix[4096];
    GLubyte suffix[4096];
    GLubyte stack[4096];
    GLubyte *stackp;
    GLubyte *raster=(GLubyte *)NULL; // holds the colour table indicies.
    GLubyte *fill;

    if (!TableSize)
    {
        cerr << "Sv_Raster::_ReadGIFLZW() - No colour table for GIF file!\n";
        return (-1);
    }

    // Null out raster image to white. (Should technically be the bkgnd colour)

    for (n=0; n<(_Width*_Height*4); n++)
        _Image[n] = (GLubyte) 255;

    // Read in initial minimum code size

    n = fread(inBuffer,1,1,in);

    if (!n)
    {
        cerr << "Sv_Raster::_ReadGIFLZW() - Unexpected EOF.\n";
        return (-1);
    }

    raster = new GLubyte [_Width*_Height+128]; // Extra fudge for broken GIFs
    fill = raster;

    datasize = (GLint) inBuffer[0];

    clear = 1 << datasize;
    eoi = clear + 1;
    avail = clear + 2;
    oldcode = -1;
    codesize = datasize + 1;
    codemask = (1<<codesize)-1;
    
    // Initialize decoding table

    for(code=0; code<clear; code++)
    {
        prefix[code] = 0;
        suffix[code] = (GLubyte) code;
    }
    stackp = stack;

    for (count = getc(in); count > 0; count = getc(in))
    {
        n = fread(inBuffer,1,count,in);

        if (n < count)
        {
            cerr << "Sv_Raster::_ReadGIFLZW() - Unexpected EOF decoding data.\n";
            delete [] raster;
            return (-1);
        }
	
        for (ch = inBuffer; count-- > 0; ch++)
        {
            datum += (unsigned long) *ch << bits;
            bits += 8;
	    
            while (bits >= codesize)
            {
		
                code = datum & codemask;
                datum >>= codesize;
                bits -= codesize;

                // Kludge for broken GIF files
                if (code == eoi)
                    goto exitloop;  // AHHHH a GOTO!  NOOOOO!!!!

                // Process the LZW code:

static GLubyte firstchar;
GLint  incode;

                if (code == clear)      // clear code
                {
                    codesize = datasize + 1;
                    codemask = (1<<codesize)-1;
                    avail = clear+2;
                    oldcode = -1;
                }
                else if (oldcode == -1)
                {
                    *fill++ = suffix[code];
                    firstchar = oldcode = code;
                }
                else if (code > avail)
                {
                    cerr << "Sv_Raster::_ReadGIFLZW() -  LZW code out of range.\n";
                    delete [] raster;
                    return (-1);
                }
                else
                {
                    incode = code;
                    if (code == avail)
                    {
                        *stackp++ = firstchar;
                        code = oldcode;
                    }

                    while (code > clear)
                    {
 
                        *stackp++ = suffix[code];
                        code = prefix[code];
                    }

                    *stackp++ = firstchar = suffix[code];
                    prefix[avail] = oldcode;
                    suffix[avail] = firstchar;
                    avail++;

                    if (((avail & codemask) == 0) && (avail < 4096))
                    {
                        codesize++;
                        codemask += avail;
                    }
                    oldcode = incode;
                    do
                    {
 
                        *fill++ = *--stackp;
                    } while(stackp > stack);
                }
            }
        }

        if (fill >= raster + _Width*_Height)
        {
            cerr << "Sv_Raster::_ReadGIFLZW() - Gif raster space full before end of input code.\n"
                 << "Attempting to continue.\n";

            break;
        }
    }

exitloop:

    if (fill != raster + _Width*_Height)
    {
        cerr << "Sv_Raster::_ReadGIFLZW() - Unexpected gif size: " << (GLint)(fill-raster)
             << " bytes.  Expected " << _Width * _Height << " bytes.\n"
             << "Attempting to continue.\n";
    }

    // We now have the decompressed colour indicies.
    // Now we must expand these from the colour table into the
    // raster _Image data array.

register GLint row,col;
GLubyte  GammaTable[256];
GLfloat  GIFGAMMA = 1.5; // Smaller values = brighter colours.

    // First build the gamma table used to modify the
    // colour table values for the GIF.  (Interpolates colours for
    // GIFs generated from < 24 bit palette, or with < 256 colours)

    for (n=0; n<256; n++)
        GammaTable[n] = (GLubyte) 0xff*pow(n/255.0,GIFGAMMA)+0.5;

    // Initialize the GLUT values to the gamma corrected values.

    for (n=0; n<TableSize; n++)
    {
        ColourTable[n].red   = GammaTable[ColourTable[n].red];
        ColourTable[n].green = GammaTable[ColourTable[n].green];
        ColourTable[n].blue  = GammaTable[ColourTable[n].blue];
    }

    // Render the image into the raster _Image array.

    register GLint currow = 0;
    
    if (interlaced)
    {
	
        for (row = 0; row < _Height; row += 8)
        {
	    
            for (col=0; col < _Width; col++)
            {
                _Image[(_Height-row-1)*_Width*4 + col*4] =
                        ColourTable[raster[currow*_Width+col]].red;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 1] =
                        ColourTable[raster[currow*_Width+col]].green;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 2] =
                        ColourTable[raster[currow*_Width+col]].blue;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 3] =
                        (raster[currow*_Width+col] == ti) ? 0 : 255;
 
            }
	    ++currow;
        }
	
       for (row = 4; row < _Height; row += 8)
        {

            for (col=0; col < _Width; col++)
            {
                _Image[(_Height-row-1)*_Width*4 + col*4] =
                        ColourTable[raster[currow*_Width+col]].red;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 1] =
                        ColourTable[raster[currow*_Width+col]].green;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 2] =
                        ColourTable[raster[currow*_Width+col]].blue;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 3] =
                        (raster[currow*_Width+col] == ti) ? 0 : 255;
  
            }
	    ++currow;
        }

       for (row = 2; row < _Height; row += 4)
        {

            for (col=0; col < _Width; col++)
            {
                _Image[(_Height-row-1)*_Width*4 + col*4] =
                        ColourTable[raster[currow*_Width+col]].red;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 1] =
                        ColourTable[raster[currow*_Width+col]].green;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 2] =
                        ColourTable[raster[currow*_Width+col]].blue;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 3] =
                        (raster[currow*_Width+col] == ti) ? 0 : 255;

            }
	    ++currow;
        }

       for (row = 1; row < _Height; row += 2)
        {

            for (col=0; col < _Width; col++)
            {
                _Image[(_Height-row-1)*_Width*4 + col*4] =
                        ColourTable[raster[currow*_Width+col]].red;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 1] =
                        ColourTable[raster[currow*_Width+col]].green;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 2] =
                        ColourTable[raster[currow*_Width+col]].blue;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 3] =
                        (raster[currow*_Width+col] == ti) ? 0 : 255;
 
            }
	    ++currow;
        }
    }
    else
    {
       for (row = 0; row < _Height; row ++)
        {
            for (col=0; col < _Width; col++)
            {
                _Image[(_Height-row-1)*_Width*4 + col*4] =
                        ColourTable[raster[row*_Width+col]].red;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 1] =
                        ColourTable[raster[row*_Width+col]].green;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 2] =
                        ColourTable[raster[row*_Width+col]].blue;

                _Image[(_Height-row-1)*_Width*4 + col*4 + 3] =
                        (raster[row*_Width+col] == ti) ? 0 : 255;
            }
        }
    }

    // And now we should have our GIF loaded into the _Image array.
    // Clean up and exit.

    delete [] raster;

    return (0); // Success.

}

GLint Sv_Raster::_WriteGIF(FILE *out)
{
    cerr << "Sv_Raster::_WriteGIF() - GIF File writing is not implemented.\n";
    fclose(out);
    return (-1);
}


#endif // SV_RASTER_GIFSUP

/* ===========================   END GIF   ================================= */

/* ===========================   TDR       ================================= */

#ifndef SV_RASTER_TDRSUP

GLint Sv_Raster::_ReadTDRScalar(const char *fileName)
{
    cerr << "Unable to load: " << fileName << " as TDRScalar file.\n"
	 << "TDRScalar support has been compiled out of the Sv_Raster library.\n";
	 
    return (-1);
}

GLint Sv_Raster::_WriteTDRScalar(const char *outFile)
{
    cerr << "Sv_Raster::_WriteTDRScalar() - TDRScalar support has been compiled out of the Sv_Raster library.\n";
    return (-1);
}

#else

#include "TDR_Manager.h"
#include "TDR_Scalar.h"

GLint Sv_Raster::_sizeTDR(const char *fileName,GLint &width,GLint &height)
{
	cerr << "Sv_Raster::_sizeTDR not yet implemented!\n";
	return -1;
}

// Currently unimplemented, this function will eventually
// read in the TDR Scalar file format.

GLint Sv_Raster::_ReadTDRScalar(const char *fileName)
{

    TDR_Manager tdrin;
    TDR_Scalar scalarin;
    GLuint tag;
    GLint  result;
    
    register GLint x, y;
    
    result = 0;    // Assume success
    
    if (tdrin.OpenFile((char *)fileName, "r"))
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }
    
    if (tdrin.ReadHeader())
    {
		cerr << "Sv_Raster::_ReadTDRScalar() - " << fileName << "is not a legal TDR file.\n";
		tdrin.CloseFile();
		return (-1);
    }
    
    while (((tag = tdrin.ReadTag()) != TDR_TAG_SCALAR) && (tag != TDR_TAG_EOF))
    {
		tdrin.SkipBlock();
    }
	
    if (tag != TDR_TAG_SCALAR)
    {
		cerr << "Sv_Raster::_ReadTDRScalar() - " << fileName << " is not a scalar TDR file.\n";
		tdrin.CloseFile();
		return (-1);
    }

    scalarin.Read(&tdrin);
    
//    if( scalarin.GetDataType() != TDR_SCALAR_TYPE_ULONG )

    if( scalarin.GetNumDimensions() != 2 )
    {
		cerr << "ERROR: Expected TDR two dimensional scalar!\n";
		tdrin.CloseFile();
		return (-1);
    }
    
    Destroy();
    
    _Width = scalarin.GetDimension( 1 );
    _Height = scalarin.GetDimension( 2 );
    
    _AllocateNewImage();
    
    // read the min/max

    switch( scalarin.GetDataType() )
    {
		case TDR_SCALAR_TYPE_ULONG:
	    	GLubyte uldata[sizeof(GLuint)];
	    	fread(uldata, sizeof (GLuint), 1, tdrin.GetFilePTR());
	    	fread(uldata, sizeof (GLuint), 1, tdrin.GetFilePTR());

	    	// read the data.  Nicely, it's stored in OpenGL friendly format! :^)	    
	    	for (y=0; y<_Height; y++)
	    	{
				for (x=0; x<_Width; x++)
				{
		    		fread(uldata, sizeof(GLuint), 1, tdrin.GetFilePTR());

		    		_Image[y * _Width * 4 + x * 4]   = uldata[0];
		    		_Image[y * _Width * 4 + x * 4+1] = uldata[1];
		    		_Image[y * _Width * 4 + x * 4+2] = uldata[2];
		    		_Image[y * _Width * 4 + x * 4+3] = uldata[3];
				}
	    	}
	    	break;

		case TDR_SCALAR_TYPE_USHORT:
	    	GLushort usdata;
	    	GLubyte  uscolor;
	    	fread(&usdata, sizeof (GLushort), 1, tdrin.GetFilePTR());
	    	fread(&usdata, sizeof (GLushort), 1, tdrin.GetFilePTR());

	    	// read the data.  Nicely, it's stored in OpenGL friendly format! :^)	    
	    	for (y=0; y<_Height; y++)
	    	{
				for (x=0; x<_Width; x++)
				{
		    		fread(&usdata, sizeof(GLushort), 1, tdrin.GetFilePTR());
		    		uscolor = (GLubyte) (usdata >> 8);

		    		_Image[y * _Width * 4 + x * 4]   = uscolor;
		    		_Image[y * _Width * 4 + x * 4+1] = uscolor;
		    		_Image[y * _Width * 4 + x * 4+2] = uscolor;
		    		_Image[y * _Width * 4 + x * 4+3] = 255;
				}
	    	}
	    	break;

		default:
	    	cerr << "ERROR: Scalar format not supported for raster operations!\n";
	    	result = -1;
	    
    }
    
    tdrin.CloseFile();
    return( result );
}


GLint Sv_Raster::_WriteTDRScalar(const char *fileName)
{
    TDR_Manager tdrout;
    TDR_Scalar scalarout;
    
    GLubyte data[sizeof(GLuint)];
    register GLint x, y;
    static GLuint min=0x00000000, max=0xFFFFFFFF;
    
    if (tdrout.OpenFile((char *)fileName, "w"))    
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }

    tdrout.AddComment("Created by UNB HCI Sv_Raster Class.\n");
    tdrout.WriteHeader();
    
    scalarout.SetNumDimensions(2);
    scalarout.SetDimension(1, _Width);
    scalarout.SetDimension(2, _Height);
    scalarout.SetDataType(TDR_SCALAR_TYPE_ULONG);
    
    tdrout.WriteTag(TDR_TAG_SCALAR);
    scalarout.Write(&tdrout);
    
    // Write min/max
    
    fwrite(&min, sizeof(GLuint), 1, tdrout.GetFilePTR());
    fwrite(&max, sizeof(GLuint), 1, tdrout.GetFilePTR());

    // Write the data

    for (y=0; y<_Height; y++)
    {
		for (x=0; x<_Width; x++)
		{
	    	data[0] = _Image[y * _Width * 4 + x * 4];
	    	data[1] = _Image[y * _Width * 4 + x * 4+1];
	    	data[2] = _Image[y * _Width * 4 + x * 4+2];
	    	data[3] = _Image[y * _Width * 4 + x * 4+3];

	    	fwrite(data, sizeof(GLuint), 1, tdrout.GetFilePTR());
		}
    }

    tdrout.SetDataBlockLength();
    tdrout.WriteTag(TDR_TAG_EOF, 0);
    tdrout.CloseFile();

    return (0);
}

#endif // SV_RASTER_TDRSUP

/* ===============================  END TDR   ============================== */

// NOTE: This class uses various libraries in its load/save
// routines.  For this reason, and program using this class
// must link with the following libraries:
//
// -ljpeg -ltiff

/*
 *  Sv_Raster::Sv_Raster()
 * 
 *  Default constructor.  No image is loaded.
 * 
 */
 
Sv_Raster::Sv_Raster()
{
    _Height = _Width = 0;
    _Image = (GLubyte *) NULL;
}

/*
 *  Sv_Raster::Sv_Raster()
 * 
 *  Constructor.  An attempt is made to initialize the class
 *  with a raster image from the given file name and optional
 *  file type.  See the LoadFromFile() method for information
 *  on the usefulness of the fileType parameter.
 * 
 */
 
Sv_Raster::Sv_Raster(const char *fileName, SVR_FileType fileType)
{
    _Height = _Width = 0;
    _Image = (GLubyte *) NULL;
    
    if (LoadFromFile( fileName, fileType ))
    {
		cerr << "Sv_Raster::Sv_Raster() - Error loading file: " << fileName << endl;
		Destroy(); // Ensure there is no image loaded.
    }
}

/*
 *  Sv_Raster::Sv_Raster()
 * 
 *  Constructor.  The class is initalized from raster data
 *  in the given memory block, passed as dataBlock.
 *  Also passed are the width and height of the block,  in pixels, 
 *  and the datatype of the block.  Note that the data block is
 *  expected to be stored in OpenGL format: the first pixel in 
 *  the data should be the lower left hand corner of the image.
 *  (i.e. the image is stored bottom-up)
 * 
 */
 
Sv_Raster::Sv_Raster( void *dataBlock, GLint dataW, GLint dataH, SVR_DataType dataType )
{
    _Height = _Width = 0;
    _Image = (GLubyte *) NULL;
    
    if (LoadFromMemory( dataBlock, dataW, dataH, dataType ))
    {
		cerr << "Sv_Raster::Sv_Raster() - Error loading from memory block!\n";
		Destroy(); // Ensure there is no image loaded.
    }
    
}

/*
 *  Sv_Raster::Sv_Raster()
 * 
 *  Copy constructor.  Initializes class from an existing Sv_Raster
 *  instantiation.
 * 
 */
 
Sv_Raster::Sv_Raster( Sv_Raster &otherRaster )
{
    _Height = _Width = 0;
    _Image = (GLubyte *) NULL;
        
    if (otherRaster.ImageLoaded())
    {
		_Height = otherRaster.GetHeight();
		_Width = otherRaster.GetWidth();

		// Only bother to copy the image if there
		// is an image to copy.

		if (_Width && _Height)
		{
	    	// Make a new block for the image.
	    	_Image = new GLubyte [ _Height * _Width * 4 ];

	    	otherRaster.SaveToMemory (_Image);
		}   
    }
    
}


/*
 *  Sv_Raster::Sv_Raster()
 * 
 *  Copy constructor.  Takes a reference to an existing
 *  instantiation of Sv_Raster and initializes the class from
 *  the image stored therein.
 * 
 */

Sv_Raster::Sv_Raster( Sv_Raster *otherRaster )
{
    _Height = _Width = 0;
    _Image = (GLubyte *) NULL;
        
    if (otherRaster->ImageLoaded())
    {
		_Height = otherRaster->GetHeight();
		_Width = otherRaster->GetWidth();

		// Only bother to copy the image if there
		// is an image to copy.

		if (_Width && _Height)
		{
	    	// Make a new block for the image.
	    	_Image = new GLubyte [ _Height * _Width * 4 ];

	    	otherRaster->SaveToMemory (_Image);
		}   
    }
    
}

// Allocates a "blank" image array which can be written to
// with various other methods below.  If 'clearColour' is
// NULL then the clear colour defaults to black with an 
// alpha value of 0.0f.  Note that if the dataType is SVR_RGBA,
// the clearColour is expected to be an array of 4 GLfloats in R,G,B,A order.

Sv_Raster::Sv_Raster( GLint dataWidth, GLint dataHeight, GLfloat *clearColour, SVR_DataType dataType )
{
    _Width = _Height = 0;
    _Image = NULL;
    
    BuildBlankImage(dataWidth, dataHeight, clearColour, dataType);   
}

/*
 *  Sv_Raster::~Sv_Raster()
 * 
 *  Destructor.
 * 
 */
 
Sv_Raster::~Sv_Raster()
{
    Destroy();
}

SVR_FileType Sv_Raster::FindRasterType( char *fileName )
{
    FILE *in;    
    SVR_FileType realFileType;
    
    in = fopen (fileName, "rb");
    
    if (!in)
    {
		cerr << "Sv_Raster::FindRasterType() - Error opening file: " << fileName << endl;
		return (SVR_Unsupported);
    }

    realFileType = Sv_Raster::_FindFileType (in );
    
    fclose (in);
    
    // Now, ensure that the file type has support coded into the
    // library, as the number of formats can be conditionally
    // chosen with the appropriate #defines.  (See Sv_Raster.h)
    
    if (IsSupported( realFileType ))
		return (realFileType);
    else
		return (SVR_Unsupported);
}

GLboolean Sv_Raster::IsSupported( SVR_FileType ft )
{
    
    // First, do a range check.
    
    if ((ft > SVR_Unsupported) && (ft < SVR_DUMMY_TAG_END))
    {

		// Now check against the types.

		// PPM support is always provided:

		if ((ft >= SVR_PPM1) && (ft <= SVR_PPM6))
	    	return (GL_TRUE);
		else if ((ft == SVR_RAWRGB)||(ft == SVR_RGBHCI)||(ft == SVR_IRISRGB))
	    	return (GL_TRUE);

		// Conditionally compiled support:

		if (ft == SVR_TIFF_BE || ft == SVR_TIFF_LE)
		{
	#ifdef SV_RASTER_TIFFSUP
	    	return (GL_TRUE);
	#else
	    	return (GL_FALSE);
	#endif
		}
		else if (ft == SVR_GIF)
		{
	#ifdef SV_RASTER_GIFSUP
	    	return (GL_TRUE);
	#else
	    	return (GL_FALSE);
	#endif
		}
		else if (ft == SVR_JPEG)
		{
	#ifdef SV_RASTER_JPEGSUP
	    	return (GL_TRUE);
	#else
	    	return (GL_FALSE);
	#endif
		}
		else if (ft == SVR_TDRSCALAR)
		{
	#ifdef SV_RASTER_TDRSUP
	    	return (GL_TRUE);
	#else
	    	return (GL_FALSE);
	#endif
		}
		else
	    	return (GL_FALSE);
	
    }
    else
		return (GL_FALSE); // Out of range.
}


bool Sv_Raster::GetSize( const char *fileName, int &width, int &height, SVR_FileType fileType)
{

    SVR_FileType realFileType;
    FILE *in;


    in = fopen (fileName, "rb");
    
    if (!in)
    {
		return false;
    }
    
    realFileType = _FindFileType (in );
    
    if ((fileType != SVR_AUTODETECT) && (realFileType != fileType))
    {
		fclose(in);
		cerr << "Sv_Raster::LoadFromFile() - Error file " << fileName 
	    	 << " is not of the requested type.\n";
		return false;
    }
 
    fclose(in);
   
    // each of the _Read* functions closes the file for us.
    
    switch (realFileType)
    {
		case SVR_PPM1:
	    	return (_sizePPM1(fileName,width,height));

		case SVR_PPM2:
	    	return (_sizePPM2(fileName,width,height));

		case SVR_PPM3:
	    	return (_sizePPM3(fileName,width,height));

		case SVR_PPM4:
	    	return (_sizePPM4(fileName,width,height));

		case SVR_PPM5:
	    	return (_sizePPM5(fileName,width,height));

		case SVR_PPM6:
	    	return (_sizePPM6(fileName,width,height));

		case SVR_JPEG:

	#ifdef SV_RASTER_JPEGSUP
	    	return (_sizeJPEG(fileName,width,height));    
	#else
	    	cerr << "Sv_Raster::GetSize() - JPEG support compiled out.\n";
	    	return false;
	#endif

		case SVR_GIF:

	#ifdef SV_RASTER_GIFSUP
	    	return (_sizeGIF(fileName,width,height));
	#else
	    	cerr << "Sv_Raster::GetSize() - GIF support compiled out.\n";
	    	return false;
	#endif

		case SVR_TIFF_BE:
		case SVR_TIFF_LE:

	#ifdef SV_RASTER_TIFFSUP
		return _sizeTIFF(fileName,width,height);
	#else
	    	cerr << "Sv_Raster::GetSize() - TIFF support compiled out.\n";
	    	return false;
	#endif

		case SVR_TDRSCALAR:

	#ifdef SV_RASTER_TDRSUP
	    	return (_sizeTDRScalar(fileName,width,height));    
	#else
	    	cerr << "Sv_Raster::GetSize() - TDR Scalar support compiled out.\n";
	    	return false;
	#endif

		case SVR_IRISRGB:
	    	return (_sizeIrisRGB(fileName,width,height));

		case SVR_RGBHCI:
	    	return (_sizeRGBHCI(fileName,width,height));

		case SVR_RAWRGB:
	    	return (_sizeRawRGB(fileName,width,height));

		default:
	    	cerr << "Sv_Raster::GetSize() - Unknown file type for " << fileName << endl;
	    	return false;
    }
    return false;
}


/*
 *  Sv_Raster::LoadFromFile()
 * 
 *  Loads a raster image from the specified file.  An optional file type
 *  can also be specified (see Sv_Raster.h).  If the file type is not
 *  specified,  the class will attempt to identify the file and will
 *  load it.  If the file type is specified,  it is used as a filter
 *  to ensure that the given file is of the specified type.  If the file is
 *  not,  the load will fail with an informative message.
 * 
 */
 
GLint Sv_Raster::LoadFromFile( const char *fileName, SVR_FileType fileType )
{
    SVR_FileType realFileType;
    FILE *in;

#ifdef SV_RASTER_TIFFSUP
    GLint tiffstatus;
#endif

    in = fopen (fileName, "rb");
    
    if (!in)
    {
		return (-1);
    }
    
    realFileType = _FindFileType (in );
    
    if ((fileType != SVR_AUTODETECT) && (realFileType != fileType))
    {
		fclose(in);
		cerr << "Sv_Raster::LoadFromFile() - Error file " << fileName 
	    	 << " is not of the requested type.\n";
		return (-1);
    }
 
    fclose(in);
   
    // each of the _Read* functions closes the file for us.
    
    switch (realFileType)
    {
		case SVR_PPM1:
	    	return (_ReadPPM1(fileName));

		case SVR_PPM2:
	    	return (_ReadPPM2(fileName));

		case SVR_PPM3:
	    	return (_ReadPPM3(fileName));

		case SVR_PPM4:
	    	return (_ReadPPM4(fileName));

		case SVR_PPM5:
	    	return (_ReadPPM5(fileName));

		case SVR_PPM6:
	    	return (_ReadPPM6(fileName));

		case SVR_JPEG:

	#ifdef SV_RASTER_JPEGSUP
	    	return (_ReadJPEG(fileName));    
	#else
	    	cerr << "Sv_Raster::LoadFromFile() - JPEG support compiled out.\n";
	    	return (-1);
	#endif

		case SVR_GIF:

	#ifdef SV_RASTER_GIFSUP
	    	return (_ReadGIF(fileName));
	#else
	    	cerr << "Sv_Raster::LoadFromFile() - GIF support compiled out.\n";
	    	return (-1);
	#endif

		case SVR_TIFF_BE:
		case SVR_TIFF_LE:

	#ifdef SV_RASTER_TIFFSUP
		tiffstatus = _ReadTIFF(fileName);
		if(!tiffstatus && realFileType == SVR_TIFF_LE)
		  ReverseChannels();
	    	return (tiffstatus);
	#else
	    	cerr << "Sv_Raster::LoadFromFile() - TIFF support compiled out.\n";
	    	return (-1);
	#endif

		case SVR_TDRSCALAR:

	#ifdef SV_RASTER_TDRSUP
	    	return (_ReadTDRScalar(fileName));    
	#else
	    	cerr << "Sv_Raster::LoadFromFile() - TDR Scalar support compiled out.\n";
	    	return (-1);
	#endif

		case SVR_IRISRGB:
	    	return (_ReadIrisRGB(fileName));

		case SVR_RGBHCI:
	    	return (_ReadRGBHCI(fileName));

		case SVR_RAWRGB:
	    	return (_ReadRawRGB(fileName));

		default:
	    	cerr << "Sv_Raster::LoadFromFile() - Unknown file type for " << fileName << endl;
	    	return (-1);
    }
        
}

// The header size is used to identify a file by reading in enough
// of the file to identify it.  

const GLint SVR_HEADERSIZE = 256;

/*
 *  Sv_Raster::_FindFileType()
 * 
 *  This is an internal method which is used to identify
 *  the type of raster image stored in the specified
 *  (and previously fopen()ed) file.
 * 
 *  Returns an SVR_FileType which identifies the file contents.
 *  See Sv_Raster.h for the list of supported types.
 * 
 */
 
SVR_FileType Sv_Raster::_FindFileType( FILE *in )
{
    unsigned char header[SVR_HEADERSIZE];
    int c;
    
    c = fread(header, 1, SVR_HEADERSIZE-1, in);
    
    rewind(in);
    
    if (c < SVR_HEADERSIZE-1)
	return SVR_Unsupported;	// Error reading file header.
	
    // check for PPM files.
    
    header[SVR_HEADERSIZE-1] = '\0'; // NULL terminate header.
    
    if (header[0] == 'P')
    {
		if (header[1] == '1')
	    	return SVR_PPM1;
		else if (header[1] == '2')
	    	return SVR_PPM2;
		else if (header[1] == '3')
	    	return SVR_PPM3;
		else if (header[1] == '4')
	    	return SVR_PPM4;
		else if (header[1] == '5')
	    	return SVR_PPM5;
		else if (header[1] == '6')
	    	return SVR_PPM6;
		else
	    	return SVR_Unsupported;
    }
    
    if ((header[0] == 0xFF) && (header[1] == 0xD8)
	     && (header[2] == 0xFF) && (header[3] == 0xE0 || header[3] == 0xE1 || header[3] == 0xE2)) //0xE1 check added by mdp 7/30/2007
    {
		return (SVR_JPEG);
    }
    else if ((header[0] == 'G') && (header[1] == 'I') && (header[2] == 'F'))
    {
		return (SVR_GIF);
    }
    else if ((header[0] == 0x4D) && (header[1] == 0X4D) && (header[2] == 0x00) && (header[3] == 0x2A))
    {
		return (SVR_TIFF_BE); // big endian TIFF
    }
    else if ((header[0] == 0x49) && (header[1] == 0X49) && (header[2] == 0x2A) && (header[3] == 0x00))
    {
		return (SVR_TIFF_LE); // little endian TIFF
    }
    else if ((header[0] == 'R') && (header[1] == 'G')  && (header[2] == 'B') &&
	     (header[3] == 'H') && (header[4] == 'C') && (header[5] == 'I'))
    {
		return (SVR_RGBHCI); //HCIRGB - Gabor texture generated by colin's code.
	
    }
    	else if ((header[0] == 0x01) && (header[1] == 0xDA))
    {
	return (SVR_IRISRGB); // Iris RGB files.
    }
    	else if (!strncmp((char *)header, "%% TDR", 6))
    {
		return (SVR_TDRSCALAR);
    }
    else
    {
		// other files would go here
		// (i.e. RAWRGB,  etc)
		return (SVR_Unsupported);
    }
}



GLint Sv_Raster::_sizePPM1(const char *fileName,GLint &width,GLint &height)
{
	cerr << "Sv_Raster::_sizePPM1 not yet implemented!\n";
	return -1;
}

// Read a PPM file into the _Image array.
// Note: reads as GL_RGBA data.  Pads the alpha
// component as 255 (1.0f)
// PPM1 files are ASCII B/W values ('0' or '1', whitespace separated)

GLint Sv_Raster::_ReadPPM1(const char *fileName)
{
    GLubyte inbuf[1024];
    int x, y;
    int ing;
    FILE *in;
    
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }
    
    // read off the header, determine the size of the file.

    char * dummy = fgets((char *)inbuf, 1023, in);
    dummy = fgets((char *)inbuf, 1023, in);
    
    // ignore comment lines
    
    while (inbuf[0] == '#')
        dummy = fgets((char *)inbuf, 1023, in);


    // Destroy old image, if any.
    
    Destroy();

    // the buffer now has two numbers separated
    // by spaces.  These are the width and height
    // of the image, respectively.
    
    sscanf((char *)inbuf, "%d %d", &_Width, &_Height);
    
    _AllocateNewImage(); 
        
    for (y=_Height-1; y>=0; y--)
    {
		for (x=0; x<_Width; x++)
		{
	    	ing  = -1;

                if (fscanf(in, "%d", &ing) != 1)
	    	{
				cerr << "Sv_Raster::_ReadPPM1() - Unexpected end of file.\n";
				fclose(in);
				return (-1);
	    	}

	    	if (ing == 1)
				ing = 255;
	    	else
				ing = 0;

	    	_Image[(y*_Width*4)+(x*4)] = (GLubyte) ing;
	    	_Image[(y*_Width*4)+(x*4)+1] = (GLubyte) ing;
	    	_Image[(y*_Width*4)+(x*4)+2] = (GLubyte) ing;
	    	_Image[(y*_Width*4)+(x*4)+3] = 255; // Alpha value.
		}
    }
    
    fclose (in);
    
    return (0);
}

GLint Sv_Raster::_sizePPM2(const char *fileName,GLint &width,GLint &height)
{
	cerr << "Sv_Raster::_sizePPM2 not yet implemented!\n";
	return -1;
}

// Read a PPM file into the _Image array.
// Note: reads as GL_RGBA data.  Pads the alpha
// component as 255 (1.0f)
// PPM2 files are ASCII Greyscale values.

GLint Sv_Raster::_ReadPPM2(const char *fileName)
{
    GLubyte inbuf[1024];
    int x, y;
    int ing;
    FILE *in;
    
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }
    
    // read off the header, determine the size of the file.

    char * dummy = fgets((char *)inbuf, 1023, in);
    dummy = fgets((char *)inbuf, 1023, in);
    
    // ignore comment lines
    
    while (inbuf[0] == '#')
        dummy = fgets((char *)inbuf, 1023, in);

    // Destroy old image, if any.
    
    Destroy();

    // the buffer now has two numbers separated
    // by spaces.  These are the width and height
    // of the image, respectively.
    
    sscanf((char *)inbuf, "%d %d", &_Width, &_Height);
    
    _AllocateNewImage(); 
    
    // remove the rest of the header.
    dummy = fgets((char *)inbuf, 1023, in);
    
    for (y=_Height-1; y>=0; y--)
    {
		for (x=0; x<_Width; x++)
		{
	    	ing  = -1;

                if (fscanf(in, "%d", &ing) != 1)
	    	{
				cerr << "Sv_Raster::_ReadPPM2() - Unexpected end of file.\n";
				fclose(in);
				return (-1);
	    	}

	    	_Image[(y*_Width*4)+(x*4)] = (GLubyte) ing;
	    	_Image[(y*_Width*4)+(x*4)+1] = (GLubyte) ing;
	    	_Image[(y*_Width*4)+(x*4)+2] = (GLubyte) ing;
	    	_Image[(y*_Width*4)+(x*4)+3] = 255; // Alpha value.
		}
    }
    
    fclose (in);
    
    return (0);
}

GLint Sv_Raster::_sizePPM3(const char *fileName,GLint &width,GLint &height)
{
	cerr << "Sv_Raster::_sizePPM3 not yet implemented!\n";
	return -1;
}

// Read a PPM file into the _Image array.
// Note: reads as GL_RGBA data.  Pads the alpha
// component as 255 (1.0f)
// PPM3 files are ASCII RGB values.

GLint Sv_Raster::_ReadPPM3(const char *fileName)
{
    GLubyte inbuf[1024];
    int x, y;
    int inr, ing, inb;
    FILE *in;
    
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }
    
    // read off the header, determine the size of the file.

    char * dummy = fgets((char *)inbuf, 1023, in);
    dummy = fgets((char *)inbuf, 1023, in);
    
    // ignore comment lines
    
    while (inbuf[0] == '#')
        dummy = fgets((char *)inbuf, 1023, in);


    // Destroy old image, if any.
    
    Destroy();


    // the buffer now has two numbers separated
    // by spaces.  These are the width and height
    // of the image, respectively.
    
    sscanf((char *)inbuf, "%d %d", &_Width, &_Height);
    
    _AllocateNewImage(); 
    
    // remove the rest of the header.
    dummy = fgets((char *)inbuf, 1023, in);
    
    for (y=_Height-1; y>=0; y--)
    {
		for (x=0; x<_Width; x++)
		{
	    	inr = ing = inb = -1;

                if (fscanf(in, "%d %d %d", &inr, &ing, &inb) != 3 || (inr < 0) || (ing < 0) || (inb < 0))
	    	{
				cerr << "Sv_Raster::_ReadPPM3() - Unexpected end of file.\n";
				fclose(in);
				return (-1);
	    	}

	    	_Image[(y*_Width*4)+(x*4)] = (GLubyte) inr;
	    	_Image[(y*_Width*4)+(x*4)+1] = (GLubyte) ing;
	    	_Image[(y*_Width*4)+(x*4)+2] = (GLubyte) inb;
	    	_Image[(y*_Width*4)+(x*4)+3] = 255; // Alpha value.
		}
    }
    
    fclose (in);
    
    return (0);
}

GLint Sv_Raster::_sizePPM4(const char *fileName,GLint &width,GLint &height)
{
	cerr << "Sv_Raster::_sizePPM4 not yet implemented!\n";
	return -1;
}

// Read a PPM file into the _Image array.
// Note: reads as GL_RGBA data.  Pads the alpha
// component as 255 (1.0f)
//
// PPM4 files are black and white binary files.
// one bit per pixel.

GLint Sv_Raster::_ReadPPM4(const char *fileName)
{
//    GLubyte inbuf[1024];
//    int x, y;
    FILE *in;
    
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }

    cerr << "Sv_Raster::_ReadPPM4() - Reading PPM4 files is currently unimplemented.\n";
    fclose(in);
    return (-1);
    
    // read off the header, determine the size of the file.

//    fgets((char *)inbuf, 1023, in);
//    fgets((char *)inbuf, 1023, in);
    
    // ignore comment lines
    
//    while (inbuf[0] == '#')
//	fgets((char *)inbuf, 1023, in);



    
    // Destroy old image, if any.

/**    
    Destroy();
 
     // the buffer now has two numbers separated
    // by spaces.  These are the width and height
    // of the image, respectively.
    
    sscanf((char *)inbuf, "%ld %ld", &_Width, &_Height);
   
    _AllocateNewImage(); 
    
    
    for (y=_Height-1; y>=0; y--)
    {
	for (x=0; x<_Width; x+=8)
	{
	    if ((cnt = fread(inbuf, 1, 1, in)) < 1)
	    {
		cerr << "Sv_Raster::_ReadPPM4() - ERROR: File seems too short!\n";
		fclose (in);
		return (-1);
	    }
**/	    
	    // each pixel has one bit value associated with it.
	    // We read in each byte of the file, and set 
/**	    THIS IS WRONG AND MUST BE FIXED.
	    _Image[(y*_Width*4)+(x*4)] = inbuf[0];
	    _Image[(y*_Width*4)+(x*4)+1] = inbuf[1];
	    _Image[(y*_Width*4)+(x*4)+2] = inbuf[2];
	    _Image[(y*_Width*4)+(x*4)+3] = 255; 
	}
    }
    
    fclose (in);
    
    return (0);
**/

}

GLint Sv_Raster::_sizePPM5(const char *fileName,GLint &width,GLint &height)
{
	cerr << "Sv_Raster::_sizePPM5 not yet implemented!\n";
	return -1;
}

// Read a PPM file into the _Image array.
// Note: reads as GL_RGBA data.  Pads the alpha
// component as 255 (1.0f)
// PPM5 files are binary grey scales.

GLint Sv_Raster::_ReadPPM5(const char *fileName)
{
    GLubyte inbuf[1024];
    int x, y;
    FILE *in;
    
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }
    
    // read off the header, determine the size of the file.

    char * dummy = fgets((char *)inbuf, 1023, in);
    dummy = fgets((char *)inbuf, 1023, in);
    
    // ignore comment lines
    
    while (inbuf[0] == '#')
        dummy = fgets((char *)inbuf, 1023, in);


    // Destroy old image, if any.
    
    Destroy();

    // the buffer now has two numbers separated
    // by spaces.  These are the width and height
    // of the image, respectively.
    
    sscanf((char *)inbuf, "%d %d", &_Width, &_Height);
    
    _AllocateNewImage(); 
    
    // remove the rest of the header.
    dummy = fgets((char *)inbuf, 1023, in);
    
    for (y=_Height-1; y>=0; y--)
    {
		for (x=0; x<_Width; x++)
		{
	    	if ((fread(inbuf, 1, 1, in)) < 1)
	    	{
				cerr << "Sv_Raster::_ReadPPM5() - ERROR: File seems too short!\n";
				fclose (in);
				return (-1);
	    	}

	    	_Image[(y*_Width*4)+(x*4)] = inbuf[0];
	    	_Image[(y*_Width*4)+(x*4)+1] = inbuf[0];
	    	_Image[(y*_Width*4)+(x*4)+2] = inbuf[0];
	    	_Image[(y*_Width*4)+(x*4)+3] = 255; // Alpha value.
		}
    }
    
    fclose (in);
    
    return (0);
}


GLint Sv_Raster::_sizePPM6(const char *fileName,GLint &width,GLint &height)
{
	cerr << "Sv_Raster::_sizePPM6 not yet implemented!\n";
	return -1;
}

// Read a PPM file into the _Image array.
// Note: reads as GL_RGBA data.  Pads the alpha
// component as 255 (1.0f)

GLint Sv_Raster::_ReadPPM6(const char *fileName)
{
    int cnt=0;
    GLubyte inbuf[1024];
    int x, y;
    FILE *in;
    
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }
    
    // read off the header, determine the size of the file.

    char *dummy = fgets((char *)inbuf, 1023, in);
    dummy = fgets((char *)inbuf, 1023, in);
    
    // ignore comment lines
    
    while (inbuf[0] == '#' && fgets((char *)inbuf, 1023, in));

    // Destroy old image, if any.
    
    Destroy();

    // the buffer now has two numbers separated
    // by spaces.  These are the width and height
    // of the image, respectively.
    
    sscanf((char *)inbuf, "%d %d", &_Width, &_Height);
   
    _AllocateNewImage(); 
    
    // remove the rest of the header.
    dummy = fgets((char *)inbuf, 1023, in);
    
    for (y=_Height-1; y>=0; y--)
    {
		for (x=0; x<_Width; x++)
		{
	    	if ((cnt = fread(inbuf, 1, 3, in)) < 3)
	    	{
				cerr << "Sv_Raster::_ReadPPM6() - ERROR: File seems too short!\n";
				cerr << "cnt = " << cnt << endl;
				fclose (in);
				return (-1);
	    	}

	    	_Image[(y*_Width*4)+(x*4)] = inbuf[0];
	    	_Image[(y*_Width*4)+(x*4)+1] = inbuf[1];
	    	_Image[(y*_Width*4)+(x*4)+2] = inbuf[2];
	    	_Image[(y*_Width*4)+(x*4)+3] = 255; // Alpha value.
		}
    }
    
    fclose (in);
    
    return (0);
}


GLint Sv_Raster::_sizeRGBHCI(const char *fileName,GLint &width,GLint &height)
{
	cerr << "Sv_Raster::_sizeRGBHCI not yet implemented!\n";
	return -1;
}


// RGBHCI is the format for loading the gabor texture.
// RGBHCI is an internal format which is generated by
// Colin's gabor texture generator, slightly modified
// to output an identifying header.  It probably doesn't
// need to be part of the main class.

GLint Sv_Raster::_ReadRGBHCI(const char *fileName)
{
    GLubyte buffer[1024];
    int x, y;
    FILE *in;
    
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }
    
    char *dummy = fgets((char *)buffer, 1023, in);
    dummy = fgets((char *)buffer, 1023, in);
    
    Destroy();
    
    sscanf((char *)buffer, "%d %d\n", &_Width, &_Height);
    
     _AllocateNewImage();
     
    for (y=0; y<_Height; y++)
    {
		for (x=0; x<_Width; x++)
		{
	    	if(fread(buffer, 1, 1, in)==1)
                {

	    	_Image[y * _Width * 4 + x * 4] = buffer[0];
	    	_Image[y * _Width * 4 + x * 4+1] = buffer[0];
	    	_Image[y * _Width * 4 + x * 4+2] = buffer[0];
	    	_Image[y * _Width * 4 + x * 4+3] = 255;
                }

		}
    }
    
    fclose (in);
    return (0);
    
}

GLint Sv_Raster::_sizeIrisRGB(const char *fileName,GLint &width,GLint &height)
{
	cerr << "Sv_Raster::_sizeIrisRGB not yet implemented!\n";
	return -1;
}


// This code parses and reads an IrisRGB file.
// Note that currently, B/W RGB files are not supported.

GLint Sv_Raster::_ReadIrisRGB(const char *fileName)
{
    char buffer[4096];
    int cnt;
    GLboolean isRLE = GL_FALSE;
    int	 bpc;	// bytes per component of colour.
    int  channels = 0;
    unsigned long *rleStartTab, *rleLengthTab;
    register GLint x;
    register GLint y;
    register GLint z;
    FILE *in;
    
    rleStartTab = NULL;
	rleLengthTab = NULL;
    
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }

    // Read the header
    
    cnt = fread(buffer, 1, 512, in);
    if (cnt < 512)
    {
		cerr << "Sv_Raster::_ReadIrisRGB() - Error reading header: Unexpected EOF.\n";
		fclose(in);
		return (-1);
    }
    
    // Process the header
    
    if (buffer[2] == 0x01)  // RGB file coding method
		isRLE = GL_TRUE;
    else if (buffer[2] != 0x00)
    {
		cerr << "Sv_Raster::_ReadIrisRGB() - Error parsing header: Unknown file compression type: " << buffer[2] << endl;
		fclose(in);
		return (-1);
    }
    
    bpc = (int) buffer[3];  // Bytes per colour component
    
    if (bpc != 1) 
    {
		cerr << "Sv_Raster::_ReadIrisRGB() - Error parsing header: Illegal number of bytes/colour: " << bpc << endl;
		cerr << "Only values of 1 and 2 are legal.  This class does not (currently) support\n";
		cerr << "2 Bytes/colour.\n";
		fclose(in);
		return (-1);
    }
    
    if ( *((unsigned short *)(&buffer[4])) != 3)
    {
		cerr << "Sv_Raster::_ReadIrisRGB() - This doesn't appear to be a straight RGB/RGBA Image file.\n";
		fclose(in);
		return (-1);
    }

    if (*((long *)(&buffer[104])) != 0)
    {
		cerr << "Sv_Raster::_ReadIrisRGB() - Error: unknown or obsolete Iris RGB file.\n";
		fclose(in); // Sigh.
		return (-1);
    }
    
    _Width = *((unsigned short *)(&buffer[6]));
    _Height = *((unsigned short *)(&buffer[8]));
    
    channels = *((unsigned short *)(&buffer[10])); // number of colour channels.
						    // 1 = BW, 2 = RGB, 3 = RGBA

    fseek(in, 512, SEEK_SET); // Ensure that we are at the start of the data.

    if (isRLE)
    {
		// RLE encoded Iris RGB file.  Read in the offset tables.

		rleStartTab = new unsigned long[_Height * channels];
		rleLengthTab = new unsigned long[_Height * channels];	

		cnt = fread(rleStartTab, sizeof(long), (_Height*channels), in);

		if (cnt < (_Height*channels))
		{
	    	cerr << "Sv_Raster::_ReadIrisRGB() - Unexpected EOF.\n";
	    	fclose(in);
	    	delete [] rleStartTab;
	    	delete [] rleLengthTab;
	    	return (-1);
		}

		cnt = fread(rleLengthTab, sizeof(long), (_Height*channels), in);

		if (cnt < (_Height*channels))
		{
	    	cerr << "Sv_Raster::_ReadIrisRGB() - Unexpected EOF.\n";
	    	fclose(in);
	    	delete [] rleStartTab;
	    	delete [] rleLengthTab;
	    	return (-1);
		}

	
    }
    
    _AllocateNewImage();
 
     // Read in image data, uncompressing if RLE.
   
    if (!isRLE)
    {
		// Read in VERBATIM data.  Data is stored with origin
		// at lower left corner (yay!).  Each channel
		// is stored in order, on a byte-planar basis.
		// (i.e. all the red components are stored for the
		// entire image, followed by all the greens, followed
		// by all the reds, etc.

		for (z=0; z<channels; z++) // B/W (1), RGB (3), RGBA (4)
		{
	    	for (y=0; y<_Height; y++)
	    	{
				cnt = fread(buffer, sizeof(unsigned char), _Width, in);

				if (cnt < _Width)
				{
		    		cerr << "Sv_Raster::_ReadIrisRGB() - Unexpected EOF.\n";
		    		fclose(in);
		    		return (-1);
				}

				for (x=0; x<_Width; x++)
				{
		    		if (channels == 1) // black and white image
		    		{
						_Image[y * _Width * 4 + x * 4] = buffer[x];
						_Image[y * _Width * 4 + x * 4+1] = buffer[x];
						_Image[y * _Width * 4 + x * 4+2] = buffer[x];
						_Image[y * _Width * 4 + x * 4+3] = 255;
		    		}
		    		else
		    		{
						if (z == 0) // set alpha value
			    		 	_Image[y * _Width * 4 + x * 4 + z+3] = 255;

						_Image[y * _Width * 4 + x * 4+z] = buffer[x];
		    		}
				}
	    	}
		}
    }
    else
    {
	// Read in RLE data.  Data is stored with origin
	// at lower left corner (yay!).  Each channel
	// is stored based on offset tables, which 
	// point to run length encoded byte streams.
	// These streams are decoded and stored in the
	// _Image[] array.
	// The start offset and RLE length tables have
	// been previously read in.

unsigned long rleoffset, rlelength;
	
	for (z=0; z<channels; z++)
	{

	    	for (y=0; y<_Height; y++)
	    	{
				rleoffset = rleStartTab[y+z*_Height];
				rlelength = rleLengthTab[y+z*_Height];

				// move to correct file offset where run is stored

				fseek(in, rleoffset, SEEK_SET);

				cnt = fread(buffer, sizeof(unsigned char), rlelength, in);

				if (cnt < (int)rlelength)
				{
		    		cerr << "_ReadIrisRGB() - Unexpected EOF.\n";
		    		fclose(in);
		    		delete [] rleStartTab;
		    		delete [] rleLengthTab;
		    		return (-1);
				}

				// Decode the RLE data.  The first byte in a run
				// encodes the length of the run, and whether or
				// not the run is compressed or not.

				register GLint pos;
				pos = 0;

				x = 0;

				while((cnt = (buffer[pos] & 0x7F))) // extract count, quit when count == 0
				{
		    		if (buffer[pos++] & 0x80)   // If high bit is set, this is an uncompressed run
		    		{
						while (cnt--)
						{
			    			if (channels == 1) // black and white
			    			{
								_Image[y * _Width * 4 + x * 4+z]   = buffer[pos];
								_Image[y * _Width * 4 + x * 4+z+1] = buffer[pos];
								_Image[y * _Width * 4 + x * 4+z+2] = buffer[pos++];
								_Image[y * _Width * 4 + x * 4+z+3] = 255;

			    			}
			    			else
			    			{
								if (z == 0) // set alpha value
				    				_Image[y * _Width * 4 + x * 4 + z+3] = 255;

								_Image[y * _Width * 4 + x * 4+z] = buffer[pos++];
			    			}

			    			x++;
						}
		    		}
		    		else  // compressed run.
		    		{
						while (cnt--)
						{
			    			if (channels == 1) // black and white
			    			{
								_Image[y * _Width * 4 + x * 4+z]   = buffer[pos];
								_Image[y * _Width * 4 + x * 4+z+1] = buffer[pos];
								_Image[y * _Width * 4 + x * 4+z+2] = buffer[pos];
								_Image[y * _Width * 4 + x * 4+z+3] = 255;

			    			}
			    			else
			    			{
								if (z == 0) // set alpha value
				    				_Image[y * _Width * 4 + x * 4 + z+3] = 255;

								_Image[y * _Width * 4 + x * 4+z] = buffer[pos];
			    			}

			    			x++;
						}

						pos++;
		    		}
				}
	    	}
		}
    }
    
    fclose(in);;
    return (0);
				        
}

GLint Sv_Raster::_sizeRawRGB(const char *fileName,GLint &width,GLint &height)
{
	cerr << "Sv_Raster::_sizeRawRGB not yet implemented!\n";
	return -1;
}


// Currently unimplemented, this function will eventually
// read in the RawRGB file format.

GLint Sv_Raster::_ReadRawRGB(const char *fileName)
{
    FILE *in;
    
    in = fopen(fileName, "rb");
    
    if (!in)
    {
		cerr << "Failed opening file: " << fileName << endl;
		return (-1);
    }
    
    cerr << "Sv_Raster::_ReadRawRGB() - Raw RGB File reading is not implemented.\n";
    fclose(in);
    return (-1);
}

// Reads in the raster image from the memory block.
// Assumes that each pixel has either 3 or 4 components, based
// on the value of dataType.  Expects that the origin of the
// data (dataBlock[0]) is the lower left corner of the image,
// as is typical of OpenGL texture images and raster imagery.
// Expects one byte per component.

GLint Sv_Raster::LoadFromMemory( GLvoid *dataBlock, GLint dataWidth, GLint dataHeight, SVR_DataType dataType )
{
    GLint x, y;
    GLubyte *ptr;
    
    if ((!dataBlock) || (dataWidth <= 0) || (dataHeight <=0))
    {
		cerr << "Sv_Raster::LoadFromMemory() - Illegal function parameter values.\n";
		return (-1);
    }

    if ((dataType != SVR_RGBA) && (dataType != SVR_ABGR) && (dataType != SVR_RGB) && (dataType != SVR_BGR))
    {
		cerr << "Sv_Raster::LoadFromMemory() - Unknown data format.\n";
		return (-1);
    }
    
    Destroy();	// Delete previous image, if any.
    
    _Width = dataWidth;
    _Height = dataHeight;
    
    _AllocateNewImage();
    
    ptr = (GLubyte *)dataBlock;
    
    if (dataType == SVR_RGBA)
		memcpy(_Image, dataBlock, _Width*_Height*4);
    else if (dataType == SVR_ABGR)
    {
		for (y=0; y<_Height; y++)
		{
	    	for (x=0; x<_Width; x++)
	    	{
				_Image[(y*_Width*4)+(x*4)] = ptr[3];
				_Image[(y*_Width*4)+(x*4)+1] = ptr[2];
				_Image[(y*_Width*4)+(x*4)+2] = ptr[1];
				_Image[(y*_Width*4)+(x*4)+3] = ptr[0];

				ptr += 4;
	    	}
		}
    }
    else if (dataType == SVR_RGB)
    {
		for (y=0; y<_Height; y++)
		{
	    	for (x=0; x<_Width; x++)
	    	{
				_Image[(y*_Width*4)+(x*4)] = ptr[0];
				_Image[(y*_Width*4)+(x*4)+1] = ptr[1];
				_Image[(y*_Width*4)+(x*4)+2] = ptr[2];
				_Image[(y*_Width*4)+(x*4)+3] =255;

				ptr += 3;
	    	}
		}
    }
    else if (dataType == SVR_BGR)
    {
		for (y=0; y<_Height; y++)
		{
	    	for (x=0; x<_Width; x++)
	    	{
				_Image[(y*_Width*4)+(x*4)] = ptr[2];
				_Image[(y*_Width*4)+(x*4)+1] = ptr[1];
				_Image[(y*_Width*4)+(x*4)+2] = ptr[0];
				_Image[(y*_Width*4)+(x*4)+3] =255;

				ptr += 3;
	    	}
		}
    }
    else
    {
		cerr << "Sv_LoadFromMemory() - Illegal dataType after dataType has been checked.\n";
		cerr << "Something is rotten in the state of this machine!\n";
		return (-1);
    }
    
    return (0);
}


// Deletes any existing image, and then allocates a blank
// image of the given size, with the given clear colour.
// Note that if the clearColour is NULL, the image is cleared
// to black, with an alpha value of 0.0 (if the data type
// is SVR_RGBA).  The clearColour must have 4 values if 
// the data type is SVR_RGBA, 3 if SVR_RGB)

GLint Sv_Raster::BuildBlankImage(GLint w, GLint h, GLfloat *clearColour, SVR_DataType dataType)
{
    GLubyte rgba[4]={0,0,0,0};
    GLint x, y;
    
    Destroy();
    
    //  cerr<< "Sv_Raster::BuildBlankImage() - NOTE: Building blank image with w/h of: " << w << ", " << h << endl;
    
    _Width = w;
    _Height = h;
    
    _AllocateNewImage();
    
    if (!_Image)
    {
		cerr << "Sv_Raster::BuildBlankImage() - ERROR: Couldn't allocate memory for image!\n";
		_Width = _Height = 0;
		return (1);
    }
    
    if ((dataType != SVR_RGB) && (dataType != SVR_RGBA))
    {
		cerr << "Sv_Raster::BuildBlankImage() - Warning: Pre-creating a buffer only supports RGB or RGBA format.\n";
		cerr << "\tassuming SVR_RGB.\n";
		dataType = SVR_RGB;
    }
    
    if (_Width && _Height)
    {	
		if (clearColour)
		{
	    	rgba[0] = GLint(clearColour[0] * 255.0f);
	    	rgba[1] = GLint(clearColour[1] * 255.0f);
	    	rgba[2] = GLint(clearColour[2] * 255.0f);

	    	if (dataType == SVR_RGBA)
				rgba[3] = GLint(clearColour[3] * 255.0f);

	    	//	cerr << "Sv_Raster::BuildBlankImage() - NOTE: Clearing with colour: " << (GLint) rgba[0] << ", " << (GLint) rgba[1] << ", " << (GLint) rgba[2] << endl;
		}
		else
		{
	    	//	cerr << "Sv_Raster::BuildBlankImage() - NOTE: No clear colour specified.  Clearing to black.\n";
	    	rgba[0] = rgba[1] = rgba[2] = rgba[3] = (GLubyte)0;
		}

		for (y=0; y<_Height; y++)
		{
	    	for (x=0; x<_Width; x++)
	    	{		
				_Image[(y*_Width*4)+(x*4)]	= rgba[0];
				_Image[(y*_Width*4)+(x*4)+1]	= rgba[1];
				_Image[(y*_Width*4)+(x*4)+2]	= rgba[2];

				if (dataType == SVR_RGBA)
		    		_Image[(y*_Width*4)+(x*4)+3] = rgba[3];
	    	}
		}	
    }
    
    return (0);
}

/*
 *  Sv_Raster::LoadFromViewport()
 * 
 *  Loads in the image from the viewport,  the origin
 *  (lower left corner) being at the values passed for x and y.
 *  The image read will be wxh pixels in size,  starting at that
 *  origin.  Note that the image is read in RGBA format from
 *  the viewport,  and stored in this manner internally.
 * 
 */
 
GLint Sv_Raster::LoadFromViewport( GLint x, GLint y, GLint w, GLint h, GLint xOrigin, GLint yOrigin )
{
    GLint rv;
    
    if ((xOrigin < 0) || (yOrigin < 0))
    {
		Destroy();

		_Width = w;
		_Height = h;

		_AllocateNewImage();

		glReadPixels (x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)_Image);

		return (0);
    }

    GLubyte *fooBuff = new GLubyte[w * h * 4];
    
    glReadPixels (x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)fooBuff);
    
    rv = _WriteToImageBuffer(xOrigin, yOrigin, fooBuff, w, h, SVR_RGBA);
    
    delete [] fooBuff;
    
    return (rv);
}

GLint Sv_Raster::_WriteToImageBuffer(GLint destX, GLint destY, GLubyte *srcBuff, GLint srcW, GLint srcH, SVR_DataType srcDataType)
{

    GLint xx, yy;

    // Copy the foo buffer into the image array at the appropriate point.

    if ((destX < 0) || (destY < 0) || (!srcBuff))
    {
		cerr << "Sv_Raster::_WriteToImageBuffer() - ERROR: Illegal destination origin values or no source buffer.\n";
		cerr << "\tNo data copied.\n";
		return (1);
    }

    if (!ImageLoaded())
    {
		cerr << "Sv_Raster::_WriteToImageBuffer() - ERROR: Tried to write to a non-existant buffer.\n";
		cerr << "\tInitialize an empty buffer with BuildBlankImage(), or load with the LoadFromXXXX() methods.\n";
		return (1);
    }
    
    if ((srcDataType != SVR_RGB) && (srcDataType != SVR_RGBA))
    {
		cerr << "Sv_Raster::_WriteToImageBuffer() - ERROR: source data type must be SVR_RGB or SVR_RGBA.\n";
		return (1);
    }
    
    // Truncate the source buffer if necessary.
    
    if ((destX + srcW) > _Width)
    {
		cerr << "Sv_Raster::_WriteToImageBuffer() - WARNING: Source data width (" << srcW << ") exceeds available dest data width (" << _Width - destX << ").  Truncating.\n";
		srcW = (_Width - destX);
    }
    
    if ((destY + srcH) > _Height)
    {
		cerr << "Sv_Raster::_WriteToImageBuffer() - WARNING: Source data height (" << srcH << ") exceeds available dest data height (" << _Height - destY << ").  Truncating.\n";

		srcH = (_Height - destY);
    }
    
    for (yy=0; yy<srcH; yy++)
    {
		for (xx=0; xx<srcW; xx++)
		{
	    	_Image[((yy+destY)*_Width*4)+((xx+destX)*4)]      = srcBuff[(yy*srcW*4)+(xx*4)];
	    	_Image[((yy+destY)*_Width*4)+((xx+destX)*4)+1]    = srcBuff[(yy*srcW*4)+(xx*4)+1];
	    	_Image[((yy+destY)*_Width*4)+((xx+destX)*4)+2]    = srcBuff[(yy*srcW*4)+(xx*4)+2];

	    	if (srcDataType == SVR_RGBA)
				_Image[((yy+destY)*_Width*4)+((xx+destX)*4)+3]    = srcBuff[(yy*srcW*4)+(xx*4)+3];
	    	else
				_Image[((yy+destY)*_Width*4)+((xx+destX)*4)+3]	  = 1;
		}
    }
    
    return (0);
    
}

/*
 *  Sv_Raster::SaveToFile()
 * 
 *  This method is used to save the stored raster image to a file
 *  with the given fileType.  Currently, only JPEG(JFIF) and TIFF
 *  (LZW Compressed) are supported.  JPEGs are saved at 80% quality.
 * 
 */
 
GLint Sv_Raster::SaveToFile( const char *fileName, SVR_FileType fileType )
{
    FILE *out;
    
    if (!((fileType > SVR_Unsupported) && (fileType < SVR_DUMMY_TAG_END)))
    {
		cerr << "Sv_Raster::SaveToFile() - Unsupported file type.\n";
		return (-1);
    }
    
    if (!ImageLoaded())
    {
		cerr << "Sv_Raster::SaveToFile() - No image to save!\n";
		return (-1);
    }
    
    out = fopen(fileName, "wb");
    
    if (!out)
    {
		cerr << "Sv_Raster::SaveToFile() - Unable to write to file " << fileName << endl;
		return (-1);
    }
    
    switch (fileType)
    {
		case SVR_PPM1:
	    	return(_WritePPM1(out));
		case SVR_PPM2:
	    	return(_WritePPM2(out));
		case SVR_PPM3:
	    	return(_WritePPM3(out));
		case SVR_PPM4:
	    	return(_WritePPM4(out));
		case SVR_PPM5:
	    	return(_WritePPM5(out));
		case SVR_PPM6:
	    	return(_WritePPM6(out));

		case SVR_GIF:
	    	return (_WriteGIF(out));

		case SVR_JPEG:
	    	return (_WriteJPEG(out));

		case SVR_TDRSCALAR:
	    	fclose(out);
	    	return (_WriteTDRScalar(fileName));

		case SVR_IRISRGB:
	    	return (_WriteIrisRGB(out));

		case SVR_RAWRGB:
	    	return (_WriteRawRGB(out));

		case SVR_TIFF_BE:
		case SVR_TIFF_LE:
	    	return (_WriteTIFF(fileName));

		case SVR_EPS:
	    	return (_WriteEPS(out));

		default:
	    	cerr << "Sv_Raster::SaveToFile() - Unknown or unsupported file type.\n";
	    	return (-1);
    }
}
    
GLint Sv_Raster::_WritePPM1(FILE *out)
{
    cerr << "Sv_Raster::_WritePPM1() - PPM1 File writing is not implemented.\n";
    fclose(out);
    return (-1);
}

GLint Sv_Raster::_WritePPM2(FILE *out)
{
    cerr << "Sv_Raster::_WritePPM2() - PPM2 File writing is not implemented.\n";
    fclose(out);
    return (-1);
}

GLint Sv_Raster::_WritePPM3(FILE *out)
{
    cerr << "Sv_Raster::_WritePPM3() - PPM3 File writing is not implemented.\n";
    fclose(out);
    return (-1);
}

GLint Sv_Raster::_WritePPM4(FILE *out)
{
    cerr << "Sv_Raster::_WritePPM4() - PPM4 File writing is not implemented.\n";
    fclose(out);
    return (-1);
}

GLint Sv_Raster::_WritePPM5(FILE *out)
{
    cerr << "Sv_Raster::_WritePPM5() - PPM5 File writing is not implemented.\n";
    fclose(out);
    return (-1);
}

GLint Sv_Raster::_WritePPM6(FILE *out)
{
    if (!ImageLoaded())
    {
		cerr << "Sv_Raster::_WritePPM6() - No image to write!\n";
		fclose(out);
		return (-1);
    }
    
    // Write the header
    
    fprintf(out, "P6%c#CREATOR: UNB HCI Lab Sv_Raster class%c%d %d%c%d%c", 
	    10, 10, _Width, _Height, 10, 255, 10);
	  
    fflush(out);
      
    // Now, write the data.
    
    register GLint x, y, cnt;
    unsigned char outbuf[4];
    
    for (y=_Height-1; y>=0; y--)
    {
		for (x=0; x<_Width; x++)
		{
	    	outbuf[0] = _Image[(y*_Width*4)+(x*4)];
	    	outbuf[1] = _Image[(y*_Width*4)+(x*4)+1];
	    	outbuf[2] = _Image[(y*_Width*4)+(x*4)+2];

	    	if ((cnt = fwrite(outbuf, 1, 3, out)) < 3)
	    	{
				cerr << "Sv_Raster::_WritePPM6() - ERROR: File write error!\n";
				cerr << "cnt = " << cnt << "expected 3." << endl;
				fclose (out);
				return (-1);
	    	}
		}
    }
    
    fclose (out);
    return (0);
}


GLint Sv_Raster::_WriteEPS( FILE *out )
{
    
    fprintf( out, "%%!PS-Adobe-2.0 EPSF-1.2\n" );
    fprintf( out, "%%%%Creator: OpenGLpixmap render output\n" );
    fprintf( out, "%%%%BoundingBox: 0 0 %d %d\n", _Width, _Height );
    fprintf( out, "%%%%EndComments\n" );
    fprintf( out, "gsave\n" );
    fprintf( out, "/bwproc {\n" );
    fprintf( out, "    rgbproc\n" );
    fprintf( out, "    dup length 3 idiv string 0 3 0 \n" );
    fprintf( out, "    5 -1 roll {\n");
    fprintf( out, "    add 2 1 roll 1 sub dup 0 eq\n" );
    fprintf( out, "    { pop 3 idiv 3 -1 roll dup 4 -1 roll dup\n" );
    fprintf( out, "       3 1 roll 5 -1 roll } put 1 add 3 0 \n" );
    fprintf( out, "    { 2 1 roll } ifelse\n" );
    fprintf( out, "    }forall\n" );
    fprintf( out, "    pop pop pop\n" );
    fprintf( out, "} def\n" );
    fprintf( out, "systemdict /colorimage known not {\n" );
    fprintf( out, "   /colorimage {\n");
    fprintf( out, "       pop\n" );
    fprintf( out, "       pop\n" );
    fprintf( out, "       /rgbproc exch def\n" );
    fprintf( out, "       { bwproc } image \n" );
    fprintf( out, "    } def\n" );
    fprintf( out, "} if\n" );
    fprintf( out, "/picstr %d string def\n", _Width * 3 );
    fprintf( out, "%d %d scale\n", _Width, _Height );
    fprintf( out, "%d %d %d\n", _Width, _Height, 8 );
    fprintf( out, "[%d 0 0 %d 0 0]\n", _Width, _Height );
    fprintf( out, "{currentfile picstr readhexstring pop}\n" );
    fprintf( out, "false %d\n", 3 );
    fprintf( out, "colorimage\n");
    
    
    char   *curpix;
    int	    pos, i;
    
    
    curpix = (char *) _Image;
    
    pos = 0;
    
    for ( i=0; i<(_Width * _Height * 4); i+=4 )
    {
		fprintf( out, "%02hx", curpix[0] );
		fprintf( out, "%02hx", curpix[1] );
		fprintf( out, "%02hx", curpix[2] );
		curpix += 4;	    
		pos += 4;
		if (++pos >= 64)
		{
	    	fprintf( out, "\n");
	    	pos = 0;
		}
    }
    
    if (pos)
    {
		fprintf( out,  "\n");
    }
    
    fprintf( out, "grestore\n" );
    fclose( out );
    return( 0 );
}


 
GLint Sv_Raster::_WriteIrisRGB(FILE *out)
{
    cerr << "Sv_Raster::_WriteIrisRGB() - Iris RGB File writing is not implemented.\n";
    fclose(out);
    return (-1);
}

GLint Sv_Raster::_WriteRawRGB(FILE *out)
{
    cerr << "Sv_Raster::_WriteRawRGB() - Raw RGB File writing is not implemented.\n";
    fclose(out);
    return (-1);
}


// Writes to the memory block such that the
// origin of the data is the lower left hand corner of the image.

GLint Sv_Raster::SaveToMemory( GLvoid *dataBlock, SVR_DataType dt )
{
    GLint x, y;
    GLubyte *ptr;
    
    if (!ImageLoaded())
    {
		cerr << "Sv_Raster::SaveToMemory() - No image to save!\n";
		return (-1);
    }
    
    if ((dt != SVR_RGBA) && (dt != SVR_ABGR) && (dt != SVR_RGB) && (dt != SVR_BGR))
    {
		cerr << "Sv_Raster::SaveToMemory() - Unknown data format.\n";
		return (-1);
    }
    
    ptr = (GLubyte *)dataBlock;
    
    switch (dt)
    {
		case SVR_RGBA:
	    	memcpy(dataBlock, _Image, _Width*_Height*4);
		break;

		case SVR_RGB:
	    	for (y=0; y<_Height; y++)
	    	{
				for (x=0; x<_Width; x++)
				{
		    		ptr[0] = _Image[(y*_Width*4)+(x*4)];    
		    		ptr[1] = _Image[(y*_Width*4)+(x*4)+1];    
		    		ptr[2] = _Image[(y*_Width*4)+(x*4)+2];    
				}    
	    	}
		break;

		case SVR_ABGR:
	    	for (y=0; y<_Height; y++)
	    	{
				for (x=0; x<_Width; x++)
				{
		    		ptr[3] = _Image[(y*_Width*4)+(x*4)];    
		    		ptr[2] = _Image[(y*_Width*4)+(x*4)+1];    
		    		ptr[1] = _Image[(y*_Width*4)+(x*4)+2];    
		    		ptr[0] = _Image[(y*_Width*4)+(x*4)+2];    
				}    
	    	}
		break;

		case SVR_BGR:
	    	for (y=0; y<_Height; y++)
	    	{
				for (x=0; x<_Width; x++)
				{
		    		ptr[2] = _Image[(y*_Width*4)+(x*4)];    
		    		ptr[1] = _Image[(y*_Width*4)+(x*4)+1];  
		    		ptr[0] = _Image[(y*_Width*4)+(x*4)+2];   
				}    
	    	}
		break;		

		default:
	    	cerr << "Sv_Raster::SaveToMemory() - Invalid data type.\n"
			 << "This should be impossible at this point!!\n";
	    	return (-1);
    }
    
    return (0);  
}


/*
 *  Sv_Raster::SaveToViewport()
 * 
 *  Copies the stored raster image to the viewport at the
 *  given origin.  This origin is the lower left corner of
 *  the image.  Note that the co-ordinates given are viewport
 *  co-ordinates which get transformed as would co-ordinates
 *  passed to glRasterPos2i().
 * 
 */
 
GLint Sv_Raster::SaveToViewport(GLint x, GLint y)
{
    if (!ImageLoaded())
    {
		cerr << "Sv_Raster::SaveToViewport() - Nothing to draw!\n";
		return (-1);
    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    
    glRasterPos2i(x, y);
    
    glDrawPixels(_Width, _Height, GL_RGBA, GL_UNSIGNED_BYTE, _Image);

    return (0);
}


/*
 *  Sv_Raster::Scale()
 * 
 *  This method will perform a box scale using the OpenGL
 *  utility library on the stored image.  The desired width
 *  and height of the image is passed as parameters to the 
 *  method.  newWidth and newHeight must be positive.
 */
 
void Sv_Raster::Scale( GLint newWidth, GLint newHeight )
{
    GLubyte *buffer;
    
    if ((newWidth <= 0) || (newHeight <= 0))
    {
		cerr << "Sv_Raster::Scale() - Illegal parameter values.\n";
		return;
    }
    
    buffer = new GLubyte [newWidth*newHeight*4];
    
    gluScaleImage ( GL_RGBA, _Width, _Height, GL_UNSIGNED_BYTE, 
		   _Image, newWidth, newHeight, GL_UNSIGNED_BYTE, 
		   buffer );
		   
    LoadFromMemory(buffer, newWidth, newHeight);
    
    delete [] buffer;
    
}

/*
 *  Sv_Raster::ReverseChannels()
 * 
 *  This method will reverse the ordering of the image channels in
 *  raster image. For example this will turn an RGBA image into a ABGR image.
 */
 
void Sv_Raster::ReverseChannels( void )
{
    GLubyte tmp;
    
    if( _Image == NULL )    // Make sure we have image data
		return;
	
    for( GLint i=0; i<_Height*_Width*4; i+= 4 )
    {
		tmp         = _Image[i+3];    // Swap 1st & last
		_Image[i+3] = _Image[i];
		_Image[i]   = tmp; 
		tmp         = _Image[i+2];    // Swap middle two bytes
		_Image[i+2] = _Image[i+1];
		_Image[i+1] = tmp;
    }
}

void Sv_Raster::AntiAlias( Sv_Raster *outRaster )
{
    GLint i, j, k;
    GLint w, h;	    // Width & Height of output image
    GLfloat v1, v2, v3, v4, vavg;
    GLubyte *image;
    
    image = outRaster->GetImageDataPTR();
    w = outRaster->GetWidth();
    h = outRaster->GetHeight();
    
    for( i=0; i<h; i++ )       // foreach row
    {
		for( j=0; j<w; j++ )      // foreach column
		{
	    	for( k=0; k<4; k++ )     // foreach component colour
	    	{
				v1 = (float)this->GetPixelValue( 2*i, 2*j, k );
				v2 = (float)this->GetPixelValue( 2*i, 2*j+1, k );
				v3 = (float)this->GetPixelValue( 2*i+1, 2*j, k );
				v4 = (float)this->GetPixelValue( 2*i+1, 2*j+1, k );
				vavg = (float)((v1 + v2 + v3 + v4) / 4.0);
				image[(i*w+j)*4+k] = (GLubyte)vavg;
	    	}
		}
    }    
}


void Sv_Raster::SetTransparent( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
	GLint x,y;
	
	if (!_Image)
	{
		cerr << "Sv_Raster::SetTransparent() - ERROR: No image to operate on!\n";
		return; 
	}
	
	for (y=0; y<_Height; y++)
	{
		for (x=0; x<_Width; x++)
		{
		    if ((_Image[(y*_Width*4)+(x*4)] == r) &&
		    	(_Image[(y*_Width*4)+(x*4)+1] == g) &&
		    	(_Image[(y*_Width*4)+(x*4)+2] == b))
		    {
		    	_Image[(y*_Width*4)+(x*4)+3] = a;
		    }    
		}    
	}
}
	

	
