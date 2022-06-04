#ifndef CAMERA_H
#define CAMERA_H

#include <uEye.h>

// define function prototypes
HIDS init_camera(HIDS cameraHandle);
int do_camera(HIDS cameraHandle, float exposure_time, unsigned int image_id);
void set_camera_exposure(HIDS cameraHandle, double exposure_time);

#endif /* CAMERA_H */
