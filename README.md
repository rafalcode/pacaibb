# pacaibb
stands for Pango-Cairo Building Blocks

Beware pango, it can be a bit tricky and v. hard to debug. Especially pango_layout which is
are high level convenience functions. Particularly its unit, pango_units, which are PANGO_SCALE=1024
times bigger normal pixel units. So if you use pixel units without multiplying by PANGO_SCALE
you'' notice no effect. I particularly foiund thi swith set_indent, which I recknoned was not working!
It was, the value just needed multiplication by PANGO_SCALE!

terminology:
- ellipsization this is where a string is cut short and then a "..." (ellipsis) is added.

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

# JPEG reduce programs
One or two things to  remember about these. 
- JPEG requires an extra library as cairo is not good with JPGs
- Whether lansdcape or potrait you need only reduce on one dimension. I.e for landscape that's width. What the height will be will depend on the height of the input image
and not what you can set. This is an easy one to miss so watch it.



# jpred0.c
This is part of the image reduction programs (imred0.c) but it uses rahra's cairo_jpg repository
to allow cairo to read in a jpg into a cairo surface. Note I compiled this as a shared
library and put it into /usr/local

In terms of output, png is a good idea as it's failry efficient. PDF is not so good
it seems to take all the jpg even if you only deal with a part of it.

# Using the python scripts for recording image click points.
We want to purposefully click an image and get the pixel coordinates of the location reocrded onto a file.
There are very many image viewers and may one or two do this, but a python script will also do it
using img.show(). cc0.py is the template for it.
So the cv2 imshow() required leanring. With big images, You can pan around with mouse by holding down <ALT> and left clicking.
You can imagine it's quite easy to left click without ALT, unfortunately these will register as click poisitons
so you have to avoid that.

# speeb*, the speech bubble programs
now the nature of the speech bubble needs some clarification. Should it be white background? And so 
block out part of the photograph. It certainly does block out, but that is actualy the traditional speech bubble.

# a.png
This is two paragraphs from Walter Pater on a 1000x1000 png with dark pink text on black background which was quite attractive. I was mystified how it was generated.
And it took me a while to find out, despite the fact that I had done it 10 days before. the answer is, it's mycarisimp.co

# son*.c series
I wante dot focus on building blocks so these had some idea of importing sonnets. In the end, chagex01.c took over

# chagex?.c series
GOt from chatgpt (therefore CHAG and EX for example). chagex2.c does the feathered outline on text
so you have black text on dark backgrounds.
