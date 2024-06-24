/* vtblkz: Vertical Text Blocks (prototype was called panx.c) */
#include <stdio.h>
#include <math.h>
#include <pango/pangocairo.h>

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

typedef struct /* cf_t: colour float type */
{
    float r, g, b;
} cf_t;

typedef struct /* rpla_t: rectanglemetn placement type xy and top left pos, w and h width and height. */
{
    float x,y;
    float w,h;
} rpla_t;

typedef struct /* larr_t line array struct */
{
    char **l;
    size_t *lz;
    int lbf;
    int asz;
} larr_t;

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

void hc2f(cf_t *rgb, char *hstr) /* hex colour string to cf_t. Note: despite name it also handle decimetal rgb triples */
{
    size_t slen=strlen(hstr);
    char hxw[3]={'\0'};
    if((slen == 7) & (hstr[0] == '#')) { // it's hex then.
        hxw[0]=hstr[1]; hxw[1]=hstr[2];
        rgb->r=(float)strtol(hxw, NULL, 16)/255.;
        hxw[0]=hstr[3]; hxw[1]=hstr[4];
        rgb->g=(float)strtol(hxw, NULL, 16)/255.;
        hxw[0]=hstr[5]; hxw[1]=hstr[6];
        rgb->b=(float)strtol(hxw, NULL, 16)/255.;
        return;
    }

    char t=',';
	char *tk=strtok(hstr, &t);
	if(!tk) {
		printf("It appears this colour string is neither a hex nor a comma delimited rgb string.\n"); 
        exit(EXIT_FAILURE);
	}
    rgb->r=atof(tk)/255.;

	tk=strtok(NULL, &t);
    rgb->g=atof(tk)/255.;

	tk=strtok(NULL, &t);
    rgb->b=atof(tk)/255.;

    return;
}

void lay0(cairo_t *cr, rpla_t r, larr_t *la, int stidxf /* starting index in file */)
{
    /* OK, over to pango, first creat out layout, it's linked to the cairo context context */
	PangoLayout *playout = pango_cairo_create_layout(cr);
    float fiddle_lmar=0; // fiddle left margin ... manual setting, when text is longer than rect and you know wrapping will occur. If not, should be set to zero

    /* select font */
	PangoFontDescription *desc= pango_font_description_from_string(la->l[stidxf+3]);
    if(desc == NULL) {
	    printf("Failed: pango_font_description_from_string() came back NULL\n");
        exit(EXIT_FAILURE);
    }
	pango_layout_set_font_description(playout, desc);
	pango_font_description_free(desc);

    char *parstr=calloc(1+la->lz[stidxf+4], sizeof(char));
    // char *parstr;
    PangoAttrList *attlst;
    pango_parse_markup(la->l[stidxf+4], -1, '\0', &attlst, &parstr, NULL, NULL);
	pango_layout_set_text(playout, parstr, -1); /* sets the text to be associated with the layout (final arg is length, -1 to calculate automatically when passing a nul-terminated string) */
    pango_layout_set_attributes(playout, attlst);
    // pango_layout_set_line_spacing(playout, 0.5);

#ifdef DBG
    printf("Rect_%i) x:%2.2f y:%2.2f, w:%2.2f h:%2.2f\n", stidxf/4, r.x, r.y, r.w, r.h); 
#endif
    // findout what's gone on.
    PangoRectangle inkrect;
    PangoRectangle logirect;
    pango_layout_get_pixel_extents(playout, &inkrect, &logirect);
#ifdef DBG
    printf("inkrect(pangus) x:%i/y:%i/w:%i/h:%i\n", inkrect.x, inkrect.y, inkrect.width, inkrect.height); 
    printf("logirect(pangus) x:%i/y:%i/w:%i/h:%i\n", logirect.x, logirect.y, logirect.width, logirect.height); 
#endif
    float hspa=r.w-(float)logirect.width;
    float vspa=r.h-(float)logirect.height;
    printf("hspa:%2.2f;vpsa:%2.2f\n", hspa, vspa); 
    if(hspa <.0) {
#ifdef DBG
        printf("layout does not fit horizontally.\n"); 
#endif
        pango_layout_set_width(playout, (int)r.w*PANGO_SCALE);
        pango_layout_get_pixel_extents(playout, &inkrect, &logirect);
        printf("new logirect(pangus) x:%i/y:%i/w:%i/h:%i\n", logirect.x, logirect.y, logirect.width, logirect.height); 
        fiddle_lmar=5; // fiddle left margin ... manual setting
        hspa=.0;
        vspa=r.h-(float)logirect.height;
    }
    if(vspa <.0)
        printf("layout does not fit vertically.\n"); 
#ifdef DBG
    printf("Space after placement =x:%2.2f y:%2.2f\n", hspa, vspa); 
#endif
    cairo_move_to (cr, fiddle_lmar+r.x+hspa/2., r.y+vspa/2.);
	pango_cairo_update_layout(cr, playout); /* if the target surface or transformation properties of the cairo instance have changed, update the pango layout to reflect this */

	pango_cairo_layout_path(cr, playout); // draw the pango layout onto the cairo surface
	g_object_unref(playout);	// free the layout
    free(parstr);
}

int main(int argc, char* argv[])
{
    if(argc!=2) {
        printf("Please give 1 argument: a (raw) configuration file whose every five lines are as follows:\n");
        printf("1) Fraction of whole image to use for the block\n");
        printf("2) Background hex colour string i.e. \"#4488DD\", 3) text hex colour string\n");
        printf("4) Font string 5) text to show.\n");
        printf("So for every text box and rectangle it fits in, there will be five lines\n");
        exit(EXIT_FAILURE);
    }
    int i;

    // read in conf .. decides everything
    larr_t *la=slurplines(argv[1]);

    /* Our first line will be the size of the whole image */
    float wh[2];
    char szdelim='x'; // common width versus height delim
	char *tk=strtok(la->l[0], &szdelim);
	if(!tk) {
		printf("The first line in the conf file does not have a proper image size definition.\n");
        exit(EXIT_FAILURE);
    }
    wh[0]=atof(tk);
    tk=strtok(NULL, &szdelim);
    wh[1]=atof(tk);

    /* number of text blocks */

    cairo_status_t status; // variable to hold the various states of the cairo lib
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, wh[0], wh[1]); // cairo_image_surface_create(pixelformat, width, height);
    cairo_t *cr = cairo_create(surface);	// tell the cairo instance 'cr' to output to 'surface'

    /* rectangle placements */
    rpla_t r0;
    /* we're going downwards so x pos and x extent won't change */
    r0.y=0; r0.h=wh[1];
    float frac;
    cf_t *rgb=malloc(sizeof(cf_t));

    float preval=0; // the previous height, don't forget we're moving down

    for(i=1;i<la->asz;i+=5) {
        frac=atof(la->l[i]);
        printf("%2.2f\n", frac); 
        r0.x=preval; r0.w=wh[0]*frac;

        /* Rect 0: bg */
        hc2f(rgb, la->l[i+1]); // first line bg
        cairo_rectangle (cr, r0.x, r0.y, r0.w, r0.h);
        cairo_set_source_rgb(cr, rgb->r, rgb->g, rgb->b);
        cairo_fill(cr);
        /* Rect 0: txt */
        hc2f(rgb, la->l[i+2]); // second line fg
        cairo_set_source_rgb(cr, rgb->r, rgb->g, rgb->b);
        lay0(cr, r0, la, i);
        cairo_fill(cr);
        preval=r0.x+r0.w;
    }

    cairo_destroy(cr);	// free the cairo instance
    status = cairo_surface_write_to_png(surface, "htblkz.png");

    cairo_surface_destroy(surface);	// free the surface
    if (status != CAIRO_STATUS_SUCCESS) {
        // if 'status' was not set to indicate a successful operation when saving as a png, error
        printf("Could not save to png, \"panx.png\"\n");
        return 1;
    }

    /* and now to free line array */
    for(i=0;i<la->asz;++i)
        free(la->l[i]);
    free(la->l);
    free(la->lz);
    free(la);
    free(rgb);

    return 0;
}

