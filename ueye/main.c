#include <stdio.h>
#include <stdlib.h>

#include "camera.h"

#define CAMERA_ID 1
#define NUM_GRABS_PER 2

int main(int argc, char * argv[]) {
    HIDS camera_id = init_camera(CAMERA_ID);

    double exp_times[10] = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000};
   
    for (int i=0; i<10; i++) {
        printf("Exposure time = %f ms\n", exp_times[i]);
        set_camera_exposure(camera_id, exp_times[i]);
        for (int j=0; j<NUM_GRABS_PER; j++) {
            do_camera(camera_id, exp_times[i], j);
        }
    }
    close_camera(camera_id);

    printf("Done\n");
}
