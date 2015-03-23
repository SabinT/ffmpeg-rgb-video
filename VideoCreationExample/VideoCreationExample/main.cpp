#include "VideoRGB.h"

#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	int width = 320;
	int height = 240;

	avcodec_register_all();

	//VideoRGB* video = new VideoRGB("test.h264", width, height, AV_CODEC_ID_H264);
	VideoRGB* video = new VideoRGB("test.mpg", width, height, AV_CODEC_ID_MPEG1VIDEO);

	// create an RGB buffer (memory aligned to 32-byte boundaries)
	uint8_t* data = (uint8_t*) _aligned_malloc(width * height * 3, 32);
	//uint8_t* data = new uint8_t[width * height * 3];

	
	// 100 frames fading from blue to yellow
	int lineSizeRGB = width * 3;
	for (int i = 0; i < 100; i++) {
		// create some RGB data
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				data[y * lineSizeRGB + x * 3] = (i / 99.0f) * 255;
					//(uint8_t) ((1 + sin(x / 320.0 * 2 * M_PI * i / 2)) * 0.5 * 255);
				data[y * lineSizeRGB + x * 3 + 1] = (i / 99.0f) * 255;
				data[y * lineSizeRGB + x * 3 + 2] = ((99.0f - i) / 99.0f) * 255;
			}
		}

		video->addFrameRGB(data);
	}

	video->finish();

	// clean up
	_aligned_free(data);
	delete video;

	system("pause");
	return 0;
}

