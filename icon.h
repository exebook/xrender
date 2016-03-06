#include "icon32.h"
//
//int ICON[32*32] = {
//	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
//	1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,1,
//	1,2,1,2,1,2,2,1,1,2,1,1,1,1,2,1,
//	1,2,2,1,2,1,2,1,1,2,2,2,2,2,2,1,
//	1,2,3,3,3,2,2,1,1,2,2,2,2,2,2,1,
//	1,2,1,2,1,2,2,1,1,2,2,2,2,2,2,1,
//	1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,1,
//	1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,1,
//	1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,1,
//	1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,1,
//	1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,1,
//	1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,1,
//	1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,1,
//	1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,1,
//	1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,1,
//	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
//}, ICONCOLOR[6] = {0, 0x44eeff, 0xff, 0xff0000, 0xffff00, 0xffff};

Pixmap load_pixmap(Drawable window) {
	int W = 32, H = 32;
	Pixmap pixmap = XCreatePixmap(MESS.d, window, W, H, 24);
	XGCValues gr_values;
	GC gr_context = XCreateGC(MESS.d, window, GCBackground, &gr_values);
	str PIX; PIX(W*H*4*4); PIX.fill(0);
	uint32_t *X = (uint32_t*) *PIX;
	XImage *ximage = XCreateImage(MESS.d, CopyFromParent, 24, XYPixmap, 0, (char*) X, W, H, 32, W);
	int i = 0;
	for (int y = 0; y < H; y++)
	for (int x = 0; x < W; x++)
		XPutPixel(ximage, x, y, ICONCOLOR[ICON[i++]]);
	XPutImage(MESS.d, pixmap, gr_context, ximage, 0, 0, 0, 0, W, H);
	return pixmap;
}

void set_icon(Drawable window) {
	unsigned int icon_width, icon_height;
	char *window_name = "Deodar Commander-";
	char *icon_name = "deodar";
	Pixmap icon_pixmap;
	XSizeHints size_hints;
	XWMHints wm_hints;
	XClassHint class_hints;
	XTextProperty windowName, iconName;
	icon_pixmap = load_pixmap(window);
	size_hints.flags = PPosition | PSize | PMinSize;
	size_hints.min_width = 300;
	size_hints.min_height = 200;
	XStringListToTextProperty(&window_name, 1, &windowName);
	XStringListToTextProperty(&icon_name, 1, &iconName);
	wm_hints.initial_state = NormalState;
	wm_hints.input = True;
	wm_hints.icon_pixmap = icon_pixmap;
	wm_hints.flags = StateHint | IconPixmapHint | InputHint;
	class_hints.res_name = "deodar";
	class_hints.res_class = "Deodar";
	XSetWMProperties(MESS.d, window, &windowName, &iconName, 0, 0, &size_hints, &wm_hints, &class_hints);
}

