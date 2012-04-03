#include "cv.h"
#include "cvaux.h"
#include "highgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

#include "facedetect.h"

#define ORIG_WIN_SIZE  20

static CvMemStorage* storage;
static CvHaarClassifierCascade* cascade = 0;


int initface()
{
	storage = cvCreateMemStorage(0);
	cascade = cvLoadHaarClassifierCascade( "/usr/share/harpia/images/haarcascade_frontalface_alt2.xml",cvSize( 20,20 ));
	return 1;
}

void releaseface()
{
     cvReleaseHaarClassifierCascade( &cascade );
     cvReleaseMemStorage( &storage );
}

void detect_and_draw( IplImage* img )
{
    int scale = 2;
    float r;
    IplImage* temp = cvCreateImage( cvSize(img->width/2,img->height/2), 8, 3 );
    IplImage* canvas = cvCreateImage( cvSize(temp->width,temp->height*3/2), 8, 3 );
    CvPoint offset = cvPoint( 0, temp->height/3 );
    CvPoint pt1, pt2,ptr,ptc;
    int i;

    cvZero( canvas );
    cvPyrDown( img, temp, CV_GAUSSIAN_5x5 );
    cvSetImageROI( canvas, cvRect( offset.x, offset.y, temp->width, temp->height ));
    cvCopy( temp, canvas, 0 );
    cvResetImageROI( canvas );
    
    if(cascade )
    {
        CvSeq* faces = cvHaarDetectObjects( temp, cascade, storage,
                                            1.2, 2, CV_HAAR_DO_CANNY_PRUNING );
        if(faces->total!=0)
        {
        //printf("\n ttl faces:%d\n",faces->total);
        	for( i = 0; i < (faces ? faces->total : 0); i++ )
        	{

            	CvRect* r = (CvRect*)cvGetSeqElem( faces, i);
 				if(r)
 				{
            		pt1.x = r->x*scale;
            		pt1.y = r->y*scale;
            		pt2.x = (r->x+r->width)*scale;
            		pt2.y = (r->y+r->height)*scale;
 					cvSetImageROI( img, cvRect(pt1.x,pt1.y, r->width*scale,r->height*scale ));
					cvZero(img);
					cvThreshold(img,img,0,255,CV_THRESH_BINARY_INV);
					cvResetImageROI(img);
					//printf("in face rect\n");
				}
	    //ptc.x=(pt1.x+pt2.x)/2;
	    //ptc.y=(pt1.y+pt2.y)/2;
    	    //r=(pt2.x-pt1.x)/2;
            //cvRectangle( img, pt1, pt2, CV_RGB(255,255,255), 3 );

	        }
        }
    }
	
    cvReleaseImage( &temp );
    cvReleaseImage( &canvas );
}
