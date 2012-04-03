#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include "cvaux.h"
#include <stdio.h>

//Kalman filter class
class KalmanFilter
{

	public:
	KalmanFilter();
	~KalmanFilter();
	
	void predictionBegin(CvRect &rect);
	CvRect predictionReport(CvRect &rect);
	CvKalman *m_pKalmanFilter;
	CvRandState rng;
	CvMat* measurement;

};

// Main Tracking Control functions
int     createTracker(const IplImage * pImg);
void    releaseTracker();
void    startTracking(IplImage * pImg, CvRect pHandRect,KalmanFilter &kfilter);
CvRect combi_track(IplImage * pImg,KalmanFilter &kfilter);
CvRect camshift_track(IplImage * pImg);


// Parameter settings
void setVmin(int vmin);
void setSmin(int smin);

//---------------------------------------------------------------------------------------------------
//Kalman definitions
//kalman constructor

KalmanFilter::KalmanFilter()
{

	int dynam_params = 8; // x,y,width,height,dx,dy,dw,dh
	int measure_params = 4;//x,y,width,height

	m_pKalmanFilter = cvCreateKalman(dynam_params, measure_params,0);
	cvRandInit( &rng, 0, 1, -1, CV_RAND_UNI );
	cvRandSetRange( &rng, 0, 1, 0 );
	rng.disttype = CV_RAND_NORMAL;
	measurement = cvCreateMat( measure_params, 1, CV_32FC1 ); 
	cvZero(measurement);

	// F matrix data
	// F is transition matrix. It relates how the states interact
	const float F[] = {
	1, 0, 0, 0, 1, 0, 0, 0, //x + dx
	0, 1, 0, 0, 0, 1, 0, 0,//y + dy
	0, 0, 1, 0, 0, 0, 1, 0,//width + dw
	0, 0, 0, 1, 0, 0, 0, 1,//height + dh
	0, 0, 0, 0, 1, 0, 0, 0,//dx
	0, 0, 0, 0, 0, 1, 0, 0,//dy
	0, 0, 0, 0, 0, 0, 1, 0,//dw
	0, 0, 0, 0, 0, 0, 0, 1 //dh
	};

	memcpy( m_pKalmanFilter->transition_matrix->data.fl, F, sizeof(F));
	cvSetIdentity( m_pKalmanFilter->measurement_matrix, cvRealScalar(1) ); // (H)
	cvSetIdentity( m_pKalmanFilter->process_noise_cov, cvRealScalar(1e-5) ); // (Q)
	cvSetIdentity( m_pKalmanFilter->measurement_noise_cov, cvRealScalar(1e-1) ); //(R)
	cvSetIdentity( m_pKalmanFilter->error_cov_post, cvRealScalar(1));

	// choose random initial state
	cvRand( &rng, m_pKalmanFilter->state_post );

}

//kalman destructor
KalmanFilter::~KalmanFilter()
{

	cvReleaseMat( &measurement );
	cvReleaseKalman( &m_pKalmanFilter );

}

//Initialize prediction
void KalmanFilter::predictionBegin(CvRect &rect)
{

	// Initialize kalman variables (change in x, change in y,...)
	// m_pKalmanFilter->state_post is prior state
	float width = (float)rect.width;
	float height = (float)rect.height;
	m_pKalmanFilter->state_post->data.fl[0] = (float)(rect.x + (width/2));//center x
	m_pKalmanFilter->state_post->data.fl[1] = (float)(rect.y + (height/2));//center y
	m_pKalmanFilter->state_post->data.fl[4] = (float)0; //dx
	m_pKalmanFilter->state_post->data.fl[5] = (float)0; //dy
	m_pKalmanFilter->state_post->data.fl[6] = (float)0; //dw
	m_pKalmanFilter->state_post->data.fl[2] = (float)width; //width
	m_pKalmanFilter->state_post->data.fl[3] = (float)height; //height
	m_pKalmanFilter->state_post->data.fl[7] = (float)0; //dh

}

//Predict the next position and size
CvRect KalmanFilter::predictionReport(CvRect &rect)
{

	const CvMat* prediction = cvKalmanPredict( m_pKalmanFilter, 0 );

	// adjust Kalman filter state
	float width=(float)rect.width;
	float height=(float)rect.height;
	measurement->data.fl[0]=(float)(rect.x+(width)/2);
	measurement->data.fl[1]=(float)(rect.y+(height)/2);
	measurement->data.fl[2]=width;
	measurement->data.fl[3]=height;	
	cvKalmanCorrect( m_pKalmanFilter, measurement );
	CvRect rect1;

	rect1.width=(int)m_pKalmanFilter->state_post->data.fl[2];
	rect1.height=(int)m_pKalmanFilter->state_post->data.fl[3];
	rect1.x=(int)(m_pKalmanFilter->state_post->data.fl[0]-(rect.width/2));
	rect1.y=(int)(m_pKalmanFilter->state_post->data.fl[1]-(rect.height/2));

	return rect1;

}

//----------------------------------------------------------------------------------------------------------
// Parameters for camshift
int   nHistBins = 30;                 // number of histogram bins
float rangesArr[] = {0,180};          // histogram range
int vmin = 65, vmax = 256, smin = 55; // limits for calculating hue


// File-level variables
IplImage * pHSVImg  = 0; // the input image converted to HSV color mode
IplImage * pHueImg  = 0; // the Hue channel of the HSV image
IplImage * pMask    = 0; // this image is used for masking pixels
IplImage * pProbImg = 0; // the hand probability estimates for each pixel
CvHistogram * pHist = 0; // histogram of hue in the original hand image

CvRect prevHandRect,prevHandRect2;  // location of hand in previous frame
CvBox2D *handBox;      // current hand-location estimate


int nFrames = 0;


// Declarations for internal functions
void updateHueImage(const IplImage * pImg);


//////////////////////////////////
// createTracker()
//
int createTracker(const IplImage * pImg)
{
	// Allocate the main data structures ahead of time
	float * pRanges = rangesArr;
	pHSVImg  = cvCreateImage( cvGetSize(pImg), 8, 3 );
	pHueImg  = cvCreateImage( cvGetSize(pImg), 8, 1 );
	pMask    = cvCreateImage( cvGetSize(pImg), 8, 1 );
	pProbImg = cvCreateImage( cvGetSize(pImg), 8, 1 );

	pHist = cvCreateHist( 1, &nHistBins, CV_HIST_ARRAY, &pRanges, 1 );

	return 1;
}

// releaseTracker()
//
void releaseTracker()
{
	// Release all tracker resources
	cvReleaseImage( &pHSVImg );
	cvReleaseImage( &pHueImg );
	cvReleaseImage( &pMask );
	cvReleaseImage( &pProbImg );

	cvReleaseHist( &pHist );
}

// startTracking()
//
void startTracking(IplImage * pImg, CvRect pHandRect,KalmanFilter &kfilter)
{
	float maxVal = 0.f;

	// Make sure internal data structures have been allocated
	if( !pHist ) createTracker(pImg);

	// Create a new hue image
	updateHueImage(pImg);

    if(!((pHandRect.x<0)||(pHandRect.y<0)||((pHandRect.x+pHandRect.width)>pImg->width)||((pHandRect.y+pHandRect.height)>pImg->height))) {


	// Create a histogram representation for the hand
    cvSetImageROI( pHueImg, pHandRect );
    cvSetImageROI( pMask,   pHandRect );
    cvCalcHist( &pHueImg, pHist, 0, pMask );
    cvGetMinMaxHistValue( pHist, 0, &maxVal, 0, 0 );
    cvConvertScale( pHist->bins, pHist->bins, maxVal? 255.0/maxVal : 0, 0 );
    cvResetImageROI( pHueImg );
    cvResetImageROI( pMask );

	}
	// Store the previous hand location
	prevHandRect =pHandRect;
	prevHandRect2 =pHandRect;

	//Pass the hand location to kalman initializer
	kfilter.predictionBegin(prevHandRect);
	
}


//////////////////////////////////
// track()
//
CvRect camshift_track(IplImage * pImg)
{
	CvConnectedComp components;

	// Create a new hue image
	updateHueImage(pImg);

	// Create a probability image based on the hand histogram
	cvCalcBackProject( &pHueImg, pProbImg, pHist );
    cvAnd( pProbImg, pMask, pProbImg, 0 );
	//cvSetImageROI(pProbImg,predrect);
	
	// Use CamShift to find the center of the new hand probability
    cvCamShift( pProbImg, prevHandRect2, cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),&components, handBox );

	// Update hand location and angle
    prevHandRect2 = components.rect;
	//if(!pImg->origin)	
	//	handBox->angle = -handBox->angle;
	//cvResetImageROI(pProbImg);
	return prevHandRect2;
}

CvRect combi_track(IplImage * pImg,KalmanFilter &kfilter)
{
	CvRect predrect=kfilter.predictionReport(prevHandRect);	
	//if((predrect.x<0)||(predrect.y<0)||((predrect.x+predrect.width)>pImg->width)||((predrect.y+predrect.height)>pImg->height))
	//	return NULL;	
	CvConnectedComp components;

	// Create a new hue image
	updateHueImage(pImg);

	// Create a probability image based on the hand histogram
	cvCalcBackProject( &pHueImg, pProbImg, pHist );
    cvAnd( pProbImg, pMask, pProbImg, 0 );
	//cvSetImageROI(pProbImg,predrect);
	
	// Use CamShift to find the center of the new hand probability
    if(!((predrect.x<0)||(predrect.y<0)||((predrect.x+predrect.width)>pImg->width)||((predrect.y+predrect.height)>pImg->height))) {
        cvCamShift( pProbImg, predrect, cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),&components, handBox );
	// Update hand location and angle
    prevHandRect = components.rect;

    }
    else
        //cvCamShift( pProbImg, prevHandRect, cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),&components, handBox );
		prevHandRect.x=-1;
	
    //if(!pImg->origin)	
	//	handBox->angle = -handBox->angle;
	//cvResetImageROI(pProbImg);
	
	return prevHandRect;

}


//////////////////////////////////
// updateHueImage()
//
void updateHueImage(const IplImage * pImg)
{

	// Convert to HSV color model
	cvCvtColor( pImg, pHSVImg, CV_BGR2HSV );

	// Mask out-of-range values
	cvInRangeS( pHSVImg, cvScalar(0, smin, MIN(vmin,vmax), 0),
	            cvScalar(180, 256, MAX(vmin,vmax) ,0), pMask );

	// Extract the hue channel
	cvSplit( pHSVImg, pHueImg, 0, 0, 0 );

}


//////////////////////////////////
// setVmin()
//
void setVmin(int _vmin)
{ vmin = _vmin; }


//////////////////////////////////
// setSmin()
//
void setSmin(int _smin)
{ smin = _smin; }


