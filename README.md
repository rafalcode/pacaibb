# pacaibb
stands for Pango-Cairo Building Blocks

## two meanings of "blocks" ...
though a block is a block of text, this repo primarily refers to the coding
building blocks to se tup pango cairo bloks of text

## came from?
pacairodems, used for AusEPT in March 2021.

# trying to understand pango layout
.. from the height perspective. Quite tricky,, it's not clear
from the docs:
https://docs.gtk.org/Pango/class.Layout.html
so actually I found out, it's all the cairo context: cr.
This line
cairo_move_to (cr, fiddle_lmar+r.x+hspa/2., r.y+vspa/2.);
was causing the vertical centering. when I did this
cairo_move_to (cr, fiddle_lmar+r.x+hspa/2., ystartpos);
and actually ystartpos can be 0 and it's fine.

however there still problems with the right side
