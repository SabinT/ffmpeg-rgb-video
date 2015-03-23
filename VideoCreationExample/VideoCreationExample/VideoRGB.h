#ifndef VIDEORGB_H
#define VIDEORGB_H

#include <string>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
};

class VideoRGB
{
public:
	VideoRGB(
		const char* filename, 
		unsigned int width_, 
		unsigned int height_, 
		enum AVCodecID codecID = AV_CODEC_ID_H264,
		unsigned int bitrate = 400000,
		float framerate = 24.0f,
		enum AVPixelFormat pixelFormat = AV_PIX_FMT_YUV420P
		);

	/**
	 * Finish up the video and close the file
	 */
	void finish();

	/**
	 * Load a picture into the current frame and add the frame to video.
	 * \param buffer The buffer containing the image data. Should be in the 'AV_PIX_FMT_RGB24' format
	 */
	void addFrameRGB(uint8_t* bufferRGB);

	virtual ~VideoRGB();

protected:
	/** The width of the video. */
	unsigned int width = 0;
	/** The height of the video. */
	unsigned int height = 0;

	/** The codec used to encode the video  */
	AVCodec *codec = NULL;

private:
	/** The codec context */
	AVCodecContext *context = NULL;

	/** Buffer to hold encoded video data */
	uint8_t* outBuffer; // should be sufficient for one frame

	/** The file this video gets written to */
	FILE* f;

	/* some useful state */
	/** The next frame that will be encoded. */
	AVFrame* frame;

	/** The pixel conversion context for RGB24 to YUV420P conversion */
	SwsContext* swsCtxRGBtoYUV;

	int frameCount;
	AVPacket pkt;
	int gotPacket;

	/** Print a message and exit */
	void exitWithMessage(const char* message);
};

#endif