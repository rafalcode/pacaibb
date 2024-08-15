/* Simple example to use pangocairo to render rotated text */

#include <math.h>
#include <pango/pangocairo.h>

static void draw_text (cairo_t *cr, int *cwh)
{
#define RADIUS 200
#define LMAR 50
#define RMAR 50
#define FONT_WITH_MANUAL_SIZE "Times new roman,Sans"
#define FONT_SIZE 3
#define DEVICE_DPI 72

/* The following number applies a cairo CTM.  Tests for
 * https://bugzilla.gnome.org/show_bug.cgi?id=700592
 */
#define TWEAKABLE_SCALE ((double) 0.1)

  PangoLayout *layout;
  PangoFontDescription *desc;

  /* Center coordinates on the middle of the region we are drawing
   */
  // cairo_translate (cr, RADIUS / TWEAKABLE_SCALE, RADIUS / TWEAKABLE_SCALE);

  /* Create a PangoLayout, set the font and text */
  layout = pango_cairo_create_layout (cr);

   // char *textinnit="His legend, as the French say, with the anecdotes which every one remembers, is one of the most brilliant chapters of Vasari. Later writers merely copied it, until, in 1804, Carlo Amoretti applied to it a criticism which left hardly a date fixed, and not one of those anecdotes untouched. The various questions thus raised have since that time become, one after another, subjects of special study, and mere antiquarianism has in this direction little more to do.\nFor others remain the editing of [99/100] the thirteen books of his manuscripts, and the separation by technical criticism of what in his reputed works is really his, from what is only half his, or the work of his pupils. But a lover of strange souls may still analyse for himself the impression made on him by those works, and try to reach through it a definition of the chief elements of Leonardo's genius. The legend, as corrected and enlarged by its critics, may now and then intervene to support the results of this analysis.";
   char *textinnit="His legend, as the French say, with the anecdotes which every one remembers, is one of the most brilliant chapters of Vasari. Later writers merely copied it, until, in 1804, Carlo Amoretti applied to it a criticism which left hardly a date fixed, and not one of those anecdotes untouched. The various questions thus raised have since that time become, one after another, subjects of special study, and mere antiquarianism has in this direction little more to do.";
   // char *textinnit="&#160; &#160; &#160; His legend,&#160; as the French say,&#160; &#160; &#160; &#160; &#160; &#160; &#160; with the anecdotes which every one remembers";
   // char *textinnit="His legend,\n as the French say,\n with the anecdotes which every one remembers,\n    is one of the most brilliant chapters of Vasari.";
  pango_layout_set_text (layout, textinnit, -1);

  desc = pango_font_description_from_string (FONT_WITH_MANUAL_SIZE);
  pango_font_description_set_absolute_size(desc, FONT_SIZE * DEVICE_DPI * PANGO_SCALE / (72.0 * TWEAKABLE_SCALE));
  //pango_font_description_set_size(desc, 27 * PANGO_SCALE / TWEAKABLE_SCALE);

  printf("PANGO_SCALE = %d\n", PANGO_SCALE);
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free (desc);

  size_t tsz = strlen(textinnit);
  char *parstr=calloc(tsz, sizeof(char)); // the actual text to show.
  PangoAttrList *attlst;
  pango_parse_markup(textinnit, -1, '\0', &attlst, &parstr, NULL, NULL);
  pango_layout_set_text(layout, parstr, -1);
  pango_layout_set_attributes(layout, attlst);

      int width, height;

      cairo_save (cr);

      /* Gradient from red at angle == 60 to blue at angle == 240 */
      cairo_set_source_rgb (cr, .9, 0, .3);

      pango_layout_set_width(layout, (cwh[0]-LMAR-RMAR)*PANGO_SCALE); // very bizarre integer hereis like in the 100K's! How can that be a width? Ans. they are pango units.
      pango_layout_set_indent(layout, 50*PANGO_SCALE);
      // float lspa=pango_layout_get_line_spacing(playout);
      pango_layout_set_line_spacing(layout, .9);
      pango_cairo_update_layout (cr, layout);

      pango_layout_get_size (layout, &width, &height);
      printf("w:%i h:%i\n", width, height);
      float x1=- (((double)width) / PANGO_SCALE) / 2.0;
      float y1=- RADIUS  / TWEAKABLE_SCALE;
      printf("x1 %2.2f y1 %2.2f\n", x1, y1); 
      // cairo_move_to (cr,( - (((double)width) / PANGO_SCALE) / 2.0) , (- RADIUS)  / TWEAKABLE_SCALE);
      cairo_move_to(cr,LMAR,0);
      // pango_layout_set_width(playout, ourscale); // very bizarre integer hereis like in the 100K's! How can that be a width? Ans. they are pango units.
      //
      // cairo_set_source_rgb (cr, .1, .5, .8);
      // cairo_stroke(cr);

      pango_cairo_show_layout (cr, layout);
       cairo_set_source_rgb (cr, .1, .5, 1);
       cairo_set_line_width(cr, 2);
      cairo_stroke(cr);

      cairo_restore (cr);

  /* free the layout object */
  g_object_unref (layout);
}

int main (int argc, char **argv)
{
  cairo_t *cr;
  cairo_status_t status;

  if (argc != 2) {
      g_printerr ("Usage: cairosimple OUTPUT_FILENAME\n");
      return 1;
    }

  char *filename = argv[1];


  // canvas widh-height
  int cwh[2]={600, 600};
  cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, cwh[0], cwh[1]);
  cr = cairo_create (surface);

  // cairo_scale(cr, 1 * TWEAKABLE_SCALE, 1 * TWEAKABLE_SCALE);

  cairo_set_source_rgb (cr, .0, .0, .0);
  cairo_paint (cr);
  draw_text(cr, cwh);
   cairo_set_source_rgb (cr, .1, .5, .8);
   cairo_set_line_width(cr, 3);
  cairo_stroke(cr);
  cairo_destroy (cr);

  status = cairo_surface_write_to_png (surface, filename);
  cairo_surface_destroy (surface);

  if (status != CAIRO_STATUS_SUCCESS) {
      g_printerr ("Could not save png to '%s'\n", filename);
      return 1;
    }

  return 0;
}
