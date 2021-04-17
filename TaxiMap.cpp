// Sautto, Marcello CS230 Section 12159  4/01/2021
// Second Laboratory Assignment - Map NYC Taxi Destinations
// Environment: Windows 10 Visual Studio 2019

#include <iostream>
#include <fstream>
#include "windows.h"
#include <math.h>
using namespace std;
// The following defines the size of the square image in pixels.
#define IMAGE_SIZE 1024

int main()
{
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;

	int i, j, k, l, x, y, imgSize, whiteBit;
	float north_bound, south_bound, east_bound, west_bound;
	// The following defines the array which holds the image.  
	char map[IMAGE_SIZE][IMAGE_SIZE] = { 0 };
	char colorTable[1024];

	imgSize = IMAGE_SIZE;
	whiteBit = 255;
	x = y = 0;
	north_bound = 40.830509;
	south_bound = 40.700455;
	east_bound = -73.914979;
	west_bound = -74.045033;

	ifstream dataFile("L2Data10K.dat", std::ifstream::binary);
	if (dataFile) {
		//determine length of file
		dataFile.seekg(0, dataFile.end);
		int len = dataFile.tellg(); //length of dataFile
		dataFile.seekg(0, dataFile.beg);
		dataFile.close();

		float* buffer = new float[len]; //used to load initial file
		float* data = new float[len]; //load all valid data entries into this array

		//load data into buffer
		dataFile.open("L2Data10K.dat", ios::binary | ios::in);
		dataFile.read((char*)buffer, len); //read binary file into buffer array
		dataFile.close();

		for (i = 0, j = 0; i < len; i += 2)
		{
			if (buffer[i] >= south_bound && buffer[i] <= north_bound && buffer[i + 1] >= west_bound && buffer[i + 1] <= east_bound) //if the current longitude and latitude are within the bounds, store it in the data array
			{
				data[j] = buffer[i];
				data[j + 1] = buffer[i + 1];
				j += 2;
			}
		}
		len -= (len - j); //length now set to # of valid inputs
		delete[]buffer;

		__asm
		{
			MOV ECX, len; loop will only iterate as many times as the size of the array
			MOV ESI, data[0]; ESI contains the array

			solve:
			;;;;;; SOLVE LONGITUDE;;;;;;
			FLD south_bound;
			FLD north_bound;
			FSUB ST(0), ST(1); //north - south
			FLD[ESI]; push south_bound into ST(0) //push current longitude onto stack
				FSUB ST(0), ST(2); // longitude - south_bound
			FDIV ST(0), ST(1); // (longitude - south_bound) / (north - south)
			FIMUL imgSize; //multiply prev value by 1024
			FRNDINT; //round float to int

			//load x register into EBX, store the converted longitude value into x and pop it from the stack
			LEA EBX, x; store the results memory address into EBX
				FISTP[EBX]; value in EBX is stored in the results arrayand ST(0) is popped

				//clear the stack
				FFREE ST(0); Clear ST(0)
				FFREE ST(1); Clear ST(1)
				FFREE ST(2); Clear ST(2)

				//increment to the latitude and decrease the counter
				ADD ESI, 4
				DEC ECX; decriment counter

				;;;;;; SOLVE LATITUDE;;;;;;
			FLD west_bound;
			FLD east_bound;
			FSUB ST(0), ST(1); //east - west
			FLD[ESI]; push west_bound into ST(0) //push current latitude onto stack
				FSUB ST(0), ST(2); subtract the current buffer value by ST(0) // latitude - west_bound
				FDIV ST(0), ST(1); // (latitude - west_bound) / (east - west)
			FIMUL imgSize; //multiply prev value 1024
			FRNDINT;

			//load y register into EBX, store the converted longitude value into y and pop it from the stack
			LEA EBX, y; store the results memory address into EBX
				FISTP[EBX]; value in EBX is stored in the results arrayand ST(0) is popped

				//clear the stack
				FFREE ST(0); Clear ST(0)
				FFREE ST(1); Clear ST(1)
				FFREE ST(2); Clear ST(2)

				;;;; PUT VALUES INTO BITMAP;;;;
			//compute the index for x and y in the bit map
			MOV EAX, x
				MOV EBX, y
				MOV EDX, imgSize
				MUL EDX //x*imgSize
				ADD EAX, EBX // add y to prev value

				//store white bit (255) into respective bit map location
				FILD whiteBit
				LEA EBX, map[EAX]
				FISTP[EBX]

				//increment the data array to the next value and decriment the counter
				ADD ESI, 4
				DEC ECX; decriment counter

				JNZ solve;

			//continue this until we have converted all values in the data array

		}

	}
	else
	{
		cout << "ERROR: File Not Read.";
		return 0;
	}

	// Define and open the output file. 
	ofstream bmpOut("bitmap.bmp", ios::out + ios::binary);
	if (!bmpOut) {
		cout << "...could not open file, ending.";
		return -1;
	}
	// Initialize the bit map file header with static values.
	bmfh.bfType = 0x4d42;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(bmfh) + sizeof(bmih) + sizeof(colorTable);
	bmfh.bfSize = bmfh.bfOffBits + sizeof(map);

	// Initialize the bit map information header with static values.
	bmih.biSize = 40;
	bmih.biWidth = IMAGE_SIZE;
	bmih.biHeight = IMAGE_SIZE;
	bmih.biPlanes = 1;
	bmih.biBitCount = 8;
	bmih.biCompression = 0;
	bmih.biSizeImage = IMAGE_SIZE * IMAGE_SIZE;
	bmih.biXPelsPerMeter = 2835;  // magic number, see Wikipedia entry
	bmih.biYPelsPerMeter = 2835;
	bmih.biClrUsed = 256;
	bmih.biClrImportant = 0;

	// Build color table.
	for (i = 0; i < 256; i++) {
		j = i * 4;
		colorTable[j] = colorTable[j + 1] = colorTable[j + 2] = colorTable[j + 3] = i;
	}

	// Write out the bit map.  
	char* workPtr;
	workPtr = (char*)&bmfh;
	bmpOut.write(workPtr, 14);
	workPtr = (char*)&bmih;
	bmpOut.write(workPtr, 40);
	workPtr = &colorTable[0];
	bmpOut.write(workPtr, sizeof(colorTable));
	workPtr = &map[0][0];
	bmpOut.write(workPtr, IMAGE_SIZE * IMAGE_SIZE);
	bmpOut.close();

	//Print bit map on mspaint
	system("mspaint bitmap.bmp");

	return 0;
}

