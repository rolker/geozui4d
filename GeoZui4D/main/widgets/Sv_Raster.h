#ifndef SV_RASTER_H
#define SV_RASTER_H

#include <gz4d/glo.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#ifdef SV_RASTER_ALLSUP		// Compile in support for all types.
#define SV_RASTER_TIFFSUP
#define SV_RASTER_JPEGSUP
#define SV_RASTER_GIFSUP

#ifndef NVISION_SOFTWARE
#define SV_RASTER_TDRSUP  // TDR file support  (UNB and IVS only)
#endif

#endif



// SVR_Unsupported must be the first, SVR_DUMMY_TAG_END must be last.
// (Used to verify a valid, in-range file type)

// Note: KEEP SVR_PPM* IN A CONTIGUOUS BLOCK!

enum SVR_FileType { SVR_Unsupported = 0, 
		     SVR_PPM1,
		     SVR_PPM2, 
		     SVR_PPM3, 
		     SVR_PPM4, 
		     SVR_PPM5, 
		     SVR_PPM6, 
		     SVR_JPEG, 
		     SVR_GIF, 
		     SVR_IRISRGB, 
		     SVR_RAWRGB, 
		     SVR_TIFF_BE, 
		     SVR_TIFF_LE, 
		     SVR_RGBHCI, 
		     SVR_TDRSCALAR, 
		     SVR_EPS, 
		     SVR_DUMMY_TAG_END
		   };


// Convenience for constructor.
const SVR_FileType SVR_AUTODETECT = SVR_Unsupported;

enum SVR_DataType  { SVR_RGB, SVR_RGBA, SVR_BGR, SVR_ABGR };

// GIF decoding (and any other method which uses colour tables)
// colour look up table.

struct CLUT	// Is it just me, or does this sound kinky?
{
    GLubyte red;
    GLubyte green;
    GLubyte blue;
};

/* Describes a class which is used to load and save raster
 * image data in a variety of formats.  Images can be
 * loaded from disk files (various formats), memory, or
 * the frame buffer.  Files can likewise be saved in these
 * formats.  
 */

class Sv_Raster
{
    public:
    
	// Reads in a bitmap from a file.
	// Autodetects file type if type is
	// passed as SVR_Unsupported.
	
	Sv_Raster();
	Sv_Raster(const char *fileName, SVR_FileType fileType = SVR_AUTODETECT);
	Sv_Raster( void *dataBlock, GLint dataWidth, GLint dataHeight, SVR_DataType dataType = SVR_RGBA );
	Sv_Raster( Sv_Raster &otherRaster ); // Copy/Reference constructor.
	Sv_Raster( Sv_Raster *otherRaster );
	
	// Allocates a "blank" image array which can be written to
	// with various other methods below.  If 'clearColour' is
	// NULL then the clear colour defaults to black with an 
	// alpha value of 0.0f.  Note that if the dataType is SVR_RGBA,
	// the clearColour is expected to be an array of 4 GLfloats in R,G,B,A order.
	
	Sv_Raster( GLint dataWidth, GLint dataHeight, GLfloat *clearColour = NULL, SVR_DataType dataType = SVR_RGBA );
	
	virtual ~Sv_Raster();
	
    GLubyte *GetImageDataPTR() { return _Image; }


        // Returns a pointer to the RGBA pixel values for a given location
    GLubyte *GetPixel( GLint row, GLint col )
    {
    	return( &_Image[(row*_Width+col)* 4] );
	}


    GLubyte GetColorValue( GLint row, GLint col, GLint channel )
    {
       return( _Image[(row*_Width+col)*4 + channel] );
	}
	
	
	void SetColorValue( GLint row,  GLint col, GLint channel, GLubyte val )
	{
	    _Image[(row*_Width+col)*4+channel] = val;
	}
	
	
    // Returns the specified pixel value for a given loc & channel
    GLubyte GetPixelValue( GLint row, GLint col, GLint channel )
    {
       return( _Image[(row*_Width+col)* 4 + channel] );
	}


	void AntiAlias( Sv_Raster *outRaster );


    // Try to get the width and height in pixels, without loading the whole file
    // Added by rja, Aug 9th, 2002    	
    bool GetSize( const char *fileName,int &width, int &height,
					SVR_FileType fileType = SVR_AUTODETECT);
    
	// Raster image loading and saving.
	// All Load* functions are used to bring an image from
	// another source (file, memory, frame buffer)
	// into the raster class.
	// All Save* functions are used to take the image stored
	// in the raster class and place it in another location.
	// This does not destroy the image saved in the raster class.
	//
	// All functions return a GLint, and return a value of
	// zero (0) when successful, non-zero if there was an error.
	
	// Load an image from file or memory into the class.
	// Autodetects file type if a fileType is not given.
	// For loading from a data block, assumes data is in 
	// unsigned byte, RGBA format, unless otherwise specified.
	// (valid options are RGB, RGBA, BGR, ABGR) 
	//
	// NOTE: Internally, the raster image data is always
	// stored as RGBA (OpenGL native format) per pixel.
	
	GLint LoadFromFile( const char *fileName,  SVR_FileType fileType= SVR_AUTODETECT );
	
	
	// Expects that the origin of the data (dataBlock[0]) is the
	// lower left corner of the image.
	
	GLint LoadFromMemory( GLvoid *dataBlock, GLint dataWidth, GLint dataHeight, SVR_DataType dataType = SVR_RGBA );
	
	// Loads from the current viewport image, from the
	// specified rectangle.  The parameterless version
	// can be used to read the entire screen.
	
	GLint LoadFromViewport( GLint minX, GLint minY, GLint width, GLint height, GLint destXOrigin = -1, GLint destYOrigin = -1 );
	
	GLint LoadFromViewport( GLint destXOrigin=-1, GLint destYOrigin=-1 )
	{
	    GLint vp[4];
	    
	    glGetIntegerv(GL_VIEWPORT, vp);
	    return (LoadFromViewport( vp[0], vp[1], vp[2], vp[3], destXOrigin, destYOrigin ));
	    
	}
	
	// Save* functions write the stored raster data
	// to the specified location (file, memory, frame buffer).
	// Note that for writing to memory, the SVR_DataType
	// parameters can be used to specify the type of data
	// that should be written.  This is automatically
	// determined by the file type.  (Normally RGB).
	// Returns zero (0) if sucessful, non-zero otherwise.
	
	GLint SaveToFile( const char *fileName,  SVR_FileType fileType );
	
	// Saves to memory in such a format as would be acceptable
	// to glDrawPixels(), and texture mapping imagery - i.e.:
	// the origin of the data (dataBlock[0]) is the lower left
	// corner of the image.
	
	GLint SaveToMemory( GLvoid *dataBlock, SVR_DataType dataType = SVR_RGBA );
	
	// Writes to the frame buffer at the specified raster position
	// as the lower left corner of the image.  Defaults to RGBA
	// format.
	
	GLint SaveToViewport( GLint minX, GLint minY );
	
	// Deletes any existing image, and then allocates a blank
	// image of the given size, with the given clear colour.
	// Note that if the clearColour is NULL, the image is cleared
	// to black, with an alpha value of 0.0 (if the data type
	// is SVR_RGBA).  The clearColour must have 4 values if 
	// the data type is SVR_RGBA, 3 if SVR_RGB)
	
	// Image data can be written to this blank image area with
	// the various "LoadFromXXXX" functions.  (Currently, only
	// LoadFromViewport() is implemented to do this, however.)
	
	GLint BuildBlankImage(GLint w, GLint h, GLfloat *clearColour = NULL, SVR_DataType dataType = SVR_RGBA);

	// Information methods.  Used to query information
	// regarding the image.
	
	GLint GetWidth(void) { return _Width; }
	GLint GetHeight(void) { return _Height; }
	void GetWidthHeight( int &w, int &h ) { w = _Width; h = _Height; }
	
	
	// Returns true if an image has been loaded.
	// This is indicated by there being both width,
	// height, and an image buffer allocated.
	
	GLboolean ImageLoaded(void) { return ((_Width && _Height && _Image) ? GL_TRUE : GL_FALSE); }
	
	// Manipulation methods.  These manipulate the
	// data already stored in the class.  Note that
	// these irretrieveably change the data!
	
	void Scale( GLint newWidth, GLint newHeight );
	void ReverseChannels( void );
	
	// Frees the memory allocated to the raster image,
	// without deleting the class.
	// Simply returns if no raster is allocated.
	
	void Destroy(void)
	{
	    if (_Image)
			delete [] _Image;
		
	    _Image = (GLubyte *) NULL;
	    _Width = _Height = 0;    
	}
	
	
	// Returns the raster type of the given file.
	// Returns SVR_Unsupported if there was an error,
	// or if the file type is unknown/unsupported raster type.
	
	static SVR_FileType FindRasterType( char *fileName );
	
	
	// Can be used to query whether or not a given file
	// format has had support compiled into the library
	// for it.
	// returns GL_TRUE if there is support, GL_FALSE otherwise.
	
	static GLboolean IsSupported( SVR_FileType ft );
	
	
	// Makes the given colour transparent, using the 'a' value.
	
	void SetTransparent( GLubyte r, GLubyte g, GLubyte b, GLubyte a );
	
	//////////////////////////////////////////////////////////////
    protected:
	
	GLint _Width;	// Dimensions of the
	GLint _Height;	// stored raster image.
	
	GLubyte *_Image; // Image data, stored as a flat array of bytes.
	
	
	// Internal methods.  Yes, with leading underscores. HI MARK!
	
	static SVR_FileType _FindFileType( FILE *in );

	// Writes an SVR_RGB or SVR_RGBA image, stored in the array pointed to by srcBuff
	// into the current image with the lower left origin being at (destX,destY).
	// Note that a previous image must have been loaded into the class, or 
	// allocated with a call to BuildBlankImage().
	
	GLint _WriteToImageBuffer(GLint destX, GLint destY, GLubyte *srcBuff, GLint srcW, GLint srcH, SVR_DataType srcDataType=SVR_RGBA);
	
	void _AllocateNewImage(void)
	{
	    // WARNING!  Assumes there is no memory
	    // attached to the _Image pointer already!
	    // Also assumes that the _Width and _Height
	    // have been set to the appropriate values!
	    		
	    _Image = new GLubyte [ _Width * _Height * 4 ];
	    
	    if (!_Image)
	    {
                std::cerr << "Sv_Raster - OUT OF MEMORY!\n";
			exit(20);
	    }
	}
	
	// Functions to figure out the image sizes before it is loaded
	
	GLint _sizePPM1(const char *fileName,GLint &width,GLint &height);
	GLint _sizePPM2(const char *fileName,GLint &width,GLint &height);
	GLint _sizePPM3(const char *fileName,GLint &width,GLint &height);
	GLint _sizePPM4(const char *fileName,GLint &width,GLint &height);
	GLint _sizePPM5(const char *fileName,GLint &width,GLint &height);
	GLint _sizePPM6(const char *fileName,GLint &width,GLint &height);
	
	GLint _sizeRGBHCI(const char *fileName,GLint &width,GLint &height);

	GLint _sizeIrisRGB(const char *fileName,GLint &width,GLint &height);
	GLint _sizeRawRGB(const char *fileName,GLint &width,GLint &height);

	GLint _sizeTIFF(const char *fileName,GLint &width,GLint &height);
	GLint _sizeJPEG(const char *fileName,GLint &width,GLint &height);
	GLint _sizeGIF(const char *fileName,GLint &width,GLint &height);
	
	GLint _sizeTDR(const char *fileName,GLint &width,GLint &height);
	
	// Functions which process various raster image file types.
	// Add new ones here as they are supported.
	
	GLint	_ReadPPM1    ( const char *fileName );
	GLint	_ReadPPM2    ( const char *fileName );
	GLint	_ReadPPM3    ( const char *fileName );
	GLint	_ReadPPM4    ( const char *fileName );
	GLint	_ReadPPM5    ( const char *fileName );
	GLint	_ReadPPM6    ( const char *fileName );

	GLint   _ReadRGBHCI(const char *fileName);

	GLint	_ReadIrisRGB ( const char *fileName );
	GLint	_ReadRawRGB  ( const char *fileName );


	GLint	_ReadTIFF	( const char *fileName );
	GLint	_WriteTIFF	( const char *fileName );


	GLint	_ReadJPEG	( const char *fileName );
	GLint	_WriteJPEG	( FILE *out );
	

	GLint	_ReadGIF	( const char *fileName );
	GLint	_WriteGIF	( FILE *out );
	
	    // Processes the compressed LZW data in a GIF file.
	    
	GLint	_ReadGIFLZW  ( FILE *in, GLboolean interlaced,
                               GLint ti, CLUT *ColourTable,
                               GLint TableSize );
	
	GLint	_ReadTDRScalar ( const char *fileName );
	GLint	_WriteTDRScalar ( const char *fileName );

	
	GLint	_WritePPM1	( FILE *out );
	GLint	_WritePPM2	( FILE *out );
	GLint	_WritePPM3	( FILE *out );
	GLint	_WritePPM4	( FILE *out );
	GLint	_WritePPM5	( FILE *out );
	GLint	_WritePPM6	( FILE *out );
	GLint	_WriteEPS	( FILE *out );
	
	GLint	_WriteIrisRGB	( FILE *out );
	GLint	_WriteRawRGB	( FILE *out );


};

#endif // SV_RASTER_H

