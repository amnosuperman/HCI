#include "cv.h"
#include <stdio.h>
#include "handdet.h"
// File-level variables
CvHaarClassifierCascade * pCascade = 0;  // the hand detector
CvMemStorage * pStorage = 0;             // memory for detector to use
CvSeq * pHandRectSeq;                    // memory-access interhand
// initHandDet()
int initHandDet(const char * haarCascadePath)
{
	pStorage = cvCreateMemStorage(0);
	pCascade = (CvHaarClassifierCascade *)cvLoad( haarCascadePath, 0, 0, 0 );
	pStorage = cvCreateMemStorage(0);

	return 1;
}
// closeHandDet()
void closeHandDet()
{
	if(pCascade) cvReleaseHaarClassifierCascade(&pCascade);
	if(pStorage) cvReleaseMemStorage(&pStorage);
}
// detectHand()
CvRect *detectHand(IplImage * pImg)
{
	CvRect* r = 0,* block1_rect_o2=0;
	block1_rect_o2=(CvRect *)malloc(sizeof(CvRect));
	float scale=1.3;
	// detect hands in image
	//pCascade = (CvHaarClassifierCascade *)cvLoad( "/home/vignesh/imgproc/hand.xml", 0, 0, 0 );
	cvClearMemStorage( pStorage );
	IplImage* gray = cvCreateImage( cvSize(pImg->width,pImg->height), 8, 1 );
	IplImage* small_img = cvCreateImage( cvSize( cvRound (pImg->width/scale), cvRound (pImg->height/scale)),8, 1 );
	cvCvtColor( pImg, gray, CV_BGR2GRAY );
	cvResize( gray, small_img, CV_INTER_LINEAR );
	cvEqualizeHist( small_img, small_img );
    //printf("hi\n");
	pHandRectSeq = cvHaarDetectObjects(small_img,pCascade,pStorage,1.1, 2.0, 0/*CV_HAAR_DO_CANNY_PRUNING*/,cvSize(20,20) );
	// if one or more hands are detected, return the first one
	if( pHandRectSeq && pHandRectSeq->total ) {
		r = (CvRect*)cvGetSeqElem(pHandRectSeq, 0);
		if(r) {
		block1_rect_o2->x = (r->x)*scale;
		block1_rect_o2->y = (r->y)*scale;
		block1_rect_o2->width = (r->width)*scale;
		block1_rect_o2->height = (r->height)*scale;}
    }
	return block1_rect_o2;
}



