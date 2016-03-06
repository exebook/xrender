struct TFont {
	xrender_font font;
	void create(str name, int size) {
		
	}
};

struct mess {
	Display *d;
	int screen;
	Window root;
	XVisualInfo *visInfo;
	XIM im;
	XContext xcontext;
	arr <win*> all;
	list <timer> timers; int nearest_timer;
	Cursor Cursors[10];
	Atom XA_WM_DELETE_WINDOW, XA_WM_NORMAL_HINTS;
   bool quit;
   list <TFont> Fonts;

	int add_font(str name, int size) {
		Fonts++;
		Fonts[-1].font = load_font(d, *name, size);
		return !Fonts - 1;
	}


	#define CHECK(value, fault, msg) \
		if (value == fault) printf("Error %s\n", msg), exit(0);
	
	void init() {
		d = XOpenDisplay(NULL);
		CHECK(d, 0, "Cannot open display");
		screen = XDefaultScreen(d);
		root = XRootWindow(d, screen);
		CHECK(root, 0, "XRootWindow returned 0");
		XIMStyles *styles;
		XIMStyle xim_requested_style;
		im = XOpenIM(d, NULL, NULL, NULL);
		CHECK(im, 0, "Could not open input method\n");
		char * failed_arg = XGetIMValues(im, XNQueryInputStyle, &styles, NULL);
		if (failed_arg) printf("XIM Can't get styles\n"), exit(0);
		Cursors[0] = XcursorLibraryLoadCursor(d, "x-cursor");
		Cursors[1] = XcursorLibraryLoadCursor(d, "xterm");
		Cursors[2] = XcursorLibraryLoadCursor(d, "v_double_arrow");
		Cursors[3] = XcursorLibraryLoadCursor(d, "move");
		Cursors[4] = XcursorLibraryLoadCursor(d, "hand1");
		Cursors[5] = XcursorLibraryLoadCursor(d, "hand2");
		Cursors[6] = XcursorLibraryLoadCursor(d, "grabbing");
		XA_WM_DELETE_WINDOW = XInternAtom(d, "WM_DELETE_WINDOW", False);
		XA_WM_NORMAL_HINTS = XInternAtom(d, "WM_NORMAL_HINTS", False);

//		XRenderSetSubpixelOrder(d, screen, 300);
	}

	mess(): quit(0), d(0), nearest_timer(0) { }
	~mess() {
		close();
	}

	bool step();
	void step_renders();
	void close();

	win* handle2win(Drawable handle) {
		win *wnd;
		XFindContext(d, handle, xcontext, (XPointer*)&wnd);
		return wnd;
	}

	void remove(win *w) {
		int i = all.find(w);
		if (i >= 0) all.del(i, 1);
	}

	void add_timer(win *w, int id, int interval) {
		del_timer(w, id);
		timers++;
		timers[-1].on = true;
		timers[-1].w = w;
		timers[-1].id = id;
		timers[-1].interval = interval;
		timers[-1].next = time1000() + interval;
		nearest_timer = 0;
	}

	void del_timer(win *w, int id) {
		each (i, timers) if (timers[i].on && timers[i].w == w && timers[i].id == id) {
			timers[i].on = false;
			break;
		}
		nearest_timer = 0;
	}

	void process_timers();
} MESS;

void mess::close() {
	if (d) {
		XFree(visInfo);
		XCloseDisplay(d);
		d = 0;
   }
}

