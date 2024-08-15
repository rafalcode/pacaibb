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

    // Create Pango layout and set text
    layout = pango_cairo_create_layout(cr);
    pango_layout_set_text(layout, "Hello, Pango with Cairo!", -1);

    // Set font description
    font_desc = pango_font_description_from_string("Sans Bold 20");
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free(font_desc);

    // Draw the text
    cairo_move_to(cr, 50, 50);
    pango_cairo_show_layout(cr, layout);

    // Save the output
    cairo_surface_write_to_png(surface, "output.png");

    // Clean up
    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return 0;
}
