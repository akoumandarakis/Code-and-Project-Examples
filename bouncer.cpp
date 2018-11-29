// Alex Koumandarakis and Tristan Willis
// bouncer.cpp
//
// Based on:
//
//   tutorial01.c,
//   written by Stephen Dranger (dranger@gmail.com)
//   https://github.com/mpenkov/ffmpeg-tutorial/blob/master/tutorial01.c
//     based on a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
//     Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1
//
//   video-transcode.c
//   written by jwerle 
//   https://github.com/littlstar/video-transcode-experiment/blob/master/video-transcode.c
//
//   encoding-decoding.c
//   Part of ffmpeg examples
//
//   writewav.c
//   Written by Ted Burke
//   https://batchloaf.wordpress.com/2017/02/10/a-simple-way-to-read-and-write-audio-and-video-files-in-c-using-ffmpeg/

#ifndef INT64_C
#define UINT64_C(c) (c ## LL)
#define INT64_C(c) (c ## LL)
#endif

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}
#include <math.h>
#include <iostream>

using namespace std;

//Creates 300 frames of a ball bouncing on a background of a provided image.
//Uses ffmpeg as well as a custom codec for ffmpeg to decode the bacground image
//and encode each frame after the ball is programatically drawn.  


//Takes a frame and draws a circle at (xPos, yPos) with the specified radius
void drawCircle(AVFrame * frame,  int xPos, int yPos, int radius)
{
  double xR = (double)xPos;
  double yR = (double)yPos;
  double r = (double)radius;

  for (int y = 0; y < frame->height; y++)
    {
      for (int x = 0; x < frame->width; x++)
	{
	  //Calculate the distance from the current pixel to the center of the ball
	  double xp = (double)x;
	  double yp = (double)y;
	  double xdist = xp -xR;
	  double ydist = yp - yR;
	  int dist = (int)sqrt((xdist * xdist) + (ydist * ydist));

	  //If the pixel is in the radius of the ball
	  //Change the color of the pixel
	  if (dist <= radius)
	    {
	      double xHPos = xR + r/2;
	      double yHPos = yR - r/2;
	      double xHDist = x - xHPos;
	      double yHDist = y - yHPos;
	      int hDist = (int)sqrt((xHDist * xHDist) + (yHDist * yHDist));
	      
	      //Reduce the values of G and B the farther they are from the sun spot (xHPos, yHpos)
	      //So that the sun spot appears white
	      int color;
	      if (255 - 4*hDist < 0)
		{
		  color = 0;
		}
	      else
		{
		  color = 255-4*hDist;
		}

	      frame->data[0][frame->linesize[0]*y + x*3] = 255;
	      frame->data[0][frame->linesize[0]*y + x*3 + 1] = color;
	      frame->data[0][frame->linesize[0]*y + x*3 + 2] = color;
	      
	    }
	}
    }
} 
 
//Change the x position of the ball based on its current velocity
//Will reverse velocity when it hits one of the edges specified
void calcXPosition(int time, int * vxo, int * xPos, int leftEdge, int rightEdge)
{
  //Calculate the new x position
  *xPos = *xPos + ((*vxo));
  if (*xPos >= rightEdge || *xPos <= leftEdge)
    {
      //If the ball hits an edge, change the direction of it's velocity in the x axis 
      *vxo = -*vxo;
    }
}

//Change the y position of the ball based on its current velocity and accelleration
//Will bounce when it hits the bottom given
void calcYPosition(int time, int * vyo, int * yPos, int a, int bottom)
{
  //Calculate the new y position
  *vyo = *vyo + a;
  *yPos = *yPos + *vyo;
  if (*yPos >= bottom && *vyo > 0)
    {
      //If the ball hits the bottom, redirect the velocity
      *vyo = -*vyo;
    }
}

//Takes a frame and copies it into a frame with RGB24 pixel format for easier pixel data manipulation
AVFrame * copyFrameToRGB(AVCodecContext * codecContext, AVFrame * jpgFrame, uint8_t * newFrameBuffer, AVPixelFormat pixFormat)
{
  //Create new frame and alloc frame
  AVFrame * newFrame = NULL;
  newFrame = av_frame_alloc();

  newFrame->height = jpgFrame->height;
  newFrame->width = jpgFrame->width;
  newFrame->format = AV_PIX_FMT_RGB24;

  //Get the number of bytes needed for the new frame and allocate the buffer
  int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 32);
  newFrameBuffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

  //Fill the new frame arrays
  int ret = av_image_fill_arrays(newFrame->data, newFrame->linesize, newFrameBuffer, 
				 AV_PIX_FMT_RGB24, jpgFrame->width, jpgFrame->height, 32);
  if (ret < 0)
    {
      //ERROR
      return NULL;
    }

  //Create empty SwsContext and get the correct context from the codec context provided
  struct SwsContext* swsContext = NULL;
  swsContext = sws_getCachedContext(NULL, codecContext->width, codecContext->height, pixFormat, 
				    codecContext->width, codecContext->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, 
				    NULL, NULL, NULL);

  //Copy the data from the original frame to the new frame
  sws_scale(swsContext, (uint8_t const * const *)jpgFrame->data, 
	    jpgFrame->linesize, 0, codecContext->height, 
	    newFrame->data, newFrame->linesize);

  return newFrame;
}

//Takes an spff encoded packet and saves it to a spff file
//Based on SaveFrame function in tutorial01.c by Stephen Dranger
void SaveFrame(AVPacket *packet, int width, int height, int frameNum) {
  FILE *file;
  char filename[32];
  
  // Open file
  sprintf(filename, "frame%'.03d.spff", frameNum);
  printf("Creating frame %s \n", filename);
  file=fopen(filename, "w");
  if(file==NULL)
    {
      printf("Could not open file\n");
      return;
    }

  //Dump packet data into the file
  fwrite(packet->data, 1, packet->size, file);

  // Close file
  fclose(file);
}

//Main fuction based on tutorial01.c by Stephen Dranger
int main(int argc, char *argv[]) {
  
  //Make sure the user supplies filename
  if(argc < 2) {
    printf("Specify a .jpg image\n");
    return -1;
  }

  string fName = argv[1];
  size_t dotPos = fName.find_last_of(".");
  string extension = fName.substr(dotPos);

  if (extension != ".jpg")
    {
      printf("Please specify a .jpg image\n");
      return -1;
    }
  
  

  // Register all formats and codecs
  av_register_all();


  AVFormatContext *formatContext = NULL;
  
  //Open .jpg file and create format context
  if(avformat_open_input(&formatContext, argv[1], NULL, NULL)!=0)
    return -1; // Couldn't open file


  //Retrieve stream information from the format context
  if(avformat_find_stream_info(formatContext, NULL)<0)
    return -1; // Couldn't find stream information

  
  //Print out information about the file
  av_dump_format(formatContext, 0, argv[1], 0);
 

  //Find the first video stream in the format context
  int videoStream=-1;
  for(int i=0; i<formatContext->nb_streams; i++)
    if(formatContext->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return -1; // Didn't find a video stream
  

  //Get a pointer to the codec context for the video stream
  AVCodecContext * codecContext = NULL; 

  //Find the decoder for the video stream using the codec_id from the codec parameters of the stream
  AVCodec * codec = NULL;
  codec=avcodec_find_decoder(formatContext->streams[videoStream]->codecpar->codec_id);
  if(codec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }

  //Allocate the codec context's memory and convert the stream paremeters to the codec context
  codecContext = avcodec_alloc_context3(codec);
  avcodec_parameters_to_context(codecContext, formatContext->streams[videoStream]->codecpar);

  // Open codec
  if(avcodec_open2(codecContext, codec, NULL) < 0)
    return -1; // Could not open codec

  //If the supplied image is using a deprecated pixel format, change it
  AVPixelFormat pixFormat;
  switch (codecContext->pix_fmt) {
  case AV_PIX_FMT_YUVJ420P :
    pixFormat = AV_PIX_FMT_YUV420P;
    break;
  case AV_PIX_FMT_YUVJ422P  :
    pixFormat = AV_PIX_FMT_YUV422P;
    break;
  case AV_PIX_FMT_YUVJ444P   :
    pixFormat = AV_PIX_FMT_YUV444P;
    break;
  case AV_PIX_FMT_YUVJ440P :
    pixFormat = AV_PIX_FMT_YUV440P;
  default:
    pixFormat = codecContext->pix_fmt;
    break;
  }

  // Allocate video frame
  AVFrame * frame = NULL;
  frame=av_frame_alloc();


  //Read through formatContext until the video stream is found
  AVPacket packet;
  int frameFinished;
  AVFrame * newFrame = NULL;
  uint8_t * newFrameBuffer = NULL;
  while (av_read_frame(formatContext, &packet) >= 0)
    {
      if (packet.stream_index == videoStream)
	{
	  //Send the encoded packet to the decoder
	  if (avcodec_send_packet(codecContext, &packet) < 0)
	    {
	      printf("Error sending packet for decoding\n");
	      return -1;
	    }

	  //Get the decoded packet from the decoder
	  if (avcodec_receive_frame(codecContext, frame) < 0)
	    {
	      printf("Error receiving decodec frame\n");
	      return -1;
	    }
	  
	  //Copy the frame to a frame with RGB2 pixel format
	  newFrame = copyFrameToRGB(codecContext, frame, newFrameBuffer, pixFormat);
	}
    }
  
  //Free the memory for the packet
  av_packet_unref(&packet);

  if (newFrame == NULL)
    {
      printf("Could not copy to RGB pixel format\n");
      return -1;
    }
  
  //Get SPFF encoder
  AVCodec * encCodec = NULL;
  encCodec = avcodec_find_encoder(AV_CODEC_ID_SPFF);
  if (encCodec == NULL)
    {
      //FAILED TO FIND ENCODER
      printf("Could not find SPFF Encoder\n");
      return -1;
    }

  
  //Allocate encoder context
  AVCodecContext * encCodecContext = NULL;
  encCodecContext = avcodec_alloc_context3(encCodec);
  if (encCodecContext == NULL)
    {
      //FAILED TO ALLOCATE CONTEXT
      printf("Could not allocate codec context for encoder\n");
      return -1;
    }

  //Populate encoder context fields
  encCodecContext->width = newFrame->width;
  encCodecContext->height = newFrame->height;
  encCodecContext->time_base = (AVRational){1,30};
  encCodecContext->pix_fmt = AV_PIX_FMT_RGB24;
 
  //Open encoder codec
  if (avcodec_open2(encCodecContext, encCodec, NULL) < 0)
    {
      //FAILED TO OPEN CODEC IN CONTEXT
      printf("Could not open encoder context\n");
      return -1;
    }

  //initialize variables for calculating position and area of ball
  int radius = (newFrame->height) / 10;    //10% of the height
  
  int Vxo = 5;                             //initial x velocity, some positive constant
  int xPos = (radius);                     //initial x position, zero

  int Vyo = 0;                             //initial y velocity, zero
  int yPos = (radius);                     //initial y position, top of image
  int a = 1;                               //y acceleration, some positive constant

  //Bounds of the image, used to determine when ball will bounce
  int leftEdge = radius;
  int rightEdge = newFrame->width - radius;
  int bottom = newFrame->height - radius - 50;

  //Define variables for creating sound sin wave
  //This, and other section dealing with creating and saving sound, are based on writewav.c by Ted Burke
  int lengthOfSound = 441000;
  int16_t buf[lengthOfSound];
  double Fs = 44100.0;
  int n = 0;

  for (int time = 1; time <= 300; time++)
    {
      //Create a new frame that the ball will be drawn on
      AVFrame * drawFrame = NULL;
      uint8_t * drawFrameBuffer = NULL;
      drawFrame = copyFrameToRGB(codecContext, frame, drawFrameBuffer, pixFormat);
      
      //Calculate the position of the ball and draw the circle on it
      calcXPosition(time, &Vxo, &xPos, leftEdge, rightEdge);
      calcYPosition(time, &Vyo, &yPos, a, bottom);
      drawCircle(drawFrame, xPos, yPos, radius);

      //Every frame, generate a sin based on the current y velocity of the ball
      int startn = n;
      while (n < startn + 1470)
	{
	  buf[n] = 16383.0 * sin(n*(200.0+100*Vyo)*2.0*M_PI/Fs);
	  n++;
	}
     
      //Send frame to encoder
      if (avcodec_send_frame(encCodecContext, drawFrame) < 0)
	{
	  //ERROR ENCODING FRAME
	  printf("Could not encode frame\n");
	  return -1;
	}

      //Get encoded data packet from encoder
      AVPacket packetToSave;
      av_new_packet(&packetToSave, 0);
      if (avcodec_receive_packet(encCodecContext, &packetToSave) < 0)
	{
	  //ERROR GETTING ENCODED PACKET
	  printf("Error receiving encoded packet\n");
	  return -1;
	}
      
      //Save encoded packet into SPFF file
      SaveFrame(&packetToSave, drawFrame->width, drawFrame->height, time);  

      //Free memory used by drawFrame and the saved packet
      av_free(drawFrameBuffer);
      av_free(drawFrame);
      av_packet_unref(&packetToSave);
    }

  //Save the sound data in a .wav file
  FILE *pipeout;
  pipeout = popen("ffmpeg -y -f s16le -ar 44100 -ac 1 -i - sound.wav", "w");
  fwrite(buf, 2, lengthOfSound, pipeout);
  pclose(pipeout);  
  
  // Free the RGB frame
  av_free(newFrameBuffer);
  av_free(newFrame);
  
  // Free the original frame
  av_free(frame);
  
  // Close the codecs
  avcodec_close(codecContext);
  avcodec_close(encCodecContext);
  
  // Close the format context
  avformat_close_input(&formatContext);
  
  return 0;
}
