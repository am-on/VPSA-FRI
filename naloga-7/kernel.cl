int getPixelCPU(
	__global const int *image,
	int width,
	int height,
	int y,
	int x
) {
	if(x<0 || x>=width)
		return 0;
	if(y<0 || y>=height)
		return 0;
	return image[y*width+x];
}


__kernel void sobel_filter(
	__global const int *image,
	__global int *out,
	int width,
	int height
) {
	// globalni indeks elementa
	int id = get_global_id(0);

	int Gx, Gy;
	int tempPixel;
	int y = (int)(id / width);
	int x = (int)(id % width);

	// izracun
	while (y < height) {
		Gx = -getPixelCPU(image, width, height, y-1, x-1)
    		-2*getPixelCPU(image, width, height, y-1, x)
    		-getPixelCPU(image, width, height, y-1, x+1)
    		+getPixelCPU(image, width, height, y+1, x-1)
    		+2*getPixelCPU(image, width, height, y+1, x)
    		+getPixelCPU(image, width, height, y+1, x+1);

		Gy = -getPixelCPU(image, width, height, y-1, x-1)
		    -2*getPixelCPU(image, width, height, y, x-1)
		    -getPixelCPU(image, width, height, y+1, x-1)
		    +getPixelCPU(image, width, height, y-1, x+1)
		    +2*getPixelCPU(image, width, height, y, x+1)
		    +getPixelCPU(image, width, height, y+1, x+1);

		tempPixel = sqrt((float)(Gx*Gx+Gy*Gy));

		if(tempPixel>255)
			out[id] = 255;
		else
			out[id] = tempPixel;

		id += get_global_size(0);
		y = (int)(id / width);
		x = (int)(id % width);
	}
}