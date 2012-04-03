#include "cv.h"
#include "highgui.h"
#include "cvaux.h"
#include "time.h"
#include "unistd.h"

#ifndef TRACKER_H
#define TRACKER_H

//Kalman filter class
class KalmanFilter
{
	public:
	KalmanFilter();
	~KalmanFilter();
	
	virtual void predictionBegin(CvRect &rect);
	virtual CvRect predictionReport(CvRect &rect);
	CvKalman *m_pKalmanFilter;
	CvRandState rng;
	CvMat* measurement;

};

// Main Tracking Control functions
int     createTracker(const IplImage * pImg);
void    releaseTracker();
void    startTracking(IplImage * pImg, CvRect pHandRect,KalmanFilter &kfilter);
CvBox2D* track(IplImage * pImg,KalmanFilter &kfilter);


// Parameter settings
void setVmin(int vmin);
void setSmin(int smin);

#endif
