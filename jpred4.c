/* Foloowing on from jpred2.c ...
 * jpred3.c looks at the centre of the image
 * clips original and use bottom strip to display the zoom
 * had good deal of trouble with text alignment. */
#include<math.h>
#include<stdlib.h>
#include<stdio.h>
#include<cairo/cairo.h>
#include<cairo/cairo-pdf.h>
#include<cairo_jpg.h>
#include<pango/pangocairo.h>
#include<libexif/exif-data.h>
#include<time.h>

// we will save to a landscape sized PNG on which img2pdf can then run
// for an efficiently sized 
// #define WPDF 842.
// #define HPDF 595.
#define WPDF 800
#define HPDF 600
#define IRF 0.5 // image reduction section fraction: x fraction of 
#define LWI 0 //landscape width index (0 or 1)
#define LHI 1
#define PWI LHI
#define PHI LWI
#define LST 2 // line stroke width
// colours for text both, now fg white and text is black noce and simple
#define BGTB #000000
#define FGTB #FFFFFF
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
    float lm, rm; // left marg and right margin
} rpla_t;

typedef struct /* larr_t line array struct */
{
    char **l;
    size_t *lz;
    int lbf;
    int asz;
} larr_t;

void convert_date(const char *input_date, char *output_date, char *otime)
{
    struct tm tm;
    
    // Initialize tm structure to zero
    memset(&tm, 0, sizeof(struct tm));
    
    // Parse the input date string "YYYY:MM:DD"
    sscanf(input_date, "%d:%d:%d %s", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, otime);
    
    // Adjust the year and month to fit the tm structure
    tm.tm_year -= 1900; // tm_year is years since 1900
    tm.tm_mon -= 1;     // tm_mon is 0-11

    // Convert to time_t to update the tm_wday field
    // time_t t = mktime(&tm); // not reallyused, why did chatgpt come up with it?

    // Format the date into "named-day named-month year"
    strftime(output_date, 64, "%a %d %b %Y", &tm);

    // Print the result
    printf("Converted date: %s\n", output_date);
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

char* get_datetime_field(ExifData *ed)
{
    char *datetime = NULL;

    // Function to extract DateTime from an ExifEntry
    void extract_datetime(ExifEntry *entry, void *user_data) {
        if (entry->tag == EXIF_TAG_DATE_TIME) {
            char buf[1024];
            exif_entry_get_value(entry, buf, sizeof(buf));
            if (*buf) {
                datetime = strdup(buf); // Duplicate the string to store it
            }
        }
    }

    // Function to iterate over ExifContent and apply the extraction function
    void foreach_content(ExifContent *content, void *user_data) {
        exif_content_foreach_entry(content, extract_datetime, user_data);
    }

    // Iterate over all Exif content and extract the DateTime field
    exif_data_foreach_content(ed, foreach_content, NULL);

    return datetime; // Return the extracted DateTime string
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

void lay0(cairo_t *cr, rpla_t r, larr_t *la)
{
    /* OK, over to pango, first creat out layout, it's linked to the cairo context context */
	PangoLayout *playout = pango_cairo_create_layout(cr);

    /* select font */
	PangoFontDescription *desc= pango_font_description_from_string(la->l[2]);
    if(desc == NULL) {
	    printf("Failed: pango_font_description_from_string() came back NULL\n");
        exit(EXIT_FAILURE);
    }
	pango_layout_set_font_description(playout, desc);

    char *parstr=calloc(1+la->lz[3], sizeof(char)); // the actual text to show.

    PangoAttrList *attlst;
    pango_parse_markup(la->l[3], -1, '\0', &attlst, &parstr, NULL, NULL);

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
    cairo_move_to (cr, r.lm+r.x+hspa/2., 0);

    printf("cr move to %2.2f, %i\n", r.x+r.lm, 0);
	pango_cairo_update_layout(cr, playout); /* if the target surface or transformation properties of the cairo instance have changed, update the pango layout to reflect this */

    pango_layout_set_alignment(playout, PANGO_ALIGN_CENTER);
    printf("PangoAlignment = %i (0 for LEFT , 1 for CENT 2 for RIGHT)\n", pango_layout_get_alignment(playout));

	pango_cairo_layout_path(cr, playout); // draw the pango layout onto the cairo surface
	g_object_unref(playout);	// free the layout
    free(parstr);
	pango_font_description_free(desc);
}

void lay00(cairo_t *cr, rpla_t r, larr_t *la, char *tstr)
{
    /* OK, over to pango, first creat out layout, it's linked to the cairo context context */
	PangoLayout *playout = pango_cairo_create_layout(cr);

    /* select font */
	PangoFontDescription *desc= pango_font_description_from_string(la->l[2]);
    if(desc == NULL) {
	    printf("Failed: pango_font_description_from_string() came back NULL\n");
        exit(EXIT_FAILURE);
    }
	pango_layout_set_font_description(playout, desc);

    char *parstr=calloc(1+la->lz[3], sizeof(char)); // the actual text to show.

    PangoAttrList *attlst;
    pango_parse_markup(tstr, -1, '\0', &attlst, &parstr, NULL, NULL);

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
    cairo_move_to (cr, r.lm+r.x+hspa/2., 0);

    printf("cr move to %2.2f, %i\n", r.x+r.lm, 0);
	pango_cairo_update_layout(cr, playout); /* if the target surface or transformation properties of the cairo instance have changed, update the pango layout to reflect this */

    pango_layout_set_alignment(playout, PANGO_ALIGN_LEFT);
    printf("PangoAlignment = %i (0 for LEFT , 1 for CENT 2 for RIGHT)\n", pango_layout_get_alignment(playout));

	pango_cairo_layout_path(cr, playout); // draw the pango layout onto the cairo surface
	g_object_unref(playout);	// free the layout
    free(parstr);
	pango_font_description_free(desc);
}

int main (int argc, char *argv[])
{
    
    if(argc!=3) {
        printf("Please give 2 arguments: 1) input image file (must be JPG) 2) text conf file\n");
        printf("conf file lines: 1st) font spec 2) xcoord of focus point 3) y coord for focus point 4) tres of lines are hte text\n");
        printf("Beware at least one of the lines should occupy and even exceed width, so that all lines are flush left\n");
        exit(EXIT_FAILURE);
    }
    int i;

    float owh[2]={WPDF, HPDF}; // owh is for landscape width height. i.e. owh[0] is width,
    // so, when you see owh[0] being used for height, you should conclude that portrait is being used.
    float hpdfw=WPDF*IRF; // half PDF width
    float hpdfh=HPDF*IRF;
    printf("- Topo left space into which image will be reduced is %2.2fx%2.2f\n", hpdfw, hpdfh);

    // let's look at EXIF DateTime
    ExifData *ed = exif_data_new_from_file(argv[1]);
    if (!ed) {
        fprintf(stderr, "Error loading Exif data from file %s\n", argv[1]);
        return 1;
    }

    char *datetime = get_datetime_field(ed);
    if (datetime) {
        printf("DateTime: %s\n", datetime);
    } else {
        printf("DateTime field not found.\n");
    }

    exif_data_unref(ed);
    //

    // our surf0, this will be used as a source.
    cairo_surface_t *surf0 = cairo_image_surface_create_from_jpeg(argv[1]);
    int w = cairo_image_surface_get_width (surf0);
    int h = cairo_image_surface_get_height (surf0);
    printf("orig input image - w=%i,h=%i\n", w, h); 
    // printf("- Can take a clip of %2.2f%% of original width and %2.2f%% of original height\n", 100.*hpdfw/(float)w, 100.*hpdfh/(float)h);

    printf("Amount of times true image width exceeds IRF*pdfwidth:\n"); 
    float wrfac= hpdfw/(float)w; // width reduce factor, for landscape, this is the "deciding one".
    printf("wrfac=%2.2f\n", wrfac);
    float hsh= h*wrfac; // height shrink. We need not do this for width, because it has to hpdfw, as we've set that way.
    printf("Reduced input image - w=%2.2f,h=%2.2f\n", hpdfw, hsh); 

    // read in conf .. decides everything
    larr_t *la=slurplines(argv[2]);
    printf("how many text lines? i.e. asz-3 :%i\n", la->asz-3); 
    int tlen=strlen(datetime);
    for(i=3;i<la->asz;++i) {
        printf("Len of text line %i: %zu\n", i-3, la->lz[i]);
         tlen += la->lz[i];
    }

    char *tstr=calloc(tlen+16, sizeof(char));
    strcat(tstr, "&#8226; ExifDateTime: ");


    char *odate=calloc(64, sizeof(char));
    char *otime=calloc(64, sizeof(char));
    convert_date(datetime, odate, otime);
    strcat(tstr, odate);
    strcat(tstr, " ");
    strcat(tstr, otime);
    strcat(tstr, "\n");
    for(i=3;i<la->asz;++i){ 
        strcat(tstr, la->l[i]);
        if(i!=la->asz-1)
            strcat(tstr, "\n");
    }
    printf("%s\n", tstr); 


    // ok so the text box will go to the left of this.
    rpla_t r0;
    r0.y=0;
    r0.h=hsh;
    r0.x=hpdfw;
    r0.w=WPDF-hpdfw;
    r0.lm=10;
    r0.rm=10;

    cairo_surface_t *surf2 = cairo_image_surface_create(CAIRO_FORMAT_RGB24, owh[LWI], owh[LHI]); // landscape A4 I think
    cairo_t *cr = cairo_create(surf2);

    // set background colour of whole output image
    cairo_rectangle (cr, 0, 0, owh[LWI], owh[LHI]); /* arg explan: topleftcorner and size of shape  */
    // cairo_set_source_rgb(cr, 0yyp, 1., 0); // high blue so my eye can detect.
    cairo_set_source_rgb(cr, 0, 0, 0); // high blue so my eye can detect.
    cairo_fill (cr);

    cairo_scale(cr, wrfac, wrfac);
    cairo_set_source_surface (cr, surf0, 0, 0);
    cairo_paint(cr);

     cairo_destroy(cr);

    // now new one, so that scale is forgotten, he expects ...
    cr = cairo_create(surf2);

    cf_t *rgb=malloc(sizeof(cf_t));
   /* Rect 0: bg */
   printf("%s\n", la->l[0]);
   hc2f(rgb, la->l[0]); // first line bg
   cairo_rectangle (cr, r0.x, r0.y, r0.w, r0.h);
   cairo_set_source_rgb(cr, rgb->r, rgb->g, rgb->b);
   cairo_fill(cr);

   /* Rect 0: txt */
   hc2f(rgb, la->l[1]); // second line fg
   cairo_set_source_rgb(cr, rgb->r, rgb->g, rgb->b);
   // lay0(cr, r0, la);
   lay00(cr, r0, la, tstr);
   cairo_fill(cr);

    // now using the r2 for the lower strip
    // Middle point
    float mpoi[2] = {(float)w/2., (float)h/2.};
    printf("Mid point of input:%2.2f,%2.2f\n", mpoi[0], mpoi[1]);
    // the c lip rectangle.
    // lower strip
    rpla_t r2;
    r2.y=hsh;
    r2.h=HPDF-hsh;
    r2.x=0;
    r2.w=WPDF;
    r2.lm=0;
    r2.rm=0;

    printf("cairo_rect x=%2.2f, y=%2.2f, w=%2.2f, h=%2.2f\n", r2.x, r2.y, r2.w, r2.h);
    cairo_rectangle(cr, r2.x+4, r2.y+4, r2.w-LST-6, r2.h-8);
   // cairo_set_source_rgb(cr, 1., 0, .5);
   // cairo_fill(cr);
    cairo_clip(cr); // somehow, this means the rectangle is the only writeable part now.
    float clipx=mpoi[0]-r2.w/2.;
    float clipy=mpoi[1]-r2.h/2.;
    printf("clipstart x=%2.2f y=%2.2f\n", clipx, clipy);
    // cairo_set_source_surface (cr, surf0, 50., 50.);
    cairo_set_source_surface(cr, surf0, r2.x-clipx-LST-1, r2.y-clipy-LST-1);
    cairo_paint (cr);
    cairo_destroy (cr);

    cr = cairo_create(surf2);
    // cairo_rectangle(cr, 200, 200, 200, 200);
    cairo_rectangle(cr, wrfac*clipx, wrfac*clipy, wrfac*r2.w, wrfac*r2.h);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, LST);
    cairo_stroke(cr);

    cairo_destroy (cr);
    cairo_surface_write_to_png(surf2, "jpred4.png");
    cairo_surface_destroy (surf2);
    cairo_surface_destroy (surf0);
    free(rgb);
    free(tstr);
    free(odate);
    free(otime);
    free(datetime); // Free the duplicated string

    return 0;
}
