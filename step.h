char keyz[1024] = {0};

void mess::process_timers() {
	int T = time1000();
	if (nearest_timer > 0 && T < nearest_timer) return;
	nearest_timer = 0;
	each (i, timers) if (timers[i].on) {
		if (timers[i].next <= T) {
			timers[i].next = time1000() + timers[i].interval;
			timers[i].w->timer(timers[i].id);
			// remember: timer can be deleted inside the callback
			break;
		}
	}
	for (int i = !timers - 1; i >= 0; i--) {
		if (timers[i].on == false) timers.del(i, 1);
	}
	each (i, timers) {
		if (nearest_timer == 0 || timers[i].next < nearest_timer)
			nearest_timer = timers[i].next;
	}
}

void mess::step_renders() {
	each (i, MESS.all) if (MESS.all[i]->renders > 0) {
		MESS.all[i]->renders = 0;
		MESS.all[i]->render();
	}
}

bool mess::step() {
	bool physical;
	XEvent event, nev;
	win *wnd;
	if (!quit) {
		if (!XPending(d)) return false;
		XNextEvent(d, &event);

		switch(event.type) {
		case ConfigureNotify:
			wnd = handle2win(event.xconfigure.window);
			wnd->x_resize(
				event.xconfigure.x, event.xconfigure.y, 
				event.xconfigure.width, event.xconfigure.height);
			break;
		case Expose:
			wnd = handle2win(event.xexpose.window);
			wnd->x_expose();
			break;
		case CirculateNotify:
			break;
		case FocusIn:
			wnd = handle2win(event.xfocus.window);
			wnd->x_focus(true);
			break;
		case FocusOut:
			wnd = handle2win(event.xfocus.window);
			wnd->x_focus(false);
		break;
		case MotionNotify:
			while (XCheckWindowEvent(
				d, 
				event.xmotion.window, 
				PointerMotionMask, &nev))
					event.xmotion.x = nev.xmotion.x, 
					event.xmotion.y = nev.xmotion.y;
			wnd = handle2win(event.xmotion.window);
			wnd->cursor(event.xmotion.x, event.xmotion.y);
			break;
		case DestroyNotify:
			quit = true;
			wnd = handle2win(event.xdestroywindow.window);
			wnd->x_destroy();
			break;
		case GravityNotify:
			printf("gravity: %i\n", event.xgravity.x, event.xgravity.y);
			break;
		case ReparentNotify:
		case MapNotify:
			wnd = handle2win(event.xmap.window);
			wnd->paint();
			break;
		case UnmapNotify:
			break;
		case KeyPress:
				int k;
				k = event.xkey.keycode;
				physical = (keyz[k] == 0);
				keyz[k] = 1;
				if (XFilterEvent(&event, event.xkey.window)) break;
				wnd = handle2win(event.xkey.window);
				char buf[20];
				KeySym keysym; Status status;
				int count, W;
				W = 0;
				count = Xutf8LookupString(wnd->ic, 
					(XKeyPressedEvent*) &event, buf, 20, &keysym, &status);
				if (count > 0) {
					str s; s(count); move(buf, *s, count);
					wstr w = utf2w(s);
					W = w[0];
				}
				wnd->keyboard(true, W, k, physical);
			break;
		case KeyRelease:
			// filter out key-repeats
			physical = true;
			if (XPending(d)) {
				XPeekEvent(d, &nev);
				if (nev.type == KeyPress 
					&& nev.xkey.time == event.xkey.time 
					&& nev.xkey.keycode == event.xkey.keycode) {
					physical = false;
				}
			}
			if (physical) keyz[event.xkey.keycode] = 0;
			wnd = handle2win(event.xkey.window);
			wnd->keyboard(false, 0, event.xkey.keycode, physical);
			break;
		case ButtonPress:
		case ButtonRelease:
			int down, d, B, x, y;
			x = event.xbutton.x, y = event.xbutton.y;
			down = event.type == ButtonPress;
			B = -1;
			d = event.xbutton.button;
			if ((d == 4 || d == 5) && !down) break; // wheel
			if (d == 4) B = 3, down = false;
			if (d == 5) B = 3;
			if (d == 1) B = 0;
			if (d == 2) B = 2;
			if (d == 3) B = 1;
			if (d == 6) B = 20; // wheel->left/right (what?!)
			if (d == 7) B = 21;
			wnd = handle2win(event.xbutton.window);
			static int doubleClickTime, doubleClickX, doubleClickY;
			if (B == 0 && down) {
				if (doubleClickX == x 
					&& doubleClickY == y 
					&& time1000() < doubleClickTime + doubleClickSetting) {
					B = 10;
					doubleClickTime = 0;
				} else
					doubleClickTime = time1000(), 
					doubleClickX = x, doubleClickY = y;
			}
			if (B == 10) wnd->mouse(down, 0, x, y);
			//if (B == 10 && !down) break; 
			wnd->mouse(down, B, x, y);
			break;
		case KeymapNotify:
			XRefreshKeyboardMapping(&event.xmapping); // what is that?
			break;
		case VisibilityNotify:
			// do something?
			break;
		case EnterNotify:
			break;
		case LeaveNotify:
			break;
		case ClientMessage:
			if (event.xclient.data.l[0] == XA_WM_DELETE_WINDOW) {
			wnd = handle2win(event.xclient.window);
			remove(wnd);
			if (!all == 0) quit = true;
			break;
		}
		default:
			printf("Type %d\n", event.type);
			break;
		}
	}
	return true;
}
