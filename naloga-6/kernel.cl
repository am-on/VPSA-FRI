__kernel void mandelbrot(__global int *image, int width, int height, int max_gray)
{

	// globalni indeks elementa
	int id = get_global_id(0);
	int my_y = (int)(id / width);
	int my_x = (int)(id % width);

	// mandelbrot vars
	float x0,y0,x,y,xtemp;
	int i,j;
	int color;
	int iter;
	int max_iteration=1000;

	// izracun
	while (my_y < height)
	{
		x0 = (float)my_x/width*(float)3.5-(float)2.5; //zacetna vrednost
		y0 = (float)my_y/height*(float)2.0-(float)1.0;
		x = 0;
		y = 0;
		iter = 0;
		while((x*x+y*y <= 4) && (iter < max_iteration))
		{ //ponavljamo, dokler ne izpolnemo enega izmed pogojev
			xtemp = x*x-y*y+x0;
			y = 2*x*y+y0;
			x = xtemp;
			iter++;
		}
		color = (int)(iter/(float)max_iteration*(float) max_gray); //pobarvamo piksel z ustrezno barvo
		image[my_y * width + my_x] = max_gray - color;

		id += get_global_size(0);
		my_y = id / width;
		my_x = id % width;
	}
}