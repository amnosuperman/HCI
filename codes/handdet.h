#ifndef HANDDET_H
#define HANDDET_H

int      initHandDet(const char * haarCascadePath);
void     closeHandDet();
CvRect*  detectHand(IplImage * pImg);

#endif
