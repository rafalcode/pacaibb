#include <cairo.h>
#include <pango/pangocairo.h>
#include <math.h>

void draw_feathered_stroke(cairo_t *cr, PangoLayout *layout, int x, int y, double line_width, double blur_radius, double r, double g, double b)
{
    int i, j;
    double alpha_step = 1.0 / (blur_radius * blur_radius);
    
    for (i = -blur_radius; i <= blur_radius; i++) {
        for (j = -blur_radius; j <= blur_radius; j++) {
            if (sqrt(i * i + j * j) > blur_radius)
                continue;  // Skip positions outside the blur radius
            cairo_set_source_rgba(cr, r, g, b, alpha_step);  // Set color with alpha
            cairo_move_to(cr, x + i, y + j);
            pango_cairo_layout_path(cr, layout);
            cairo_set_line_width(cr, line_width);
            cairo_stroke(cr);
        }
    }
}

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
    pango_layout_set_text(layout, "Feathered Stroke Example", -1);

    // Set font description
    font_desc = pango_font_description_from_string("Sans Bold 16");
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free(font_desc);

    // Draw the feathered stroke: actually line widht=1.0 and blue radius=1.0 are good setting.
    // draw_feathered_stroke(cr, layout, 50, 100, .5, .5, 1.0, 1.0, 1.0);  // Good for Sans Bold 22
    draw_feathered_stroke(cr, layout, 50, 100, 1, 1.5, 1.0, 1.0, 1.0);  // White stroke

    // Set fill color
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);  // Red fill

    // Render the fill
    cairo_move_to(cr, 50, 100);  // Position of the text
    pango_cairo_show_layout(cr, layout);

    // Save the output
    cairo_surface_write_to_png(surface, "chagex2.png");

    // Clean up
    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return 0;
}
