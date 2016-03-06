{
  "targets": [
    {
	"target_name": "x11win",
	"include_dirs": ["/usr/include/freetype2", "/usr/include/freetype2/freetype", "osnova", "extra"],
	"sources": [ "x11win.cpp" ],
	"cflags": ["-O0 -w -Wfatal-errors -fshort-wchar -g"],
	"defines": ["UNIX"],
	"libraries": ["-lfreetype", "-lv8", "-lX11", "-lGL", "-lutil", "-lXcursor"]
    }
  ],"variables" : { "clang" : 1 }
}
