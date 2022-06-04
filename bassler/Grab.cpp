// Include files to use the pylon API.
#include <pylon/PylonIncludes.h>
#include <string>
#include <fitsio.h>

#ifdef PYLON_WIN_BUILD
#include <pylon/PylonGUI.h>
#endif

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using cout.
using namespace std;

void saveImage(const char *filename, uint8_t *bufferData, int32_t width, int32_t height, double exp_time);

// script parameters
double exp_time = 100000;                          /* in microsecond */
static const uint32_t c_countOfImagesToGrab = 100; /* Number of images to be grabbed */
string directory = "data/";
string image_type = "dark";
string save_path = directory + image_type;

int main(int /*argc*/, char * /*argv*/[])
{
    // The exit code of the sample application.
    int exitCode = 0;

    // Before using any pylon methods, the pylon runtime must be initialized.
    PylonInitialize();

    try
    {
        // Create an instant camera object with the camera device found first.
        CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

        // Print the model name of the camera.
        cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

        // Determine the current exposure time
        double d = camera.ExposureTime.GetValue();
        // Set the exposure time mode to Standard
        // NOTE: May not be available on all camera models
        camera.ExposureTimeMode.SetValue(ExposureTimeMode_Standard);
        // Set the exposure time to 3500 microseconds
        camera.ExposureTime.SetValue(3500.0);

        // The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.
        camera.MaxNumBuffer = 5;

        // Start the grabbing of c_countOfImagesToGrab images.
        // The camera device is parameterized with a default configuration which
        // sets up free-running continuous acquisition.
        camera.StartGrabbing(c_countOfImagesToGrab);

        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;

        // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
        // when c_countOfImagesToGrab images have been retrieved.
        while (camera.IsGrabbing())
        {
            // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
            camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

            // Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {

                // generate image name
                std::string fullPath = save_path;
                fullPath += "_";
                fullPath += std::to_string(exp_time);
                fullPath += "_";
                fullPath += std::to_string(std::time(0));
                fullPath += ".fits";

                // Save image
                saveImage(fullPath, ptrGrabResult->GetBuffer(), ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(), exp_time);

                // Heartbeat
                cout << "Image grabbed " << std::time(0) << endl;

#ifdef PYLON_WIN_BUILD
                // Display the grabbed image.
                Pylon::DisplayImage(1, ptrGrabResult);
#endif
            }
            else
            {
                cout << "Error: " << std::hex << ptrGrabResult->GetErrorCode() << std::dec << " " << ptrGrabResult->GetErrorDescription() << endl;
            }
        }
    }
    catch (const GenericException &e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl
             << e.GetDescription() << endl;
        exitCode = 1;
    }

    // Comment the following two lines to disable waiting on exit.
    cerr << endl
         << "Press enter to exit." << endl;
    while (cin.get() != '\n')
        ;

    // Releases all pylon resources.
    PylonTerminate();

    return exitCode;
}

void saveImage(const char *filename, uint8_t *bufferData, int32_t width, int32_t height, double exp_time)
{
    remove(filename); // Delete old file if it already exists

    // create file
    fitsfile *fptr;
    int status = 0;
    long naxes[2] = {width, height};

    fits_create_file(&fptr, filename, &status);
    fits_create_img(fptr, USHORT_IMG, 2, naxes, &status);

    // write the image to the file
    fits_write_img(fptr, TUSHORT, 1, width * height, bufferData, &status);

    // write some useful header info
    fits_write_key(fptr, TDOUBLE, "EXPTIME", &exp_time, "Exposure time [s]", &status);
    long time = std::time(0);
    fits_write_key(fptr, TLONG, "TIME", &time, "UNIX Time", &status);

    // close file
    fits_close_file(fptr, &status);
}