// using chagex01.c to generate arrows with textinside them
#include <cairo.h>
#include <pango/pangocairo.h>

#define GBUF 4
#define CONDREALLOC(x, b, c, a, t, a2, t2); \
    if((x)>=((b)-1)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(t)); \
        (a2)=realloc((a2), (b)*sizeof(t2)); \
        for(i=((b)-(c));i<(b);++i) { \
            ((a)[i]) = NULL; \
            ((a2)[i]) = 0; \
        } \
    }

#define CHOSENFONT "Sans Bold 28"
#define LWID 3

// short strings expressing outrage
#define OUTR0 "Outrage!\n"
#define OUTR1 "Look\nat this!\n"
#define OUTR2 "WTF!\n"
#define OUTR3 "Stupefaction!\n"
#define OUTR4 "Can you believe this is going on?\n"
#define OUTR5 "This guy's a nutter!\n"

const char *s[6]={"Outrage!\n",
                  "Look\nat this!\n",
                  "WTF!\n", 
                  "Stupefaction!\n",
                  "Can you believe this is going on?\n",
                  "This guy's a nutter!\n"};

typedef struct /*pos_t position type*/
{
    float x,y;
} pos_t;

typedef struct /* seg_t line segment type: bx, beginning x, by, ex (endx), ey */
{
    float bx, by, ex, ey;
} seg_t;

typedef struct /*col_t colour type*/
{
    float r,g,b,a; // must be 0 to 1
} col_t;

void draw0(cairo_t *cr, seg_t *m, float ysep1, float ysep2, float vprat, float agfac, col_t *ocol, col_t *icol) // agfac is aggression factor for arrow head. 1.05 normal
{
    // block arrow modelled on middle line segment represented by m.
    // There are 2 notional parallel lines above it, the first is th upper part of the thin stem to the arrowhead.
    // the second holds the extent of the upp er "wing" of the arrowhead.
    cairo_set_line_width (cr, LWID);

    double dx = m->ex - m->bx;

    // we start with the upper line segment.
    seg_t u1={.bx=m->bx,
              .by=m->by-ysep1,
              .ex=m->bx+dx*vprat*agfac, // here we extend the x on the right direction (slope) by a certain percentage
              .ey=m->by-ysep1}; // same a x, but multiplying by slope as y moves in same direction as x this time.
    cairo_move_to (cr, u1.bx, u1.by);
    cairo_line_to (cr, u1.ex, u1.ey);

    // u2 need only be a pos_t, as point is currently at u1.ex and u1.ey, this will be on the second/outer notional parallel line.
    pos_t u2={.x=m->bx+dx*vprat,
              .y=m->by-ysep1-ysep2};
    cairo_line_to (cr, u2.x, u2.y);
    // now we go the core point exention of arrow.
    cairo_line_to (cr, m->ex, m->ey);

    // we go back the other way now: l2 is the equivlent of u2 on the other side
    pos_t l2={.x=m->bx+dx*vprat,
              .y=m->by+ysep1+ysep2};
    cairo_line_to(cr, l2.x, l2.y);

    seg_t l1={.bx=m->bx+dx*vprat*agfac,
              .by=m->by+ysep1,
              .ex=m->bx,
              .ey=m->by+ysep1};

    cairo_line_to (cr, l1.bx, l1.by);
    cairo_line_to (cr, l1.ex, l1.ey);
    cairo_close_path(cr);
    // cairo_set_source_rgba(cr, ocol->r, ocol->g, ocol->b, ocol->a);
    cairo_set_source_rgb(cr, ocol->r, ocol->g, ocol->b);
    cairo_stroke_preserve (cr);
    cairo_set_source_rgba(cr, icol->r, icol->g, icol->b, icol->a);
    cairo_fill(cr);

    return;
}

int main(int argc, char *argv[])
{
    if(argc!=2) {
        printf("Please give 1 argument: the index of the string you want outputted\n");
        printf("Yes, that does mean soon a tthe source code first, sure.\n");
        exit(EXIT_FAILURE);
    }
    int idx=atoi(argv[1]);

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 800, 600);
    cairo_t *cr = cairo_create(surface);

    // there could be mark up
    size_t strsz=strlen(s[idx]);
    char *parstr=calloc(2*strsz, sizeof(char));
    PangoAttrList *attlst=NULL;
    pango_parse_markup(s[idx], -1, '\0', &attlst, &parstr, NULL, NULL);

    // Get the layout going. Don't foget Layout is the pango magick which will wrap into a paragraph
    PangoLayout *layout = pango_cairo_create_layout(cr);
    pango_layout_set_text(layout, parstr, -1);

    // Set font description
    PangoFontDescription *font_desc = pango_font_description_from_string(CHOSENFONT);
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free(font_desc);

    // tweak line spacing
    pango_layout_set_line_spacing(layout, 0.75);    // also squeeze
    pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);

    // findout what's gone on, so far.
    PangoRectangle inkrect;
    PangoRectangle logirect; // just needed for extents function.
    pango_layout_get_pixel_extents(layout, &inkrect, &logirect);
    printf("new inkrect(pangus) x:%i/y:%i/w:%i/h:%i\n", inkrect.x, inkrect.y, inkrect.width, inkrect.height); 

    // Draw the text: where to start it?
    float xstart=50, ystart=50;
    cairo_move_to(cr, xstart, ystart);
    pango_cairo_show_layout(cr, layout);

    // use this data to draw a rect.
    cairo_rectangle (cr, xstart, ystart, inkrect.width, inkrect.height-inkrect.y); // height goes over a bit some 20pixels.
    cairo_set_source_rgba(cr, .5, .6, .7, .3);
    cairo_fill(cr);

    // get arrow ready
    float tlph=inkrect.width *1.2; // text line plus arrowhead
    float xnudgeback=3; // we need to undge slightly back xwise on the text otherwise it's too crammed.
    float ymidline=(inkrect.height-inkrect.y)/2.; // convenience variable
    seg_t m= {.bx=xstart-xnudgeback, .by=ystart+ymidline, .ex=xstart+tlph, .ey=ystart+ymidline};
    col_t ocol={.r=.0, .g=.0, .b=.0, .a=1.0}; //outline
    col_t icol={.r=.2, .g=.2, .b=.3, .a=.35}; //outline
    // these hardcoded numbers are fairly vital
    draw0(cr, &m, ymidline, 15., .8, 1.03, &ocol, &icol);

    // Save the output
    cairo_surface_write_to_png(surface, "outrarr0.png");

    // Clean up
    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    free(parstr);

    return 0;
}
