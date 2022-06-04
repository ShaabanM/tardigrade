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

HIDS cameraHandle = 1;  // set by ueyesetid

int bufferNumber = 0;
char* memory = NULL;
char* waitingMem = NULL;

IMAGE_FILE_PARAMS ImageFileParams;

int status; //variable to verify completion of methods

int load_camera();

void init_camera() {
  // load the camera parameters
  load_camera();
  // set exposure time min .03 ms, currently can't go above 150 but max should be 998 ms
  double exposure = 300; // NOTE: test alternative values
  status = is_Exposure(cameraHandle, IS_EXPOSURE_CMD_SET_EXPOSURE, (void * ) &exposure, sizeof(double));
  if (status != IS_SUCCESS) {
    printf("Set exposure time fails.\n");
  }
  // sets save image params
  ImageFileParams.pwchFileName = L"save1.bmp";
  ImageFileParams.pnImageID = NULL;
  ImageFileParams.ppcImageMem = NULL;
  ImageFileParams.nQuality = 80;
  ImageFileParams.nFileType = IS_IMG_BMP;
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
  return 1;
}

int do_camera() {
  //double ra = all_astro_params.ra, dec = all_astro_params.dec, fr = all_astro_params.fr, ps = all_astro_params.ps;
  wchar_t filename[200] = L"";

  // set up time
  time_t seconds;
  struct tm * tm_info;
  char datename[256];

  time(&seconds);
  tm_info = localtime(&seconds);

  // captures image (comment out when loading dummy pictures for testing, but be sure to uncomment when performing actual tests on the sky)
  status = is_FreezeVideo(cameraHandle, IS_WAIT);
  if (status == -1) {
    printf("Failed to capture image.");
    exit(2);
  }

  // names file with time
  strftime(datename, sizeof(datename), "data/ueye_%Y-%m-%d_%H-%M-%S.bmp", tm_info);

  swprintf(filename, 200, L"%s", datename);
  ImageFileParams.pwchFileName = filename;

  // get the image from memory
  status = is_GetActSeqBuf(cameraHandle, &bufferNumber, &waitingMem, &memory);

  // saves image
  status = is_ImageFile(cameraHandle, IS_IMAGE_FILE_CMD_SAVE, (void*)&ImageFileParams, sizeof(ImageFileParams));
  if (status == -1) {
    char* lastErrorString;
    int lastError = 0;
    is_GetError(cameraHandle, &lastError, &lastErrorString);
    printf("FAILED %s\n", lastErrorString);
    exit(2);
  }

  printf("Saving to \"%s\"\n", datename);

  return 1;

}
