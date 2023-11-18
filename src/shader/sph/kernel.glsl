
#define PI 3.1415926535

#define sigma_1D (4 / (3*h))
#define sigma_2D (40 / (7*PI*h*h))
#define sigma_3D (8 / (PI*h*h*h))

float kernel(float distance) {
    float h = G.smoothing_radius;
    float sigma = sigma_2D;
    float q = distance / h;
    float val = 0;
    if (0 <= q && q <= 0.5) {
        val = 6*(q*q*q - q*q) + 1;
    } else if (0.5 < q && q <= 1) {
        val = 2*(1-q)*(1-q)*(1-q);
    } // else val = 0
    return sigma * val;
}
