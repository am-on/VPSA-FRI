int getOriginalPixel(
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

int getBufferedPixel(
	__local int *buffer,
	int buffer_width,
	int id,
	int y,
	int x
) {
	int loc = id + buffer_width + buffer_width * y + x;

	if (loc >= buffer_width * 4 || loc < 0)
		return 0;

	return buffer[loc];
}


__kernel void sobel_filter(
	__global const int *image,
	__global int *out,
	int width,
	int height
) {
	// globalni indeks elementa
	int id = get_global_id(0);
	int local_id = get_local_id(0) + 1;

	__local int buffer[774];
	int buffer_width = 256 + 2;

	int Gx, Gy;
	int tempPixel;
	int y = (int)(id / width);
	int x = (int)(id % width);

	bool in_new_line = (int)((id-1) / width) != y;
	bool last_in_line = (int)((id+1) / width) != y;

	// izracun
	while (y < height) {

		buffer[0 + local_id] = getOriginalPixel(image, width, height, y-1, x);
		buffer[buffer_width + local_id] = getOriginalPixel(image, width, height, y, x);
		buffer[buffer_width * 2 + local_id] = getOriginalPixel(image, width, height, y+1, x);

		if (local_id == 1) {
			// up left
			buffer[0] = getOriginalPixel(image, width, height, y-1, x-1);

			// left
			buffer[buffer_width] = getOriginalPixel(image, width, height, y, x);

			// down left
			buffer[buffer_width * 2] = getOriginalPixel(image, width, height, y+1, x-1);
		}

		if (local_id == 256) {
			// up right
			buffer[buffer_width - 1] = getOriginalPixel(image, width, height, y-1, x+1);

			// right
			buffer[buffer_width * 2 - 1] = getOriginalPixel(image, width, height, y, x+1);

			// down right
			buffer[buffer_width * 3 - 1] = getOriginalPixel(image, width, height, y+1, x+1);
		}

		barrier(CLK_LOCAL_MEM_FENCE);

		if(in_new_line) {
			Gx = -2 * getBufferedPixel(buffer, buffer_width, local_id, -1, 0)
				-getBufferedPixel(buffer, buffer_width, local_id, -1, 1)
				+2 * getBufferedPixel(buffer, buffer_width, local_id, 1, 0)
				+getBufferedPixel(buffer, buffer_width, local_id, 1, 1);

			Gy = getBufferedPixel(buffer, buffer_width, local_id, -1, 1)
				+2 * getBufferedPixel(buffer, buffer_width, local_id, 0, 1)
				+getBufferedPixel(buffer, buffer_width, local_id, 1, 1);
		} else if(last_in_line) {
			Gx = -getBufferedPixel(buffer, buffer_width, local_id, -1, -1)
				-2 * getBufferedPixel(buffer, buffer_width, local_id, -1, 0)
				+getBufferedPixel(buffer, buffer_width, local_id, 1, -1)
				+2 * getBufferedPixel(buffer, buffer_width, local_id, 1, 0);

			Gy = -getBufferedPixel(buffer, buffer_width, local_id, -1, -1)
				-2 * getBufferedPixel(buffer, buffer_width, local_id, 0, -1)
				-getBufferedPixel(buffer, buffer_width, local_id, 1, -1);
		} else {
			Gx = -getBufferedPixel(buffer, buffer_width, local_id, -1, -1)
				-2 * getBufferedPixel(buffer, buffer_width, local_id, -1, 0)
				-getBufferedPixel(buffer, buffer_width, local_id, -1, 1)
				+getBufferedPixel(buffer, buffer_width, local_id, 1, -1)
				+2 * getBufferedPixel(buffer, buffer_width, local_id, 1, 0)
				+getBufferedPixel(buffer, buffer_width, local_id, 1, 1);

			Gy = -getBufferedPixel(buffer, buffer_width, local_id, -1, -1)
				-2 * getBufferedPixel(buffer, buffer_width, local_id, 0, -1)
				-getBufferedPixel(buffer, buffer_width, local_id, 1, -1)
				+getBufferedPixel(buffer, buffer_width, local_id, -1, 1)
				+2 * getBufferedPixel(buffer, buffer_width, local_id, 0, 1)
				+getBufferedPixel(buffer, buffer_width, local_id, 1, 1);
		}

		tempPixel = sqrt((float)(Gx*Gx+Gy*Gy));

		if(tempPixel > 255)
			out[id] = 255;
		else
			out[id] = tempPixel;

		id += get_global_size(0);
		y = (int)(id / width);
		x = (int)(id % width);
	}
}
