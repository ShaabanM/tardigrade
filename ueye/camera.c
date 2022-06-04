#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <dirent.h>
#include <sched.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <ueye.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>
#include <inttypes.h>

#include "camera.h"
#include "astrometry.h"
#include "commands.h"
#include "telemetry.h"

/* NOTE: needs to be cleaned up. */

// define some functions for sorting blobs later
void merge (double A[], int p, int q, int r, double X[],double Y[]);
void part (double A[], int p, int r, double X[], double Y[]);

HIDS cameraHandle = 12;  // set by ueyesetid
IS_POINT_2D locationRectangle;

int bufferNumber = 0;
char* memory = NULL;
char* waitingMem = NULL;

IMAGE_FILE_PARAMS ImageFileParams;

int status; //variable to verify completion of methods

void init_camera() {
  // load the camera parameters
  load_camera();
  // set exposure time min .03 ms, currently can't go above 150 but max should be 998 ms
	double exposure = 300; // NOTE: test alternative values
  status = is_Exposure(cameraHandle, IS_EXPOSURE_CMD_SET_EXPOSURE, (void * ) &exposure, sizeof(double));
	if (status != IS_SUCCESS) {
		printf("Set exposure time fails.\n");
	}
	// initializes astrometry
  init_astrometry();
  // sets save image params
	saveImage();
}

//sets starting values for certain camera atributes
void set_camera_params(unsigned int cameraHandle) {
	/*int delay =*/ is_SetTriggerDelay(IS_GET_TRIGGER_DELAY, 0);

  status = is_SetHardwareGain(cameraHandle, 0, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);
  if (status != IS_SUCCESS) {
    printf("Setting gain failed.\n");
  }

  double autoGain = 0;
  status = is_SetAutoParameter(cameraHandle, IS_SET_ENABLE_AUTO_GAIN, &autoGain, NULL);
  if (status != IS_SUCCESS) {
    printf("Disabling auto gain fails.\n");
  }

  status = is_SetHardwareGamma(cameraHandle, IS_SET_HW_GAMMA_OFF);
  if (status != IS_SUCCESS) {
    printf("Disabling hardware gamma corection fails.\n");
  }

  double autoShutter = 0;
  status = is_SetAutoParameter(cameraHandle, IS_SET_ENABLE_AUTO_SHUTTER, &autoShutter, NULL);
  if (status != IS_SUCCESS) {
    printf("Disabling auto shutter speed fails.\n");
  }

  double autoFramerate = 0;
  status = is_SetAutoParameter(cameraHandle, IS_SET_ENABLE_AUTO_FRAMERATE, &autoFramerate, NULL);
  if (status != IS_SUCCESS) {
    printf("Disabling auto framerate fails.\n");
  }

  status = is_SetGainBoost(cameraHandle, IS_SET_GAINBOOST_ON);
  if (status != IS_SUCCESS) {
    printf("Disabling gain boost fails.\n");
  }

  int blackLevelMode = IS_AUTO_BLACKLEVEL_OFF;
  status = is_Blacklevel(cameraHandle, IS_BLACKLEVEL_CMD_GET_MODE, (void*) &blackLevelMode, sizeof(blackLevelMode));
  if (status != IS_SUCCESS) {
    printf("Getting auto black level mode fails, status %d.\n", status);
  }
	
  status = is_Blacklevel(cameraHandle, IS_BLACKLEVEL_CMD_SET_MODE, (void*) &blackLevelMode, sizeof(blackLevelMode));
  if (status != IS_SUCCESS) {
    printf("Turning off auto black level mode fails, status %d.\n", status);
  }

  blackLevelMode = 50;
  status = is_Blacklevel(cameraHandle, IS_BLACKLEVEL_CMD_SET_OFFSET, 
  (void *) &blackLevelMode, sizeof(blackLevelMode));
  if (status != IS_SUCCESS) {
    printf("Setting black level to 0 fails, status %d.\n", status);
  }

  status = is_Blacklevel(cameraHandle, IS_BLACKLEVEL_CMD_GET_OFFSET, 
  (void *) &blackLevelMode, sizeof(blackLevelMode));
  if (status != IS_SUCCESS) {
    printf("Getting current black level fails, status %d.\n", status);
  }
  blackLevelMode = 0;

  // This for some reason actually affects the time it takes to set aois only on the focal plane camera. 
  // Don't use if for the trigger timeout. It is also not in milliseconds.
  status = is_SetTimeout(cameraHandle, IS_TRIGGER_TIMEOUT, 500);
  if (status != IS_SUCCESS) {
    printf("Setting trigger timeout fails with status %d.\n", status);
  }
}

int load_camera(){

  SENSORINFO sensorInfo;
  
  int status;	
  // printf("LOADING CAMERA\n");

  // initialize camera
	if ((status = is_InitCamera(&cameraHandle, NULL)) != IS_SUCCESS){
    printf("Camera initialization failed\n");
    exit(2);
	}
  
  // get sensor info
	if ((status = is_GetSensorInfo(cameraHandle, &sensorInfo)) != IS_SUCCESS) {
    printf("failed to Receive camera sensor info\n");
    exit(2);
	}

	set_camera_params(cameraHandle);

  // set display mode
	if ((status = is_SetColorMode (cameraHandle, IS_CM_SENSOR_RAW8)) != IS_SUCCESS) {
    printf("Setting display mode failed\n");
    exit(2);
	}
 	
  // allocate camera memory
	char* memoryStartingPointer;
	int memoryId;
	int colourDepth = 8;
	if ((status = is_AllocImageMem(cameraHandle, sensorInfo.nMaxWidth, 
  sensorInfo.nMaxHeight, colourDepth, &memoryStartingPointer, &memoryId)) != IS_SUCCESS)
	{
		printf("Allocating camera memory failed\n");
    exit(2);
	}

  // set memory for image
	if ((status = is_SetImageMem(cameraHandle, memoryStartingPointer, memoryId)) != IS_SUCCESS){
    printf("Setting memory to active failed\n");
    exit(2);
  }

  // get image memory
	void* activeMemoryLocation;
	if ((status = is_GetImageMem(cameraHandle, &activeMemoryLocation)) != IS_SUCCESS) {
    printf("Getting image memory failed\n");
    exit(2);
  }


  // how clear images can be is affected by pixelclock and fps 
  int pixelclock = 30;
	if ((status = is_PixelClock(cameraHandle, IS_PIXELCLOCK_CMD_SET, (void*)&pixelclock, 
  sizeof(pixelclock))) != IS_SUCCESS) {
    printf("Pixel Clock failed to set\n");
    exit(2);
  }

	double newFPS = 10;
	if ((status = is_SetFrameRate(cameraHandle, IS_GET_FRAMERATE, (void*)&newFPS)) != IS_SUCCESS) {
    printf("Framerate failed to set\n");
    exit(2);
  }

  // set trigger
	if ((status = is_SetExternalTrigger (cameraHandle, IS_SET_TRIGGER_SOFTWARE)) != IS_SUCCESS) {
    printf("trigger failed to set\n");
    exit(2);
	}
  printf("Done initing camera\n");
}

// save image as bmp 
int saveImage(){
    ImageFileParams.pwchFileName = L"save1.bmp";
    ImageFileParams.pnImageID = NULL;
    ImageFileParams.ppcImageMem = NULL;
    ImageFileParams.nQuality = 80;
    ImageFileParams.nFileType = IS_IMG_BMP;
}

// struct with default values
struct blob_params all_blob_params = {
  .spike_limit = 4.0, // how agressive is the dynamic hot pixel finder.  Small is more agressive
  .dynamic_hot_pixels = 0, // 0 == off, 1 == on
  .r_smooth = 2,   // image smooth filter radius [px]
  .high_pass_filter = 0, // 0 == off, 1 == on
  .r_high_pass_filter = 10, // image high pass filter radius [px]
  .centroid_search_border = 1, // distance from image edge from which to start looking for stars [px]
  .filter_return_image = 0, // 1 == true; 0 = false
  .n_sigma = 2.0, // pixels brighter than this time the noise in the filtered map are blobs (this number * sigma + mean)
  .unique_star_spacing = 15,
};

unsigned char *mask = NULL;

// not entirely sure what a mask is (look into this)
void makeMask(char *ib, int i0, int j0, int i1, int j1, int x0, int y0, bool subframe) {
  static int first_time = 1;

  if (first_time) {
    // printf("called mask for the first time\n");
    mask = calloc(CAMERA_WIDTH*CAMERA_HEIGHT, 1);
    first_time = 0;
  }

  int i, j;
  int p0, p1, p2, p3, p4;
  int a, b;
  
  int cutoff = all_blob_params.spike_limit*100.0;

  for (i=i0; i<i1; i++) {
    mask[i+ CAMERA_WIDTH*j0] = mask[i + (j1-1)*CAMERA_WIDTH] = 0;
  }
  for (j=j0; j<j1; j++) {
    mask[i0+j*CAMERA_WIDTH] = mask[i1-1 + j*CAMERA_WIDTH] = 0;
  }

  i0++;
  j0++;
  i1--;
  j1--;
  
  
  if (all_blob_params.dynamic_hot_pixels) {
    int nhp = 0;
    for (j = j0; j<j1; j++) {
      for (i=i0; i<i1; i++) {
        p0 = 100*ib[i+j*CAMERA_WIDTH]/cutoff;
        p1 = ib[i-1+(j)*CAMERA_WIDTH];
        p2 = ib[i+1+(j)*CAMERA_WIDTH];
        p3 = ib[i+(j+1)*CAMERA_WIDTH];
        p4 = ib[i+(j-1)*CAMERA_WIDTH];
        a = p1+p2+p3+p4+4;
        p1 = ib[i-1+(j-1)*CAMERA_WIDTH];
        p2 = ib[i+1+(j+1)*CAMERA_WIDTH];
        p3 = ib[i-1+(j+1)*CAMERA_WIDTH];
        p4 = ib[i+1+(j-1)*CAMERA_WIDTH];
        b = p1+p2+p3+p4+4;
        mask[i+j*CAMERA_WIDTH] = ((p0 < a) && (p0 < b));
        if (p0>a || p0 > b) nhp++;
      }
    }
    //printf("Dynamic hot pixels (%d)\n", nhp);
  } else {
    // printf("Dynamic hot pixels are off\n");
    for (j = j0; j<j1; j++) {
      for (i=i0; i<i1; i++) {
        mask[i+j*CAMERA_WIDTH] = 1;
      }
    }
  }

/*
  // TODO: decide if we want a static hot pixel map
  if (use_hpList) {
    int ix, iy;
    int n = hpList.size();
    if (subframe) {
      for (i=0; i<n; i++) {
        ix = hpList[i].x - x0;
        iy = hpList[i].y-y0;
        if ((ix>=0) && (iy >=0) && (ix < i1) && (ix < j1)) {
          mask[ix + CAMERA_WIDTH * (iy)] = 0;
        }
      }
    } else {
      for (i=0; i<n; i++) {
        mask[hpList[i].x - x0 + CAMERA_WIDTH * (hpList[i].y-y0)] = 0;
      }
    }
  }
*/
}

//read comment above mask
void boxcarFilterImage(char *ib, int i0, int j0, int i1, int j1, int r_f, double *filtered_image) {

  static int first_time = 1;
  static char *nc = NULL;
  static uint64_t * ibc1 = NULL;

  if (first_time) {
    nc = calloc(CAMERA_WIDTH*CAMERA_HEIGHT, 1);
    ibc1 = calloc(CAMERA_WIDTH*CAMERA_HEIGHT, sizeof(uint64_t));
    first_time = 0;
  }

  //printf("in boxcar filter, w = %d, h = %d, r_f = %d\n", w, h, r_f);
  int b = r_f;
  int64_t isx;
  int s, n;
  double ds, dn;
  double last_ds=0;

  for (int j = j0; j<j1; j++) {
    n = 0;
    isx = 0;
    for (int i = i0; i<i0+2*r_f+1; i++) {
      n += mask[i+j*CAMERA_WIDTH];
      isx += ib[i+j*CAMERA_WIDTH]*mask[i+j*CAMERA_WIDTH];
    }
    int idx = CAMERA_WIDTH*j+i0+r_f;
    //int iN = i1-r_f-1;
    for (int i=r_f+i0; i<i1-r_f-1; i++) {
      ibc1[idx] = isx;
      nc[idx] = n;
      isx = isx + mask[idx+r_f+1]*ib[idx+r_f+1] - mask[idx-r_f]*ib[idx-r_f];
      n = n + mask[idx+r_f+1] - mask[idx-r_f];
      idx++;
    }
    ibc1[idx] = isx;
    nc[idx] = n;
  }

  for (int j = j0+b; j<j1-b; j++) {
    for (int i=i0+b; i<i1-b; i++) {
      n = s = 0;
      for (int jp=-r_f; jp<=r_f; jp++) {
        int idx = i + (j+jp)*CAMERA_WIDTH;
        s += ibc1[idx];
        n += nc[idx];
      }
      ds = s;
      dn = n;
      if (dn>0.0) {
        ds/=dn;
        last_ds = ds;
      } else {
        ds = last_ds;
      }

      filtered_image[i+j*CAMERA_WIDTH] = ds;
    }
  }
}

int get_blobs(char *input_buffer, int w, int h, double ** starX, double ** starY, double ** starMag, char* output_buffer) {
  static int first_time = 1;
  static double * ic = NULL;
  static double * ic2 = NULL;
  static int num_blobs_alloc = 0;

	//create timer
	int msec = 0, trigger = 60000; /* ms */
	clock_t before = clock();
 	clock_t difference = clock() - before;
	msec = difference  * 1000/CLOCKS_PER_SEC;
 	int diff = trigger-msec;

  difference = clock() - before;
  msec = difference * 1000/CLOCKS_PER_SEC;
   //printf("%d\n",msec);

  // allocate the proper amount of storage space to start
  if (first_time) {
    ic = calloc(CAMERA_WIDTH*CAMERA_HEIGHT, sizeof(double));
    ic2 = calloc(CAMERA_WIDTH*CAMERA_HEIGHT, sizeof(double));
    first_time = 0;
  }
  
  // we use half-width internally, but the api gives us
  // full width.
  int x_size = w/2;
  int y_size = h/2;

  int j0, j1, i0, i1;
  int b = 0; // extra image border

  j0 = i0 = 0;
  j1 = h;
  i1 = w;
  
  b = all_blob_params.centroid_search_border;

  makeMask(input_buffer, i0, j0, i1, j1, 0, 0, 0);

/* 
  for (int i = 0; i < 200; i++) {
    if (i%16 == 0) printf("\n");
    printf("%d ", (int) mask[CAMERA_WIDTH*b+i]);
  }
  printf("\n");
*/



  double sx=0, sx2=0;
  double sx_raw = 0; // sum of non-highpass filtered field
  int num_pix=0;

  // lowpass filter the image - reduce noise.
  boxcarFilterImage(input_buffer, i0, j0, i1, j1, all_blob_params.r_smooth, ic);

/* 
  for (int i = 0; i < 200; i++) {
    if (i%16 == 0) printf("\n");
    printf("%g ", ic[CAMERA_WIDTH*(b+10)+i]);
  }
  printf("\n");
*/

  
  if (all_blob_params.high_pass_filter) { // only high-pass filter full frames
    //fprintf (stderr, "hp filter active\n");
    b += all_blob_params.r_high_pass_filter;
    boxcarFilterImage(input_buffer, i0, j0, i1, j1, all_blob_params.r_high_pass_filter, ic2);

    for (int j = j0+b; j<j1-b; j++) {
      for (int i = i0+b; i<i1-b; i++) {
        int idx = i+j*w;
        sx_raw += ic[idx]*mask[idx];
        ic[idx] -= ic2[idx];
        sx += ic[idx]*mask[idx];
        sx2 += ic[idx]*ic[idx]*mask[idx];
        num_pix += mask[idx];
        
      }
    }
  } else {
    for (int j = j0+b; j<j1-b; j++) {
      for (int i = i0+b; i<i1-b; i++) {
        int idx = i+j*w;
        sx += ic[idx]*mask[idx];
        sx2 += ic[idx]*ic[idx]*mask[idx];
        num_pix += mask[idx];
      }
    }
    sx_raw = sx;
  }

  double mean = sx/num_pix;
  double mean_raw = sx_raw/num_pix;

  double sigma = sqrt((sx2 - sx*sx/num_pix)/num_pix);
  // fprintf(stdout,"sigma: %g mean: %g num_pix: %d sx: %f sx2: %f\n", sigma, mean, num_pix, sx, sx2);

  /*** fill output buffer if the variable is defined ***/
  if (output_buffer) {
    int pixel_offset = 0;
    if (all_blob_params.high_pass_filter) pixel_offset = 50;

    if (all_blob_params.filter_return_image) {
      for (int j = j0+1; j<j1-1; j++) {
        for (int i=i0+1; i<i1-1; i++) {
          output_buffer[i+j*w] = ic[i+j*w]+pixel_offset;
        }
      }
      for (int j=0; j<b; j++) {
        for (int i=i0; i<i1; i++) {
          output_buffer[i+(j+j0)*w] = output_buffer[i + (j1-j-1)*w] = mean+pixel_offset;
        }
      }
      for (int j=j0; j<j1; j++) {
        for (int i=0; i<b; i++) {
          output_buffer[i+i0+j*w] = output_buffer[i1-i-1 + j*w] = mean+pixel_offset;
        }
      }
    } else {
      for (int j=j0; j<j1; j++) {
        for (int i=i0; i<i1; i++) {
          int idx = i+j*w;
          output_buffer[idx] = input_buffer[idx];
        }
      }
    }
  }

  /*** Find the blobs ***/
  double ic0;
  int blob_count = 0;
  for (int j = j0+b; j<j1-b-1; j++) {
    for (int i=i0+b; i<i1-b-1; i++) {

     
		    difference = clock() - before;
        msec = difference  * 1000/ CLOCKS_PER_SEC;		      
        diff = trigger-msec;
  			// printf("%d\n",diff);
			  if(diff < 0){
				  printf("BLOBFINDER timeout!!!!\n");
				  return 0;
			  }
	    


      // if pixel exceeds threshold
      if (((double)ic[i+j*w] > mean + all_blob_params.n_sigma*sigma) || ((double)ic[i+j*w] > 254) || 
      /* this is pretty arbitrary. its purely based on the normal value for dark sky being 6 or 7 -->*/ ((double)ic[i+j*w]>7)) {
        ic0 = ic[i+j*w];
        // If pixel is a local maximum or saturated
        if (((ic0 >= ic[i-1 + (j-1)*w]) &&
             (ic0 >= ic[i   + (j-1)*w]) &&
             (ic0 >= ic[i+1 + (j-1)*w]) &&
             (ic0 >= ic[i-1 + (j  )*w]) &&
             (ic0 >  ic[i+1 + (j  )*w]) &&
             (ic0 >  ic[i-1 + (j+1)*w]) &&
             (ic0 >  ic[i   + (j+1)*w]) &&
             (ic0 >  ic[i+1 + (j+1)*w])) ||
            (ic0>254)){

          int unique = 1;

          // realloc array if necessary (when the camera looks at a really bright image, this slows everything down severely)
         if (blob_count >= num_blobs_alloc) {
              num_blobs_alloc += 500;
              *starX = realloc(*starX, sizeof(double)*num_blobs_alloc);
              *starY = realloc(*starY, sizeof(double)*num_blobs_alloc);
              *starMag = realloc(*starMag, sizeof(double)*num_blobs_alloc);
              // printf("REalloc'ed blobs to %d\n", num_blobs_alloc);
          }

          (*starX)[blob_count] = i;
          (*starY)[blob_count] = j;
          (*starMag)[blob_count] = 100*ic[i+j*CAMERA_WIDTH];

          // FIXME: not sure why this is necessary..
          if((*starMag)[blob_count] < 0){
            (*starMag)[blob_count] = UINT32_MAX;
          }

          // If we already found a blob within SPACING and this
          // one is bigger, replace it.
          int spacing = all_blob_params.unique_star_spacing;
          if ((*starMag)[blob_count] > 25400) {
            spacing = spacing * 4;
          }
          for (int ib = 0; ib < blob_count; ib++) {
            if ((abs((*starX)[blob_count]-(*starX)[ib]) < spacing) &&
                (abs((*starY)[blob_count]-(*starY)[ib]) < spacing)) {
              unique = 0;
              // keep the brighter one
              if ((*starMag)[blob_count] > (*starMag)[ib]) {
                (*starX)[ib] = (*starX)[blob_count];
                (*starY)[ib] = (*starY)[blob_count];
                (*starMag)[ib] = (*starMag)[blob_count];
              }
            }
          }
          // if we didn't find a close one, it is unique.
          if (unique) {
            blob_count++;
          }
          
        }
      }
    }
  }
  //this loop flips the vertical position of the blobs back to their normal location
  for(int ibb = 0; ibb < blob_count; ibb++){
    (*starY)[ibb] = CAMERA_HEIGHT-(*starY)[ibb];
  }

  // printf("Finished finding blobs... now sorting\n");

  //merge sort boi
  part(*starMag,0,blob_count-1,*starX,*starY); //implementation below

  // printf("Sorted!\n");

  return blob_count;

  // TODO: determine if sub-pixel centroiding is necessary
  /*** find centroids: ***
   * re-map a sub-image around the blob
   * onto a 'res' times
   * higher resolution map 1D map, then low-pass
   * filter it, and then find the peak.
   * Do this first for X, then for Y.
   * Clearly, this has a precision of
   * pixelScale/res. */
   /*
  int nb = blobs.size();
  int res = 10; // subpixels per pixel
  int filt = 2; // filter half-width in pixels
  int w_sp = res*(2*filt*(STAGES-1)+1); // size of the array to filter
  double sp[w_sp];
  for (int ib = 0; ib < nb; ib++) {
    // find X centroid
    // collapse into 1D X array
    int i_min = blobs[ib].icentroid.x - filt*(STAGES-1);
    int j_min = blobs[ib].icentroid.y - filt*(STAGES-1);

    for (int i = 0; i <= 2*filt*(STAGES-1); i++) {
      int i_p = i+i_min;
      double s = 0.0;
      for (int j = 0; j<= 2*filt; j++) {
        int j_p = j+j_min;
        s += ic[i_p + j_p*w];
      }
      for (int k=0; k<res; k++) {
        sp[i*res + k] = s;
      }
    }
    int max = 0;
    int i_xmax = Box1DCenter(sp, w_sp, filt, res, max);

    // find Y centroid
    // collapse into 1D Y array
    for (int j = 0; j <= 2*filt*(STAGES-1); j++) {
      int j_p = j+j_min;
      double s = 0.0;
      for (int i = 0; i<= 2*filt; i++) {
        int i_p = i+i_min;
        s += ic[i_p + j_p*w];
      }
      for (int k=0; k<res; k++) {
        sp[j*res + k] = s;
      }
    }

    int i_ymax = Box1DCenter(sp, w_sp, filt, res, max);
    blobs[ib].totalIntensity = max;

    blobs[ib].centroid.x = blobs[ib].icentroid.x + double(i_xmax - w_sp/2)/double(res)+0.5+double(x_center-x_size);
    if (subframe) {
      blobs[ib].centroid.y = blobs[ib].icentroid.y + double(i_ymax - w_sp/2)/double(res)+0.5+double(y_center-y_size);
    } else {
      blobs[ib].centroid.y = blobs[ib].icentroid.y + double(i_ymax - w_sp/2)/double(res)+0.5+double(y_center-y_size);
    }
  }

  std::sort(blobs.begin(), blobs.end(), blobCompareFunction);

  if (blobs.size()==0) { // no blobs: lets see if it was saturated
    if (mean_raw>250.0) {
      B.icentroid.x = (i0+i1)/2;
      B.icentroid.y = (j0+j1)/2;
      B.intensity = B.totalIntensity = 100.0*mean_raw;
      blobs.push_back(B);
    }
  }
  //if (subframe && blobs.size()) printf(" **** found blob in subframe ***\n");
  /*
  for(int i = 0; i< blobs.size(); i++){
    printf("found a blob at %lf, %lf, intensity: %d total intensity: %d\n", blobs[i].centroid.x, blobs[i].centroid.y, blobs[i].intensity, blobs[i].totalIntensity);
  }
  */
}

  //sorting implementation
  void merge(double * A, int p, int q, int r, double * X, double * Y){
    int n1 = q-p+1, n2 = r-q;
    int lin = n1+1, rin = n2+1;
    double LM[lin],LX[lin],LY[lin],RM[rin],RX[rin],RY[rin];
    int i,j,k;
    LM[n1]=0;
    RM[n2]=0;

    for(i=0;i<n1;i++){
      LM[i] = A[p+i];
      LX[i] = X[p+i];
      LY[i] = Y[p+i];
    }

    for(j=0;j<n2;j++){
      RM[j] = A[q+j+1];
      RX[j] = X[q+j+1];
      RY[j] = Y[q+j+1];
    }

    i=0;j=0;

    for(k=p;k<=r;k++){
      if(LM[i]>=RM[j]){
        A[k]=LM[i];
        X[k]=LX[i];
        Y[k]=LY[i];
        i++;
      }
      else{
        A[k]=RM[j];
        X[k]=RX[j];
        Y[k]=RY[j];
        j++;
      }
    }
  }
  // recursive part of sorting (dont be scared just assume it works)
  void part(double * A, int p, int r, double * X, double * Y){
    if (p<r){
      int q = (p+r)/2;
      part(A,p,q,X,Y);
      part(A,q+1,r,X,Y);
      merge(A,p,q,r,X,Y);
    }
  }

  // // for testing saved images in astrometry implementation ***DO NOT USE DURING FLIGHT***
  int loadDummyPicture(char * filename, char * buffer) {
  FILE * fp = NULL;

  if (!(fp = fopen(filename, "rb"))) {
    printf("Could not load file %s\n", filename);
    return 0;
  }
  // find the offset to image data (first 8 bytes)
  char header[14] = {0};
  fread(header, 1, 14, fp);
  int offset = (int) *((int*)((long long int)(header + 10)));

  // go to image data
  fseek(fp, offset, SEEK_SET);

  // read image data
  fread(buffer, 1, CAMERA_WIDTH * CAMERA_HEIGHT, fp);

  fclose(fp);

  return 1;
  }


// make a table of stars (mostly used for testing)
int makeTable(char * filename, char * buffer,double *starMag, double *starX, double *starY, int blob_count) {
  FILE * fp = NULL;

  if (!(fp = fopen(filename, "w"))) {
    printf("Could not load file %s.\n", filename);
    return 0;
  }

  for(int i=0; i < blob_count; i++){
    fprintf(fp ,"%f,%f,%f\n", starMag[i],starX[i], starY[i]);
  }
  // printf("table made!\n");
  fclose(fp);

  return 1;
}

unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader) {
    FILE *filePtr; //our file pointer
    BITMAPFILEHEADER bitmapFileHeader; //our bitmap file header
    unsigned char *bitmapImage;  //store image data
    int imageIdx=0;  //image index counter
    unsigned char tempRGB;  //our swap variable

    //open filename in read binary mode
    filePtr = fopen(filename,"rb");
    if (filePtr == NULL)
        return NULL;

    //read the bitmap file header
    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

    //verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader.bfType !=0x4D42) {
        fclose(filePtr);
        return NULL;
    }

    //read the bitmap info header
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr); // small edit. forgot to add the closing bracket at sizeof

    //move file point to the begging of bitmap data
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

    //allocate enough memory for the bitmap image data
    bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

    //verify memory allocation
    if (!bitmapImage) {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    //read in the bitmap image data
    fread(bitmapImage, bitmapInfoHeader->biSizeImage, 1, filePtr);

    //make sure bitmap image data was read
    if (bitmapImage == NULL) {
        fclose(filePtr);
        return NULL;
    }

    //swap the r and b values to get RGB (bitmap is BGR)
    for (imageIdx = 0;imageIdx < bitmapInfoHeader->biSizeImage;imageIdx+=3) {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }

    //close file and return bitmap image data
    fclose(filePtr);
    return bitmapImage;
}

int doCameraAndAstrometry() {
  // printf("in camera.c, all_blob_params.spike_limit is: %d\n", all_blob_params.spike_limit);
  // printf("in camera.c, all_blob_params.dynamic_hot_pixels is: %d\n", all_blob_params.dynamic_hot_pixels);
  // printf("in camera.c, all_blob_params.centroid_search_border is: %d\n", all_blob_params.centroid_search_border);
  // printf("in camera.c, all_blob_params.high_pass_filter is: %d\n", all_blob_params.high_pass_filter);
  // printf("in camera.c, all_blob_params.r_smooth is: %d\n", all_blob_params.r_smooth);
  // printf("in camera.c, all_blob_params.filter_return_image is: %d\n", all_blob_params.filter_return_image);
  // printf("in camera.c, all_blob_params.r_high_pass_filter is: %d\n", all_blob_params.r_high_pass_filter);
  // printf("in camera.c, all_blob_params.n_sigma is: %f\n", all_blob_params.n_sigma);
  // printf("in camera.c, all_blob_params.unique_star_spacing is: %d\n", all_blob_params.unique_star_spacing);
  int stop = 1; // test purposes
  
  // starX, starY, and starMag get allocated and set in get_blobs()
  static double * starX = NULL, * starY = NULL, * starMag = NULL;
  static char * output_buffer = NULL;
  static int first_time = 1;
  
  if (first_time) {
    output_buffer = calloc(1, CAMERA_WIDTH * CAMERA_HEIGHT);
    first_time = 0;
  }
  //double ra = all_astro_params.ra, dec = all_astro_params.dec, fr = all_astro_params.fr, ps = all_astro_params.ps;
  wchar_t filename[200] = L"";

  // set up time
  time_t seconds;
  struct tm * tm_info;
  char date[256];

  time(&seconds);
  tm_info = localtime(&seconds);
  int img_counter = 0;

  // alternative time set-up
  //time_t t;
  //t = time(NULL);

  // captures image (comment out when loading dummy pictures for testing, but be sure to uncomment when performing actual tests on the sky)
  // status = is_FreezeVideo(cameraHandle, IS_WAIT);
  // if (status == -1) {
  //   printf("Failed to capture image.");
  //   exit(2);
  // }

  // names file with time
  // strftime(date, sizeof(date), "/home/xscblast/Desktop/blastcam/BMPs/saved_image_%Y-%m-%d_%H:%M:%S.bmp", tm_info);
  // strftime(date, sizeof(date), "/home/xscblast/Desktop/blastcam/BMPs/saved_image_%Y-%m-%d_%H:%M:%S.bmp", localtime(&t));

  swprintf(filename, 200, L"%s", date);
  ImageFileParams.pwchFileName = filename;

  // get the image from memory
  status = is_GetActSeqBuf(cameraHandle, &bufferNumber, &waitingMem, &memory);

  // testing pictures that have already been taken ***DO NOT USE DURING FLIGHT***
  loadDummyPicture("/home/xscblast/Desktop/blastcam/BMPs/saved_image_2019-07-01-23-34-22.bmp", memory); 
  // loadDummyPicture("/home/xscblast/Desktop/blastcam/BMPs/Success/saved_image_2019-07-01-23-42-16.bmp", memory);

  // Edits by B. DiGia: trying to get image array to send to user in addition to telemetry


  // find the blobs in the image
  int blob_count = get_blobs(memory, CAMERA_WIDTH, CAMERA_HEIGHT, &starX, &starY, &starMag, output_buffer);
  // printf("%d.\n", blob_count);
  // memcpy(memory, output_buffer, CAMERA_WIDTH * CAMERA_HEIGHT); // this line makes it so kst shows the filtered image

  // saves image
  status = is_ImageFile(cameraHandle, IS_IMAGE_FILE_CMD_SAVE, (void*)&ImageFileParams, sizeof(ImageFileParams));
  if (status == -1) {
    char* lastErrorString;
    int lastError = 0;
    is_GetError(cameraHandle, &lastError, &lastErrorString);
    printf("FAILED %s\n", lastErrorString);
    exit(2);
  }

  wprintf(L"Saving to \"%s\"\n", filename);
  unlink("/home/xscblast/Desktop/blastcam/BMPs/latest_saved_image.bmp");
  symlink(date, "/home/xscblast/Desktop/blastcam/BMPs/latest_saved_image.bmp"); // for relatively live updates in kst

  // make a table of the blobs for kst2
  makeTable("makeTable.txt",memory,starMag,starX,starY,blob_count);    

  // solve astrometry
  printf("Trying to solve astrometry...\n");
  status = lost_in_space_astrometry(starX, starY, starMag, blob_count);

  if (status) {
    // change to 0 to make loop stop on solve 
    stop = 1;
  } else {
    printf("Could not solve astrometry :(\n");
  }
}