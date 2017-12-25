
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

int getEnergy(
	__global const int *image,
	int width,
	int height,
	int y,
	int x
) {
	if(x<0 || x>=width)
		return INT_MAX;
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

__kernel void findSeam(
	__global const int *image,
	__global int *out,
	__global int *energy,
	__global int *sort_index,
	int width,
	int height
) {
	// globalni indeks elementa
	int id = get_global_id(0);
	int global_size = get_global_size(0);
	int a, b, c, index, old_y;

	int original_x = (int)(id % width);
	int x = original_x;
	int y = 0;

	// copy first row
	while(x < width) {
		energy[x] = image[x];
		x += global_size;
	}

	// reset vars
	x = original_x;

	int tMin = INT_MAX;
	int tInd = x;
	int value;

	// calculate energy for other rows
	while (y < height) {
		x = original_x;
		old_y = y;
		y++;

		barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);

		index = y*width;
		while(x < width) {
			a = min(
					getEnergy(energy, width, height, old_y, x-1),
					getEnergy(energy, width, height, old_y, x)
				);

			b = min(
					a,
					getEnergy(energy, width, height, old_y, x+1)
				);

			value = b + image[index+x];

			// find threads minimum in last row
			if(y == height) {
				if(tMin > value) {
					tMin = value;
					tInd = x;
				}
			} else {
				energy[index+x] = value;
				//out[index+x] = value; // TEMP
			}
			x += global_size;

		}
	}

	barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);

	// reset vars
	x = original_x;
	y = height - 1;

	// find minimum
	sort_index[x] = tInd;

	int working = get_global_size(0) / 2;
	while(working > 0) {
		barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);

		if(id < working) {
			a = getEnergy(energy, width, height, y, sort_index[x]);
			b = getEnergy(energy, width, height, y, sort_index[x + working]);
			sort_index[x] = (a < b) ? sort_index[x] : sort_index[x+working];
		}
		working /= 2;
	}

	x = sort_index[0];
	if(id == 0) {
		out[height-1] = x;
		for(int i=height-2; i>=0; i--) {

			int up_left = getEnergy(energy, width, height, i, x-1);
			int up = getEnergy(energy, width, height, i, x);
			int up_right = getEnergy(energy, width, height, i, x+1);

			if (up_left > up) {
				if (up_right > up) {
					x+=0;
				} else {
					x+=1;
				}
			} else {
				if (up_right > up_left) {
					x-=1;
				} else {
					x+=1;
				}
			}

			out[i] = x;

		}
	}
}

__kernel void removeSeam(
	__global const int *image,
	__global int *out,
	__global int *seam,
	int width,
	int height
) {
	// globalni indeks elementa
	int id = get_global_id(0);
	int global_size = get_global_size(0);

	int y = (int)(id / width);
	int x = (int)(id % width);

	// izracun
	if(id <= width*height) {
		if(x >= seam[y]) {
			out[id] = image[id+y+1];
		} else {
			out[id] = image[id+y];
		}

	}

}