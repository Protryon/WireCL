__kernel void wireworld(const __global uchar* input, __global uchar* out, const int width, const int height) {
	const int fx = get_global_id(0);
	const int y = get_global_id(1);
	for(int ax = 0; ax<4; ax++) {
		const int x = fx * 4 + ax;
		uchar oldType = 0;
		oldType = input[y * width / 4 + x / 4] & (0x03 << ((x % 4) * 2));
		oldType >>= (x % 4) * 2;
		uchar newType = 0;
		switch (oldType) {
		case 2:
			newType = 3;
			break;
		case 3:
			newType = 1;
			break;
		case 1: {
				uchar n = 0;
				int c = 1;
				for (int j = -1; j <= 1 && c; j++) {
					for (int i = -1; i <= 1 && c; i++) {
						long k = x + i;
						long l = y + j;
		
						if (k < 0 || l < 0 || k >= width || l >= height) {
							continue;
						}
						unsigned char nei = 0;
						nei = input[l * width / 4 + k / 4] & (0x03 << ((k % 4) * 2));
						nei >>= (k % 4) * 2;
						if (nei == 2) {
							n++;
							if (n > 2) {
								c = 0;
							}
						}
					}
				}
				newType = (n == 1 || n == 2) ? 2 : 1;
				break;
			}
		}
		out[y * width / 4 + x / 4] |= (newType << ((x % 4) * 2));
	}
}