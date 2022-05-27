#include "capture.h"
#include <string>
using namespace std;

// script parameters
double exp_time = 0.1; // in seconds
float tec_temp = -20;  // tec tempint fwpos = 1;
string directory = "/home/luvs/SBIG-Dev/test_data_ajay/";
int read_mode = 1;

string image_type = "dark";
string save_path = directory + image_type;

int fwpos = 0;

//const char *name = "test";
double times[6] = {0.1, 1, 10, 30, 60, 100};
float temps[7] = {0, 0, 0, 0, 0, 0, 0};
//const char *name = image_type;

int main(int argc, char *argv[])
{
	//std::cout << "Save path is: \n" << save_path;
	// Establish gateway
	auto pGateway = getGateway();

	// Initialize the camera
	auto pCamera = initCamera(pGateway);
	// If initialization fails exit
	if (pCamera == NULL)
		return 1;

	// Get camera
	auto pSensor = pCamera->getSensor(0);

	// force stop any ongoing exposure etc
	handlePromise(pSensor->abortExposure());
	setSubFrame(pSensor, pGateway);

	// Print out the different read modes
	auto read_modes = getReadoutModes(pSensor);
	std::cout << "Readout modes are:\n"
			  << read_modes << std::endl;

	for (size_t i = 0; i < 7; i++)
	{
		for (size_t j = 0; j < 6; j++)
		{
			exp_time = times[j];

			for (size_t k = 0; k < 3; k++)
			{
				tec_temp = temps[i];

				// Set TEC to desired temprature
				setTECTemp(pCamera, tec_temp);

				std::cout << "Exptime is: \n"
						  << exp_time;

				// wait for the camera to be ready to expose

				do
				{
					try
					{
						handlePromise(pCamera->queryStatus());
					}
					catch (std::exception &ex)
					{
						std::cout << "Failed to query camera status: " << std::endl;
						deleteGateway(pGateway);
						throw(ex);
					}

					auto status = pCamera->getStatus();
					if (status.mainSensorState == ISensor::Idle)
						break;
				} while (true);

				expose(pCamera, pSensor, pGateway, exp_time, read_mode, fwpos);

				//std::string fullPath = name;
				std::string fullPath = save_path;
				fullPath += std::to_string(k);
				fullPath += "_";
				fullPath += std::to_string(exp_time);
				fullPath += "_";
				fullPath += std::to_string(read_mode);
				fullPath += "_";
				fullPath += std::to_string(fwpos);
				fullPath += "_";
				fullPath += std::to_string(tec_temp);
				fullPath += "_";
				fullPath += std::to_string(std::time(0));
				fullPath += ".fits";

				auto PTEC = pCamera->getTEC();

				downloadImageTo(fullPath.c_str(), pCamera, pSensor, pGateway, PTEC->getSensorThermopileTemperature(), PTEC->getHeatSinkThermopileTemperature());
			}
		}
	}

	deleteGateway(pGateway);

	return 0;
}

// Some Helper Functions
//====================================================================

void handlePromise(IPromisePtr pPromise)
{
	auto result = pPromise->wait();
	if (result != IPromise::Complete)
	{
		char buf[512] = {0};
		size_t blng = 512;
		pPromise->getLastError(&(buf[0]), blng);
		pPromise->release();
		throw std::logic_error(std::string(&(buf[0]), blng));
	}
	pPromise->release();
}

std::string getSerial(ICameraPtr pCamera)
{
	char buf[512] = {0};
	size_t blng = 512;
	pCamera->getSerial(&(buf[0]), blng);
	return std::string(&(buf[0]), blng);
}

std::string getReadoutModes(ISensorPtr pSensor)
{
	char buf[1028];
	size_t lng = 1028;
	pSensor->getReadoutModes(&(buf[0]), lng);
	return std::string(&(buf[0]), lng);
}

ICameraPtr initCamera(IGatewayPtr pGateway)
{
	pGateway->queryUSBCameras();
	auto count = pGateway->getUSBCameraCount();
	if (count == 0)
	{
		std::cout << "Failed to retrieve any USB cameras" << std::endl;
		return NULL;
	}

	auto pCamera = pGateway->getUSBCamera(0);
	if (!pCamera)
	{
		std::cout << "Failed to retrieve camera from API" << std::endl;
		return NULL;
	}
	pCamera->initialize();
	auto serial = getSerial(pCamera);
	std::cout << "\nSerial: " << serial << std::endl;

	return pCamera;
}

void setFWPosition(ICameraPtr pCamera, int fwpos)
{
	auto pFW = pCamera->getFW();

	// TODO: This is a temporary hack needs to be removed
	// this is done so that the camera reports correct filter
	if (fwpos != 0)
	{
		handlePromise(pFW->setPosition(fwpos - 1));
	}

	// set filter wheel position
	handlePromise(pFW->setPosition(fwpos));

	// Wait for filter to be set
	do
	{
		handlePromise(pFW->queryStatus());
		auto status = pFW->getStatus();
		if (status == IFW::Status::FWIdle)
			break;
	} while (true);

	// report filter wheel posiotion as human readble filter
	std::string filter;
	switch (pFW->getPosition())
	{
	case 0:
		filter = std::string("Luminace");
		break;

	case 1:
		filter = std::string("Red");
		break;

	case 2:
		filter = std::string("Green");
		break;

	case 3:
		filter = std::string("Blue");
		break;

	case 4:
		filter = std::string("H-alpha");
		break;

	case 5:
		filter = std::string("S-II");
		break;

	case 6:
		filter = std::string("O-III");
		break;

	case 7:
		filter = std::string("Dark");
		break;
	}

	std::cout << "Filter position is now set to: " << filter << std::endl;
}

void setTECTemp(ICameraPtr pCamera, float tec_temp)
{

	auto PTEC = pCamera->getTEC();
	if (PTEC == NULL)
	{
		std::cout << "Hasa diga ebawai" << std::endl;
	}

	handlePromise(PTEC->setState(true, tec_temp));
	std::cout << "TEC set point is : " << PTEC->getSetpoint() << std::endl;
	std::cout << "Setting TEC temprature " << std::endl;

	do
	{
		handlePromise(pCamera->queryStatus());
		float diff = abs(tec_temp - PTEC->getSensorThermopileTemperature());

		std::cout << "Sensor Temprature is: " << PTEC->getSensorThermopileTemperature() << std::endl;
		if (diff < 0.5)
			break;

		unsigned int microsecond = 1000000;
		usleep(2 * microsecond); //sleeps for 1 second
	} while (true);

	std::cout << "Sensor Temprature is: " << PTEC->getSensorThermopileTemperature() << std::endl;
	std::cout << "HeatSink Temprature is: " << PTEC->getHeatSinkThermopileTemperature() << std::endl;
}

void setSubFrame(ISensorPtr pSensor, IGatewayPtr pGateway)
{
	// Set the subframe for a full frame exposure
	auto info = pSensor->getInfo();
	TSubframe subf;
	subf.top = 0;
	subf.left = 0;
	subf.width = info.pixelsX;
	subf.height = info.pixelsY;
	subf.binX = 1;
	subf.binY = 1;

	try
	{
		handlePromise(pSensor->setSubframe(subf));
	}
	catch (std::exception &ex)
	{
		std::cout << "Failed to set subframe: " << ex.what() << std::endl;
		deleteGateway(pGateway);
		throw(ex);
	}
}

void expose(ICameraPtr pCamera, ISensorPtr pSensor, IGatewayPtr pGateway, double exp_time, int read_mode, int fwpos)
{

	// Start the exposure
	TExposureOptions options;
	options.duration = exp_time;
	options.binX = 1;
	options.binY = 1;
	options.readoutMode = read_mode;
	if (fwpos == 7)
	{
		options.isLightFrame = false;
	}
	else
	{
		options.isLightFrame = true;
	}
	options.useRBIPreflash = false;
	options.useExtTrigger = false;

	try
	{
		handlePromise(pSensor->startExposure(options));
	}
	catch (std::exception &ex)
	{
		std::cout << "Failed to start exposure: " << ex.what() << std::endl;
		deleteGateway(pGateway);
		throw(ex);
	}

	// Wait for exposure to complete
	do
	{
		try
		{
			handlePromise(pCamera->queryStatus());
		}
		catch (std::exception &ex)
		{
			std::cout << "Failed to query camera status: " << std::endl;
			deleteGateway(pGateway);
			throw(ex);
		}

		auto status = pCamera->getStatus();
		if (status.mainSensorState == ISensor::ReadyToDownload)
			break;
	} while (true);
}

void downloadImageTo(const char *path, ICameraPtr pCamera, ISensorPtr pSensor, IGatewayPtr pGateway, long int_temp, long ext_temp)
{
	try
	{
		handlePromise(pSensor->startDownload());
	}
	catch (std::exception &ex)
	{
		std::cout << "Failed to download the image: " << ex.what() << std::endl;
		deleteGateway(pGateway);
		throw(ex);
	}

	auto info = pSensor->getInfo();
	long bufferLength = info.pixelsX * info.pixelsY;
	unsigned short *pBuffer = new unsigned short[bufferLength];
	copyImageBuffer(pSensor, pBuffer, bufferLength);

	saveImage(pCamera, path, pBuffer, info.pixelsX, info.pixelsY, int_temp, ext_temp);
}

void saveImage(ICameraPtr pCamera, const char *filename, unsigned short *bufferData, long xdim, long ydim, long int_temp, long ext_temp)
{
	remove(filename); // Delete old file if it already exists

	//auto PTEC = pCamera->getTEC();

	// create file
	fitsfile *fptr;
	int status = 0;
	long naxes[2] = {xdim, ydim};

	fits_create_file(&fptr, filename, &status);
	fits_create_img(fptr, USHORT_IMG, 2, naxes, &status);

	// write the image to the file
	fits_write_img(fptr, TUSHORT, 1, xdim * ydim, bufferData, &status);

	// write header
	//float ext_temp = PTEC->getHeatSinkThermopileTemperature();
	fits_write_key(fptr, TFLOAT, "EXTTEMP", &ext_temp, "TEC external (hot) temperature [C]", &status);
	//float int_temp = PTEC->getSensorThermopileTemperature();
	fits_write_key(fptr, TFLOAT, "INTTEMP", &int_temp, "TEC internal (cold) temperature [C]", &status);
	fits_write_key(fptr, TDOUBLE, "EXPTIME", &exp_time, "Exposure time [s]", &status);
	fits_write_key(fptr, TDOUBLE, "FWPOS", &fwpos, "Filter wheel position", &status);
	fits_write_key(fptr, TINT, "READMODE", &read_mode, "Read Mode", &status);
	long time = std::time(0);
	fits_write_key(fptr, TLONG, "TIME", &time, "UNIX Time", &status);

	// close file
	fits_close_file(fptr, &status);
}

void copyImageBuffer(ISensorPtr pSensor, unsigned short *pBuffer, size_t bufferLength)
{
	IImagePtr pImage = pSensor->getImage();
	auto pImgBuf = pImage->getBufferData();
	auto bufLng = pImage->getBufferLength();

	// Take the minimum of the image's size and the pre-allocated pBuffer size, and multiply it by the size of its elements.
	auto cpyLng = sizeof(unsigned short) * std::min<size_t>(bufLng, bufferLength);
	memmove(pBuffer, pImgBuf, cpyLng);
}
