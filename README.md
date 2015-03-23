# ffmpeg-rgb-export
This is a simple utility that makes it straightforward to create videos from raw RGB data. No need to learn the details of ffmpeg. Remember, this is *just enough* code that will allow you to export a video so that you don't have to waste time learning about encoding and decoding and can focus on your application instead. 

There's only one class "VideoRGB" which you can use in your C++ project to export videos (I have tested the h264 and MPEG1 formats).

This repository also contains the necessary windows library and dll files, so you can get started without any additional downloads. To check the version of the ffmpeg builds, see ffmpeg-version.txt. Everything was downloaded from [here](http://ffmpeg.zeranoe.com/builds/).

Please check the included Visual Studio solution/project for project settings (linking libraries etc...). All required headers and libraries (including those you don't need for this simple application) are in the MyLib folder.

Using the VideoRGB class is pretty straight forward. See main.cpp for reference.

Start by initializing the required codecs:
  avcodec_register_all();
Then create a new VideoRGB object:
  VideoRGB* video = new VideoRGB("test.h264", width, height, AV_CODEC_ID_H264); // for h264 video
OR
	VideoRGB* video = new VideoRGB("test.mpg", width, height, AV_CODEC_ID_MPEG1VIDEO); // for mpeg-1 video
	
You can choose the framerate and bit-rate(quality) of the video by providing extra constructor parameters. The default values are 24 frames per second and 400000 bits per second.

Create your RGB buffer (or get it from somewhere, e.g. the OpenGL framebuffer). Note that I have allocated the memory with word alignment, which should improve performance. This might not be very relevant for RGB24 data type, but is highly recommended when dealing with 32-byte data types like RGBA or RGBX.

  uint8_t* data = (uint8_t*) _aligned_malloc(width * height * 3, 32);
  // fill this buffer with your picture data
  
Then add this picture as a frame in the video
  video->addFrameRGB(data);
  
Add as many frames as you want, then finish up the video by calling finish()
  video->finish();
  
Finally, don't forget to clean up the objects you constructed
  _aligned_free(data);
	delete video;
	
Cheers.
