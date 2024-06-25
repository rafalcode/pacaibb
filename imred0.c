/* FOcus on genertaing a PNG of the right size for an A4 pdf */
#include <cairo/cairo.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// #define WPDF 842
// #define HPDF 595
#define WPDF 800
#define HPDF 600
#define IRF 0.5 // image reduction section fraction: x fraction of 
#define LWI 0 //landscape width index (0 or 1)
#define LHI 1
#define PWI LHI
#define PHI LWI
#define OPX 400 // orig point of interest, x coord
#define OPY 300 // orig point of interest, y coord
#define LST 10

int main (int argc, char *argv[])
{
    
    if(argc!=4) {
        printf("Please give 3 arguments: 1) input image file (must be PNG) 2) xcoord point of interest 3) ycoord POI\n");
        exit(EXIT_FAILURE);
    }
    float xpoi=atof(argv[2]);
    float ypoi=atof(argv[3]);

    int lwh[2]={WPDF, HPDF}; // lwh is for landscape width height. i.e. lwh[0] is width,
    // so, when you see lwh[0] being used for height, you should conclude that portrait is being used.

    // unfort, no such thang:
    // image = cairo_image_surface_create_from_jpg("marspopm.jpg");
    // so you must convert to png ...

    // our surf0, this will be used as a source.
    cairo_surface_t *surf0 = cairo_image_surface_create_from_png(argv[1]);
    int w = cairo_image_surface_get_width (surf0);
    int h = cairo_image_surface_get_height (surf0);
    printf("w=%i,h=%i\n", w, h); 

    // now we split the space into an reudced image part on left and an annotation on right. I.e. it's awidth operation.
    float iredfrac=IRF; 
    float annfrac = 1.0 -IRF; 
    printf("Amount of times true image width exceeds iredfrac*pdfwidth:\n"); 
    float wfac= (float)w/(lwh[LWI]*iredfrac);
    printf("wfac=%2.2f\n", wfac);

    float cxy[2];
    // cxy[0]=lwh[LWI]*iredfrac;
    // cxy[1]=h*1./wfac;
    cxy[0]=lwh[LWI]*iredfrac;
    cxy[1]=h*1./wfac;
    float cwh[2];
    cwh[0]=lwh[LWI]*(1-iredfrac);
    cwh[1]=lwh[LHI]-cxy[1];
    float opx=xpoi-cwh[0];
    float opy=ypoi-cwh[1];
    printf("xpoi=%2.2f ypoi=%2.2f opx=%2.2f opy=%2.2f\n", xpoi, ypoi, opx, opy);
    printf("Clip in which to insert unreduced image is at x=%2.2f y=%2.2f w=%2.2f h=%2.2f\n", cxy[0], cxy[1], cwh[0], cxy[1]);

    // that we we calculate
    float wsh=1./wfac; // width shrink
    float hsh=wsh; // height shrink, yep the same as width

    cairo_surface_t *surf2 = cairo_image_surface_create(CAIRO_FORMAT_RGB24, lwh[LWI], lwh[LHI]); // landscape A4 I think
    cairo_t *cr = cairo_create(surf2);

    // set background colour of whole output image
    cairo_rectangle (cr, 0, 0, lwh[LWI], lwh[LHI]); /* arg explan: topleftcorner and size of shape  */
    cairo_set_source_rgba (cr, .75, .75, .75, 1); /*  final number is alpha, 1.0 is opaque */
    cairo_fill (cr);

    // cairo_scale(cr, 1./wfac, 1./wfac);
    printf("wsh=%2.2f hsh=%2.2f\n", wsh, hsh);
    cairo_scale(cr, wsh, hsh);
    cairo_set_source_surface (cr, surf0, 0, 0);
    cairo_paint(cr);

    cairo_destroy(cr);

    // now new one, so that scale is forgotten, he expects ...
    cr = cairo_create(surf2);
    // now lower right clip:

    cairo_rectangle (cr, opx*wsh-LST, opy*hsh-LST, wsh*cwh[0]+2*LST, hsh*cwh[1]+2*LST);
    // cairo_rectangle (cr, opx*wsh*wsh, opy*hsh*hsh, wsh*cwh[0], hsh*cwh[1]);
    printf("rectoutline pos: x=%2.2f y=%2.2f\n", opx*wsh, opy*hsh);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, LST);
    cairo_stroke(cr);

    // the c lip rectangle.
    cairo_rectangle(cr, cxy[0], cxy[1], cwh[0], cwh[1]);
    cairo_clip(cr); // somehow, this means the rectangle is the only writeable part now.

    // the next coords set the x and y position in the original image, the width and height of the clip are dependent on the rectangle clip.
    // there's a gotcha here, the docs are a bit opaque, theyh say "The x and y parameters are negated"
    // and the clip is already posiioned at cxy[0] so it has to be brought back that amount and forward to the place we want the original image to start at
    // this is quite a gotcha ... you can spend a few hours owrkignout what;s going on.
    cairo_set_source_surface (cr, surf0, cxy[0]-opx, cxy[1]-opy);
    cairo_paint (cr);
    cairo_destroy (cr);

    cairo_surface_write_to_png(surf2, "imred0.png");
    cairo_surface_destroy (surf2);
    cairo_surface_destroy (surf0);

    return 0;
}
