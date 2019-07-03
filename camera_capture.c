#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<raspicam/raspicam.h>
#include<unistd.h>

#define imageWidth 640 //allowable width : 320, 6 40, 1280
#define imageHeight 480 //allowable height : 240, 480, 960

char verbose='n';
unsigned long byte;
unsigned char *data;

void verbosePrint(char *s){
	printf("in verbosePrint\n");
	if(verbose=='y')
		printf("%s\n",s);
	return;
}

void caputre(){
	//capture frames
	Camera.grab();
	
	//extract the image
	Camera.retrive(data,raspicam::RASPICAM_FORAMT_IGNORE);	
}

void initialize(char ver){
	
	if(ver=='n'||ver=='y')
		verbose=ver;
		
	//setting up the camera 
	raspicam::RaspiCam Camera;
	if(!Camera.setForamt(raspicam::RASPICAM_FORMAT_RGB))printf("error in Format initalization\n");//formats available : RASPICAM_FORMAT_GRAY,RASPICAM_FORMAT_RGB,RASPICAM_FORMAT_BGR,RASPICAM_FORMAT_YUV420  
	if(!Camera.setCaptureSize(imageWidth,imageHeight))printf("error in CaptureSize initialization\n");
	
	//opening camera
	if(!Camera.open())printf("error opening the camera\n");
	usleep(100000);
	
	//Allocating memory for camera buffer
	bytes = Camera.getImageBufferSize();
 	data  = new unsigned char[bytes];

	//sending the confirmation to the output in verbose
	verbosePrint("initaliztion complete");
	
}

void main(){
	int i;
	intialize('y');
	for(i=0;i<1;i++)
		capture();
	printf("complete");
	return;	
}
