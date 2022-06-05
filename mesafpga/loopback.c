#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "mesafpga.h"

int main(int argc, char *argv[])
{
    board_t *mesa4i69 = mesafpga_init(argv[1]);

    if (!mesa4i69)
        return -1;

    // set inputs
    for (int i = 0; i < 23; i++)
    {
        mesafpga_set_io(mesa4i69, i, 0);
    }

    // set outputs
    for (int i = 24; i < 47; i++)
    {
        mesafpga_set_io(mesa4i69, i, 1);
    }

    int value = 0;

    while (1)
    {
        value = !value;

        for (int i = 24; i < 47; i++)
        {
            mesafpga_write_io(mesa4i69, i, value);
        }

	printf("Time,%ld,", time(0));

        for (int i = 0; i < 23; i++)
        {
            printf("Port,%d,%d,", i, mesafpga_read_io(mesa4i69, i));
        }
        printf("\n");
        usleep(10000);
    }
}
