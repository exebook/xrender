struct xrender_font {
	GlyphSet gs; FT_Face face; char *loaded;
	int width, height, ascent;
};


//typedef unsigned char byte;
void freetype2xrender(byte *b, byte *a, int W, int H) {
    byte *end = b + (W * H * 4);
    int x = 0;
    int z = W & 3;
    while(1) {
        if (b >= end) break;
        b[0] = a[2];
        b[1] = a[1];
        b[2] = a[0];
        b[3] = 0;
        a += 3;
        b += 4;
        x++;
        if (x == W) {
            x = 0;
            a += z;
        }
    }
}
int XX = 0;
void load_glyph(Display *display, xrender_font F,int charcode,bool monospace=true)
{
	if (F.loaded[charcode] == 1) { return; }
	F.loaded[charcode] = 1;

	Glyph gid;
	XGlyphInfo ginfo;
	GlyphSet gs = F.gs;
	FT_Face face = F.face;
	
	int glyph_index=FT_Get_Char_Index(face, charcode);
	FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER
		|FT_LOAD_TARGET_LCD
		|FT_LOAD_IGNORE_TRANSFORM
//		|FT_LOAD_NO_AUTOHINT
		|FT_LOAD_FORCE_AUTOHINT
	);
	FT_Bitmap *bitmap=&face->glyph->bitmap;
	ginfo.x=-face->glyph->bitmap_left;
	ginfo.y=face->glyph->bitmap_top;
	ginfo.width=bitmap->width/3;
	if (ginfo.width * 3 != bitmap->width) ginfo.width++;
	ginfo.height=bitmap->rows;
	if (monospace) ginfo.xOff=F.width;
	else ginfo.xOff=face->glyph->advance.x/64;
	/*
		using `F.width;` because
		MONOSPACE, (for some reason advance is not equal for every character.)
		face->glyph->advance.x/64;
	*/
	ginfo.yOff=face->glyph->advance.y/64;
	gid=charcode;
	int count = ginfo.width*ginfo.height*4;
	byte A[count];
	freetype2xrender(A, bitmap->buffer, ginfo.width, ginfo.height);
	XRenderAddGlyphs(display, gs, &gid, &ginfo, 1, (const char*)A, count);
	XSync(display, 0);
}

void load_glyph_1(Display *D, xrender_font F, int charcode) {
	Glyph gid;
	XGlyphInfo ginfo;
	GlyphSet gs = F.gs;
	FT_Face face = F.face;
	if (F.loaded[charcode] == 1) { return; }
	F.loaded[charcode] = 1;
	int glyph_index = FT_Get_Char_Index(face, charcode);
	FT_Load_Glyph(face, glyph_index,
		FT_LOAD_RENDER|FT_LOAD_TARGET_LIGHT|FT_RENDER_MODE_NORMAL);
	FT_Bitmap *bitmap = &face->glyph->bitmap;
	ginfo.x = -face->glyph->bitmap_left;
	ginfo.y = face->glyph->bitmap_top;
	ginfo.width = bitmap->width;
	ginfo.height = bitmap->rows;
	ginfo.xOff = face->glyph->advance.x/64;
	ginfo.yOff = face->glyph->advance.y/64;
	gid = charcode;
	int stride = (ginfo.width+3)&~3;
	char tmpbitmap[stride*ginfo.height];
	for(int y = 0; y < ginfo.height; y++)
		memcpy(tmpbitmap+y*stride, bitmap->buffer+y*ginfo.width, ginfo.width);

	XRenderAddGlyphs(D, gs, &gid, &ginfo, 1, tmpbitmap, stride*ginfo.height);
	XSync(D, 0);
}

xrender_font load_font(Display *D, char *filename, int size) {
	static int ft_lib_initialized = 0;
	static FT_Library library;
	XRenderPictFormat *fmt = XRenderFindStandardFormat(D, PictStandardARGB32);//A8);
	GlyphSet gs = XRenderCreateGlyphSet(D, fmt);
	if (!ft_lib_initialized) FT_Init_FreeType(&library);
	FT_Face face;
	FT_New_Face(library, filename, 0, &face);
	FT_Set_Char_Size(face, 0, size * 64, 90, 90);
	xrender_font fnt = {gs, face};// = new xrender_font;
	fnt.loaded = new char[0x10000];
	for (int i = 0; i < 0x10000; i++) fnt.loaded[i] = 0;
	fnt.width = (face->size->metrics.max_advance/64);
	fnt.height = face->size->metrics.height/64;
	fnt.ascent = face->size->metrics.ascender/64;
	return fnt;
}

void close_font(xrender_font font) {
	FT_Done_Face(font.face);
}

void xrender_color(unsigned int C, XRenderColor &X) {
	int r, g, b, a;
	r = C & 0xff;       r = (r <<  8) | r;
	g = C & 0xff00;     g = (g >>  8) | g;
	b = C & 0xff0000;   b = (b >> 16) | (b >> 8);
	a = C & 0xff000000; a = (a >> 16) | (a >> 24);
	X.red = r, X.green = g, X.blue = b, X.alpha = a;
}

Picture xrender_create_pen(Display *D, unsigned int color) {
	XRenderColor xcolor;
	xrender_color(color, xcolor);
	XRenderPictFormat *fmt = XRenderFindStandardFormat(D, PictStandardARGB32);
	
	Window root = DefaultRootWindow(D);
	Pixmap pm = XCreatePixmap(D, root, 1, 1, 32);
	XRenderPictureAttributes A;
	A.repeat = 1;
	Picture picture = XRenderCreatePicture(D, pm, fmt, CPRepeat, &A);
	XRenderFillRectangle(D, PictOpOver, picture, &xcolor, 0, 0, 1, 1);
	XFreePixmap(D, pm);
	return picture;
}

// XRenderFreePicture

void xrender_rect(Display *D,Picture P,XRenderColor *C,int x,int y,int w,int h) {
	XRenderFillRectangle(D, PictOpOver, P, C, x, y, w, h);
}

//void xrender_print(Display *D, GlyphSet font, Picture P, Picture pen,
//	char *txt, int count, int x,int y) {
//	XRenderCompositeString8(D, PictOpOver,
//		pen, P, 0, font, 0, 0, x, y, txt, count);
//}

void xrender_print16(Display *D, GlyphSet font, Picture P, Picture pen,
	wchar_t *txt, int count, int x,int y) {
	XRenderCompositeString16(D, PictOpOver,
		pen, P, 0, font, 0, 0, x, y, (const unsigned short*)txt, count);
}

Picture xrender_window_picture(Display *D, Drawable window) {
	XRenderPictFormat *fmt = XRenderFindStandardFormat(D, PictStandardRGB24);
	XRenderPictureAttributes A;
	A.component_alpha = true;
	A.poly_edge = PolyEdgeSmooth;
	A.poly_mode = PolyModeImprecise;
	Picture picture = XRenderCreatePicture(
		D, window, fmt, CPPolyEdge|CPPolyMode|CPComponentAlpha, &A);
//	XRenderSetPictureFilter(D, picture, FilterBilinear, 0, 0); // not needed
	return picture;
}

