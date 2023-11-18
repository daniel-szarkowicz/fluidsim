uniform float kernel_radius = 0.5f;

#define PI 3.1415926535
float kernel(float distance) {
    float sigma = 8 / (PI*kernel_radius*kernel_radius*kernel_radius);
    float q = distance/kernel_radius;
    float val = 0;
    if (0 <= q && q <= 0.5) {
        val = 6*(q*q*q - q*q) + 1;
    } else if (0.5 < q && q <= 1) {
        val = 2*(1-q)*(1-q)*(1-q);
    } // else val = 0
    return sigma * val;
}
