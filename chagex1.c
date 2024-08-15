#include <cairo.h>
#include <pango/pangocairo.h>

int main(int argc, char *argv[])
{
    cairo_surface_t *surface;
    cairo_t *cr;
    PangoLayout *layout;
    PangoFontDescription *font_desc;

    // Create Cairo surface and context
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 800, 600);
    cr = cairo_create(surface);

    // Set background color (optional)
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);  // Black background
    cairo_paint(cr);

    // Create Pango layout and set text
    layout = pango_cairo_create_layout(cr);
    pango_layout_set_text(layout, "Hello, Pango with Cairo!", -1);

    // Set font description
    font_desc = pango_font_description_from_string("Sans Bold 20");
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free(font_desc);

    // Set stroke color and width
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);  // White stroke
    cairo_set_line_width(cr, 2.0);

    // Render the stroke by drawing the text path and stroking it
    cairo_move_to(cr, 50, 100);  // Position of the text
    pango_cairo_layout_path(cr, layout);  // Create path from layout
    cairo_stroke(cr);  // Stroke the path

    // Set fill color
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);  // Red fill

    // Render the fill
    cairo_move_to(cr, 50, 100);  // Position of the text (same as before)
    pango_cairo_show_layout(cr, layout);

    // Save the output
    cairo_surface_write_to_png(surface, "chagex1.png");

    // Clean up
    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return 0;
}
