#include <stdint.h>
#include <wchar.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/Xrender.h>

#include <ft2build.h>
#include <freetype.h>
#include FT_FREETYPE_H

#define UNICODE
#include "libosnova.cpp"
#include "utf.cpp"

#define doubleClickSetting 300
#include "xglyph.h"


struct win;
#  define  on_paint    void render()
#  define  on_cursor   void cursor(int x, int y)
#  define  on_mouse    void mouse(bool down, int button, int x, int y)
#  define  on_created  void creating()
//#  define  on_char     void character(wchar_t ch)
#  define  on_key      void keyboard(bool down, int charcode, int key, bool physical)
#  define  on_size     void sizing(int w, int h)
#  define  on_timer    void timer(int id)
#  define  on_focus    void focus(bool on)

struct timer { win *w; int on, id, interval, next; };

#include "mess.h"
#include "icon.h"

Picture create_buffer(Display *display, int w, int h) {
	XRenderPictFormat *fmt=XRenderFindStandardFormat(display, PictStandardARGB32);
	Window root = DefaultRootWindow(display);
	Pixmap pm = XCreatePixmap(display, root, w, h, 32);
	XRenderPictureAttributes pict_attr;
	pict_attr.repeat = 1;
	Picture picture=XRenderCreatePicture(display, pm, fmt, CPRepeat, &pict_attr);
	XFreePixmap(display, pm);
	return picture;
}

struct win {
	Drawable window;
	Picture P, BUF; // XRender thing
   double x, y, w, h;
   XIC ic;
   str bla;
   int renders, curfont, font_color;

	virtual on_paint  {}
	virtual on_cursor {}
	virtual on_mouse  {}
	virtual on_created{}
// virtual on_char   {}
	virtual on_key    {}
	virtual on_size   {}
	virtual on_timer  {}
	virtual on_focus  {}

GC gc;
XGCValues gcv;
void create_gc() {
	u32 bpx = BlackPixel(MESS.d, MESS.screen);
	u32 wpx = WhitePixel(MESS.d, MESS.screen);
	gcv.foreground = wpx;
	gcv.background = bpx;
	gcv.line_width = 3;
	gcv.function   = GXcopy;
	int gcmask = GCForeground | GCBackground | GCLineWidth | GCFunction;
	gc = XCreateGC(MESS.d, window, gcmask, &gcv);
}

void create() {
	if (MESS.d == 0) MESS.init();
	renders = curfont = font_color = 0; 
	w = 600; 
	h = 200;
	BUF = 0;
	XSetWindowAttributes windowAttr;
	window = XCreateWindow(MESS.d, MESS.root, w,h, w,h, 0, 24, 
		InputOutput, CopyFromParent, 0, &windowAttr);
		
	ic = XCreateIC(MESS.im, XNInputStyle, 
		XIMPreeditNothing | XIMStatusNothing, XNClientWindow, 
		window, NULL);
		
	if (ic == NULL) printf("Could not open IC\n");
	XSetICFocus(ic);
	XSelectInput(MESS.d, window, 0
		| StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask 
		| ButtonPressMask | ButtonReleaseMask |KeymapStateMask 
		| PointerMotionMask | VisibilityChangeMask | KeymapStateMask 
		| EnterWindowMask | LeaveWindowMask | FocusChangeMask
		//| PropertyChangeMask
	);
	
	set_icon(window);
	XSaveContext(MESS.d, window, MESS.xcontext, (XPointer)this);
   XSetWMProtocols(MESS.d, window, &MESS.XA_WM_DELETE_WINDOW, 1);
	P = xrender_window_picture(MESS.d, window);
   setSizeSteps(20, 20);
	MESS.all << this;
	x_resize(0, 0, w, h);
//	create_gc();
	creating();
}

void setSizeSteps(int w, int h) {
   XSizeHints *hint = XAllocSizeHints();
   hint->flags = PResizeInc;
   hint->width_inc = w;
   hint->height_inc = h;
   XSetWMSizeHints(MESS.d, window, hint, MESS.XA_WM_NORMAL_HINTS);
}

void setCursor(int n) {
	XDefineCursor (MESS.d, window, MESS.Cursors[n]);
}

void show() {
   XMapWindow(MESS.d, window);
}

void hide() {
   XUnmapWindow(MESS.d, window);
}

bool step() {
	return MESS.step();
}

void paint() {
	renders++;
}

void add_timer(int id, int T) {
	MESS.add_timer(this, id, T);
}

void del_timer(int id) {
	MESS.del_timer(this, id);
}

void size(int _w, int _h) {
	w = _w, h = _h;
	XResizeWindow(MESS.d, window, w, h);
}

void move(int x, int y) {
	XMoveWindow(MESS.d, window, x, y);
}

int* text_extent(wstr s) {
	static int wh[2];
	xrender_font &F = MESS.Fonts[curfont].font;
	wh[0] = F.width * !s;
	wh[1] = F.height;
//	printf("wh[] = %i %i\n", wh[0], wh[1]);
	return wh;
}

// translators:
void x_expose() {
	paint();
}
void x_destroy() {
	MESS.remove(this);
}
void x_resize(int _x, int _y, int _w, int _h) {
	if (BUF != 0) XRenderFreePicture(MESS.d, BUF);
	BUF = create_buffer(MESS.d, w, h);

	w = _w, h = _h; x = _x, y = _y;
	sizing(w, h);
}

void x_focus(bool on) {
	focus(on);
}

void clear() {
	printf("crear not implemented\n");
}
void flush() {
	XRenderComposite(MESS.d, PictOpSrc, BUF, 0, P, 0, 0, 0, 0, 0, 0, w, h);
}

u32 getBG(u32 c) {
	return (c & 0xf0000) >> 16 | (c & 0xff0000) >> 12
	| (c & 0xff00000) >> 8 | (c & 0xf000000) >> 4
	| (~c & 0xf0000000) >> 0 | (~c & 0xf0000000) >> 4;
}

u32 getFG(u32 c) {
	return (c & 0xff) << 4 | c & 0xf | (c & 0xff0) << 8
	| (c & 0xf00) << 12 | (~c & 0xf000) << 16 | (~c & 0xf000) << 12;
}

void crect(int x, int y, int x1, int y1, int color) {
//	XSetForeground(MESS.d, gc, color);
//	XFillRectangle(MESS.d, window, gc, x, y, x1-5,y1-5);
//	return;
	XRenderColor C;
	xrender_color(0xff000000 | color, C);
	xrender_rect(MESS.d, BUF, &C, x, y, x1-x, y1-y);
//	printf("CPP.crect(): %i %i %i %i\n", x, y, x1, y1);
}

void simple_back_blocks(int X, int Y, 
	wchar_t* p, wchar_t* E, u32* C, xrender_font &F)
{
	while (p < E) {
		int BG = getBG(*C);
		int y1 = Y-F.ascent;
		crect(X, y1, X+F.width-1, y1 + 18, BG);
		X += F.width; p++; C++;
	}
}

void smart_back_blocks(int X, int Y, 
	wchar_t* p, wchar_t* E, u32* C, xrender_font &F)
{
	int x0 = X, N = 1;
	while (p < E) {
		int BG = getBG(C[0]);
		if (p == E - 1 || BG != getBG(C[1])) {
			int y1 = Y-F.ascent;
			crect(x0, y1, x0+N*F.width, y1+F.height, BG);
			x0 = X + F.width;
			N = 1;
		} else N++;
		X += F.width; p++; C++;
	}
}

void simple_text_blocks(int X, int Y, 
	wchar_t* p, wchar_t* E, u32* C, xrender_font &F)
{
	Picture pen = 0;
	u32 curC = !*C;
	while (p < E) {
		int FG = getFG(*C);
		if (curC != FG) {
			if (pen) XRenderFreePicture(MESS.d, pen);
			curC = FG;
			pen = xrender_create_pen(MESS.d, curC);//0xff000000 | 
		}
		xrender_print16(MESS.d, F.gs, BUF, pen, p, 1, X, Y);
		X += F.width; p++; C++;
	}
	XRenderFreePicture(MESS.d, pen);
}

void smart_text_blocks(int X, int Y, 
	wchar_t* p, wchar_t* E, u32* C, xrender_font &F)
{
	Picture pen = 0;
	int x0 = X, N = 1;
	wchar_t* p0 = p;
	while (p < E) {
		int FG = getFG(*C);
		if (p == E - 1 || FG != getFG(C[1])) {
			pen = xrender_create_pen(MESS.d, FG|0xff000000);
			xrender_print16(MESS.d, F.gs, BUF, pen, p0, N, x0, Y);
			XRenderFreePicture(MESS.d, pen);
			x0 = X + F.width;
			p0 = p + 1;
			N = 1;
		} else N++;
		X += F.width; p++; C++;
	}
}

void print_line_with_colors(wchar_t *text, u32* colors, int n, int x, int y) {
	if (BUF == 0) { printf("ZERO\n"); return; }
	if (n < 1) return;
	xrender_font &F = MESS.Fonts[0].font;
	char *loaded = F.loaded;
	int X = x, Y = y + F.ascent;
	wchar_t *p = &text[0], *E = &text[n];
	u32 *C = &colors[0];
	for (int i = 0; i < n; i++) {
		if (loaded[text[i]] == 0) load_glyph(MESS.d, F, text[i]);
	}
	smart_back_blocks(X, Y, p, E, C, F);
	p = &text[0]; X = x; C = &colors[0];
	smart_text_blocks(X, Y, p, E, C, F);
}

void print(wchar_t* p, int count, int X, int Y, u32 C = 0) {
	xrender_font &F = MESS.Fonts[0].font;
	for (int i = 0; i < count; i++) {
		if (F.loaded[p[i]] == 0) load_glyph(MESS.d, F, p[i], false);
	}
	Picture pen = 0;
//	int FG = getFG(*C);
	pen = xrender_create_pen(MESS.d, C|0xff000000);
	xrender_print16(MESS.d, F.gs, BUF, pen, p, count, X, Y);
	XRenderFreePicture(MESS.d, pen);
}


void print_TEST(wchar_t* str, int count, int x, int y) {
	u32 *C = new u32[count];
	u32 c = font_color;
	u32 mix = ((c & 0xf0) >> 4)|((c & 0xf000) >> 8)|((c & 0xf00000) >> 12);
//	mix |= 0x080;
	int i = 0;
	if (count > 25) {
		while (i < 5) C[i++] = 0x38a;
		while (i < 15) C[i++] = 0x4f0;
		while (i < 25) C[i++] = 0x0f0;
	}
	i = 0;
	if (count > 25) {
		while (i < 10) C[i++] |= 0xf000000;
		while (i < 18) C[i++] |= 0x00f0000;
		while (i < 24) C[i++] |= 0x0800000;
	}
	while (i < count) C[i++] = mix | 0xf080000;
	print_line_with_colors(str, C, count, x, y);
	delete[] C;
}
//void print(char *s, int x, int y) {
//	wstr w = utf2w(s);
//	print(w, x, y);
//}

};
#include "step.h"
