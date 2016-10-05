#ifndef MANDELBROT_H
#define MANDELBROT_H

struct MandelbrotPixel{
	double x,y;
	int distance;
};


int getDistance(double x, double y);
#endif
