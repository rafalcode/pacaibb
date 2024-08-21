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

#define CHOSENFONT "Sans Bold"

typedef struct /* larr_t line array struct */
{
    char **l;
    size_t *lz;
    int lbf;
    int asz;
} larr_t;

larr_t *slurplines2(char *fn)
{
    // this version also east the new lines and retains them.
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

int main(int argc, char *argv[])
{
    if(argc!=2) {
        printf("Please give 1 argument: the text file\n");
        exit(EXIT_FAILURE);
    }

    int i;
    larr_t *la=slurplines2(argv[1]); // newlines kept, remember this when concatenating!
    // how many chars?
    size_t lachars=0;
    for(i=0;i<la->asz;++i) 
        lachars += la->lz[i];
    // OK create temp str which is easily bigger htan that.
    char *tstr=calloc(lachars+2*la->asz, sizeof(char));
    // concatenate into one long string, but putting ht newlines in
    for(i=0;i<la->asz;++i)
        strcat(tstr, la->l[i]);

    // Create Cairo surface and context
    cairo_surface_t *surface = NULL;
    cairo_t *cr = NULL;
    PangoLayout *layout = NULL;

    // there could be mark up
    char *parstr=calloc(lachars+2*la->asz, sizeof(char));
    PangoAttrList *attlst;
    pango_parse_markup(tstr, -1, '\0', &attlst, &parstr, NULL, NULL);
    free(tstr); // not need no more.

    // Set font description
    PangoFontDescription *font_desc = NULL;
    char *thisfontsz=calloc(128, sizeof(char));
    char *ofname=calloc(128, sizeof(char));
    PangoRectangle inkrect;
    PangoRectangle logirect;

    for(i=16;i<21;++i) {
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 800, 600);
        cr = cairo_create(surface);
        // Get the layout going. Don't foget Layout is the pango magick which will wrap into a paragraph
        layout = pango_cairo_create_layout(cr);
        pango_layout_set_text(layout, parstr, -1);

        sprintf(thisfontsz, "%s %i", CHOSENFONT, i);
        printf("thisfontsz: %s\n", thisfontsz);
        font_desc = pango_font_description_from_string(thisfontsz);
        pango_layout_set_font_description(layout, font_desc);
        pango_font_description_free(font_desc);

        // tweak line spacing
        pango_layout_set_line_spacing(layout, 0.9);
        // also squeeze
        // int ourwidth=300*PANGO_SCALE;
        // pango_layout_set_width(layout, ourwidth);

        // findout what's gone on, so far.
        pango_layout_get_pixel_extents(layout, &inkrect, &logirect);
        printf("new inkrect(pangus) x:%i/y:%i/w:%i/h:%i\n", inkrect.x, inkrect.y, inkrect.width, inkrect.height); 
        sprintf(ofname, "%s_%02i.png", "chagex02", i);

        // Draw the text
        cairo_move_to(cr, 20, 20);
        pango_cairo_show_layout(cr, layout);

        // use this data to draw a rect.
        cairo_rectangle (cr, 20, 20, inkrect.width, inkrect.height-inkrect.y); // height goes over a bit some 20pixels.
        cairo_set_source_rgba(cr, .5, .6, .7, .3);
        cairo_fill(cr);

        // Save the output
        cairo_surface_write_to_png(surface, ofname);
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
    }

    // Clean up
    g_object_unref(layout);
    free(parstr);
    for(i=0;i<la->asz;++i)
        free(la->l[i]);
    free(la->l);
    free(la->lz);
    free(la);
    free(thisfontsz);
    free(ofname);

    return 0;
}
