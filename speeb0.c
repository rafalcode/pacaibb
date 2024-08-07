/* speeb0.c SPEEch Bubble 0!
 * takes a JPG and put a ubble in it 
 * according to points you defin ein a text file */
#include<stdio.h>
#include<math.h>
#include<pango/pangocairo.h>
#include<cairo_jpg.h>

#define DBG
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

#define IMW 800
#define IMH 600
#define LWD 3 // line width
#define FONTDEF "FS Albert 24"
#define THROWAWAYMEMSZ 128

typedef struct /* rpla_t: rectanglemetn placement type xy and top left pos, w and h width and height. */
{
    float x,y;
    float w,h;
    float lm, rm; // left marg and right margin
} rpla_t;

typedef struct /* larr_t line array struct */
{
    char **l;
    size_t *lz;
    int lbf;
    int asz;
} larr_t;

char *genofn(char *ifn)
{
    char *outname=calloc(THROWAWAYMEMSZ, sizeof(char));
    char *dot=strchr(ifn, '.');
    sprintf(outname, "%.*s_speeb.png", (int)(dot-ifn), ifn);
    return outname;
}

larr_t *slurplines(char *fn)
{
    FILE *stream;
    // char *line = NULL;
    size_t len = 0;
    int i;

    stream = fopen(fn, "r");
    if (stream == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    larr_t *la=malloc(sizeof(larr_t));
    la->lbf=GBUF;
    la->asz=0;
    la->l=malloc(la->lbf*sizeof(char*)); // does need this, yes, though later it won't be free'd
    la->lz=malloc(la->lbf*sizeof(size_t)); // does need this, yes, though later it won't be free'd
    for(i=0;i<la->lbf;++i) {
        la->lz[i]=0;
        la->l[i]=NULL;
    }

    while ((la->lz[la->asz] = getline(la->l+la->asz, &len, stream)) != -1) {
        // nread is number of returned characters
        CONDREALLOC(la->asz, la->lbf, GBUF, la->l, char*, la->lz, size_t);
        // printf("Retrieved line of length (returned val method): %zu\n", nread);
        // printf("Retrieved line of length (strlen method): %zu\n", strlen(line));
        // fwrite(line, nread, 1, stdout);
        if(la->l[la->asz][la->lz[la->asz]-1]=='\n') // check the newline
            la->l[la->asz][la->lz[la->asz]-1]='\0'; // smash the newline, of course you could save a byte and reallocate, etc. etc.
        la->asz++;
    }

    /* now to see if normalise works */
    for(i=la->asz;i<la->lbf;++i)
        free(la->l[i]);
    la->l=realloc(la->l, la->asz*sizeof(char*)); // normalize
    la->lz=realloc(la->lz, la->asz*sizeof(size_t)); // normalize

    /* Ok that's it */
    fclose(stream);
    return la;
}

void lay0(cairo_t *cr, rpla_t r, char *thetext)
{
    /* OK, over to pango, first creat out layout, it's linked to the cairo context context */
	PangoLayout *playout = pango_cairo_create_layout(cr);

    /* select font */
	PangoFontDescription *desc= pango_font_description_from_string(FONTDEF);
    if(desc == NULL) {
	    printf("Failed: pango_font_description_from_string() came back NULL\n");
        exit(EXIT_FAILURE);
    }
	pango_layout_set_font_description(playout, desc);

    size_t tsz = strlen(thetext);
    char *parstr=calloc(tsz, sizeof(char)); // the actual text to show.

    PangoAttrList *attlst;
    pango_parse_markup(thetext, -1, '\0', &attlst, &parstr, NULL, NULL);

	/* this next one important, the -1 is for free run of text, no wrapping width is appliedi, the NUL termination is all that is needed. */
	pango_layout_set_text(playout, parstr, -1);
    pango_layout_set_attributes(playout, attlst);
    // pango_layout_set_line_spacing(playout, 0.5);

    printf("rect is at x:%2.2f y:%2.2f, w:%2.2f h:%2.2f\n", r.x, r.y, r.w, r.h); 

    // findout what's gone on, so far.
    PangoRectangle inkrect;
    PangoRectangle logirect;
    pango_layout_get_pixel_extents(playout, &inkrect, &logirect);
    int bl = pango_layout_get_baseline(playout);
    int ourscale;
    printf("baseline=%i\n", bl); 
#ifdef DBG
    printf("inkrect(pangus) x:%i/y:%i/w:%i/h:%i\n", inkrect.x, inkrect.y, inkrect.width, inkrect.height); 
    printf("logirect(pangus) x:%i/y:%i/w:%i/h:%i\n", logirect.x, logirect.y, logirect.width, logirect.height); 
#endif
    float hspa=r.w-r.rm-r.lm-(float)logirect.width;
    float vspa=r.h-(float)logirect.height;
    printf("hspa and vspa for the text without wrapping:\n"); 
    printf("hspa:%2.2f;vpsa:%2.2f\n", hspa, vspa); 

    if(hspa <.0) {
#ifdef DBG
        printf("layout does not fit horizontally.\n"); 
#endif
        printf("pang_scale=%i\n", PANGO_SCALE); // it's 1024
        ourscale=(int)(r.w-r.rm-r.lm)*PANGO_SCALE;
        pango_layout_set_width(playout, (int)(r.w-r.rm-r.lm)*PANGO_SCALE); // very bizarre integer hereis like in the 100K's! How can that be a width?
        printf("Using pango_layout_set_width() to set layout to width (must be an int) %i\n", ourscale);
        pango_layout_get_pixel_extents(playout, &inkrect, &logirect);
        printf("new inkrect(pangus) x:%i/y:%i/w:%i/h:%i\n", inkrect.x, inkrect.y, inkrect.width, inkrect.height); 
        printf("new logirect(pangus) x:%i/y:%i/w:%i/h:%i\n", logirect.x, logirect.y, logirect.width, logirect.height); 
        hspa=.0;
        vspa=r.h-(float)logirect.height;
    }

    // check for vert space overshoot. You have to look at reducing font size for this though.
    if(vspa <.0)
        printf("layout does not fit vertically.\n"); 
#ifdef DBG
    printf("Space after placement =x:%2.2f y:%2.2f\n", hspa, vspa); 
#endif

    // cairo_move_to (cr, fiddle_lmar+r.x+hspa/2., r.y+vspa/2.);
    // cairo_move_to (cr, r.lm+r.x+hspa/2., ystartpos);
    cairo_move_to (cr, r.lm+r.x, r.y);
    printf("cr move to %2.2f, %2.2f\n", r.x+r.lm, r.y);
	pango_cairo_update_layout(cr, playout); /* if the target surface or transformation properties of the cairo instance have changed, update the pango layout to reflect this */

	pango_cairo_layout_path(cr, playout); // draw the pango layout onto the cairo surface
	g_object_unref(playout);	// free the layout
    free(parstr);
	pango_font_description_free(desc);
}

int main(int argc, char* argv[])
{
    if(argc!=2) {
        printf("Please give 1 argument: a (raw) configuration file with 8 lines as follows:\n");
        printf("1) name of JPG file on which textrect will go\n");
        printf("2) xcoord of originating mouth\n");
        printf("3) ycoord of originating mouth\n");
        printf("4) xcoord of topleft edge textrect\n");
        printf("5) ycoord of topleft edge textrect\n");
        printf("6) width of textrect\n");
        printf("7) height of textrect\n");
        printf("8) Text which you want to appear\n");
        exit(EXIT_FAILURE);
    }
    int i;

    // read in conf .. decides everything
    larr_t *la=slurplines(argv[1]);

    cairo_status_t status; // variable to hold the various states of the cairo lib
    cairo_surface_t *surf0 = cairo_image_surface_create_from_jpeg(la->l[0]);
    int w = cairo_image_surface_get_width (surf0);
    int h = cairo_image_surface_get_height (surf0);
    printf("Input JPG - w=%i,h=%i\n", w, h); 
    cairo_t *cr = cairo_create(surf0);	// tell the cairo instance 'cr' to output to 'surface'

    /* here is our text rect */
    /* We're only going to split x, the horiz axis, up. We will use the canvas' y or vertspace */
    rpla_t r0;
    r0.x=atoi(la->l[3]);
    r0.y=atoi(la->l[4]);
    r0.w=atoi(la->l[5]);
    r0.h=atoi(la->l[6]);
    r0.lm = 10;
    r0.rm =0;

   /* Rect 0: bg */
   // cairo_rectangle (cr, r0.x, r0.y, r0.w, r0.h);
   // cairo_set_source_rgba(cr, .8, .8, .4, .5);
   // cairo_set_source_rgba(cr, 1, 1, 1, 1);
   // cairo_fill(cr);

   /* from top right to mouth-xy */
   float mx = atoi(la->l[1]);
   float my = atoi(la->l[2]);
   cairo_move_to(cr, mx, my);
   cairo_line_to(cr, r0.x+r0.w*0.9, r0.y);
   cairo_line_to(cr, r0.x, r0.y); //across left
   cairo_line_to(cr, r0.x, r0.y+r0.h); //down
   cairo_line_to(cr, r0.x+r0.w, r0.y+r0.h); //across right
   cairo_line_to(cr, r0.x+r0.w, r0.y+r0.h*0.1); //up
   cairo_line_to(cr, mx, my); // back to mouth
   cairo_close_path(cr);
   cairo_set_source_rgb(cr, 0, 0, 0);
   cairo_stroke_preserve(cr);
   cairo_set_source_rgb(cr, 1, 1, 1);
   cairo_fill(cr);

   /* Rect 0: txt */
   cairo_set_source_rgb(cr, 0, 0, 0);
   lay0(cr, r0, la->l[7]);
   cairo_fill(cr);

   cairo_destroy(cr);	// free the cairo instance
   char *outn=genofn(la->l[0]);
   status = cairo_surface_write_to_png(surf0, outn);

    cairo_surface_destroy(surf0);	// free the surface

    /* and now to free line array */
    for(i=0;i<la->asz;++i)
        free(la->l[i]);
    free(la->l);
    free(la->lz);
    free(la);
    free(outn);

    return 0;
}
