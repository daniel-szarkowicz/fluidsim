
#define PI 3.1415926535

// #define sigma_1D (4 / (3*h))
// #define sigma_2D (40 / (7*PI*h*h))
// #define sigma_3D (8 / (PI*h*h*h))
//
// float kernel(float distance) {
//     float h = G.smoothing_radius;
//     float sigma = sigma_2D;
//     float q = distance / h;
//     float val = 0;
//     if (0 <= q && q <= 0.5) {
//         val = 6*(q*q*q - q*q) + 1;
//     } else if (0.5 < q && q <= 1) {
//         val = 2*(1-q)*(1-q)*(1-q);
//     } // else val = 0
//     return sigma * val;
// }
//
// float kernel_derived(float distance) {
//     float h = G.smoothing_radius;
//     float sigma = sigma_2D;
//     float q = distance / h;
//     float val = 0;
//     if (0 <= q && q <= 0.5) {
//         val = 6 * (3*q*q - 2*q);
//     } else if (0.5 <= q && q <= 1) {
//         val = 2 * -3*(1-q)*(1-q);
//     } // else val = 0
//     return sigma * val;
// }

#define h G.smoothing_radius
#define volume_2D (PI * h*h*h*h / 6)
#define volume_3D (2 * PI * h*h*h*h*h / 15)
#define volume volume_2D

float kernel(float distance) {
    float val = 0;
    if (0 <= distance && distance <= h) {
        val = (h - distance) * (h - distance);
    } // else val = 0
    return val / volume;
}

float kernel_derived(float distance) {
    float val = 0;
    if (0 <= distance && distance <= h) {
        val = -2 * (h - distance);
    } // else val = 0
    return val / volume;
}


float viscosity_kernel(float distance){
    float val = 0;
    if(0 <= distance && distance <= h){
        //Linear kernel based on paper 
        //http://www.ligum.umontreal.ca/Clavet-2005-PVFS/pvfs.pdf
        val = 1 - distance/h;
    }
    return val;
}