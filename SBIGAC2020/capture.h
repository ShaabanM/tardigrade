#ifndef CAPTURE_HEADER
#define CAPTURE_HEADER

// external dependencies
#include <dlapi.h>
#include <fitsio.h>

// internal dependencies
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <valarray>
#include <string>
#include <unistd.h>
#include <algorithm>
#include <stdio.h>
#include <cstdio>
#include <cstring>
#include <ctime>

using namespace dl;

// Helper functions
void handlePromise(IPromisePtr pPromise);
std::string getSerial(ICameraPtr pCamera);
ICameraPtr initCamera(IGatewayPtr pGateway);
void saveImage(ICameraPtr pCamera, const char *filename, unsigned short *bufferData, long xdim, long ydim, long int_temp, long ext_temp);
void setFWPosition(ICameraPtr pCamera, int fwpos);
void setTECTemp(ICameraPtr pCamera, float tec_temp);
void setSubFrame(ISensorPtr pSensor, IGatewayPtr pGateway);
void expose(ICameraPtr pCamera, ISensorPtr pSensor, IGatewayPtr pGateway, double exp_time, int read_mode, int fwpos);
void downloadImageTo(const char *path, ICameraPtr pCamera, ISensorPtr pSensor, IGatewayPtr pGateway, long int_temp, long ext_temp);
bool isXferComplete(IPromise *pPromise);
void copyImageBuffer(ISensorPtr pSensor, unsigned short *pBuffer, size_t bufferLength);
std::string getReadoutModes(ISensorPtr pSensor);

#endif // CAPTURE HEADER