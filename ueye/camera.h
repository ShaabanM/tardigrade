#ifndef CAMERA_H
#define CAMERA_H

#define CAMERA_WIDTH 1936 	// [px]
#define CAMERA_HEIGHT 1216	// [px]
#define CAMERA_MARGIN 0		// [px]
#define MIN_PS 6.0        	// [as/px]
#define NOM_PS 7.66			// [as/px]
#define MAX_PS 8.0			// [as/px]

// global structure for blob parameters
#pragma pack(push, 1)
struct blob_params {
  int spike_limit; // how agressive is the dynamic hot pixel finder.  Small is more agressive
  int dynamic_hot_pixels; // 0 == off, 1 == on
  int r_smooth;   // image smooth filter radius [px]
  int high_pass_filter; // 0 == off, 1 == on
  int r_high_pass_filter; // image high pass filter radius [px]
  int centroid_search_border; // distance from image edge from which to start looking for stars [px]
  int filter_return_image; // 1 == true; 0 = false
  double n_sigma; // pixels brighter than this time the noise in the filtered map are blobs (this number * sigma + mean)
  int unique_star_spacing;
};
// global structure for StarCamera image (bmp) data
typedef struct tagBITMAPFILEHEADER {
    unsigned short bfType;  //specifies the file type
    unsigned int bfSize;  //specifies the size in bytes of the bitmap file
    unsigned short bfReserved1;  //reserved; must be 0
    unsigned short bfReserved2;  //reserved; must be 0
    unsigned int bfOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
}BITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER {
    unsigned int biSize;  //specifies the number of bytes required by the struct
    long biWidth;  //specifies width in pixels
    long biHeight;  //species height in pixels
    unsigned short biPlanes; //specifies the number of color planes, must be 1
    unsigned short biBitCount; //specifies the number of bit per pixel
    unsigned int biCompression;//spcifies the type of compression
    unsigned int biSizeImage;  //size of image in bytes
    long biXPelsPerMeter;  //number of pixels per meter in x axis
    long biYPelsPerMeter;  //number of pixels per meter in y axis
    unsigned int biClrUsed;  //number of colors used by th ebitmap
    unsigned int biClrImportant;  //number of colors that are important
}BITMAPINFOHEADER;
#pragma pack(pop)

// make all blob_params acessible from any file that includes camera.h
extern struct blob_params all_blob_params;
// make image data structure accessible as well
//extern BITMAPFILEHEADER bitmapFileHeader;
//extern BITMAPINFOHEADER bitmapInfoHeader;

// define function prototypes
void set_camera_params(unsigned int cameraHandle);
int saveImage();
int whileLoop();
int load_camera();
void init_camera();
unsigned char * LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader);
int doCameraAndAstrometry();

#endif /* CAMERA_H */