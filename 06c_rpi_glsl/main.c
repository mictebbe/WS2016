#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#include "render.h"



int main() {


	render_init();
	set_color (1.0, 0.0, 0.0, 1.0);
	int i, j;

	int N = 5;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			float x = -1.0 + i * 2.0/N;
			float y = -1.0 + j * 2.0/N;
			add_quad (x, y, 1.0/N, 1.0/N);
		}
	}

	int run = 1;

	struct timespec ts;
	clock_gettime (CLOCK_REALTIME, &ts);
	long tn;
	float dt;

	int cnt = 0;

	while(run) {

		if (cnt++==100)
			init_shaders ("vertex.glsl", "fragment2.glsl");

		tn = ts.tv_nsec;
		clock_gettime (CLOCK_REALTIME, &ts);
		dt = (ts.tv_nsec - tn) / 10000000.0;

		dt = 25.0;
		//printf("%g\n", dt);
		render_update (dt);
	}

	return 0;
}


