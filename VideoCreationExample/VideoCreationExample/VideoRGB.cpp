#include "VideoRGB.h"

#include <iostream>

extern "C" {
	#include <libavutil/opt.h>
	#include <libavutil/imgutils.h>
}

using namespace std;

VideoRGB::VideoRGB(
	const char* filename,
	unsigned int width_,
	unsigned int height_,
	enum AVCodecID codecID,
	unsigned int bitrate,
	float framerate,
	enum AVPixelFormat pixelFormat
	) 
{
	width = width_;
	height = height_;
	
	// find the encoder
	codec = avcodec_find_encoder(codecID);
	if (!codec) {
		cerr << "Codec not found. [codecID = " << codecID << "]" << endl;
		exit(-1);
	}

	// allocate the codec context
	context = avcodec_alloc_context3(codec);
	if (!context) {
		cerr << "Could not allocate codec context[codecID = " << codecID << "]" << endl;
		exit(-2);
	}

	// configure the codec context (specify bit-rate, frame-rate, width, height, pixel format, etc)
	context->bit_rate = 400000;
	context->width = width;
	context->height = height;
	AVRational secondsPerFrame = { 1, framerate };
	context->time_base = secondsPerFrame; // seconds per frame

	context->gop_size = 10; // interval between I-frames
	context->max_b_frames = 1; 
	context->pix_fmt = pixelFormat;

	// special check for H264, not sure why
	if (codecID == AV_CODEC_ID_H264) {
		av_opt_set(context->priv_data, "preset", "slow", 0);
	}
	
	// open codec
	if (avcodec_open2(context, codec, NULL) < 0) {
		cerr << "Could not open codec[codecID = " << codecID << "]" << endl;
		exit(-3);
	}

	// open file for writing
	f = fopen(filename, "wb");
	if (!f) {
		cerr << "Cannot open " << filename << endl;
		exit(-4);
	}

	// create buffer to hold frame data in YUV420P format
	int size = width * height;
	// create a frame (re-usable)
	frame = av_frame_alloc();
	frame->width = width;
	frame->height = height;
	frame->format = pixelFormat;
	// frameBuffer = new uint8_t[(size * 3) / 2];
	int ret = av_image_alloc(frame->data, frame->linesize, context->width, context->height, context->pix_fmt, 32);
	if (ret < 0) {
		exitWithMessage("Effor allocating raw frame data.");
	}
	//frame->data[0] = frameBuffer; // the Y-plane starts at &data[0][0]
	//frame->data[1] = frame->data[0] + size; // start of U plane
	//frame->data[2] = frame->data[1] + size / 4; // start of V plane
	//frame->linesize[0] = width;
	//frame->linesize[1] = width / 2;
	//frame->linesize[2] = width / 2;

	// buffer to hold encoded frames temporarily before writing to disk
	outBuffer = new uint8_t[100000];
	
	// create a pixel buffer conversion context
	swsCtxRGBtoYUV = sws_getContext(width, height, AV_PIX_FMT_RGB24, width, height, AV_PIX_FMT_YUV420P, 0, 0, 0, 0);
	if (!swsCtxRGBtoYUV) {
		cerr << "Could not initialize the conversion context\n" << endl;
		exit(1);
	}

	frameCount = 0;
}

VideoRGB::~VideoRGB() {
	delete outBuffer;

	sws_freeContext(swsCtxRGBtoYUV);
	avcodec_close(context);
	av_free(context);

	// delete any raw data inside the frame
	av_freep(&frame->data[0]);
	av_free(frame);
}


void VideoRGB::addFrameRGB(uint8_t* bufferRGB) {
	uint8_t* rgbPlanes[1] = {bufferRGB};
	int rgbLineSize[1] = { 3 * width };
	// convert from RGB to YUV420P and store in the frame
	int ret = sws_scale(swsCtxRGBtoYUV, rgbPlanes, rgbLineSize, 0, width, frame->data, frame->linesize);
	frame->pts = frameCount++;

	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	ret = avcodec_encode_video2(context, &pkt, frame, &gotPacket);

	if (gotPacket) {
		printf("Write frame %3d (size=%5d)\n", frameCount, pkt.size);
		// write frame to file
		fwrite(pkt.data, 1, pkt.size, f);
		av_free_packet(&pkt);
	}
}

void VideoRGB::finish() {
	/* get the delayed frames */
	for (gotPacket = 1; gotPacket; frameCount++) {
		fflush(stdout);

		int ret = avcodec_encode_video2(context, &pkt, NULL, &gotPacket);
		if (ret < 0) {
			exitWithMessage("Error encoding frame.");
		}

		if (gotPacket) {
			printf("Write frame %3d (size=%5d)\n", frameCount, pkt.size);
			fwrite(pkt.data, 1, pkt.size, f);
			av_free_packet(&pkt);
		}
	}

	/* add sequence end code to have a real mpeg file */
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);
}

void VideoRGB::exitWithMessage(const char* message) {
	cerr << message << endl;
	exit(-1);
}