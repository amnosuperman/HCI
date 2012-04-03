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

#ifndef FACEDETECT_H
#define FACEDETECT_H

int initface();
void releaseface();

void detect_and_draw( IplImage* img );

#endif


