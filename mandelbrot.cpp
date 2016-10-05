#include "mandelbrot.h"
#include <math.h>

#define MAXITER 500

int getDistance(double x, double y){
   int iterations = 1;
   double d = sqrt(x * x + y * y);
   double zx2 = 0, zy2 = 0, zx1, zy1;

   while (iterations < MAXITER && d <= 2)
   {
      zx1 = zx2;
      zy1 = zy2;
      zx2 = zx1 * zx1 - zy1 * zy1 + x;
      zy2 = 2 * zx1 * zy1 + y;
      d = sqrt(zx2 * zx2 + zy2 * zy2);
      iterations++;
   }
	return iterations;
}
