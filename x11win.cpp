#define BUILDING_NODE_EXTENSION
#define fassign(name) name = Persistent<Function>::New(Handle<Function>::Cast(Handle<Object>::Cast(a[0])->Get(String::NewSymbol(#name))));

#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include "xwin.h"

using namespace v8;

Persistent<Function> onChar, onCursor, onMouse, 
onTimer, onSize, onPipe, onFocus;
Persistent<Object> glxwin, node_exports; // наверное это одно и то же
Persistent<Array> callbacks;

Persistent<Context> context;
Handle<Value> ARGS[10];
double HANDLE(void *x) { return 0 + (long)x; }
void *HANDLE(double x) { return (void*) (long) x; }
void* HANDLE(Local<Value> t) { return HANDLE(Handle<Number>::Cast(t)->Value()); }
double NUMBER(Local<Value> t) { return Handle<Number>::Cast(t)->Value(); }
wstr WSTR(Local<Value> t) {
	String::Value s(t);
	wstr w; w(s.length()); if (!w) move(*s, *w, w.occu());
	return w;
}


#define function(name) Handle<Value> name(const Arguments& a)
#include "v8util.h"
#include "v8file.h"

namespace unixstuff {
	#include <time.h>
	#include <dirent.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <sys/wait.h>
	#include <termios.h>
	#include <pty.h>
	#include <stdlib.h>
	#include <sys/types.h>
}

Handle<Value> js_call(Handle<Function> f, int arg_count) {
	TryCatch t;
	Handle<Value> R = f->Call(glxwin, arg_count, ARGS);
	if (R.IsEmpty()) {
		printf("%s\n", *v8_error_as_string(& t).compat());
	}
	return R;
}

struct inherit(v8win, win) {
	v8win() { }
	~v8win() { }
	on_created { }

	on_timer {
		if (id == 1000) {
			return;
		}
		HandleScope handle_scope;
		ARGS[0] = Number::New(HANDLE(this));
		ARGS[1] = Number::New(id);
		js_call(onTimer, 2);
	}
	on_key {
		HandleScope handle_scope;
		Handle<Object> B = Object::New();
		B->Set(String::New("call"), String::New("onKey"));
		B->Set(String::New("handle"), Number::New(HANDLE(this)));
		B->Set(String::New("down"), Boolean::New(down));
		if (charcode != 0) B->Set(
			String::New("char"),
			String::New((uint16_t*)&charcode, 1));
		B->Set(String::New("key"), Integer::New(key));
		B->Set(String::New("physical"), Boolean::New(physical));
		int len = callbacks->Length();
		callbacks->Set(len, B);
	}
	on_paint {
		Handle<Object> B = Object::New();
		B->Set(String::New("call"), String::New("onPaint"));
		B->Set(String::New("handle"), Number::New(HANDLE(this)));
		int len = callbacks->Length();
		callbacks->Set(len, B);
	}

	on_mouse {
		HandleScope handle_scope;
		ARGS[0] = Number::New(HANDLE(this));
		ARGS[1] = Number::New(button);
		ARGS[2] = Number::New(down);
		ARGS[3] = Number::New(x);
		ARGS[4] = Number::New(y);
		js_call(onMouse, 5);
	}
	on_cursor {
		HandleScope handle_scope;
		ARGS[0] = Number::New(HANDLE(this));
		ARGS[1] = Number::New(x);
		ARGS[2] = Number::New(y);
		js_call(onCursor, 3);
	}
	on_focus {
		HandleScope handle_scope;
		ARGS[0] = Number::New(HANDLE(this));
		ARGS[1] = Boolean::New(on);
		js_call(onFocus, 2);
	}
	on_size {
		HandleScope handle_scope;
		ARGS[0] = Number::New(HANDLE(this));
		ARGS[1] = Number::New(w);
		ARGS[2] = Number::New(h);
		js_call(onSize, 3);
	}
};


//struct bool wchar_t sizeof u32 arr list str wstr word

function (color_text_new) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	int x = NUMBER(a[1]);
	int y = NUMBER(a[2]);
	int w = NUMBER(a[3]);
	int h = NUMBER(a[4]);
	word *T = (word*)node::Buffer::Data(a[5]), *T0 = (word*)T;
	u32 *C = (u32*)node::Buffer::Data(a[6]), *C0 = C;
	xrender_font &F = MESS.Fonts[W->curfont].font;
	for (int i = 0; i < h; i++) {
		W->print_line_with_colors((wchar_t*)T, C, w, 
			x*F.width, i*F.height+y*F.height);
		T += w;
		C += w;
	}
	return Undefined();
}

function (create_win) {
	HandleScope handle_scope;
	v8win *parent = (v8win*) HANDLE(a[0]);
	v8win *W = new v8win;
	wstr fontName = "/home/ya/Jil.ttf";
	int fontSize = 18;
	if (a[1] != Undefined()) fontName = WSTR(a[1]);
	if (a[2] != Undefined()) fontSize = NUMBER(a[2]);
	W->create();
	W->curfont = MESS.add_font(fontName, fontSize);
	return Number::New(HANDLE(W));
}

function (step) {
	HandleScope handle_scope;
	bool result = MESS.step();
	return Boolean::New(result);
}

function (step_renders) {
	HandleScope handle_scope;
	MESS.step_renders();
	return Undefined();
}

function (show) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	W->show();
	return Undefined();
}

function (hide) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	W->hide();
	return Undefined();
}

function (repaint) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	W->paint();
	return Undefined();
}

function (force_repaint) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	W->render();
	return Undefined();
}

function (print) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	wstr txt = WSTR(a[1]);
	int x = NUMBER(a[2]);
	int y = NUMBER(a[3]);
	int color = NUMBER(a[4]);
	W->print(*txt, !txt, x, y, color);
	return Undefined();
}

function (text_extent) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	wstr txt = WSTR(a[1]);
	int *N = W->text_extent(txt);
	Handle<Array> A = Array::New(2);
	A->Set(0, Integer::New(N[0]));
	A->Set(1, Integer::New(N[1]));
	return handle_scope.Close(A);
}

function (crect) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	int x = NUMBER(a[1]);
	int y = NUMBER(a[2]);
	int x1 = NUMBER(a[3]);
	int y1 = NUMBER(a[4]);
	unsigned int color = NUMBER(a[5]);
	W->crect(x, y, x1, y1, color);
	return Undefined();
}

function(set_xywh) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	if (a[1] != Undefined()) {
		double x = NUMBER(a[1]);
		double y = NUMBER(a[2]);
		W->move(x, y);
	}
	if (a[3] != Undefined()) {
		double w = NUMBER(a[3]);
		double h = NUMBER(a[4]);
		W->size(w, h);
	}
	return Undefined();
}

function(get_xywh) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	Handle<Array> A = Array::New(4);
	A->Set(0, Integer::New(W->x));// TODO: fix this
	A->Set(1, Integer::New(W->y));
	A->Set(2, Integer::New(W->w));
	A->Set(3, Integer::New(W->h));
	return handle_scope.Close(A);
}

function(font_color) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	unsigned int color = NUMBER(a[1]);
	W->font_color = color | 0xff000000;
	return Undefined();
}

function(apply_font) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	wstr name = WSTR(a[1]);
	int size = NUMBER(a[2]);
	int color = NUMBER(a[3]);
	int bgcolor = NUMBER(a[4]);
	Handle<Array> A = Array::New(3);
	int *N = W->text_extent('A');
	A->Set(0, Integer::New(N[0]));
	A->Set(1, Integer::New(N[1]));
	A->Set(2, Integer::New(W->curfont));
	return handle_scope.Close(A);
}

function(get_xwindow_handle) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	return Number::New((int)(W->window));
}

function (paintBegin) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
//	printf("paintBegin not implemented\n");
	return Undefined();
}

function (paintEnd) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	W->flush();
	return Undefined();
}

Handle<Array> convert_time(tm *m) {
	using namespace unixstuff;
	Handle<Array> C = Array::New(6);
	int j = 0;
	C->Set(j++, Integer::New(m->tm_year+1900));
	C->Set(j++, Integer::New(m->tm_mon + 1));
	C->Set(j++, Integer::New(m->tm_mday));
	C->Set(j++, Integer::New(m->tm_hour));
	C->Set(j++, Integer::New(m->tm_min));
	C->Set(j++, Integer::New(m->tm_sec));
	return C;
}

function(register_callbacks) {
	glxwin = Persistent<Object>::New(Handle<Object>::Cast(a[0]));
	HandleScope scope;
//	fassign(onPaint);
	fassign(onCursor);
	fassign(onMouse);
//	fassign(onPaint);
//	fassign(onKey);
	fassign(onTimer);
	fassign(onChar);
	fassign(onSize);
	fassign(onPipe);
	fassign(onFocus);
	return scope.Close(Undefined());
}

Handle<Value> Method(const Arguments& args) {
  HandleScope scope;
  return scope.Close(String::New("GLXWIN!"));
}

function(setCursor) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	int cursor = NUMBER(a[1]);
	W->setCursor(cursor);
	return Undefined();
}

function (setSizeSteps) {
	HandleScope handle_scope;
	v8win *W = (v8win*) HANDLE(a[0]);
	int w = NUMBER(a[1]);
	int h = NUMBER(a[2]);
	W->setSizeSteps(w, h);
	return Undefined();
}

function (x11quit) {
	return Boolean::New(MESS.quit);
}

function (native_readdir) {
	HandleScope handle_scope;
	using namespace unixstuff;
	DIR *dp;
	dirent *dirp; struct stat t;
	str path = w2utf(WSTR(a[0]));
	if ((dp  = opendir(*path)) == NULL) {
		Handle<Array> A = Array::New(0);
		Handle<Object> B = Object::New();
		int j = 0;
		B->Set(String::New("name"), String::New(strerror(errno)));
		B->Set(String::New("hint"), Boolean::New(true));
		A->Set(0, B);
		return handle_scope.Close(A);
	}
	Handle<Array> A = Array::New(0);
	int i = 0;
	if (path[-1] != '/') path/'/';
	while ((dirp = readdir(dp)) != NULL) {
		str s = dirp->d_name;
		if (s == "." || s == "..") continue;
		str f = path + s;
		stat(*f, &t);
		str p = "----------"; int pp = t.st_mode;
		if ((pp & 0400) != 0) p[0] = 'r';
		if ((pp & 0200) != 0) p[1] = 'w';
		if ((pp & 0100) != 0) p[2] = 'x';
		if ((pp & 0040) != 0) p[3] = 'r';
		if ((pp & 0020) != 0) p[4] = 'w';
		if ((pp & 0010) != 0) p[5] = 'x';
		if ((pp & 0004) != 0) p[6] = 'r';
		if ((pp & 0002) != 0) p[7] = 'w';
		if ((pp & 0001) != 0) p[8] = 'x';
		Handle<Object> B = Object::New();
		B->Set(String::New("name"), String::New(dirp->d_name));
		B->Set(String::New("size"), Number::New(t.st_size));
		B->Set(String::New("mode"), Integer::New(t.st_mode & 0xfff));
		B->Set(String::New("flags"), String::New(*p, !p));
		B->Set(String::New("dir"), Boolean::New(S_ISDIR(t.st_mode)));
		B->Set(String::New("ctime"), convert_time(localtime(&t.st_ctime)));
		B->Set(String::New("mtime"), convert_time(localtime(&t.st_mtime)));
		B->Set(String::New("atime"), convert_time(localtime(&t.st_atime)));
		A->Set(i++, B);
	}
	closedir(dp);
	return handle_scope.Close(A);
}

void init(Handle<Object> exports) {
	node_exports = Persistent<Object>::New(Handle<Object>::Cast(exports));
	exports->Set(String::NewSymbol("hello"), FunctionTemplate::New(Method)->GetFunction());
	callbacks = Persistent<Array>::New(Handle<Array>::Cast(Array::New(0)));
	exports->Set(String::New("c++callbacks"), callbacks);

//      context->Global()
	#define function(name) exports->Set(String::NewSymbol(#name), FunctionTemplate::New(name)->GetFunction());
	function(register_callbacks)
	function(native_readdir)
	function(v8_exit)

	function(get_xwindow_handle)
	function(create_win)
	function(show)
	function(hide)
	function(step)
	function(step_renders)
	function(x11quit)
	function(print)
	function(repaint)
	function(force_repaint)
	function(text_extent)
	function(crect)
	function(set_xywh)
	function(get_xywh)
	function(font_color)
	function(apply_font)
	function(color_text_new)
	function (paintBegin)
	function (paintEnd)
	function (setCursor)
	function (setSizeSteps)
}

NODE_MODULE(x11win, init)
