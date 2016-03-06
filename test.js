// MINIMAL PROGRAM
require('dnaof')
glxwin = require('./x11win.js')

var w = TGLXWin.create(), textX = 0, textY = 0

w.setXYWH(0, 0, 300, 200)
w.onMouse = function(btn, down, c, d) {
	if (btn == 0 && down == 1) {
		textX = c/13//pixels to charpos
		textY = d/23
		w.repaint()
	}
//	var X = this.textExtent('A')
}
w.onKey = function(down, char, key, physical) {
	if (key == 9) process.exit()
	if (char) setText('char:' + char), w.repaint()
}
w.onPaint = function() {
	w.crect(0, 0, 1000, 1000, 0x8fffffff)
//	w.crect(0, 0, 30, 30, 0xffffffff)
//	w.crect(30, 30, 35, 35, 0xffffffff)
//	w.print(delme,textX,textY);
//	w.colorText(textX, textY, TXT.length, 1, TXT, CLR)
	w.print('asasdпроверка', 10, 80,0)
}
function setText(s) {
delme=s
	TXT = new Uint16Array(s.length)
	CLR = new Uint32Array(s.length)
	var colors = [0x18f, 0x184, 0xf8e, 0xf02, 0x91c, 0x33d, 0xd3f, 0x39a]
	var cx = 0
	for (var i = 0; i < s.length; i++) {
		TXT[i] = s.charCodeAt(i), CLR[i] = colors[cx++]
		if (cx >= colors.length) cx = 0
	}
}
setText('type or click')
w.show()
glxwin.mainLoop()


