/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

/* macros */
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X)) + lrpad)

/* patch - definitions */
#define OPAQUE                  1.0

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum { SchemeBar, SchemeSelect, SchemeBorder, SchemeFocus, SchemeUnfocus, SchemeTag }; /* color schemes */
enum { NetSupported, NetWMName, NetWMState, NetWMCheck,
       NetWMFullscreen, NetActiveWindow, NetWMWindowType,
       NetWMWindowTypeDialog, NetClientList, NetLast }; /* EWMH atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast }; /* clicks */

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;
struct Client {
	char name[256];
	float mina, maxa;
	float cfact;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	unsigned int tags;
	int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
	Client *next;
	Client *snext;
	Monitor *mon;
	Window win;
};

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
} Layout;

typedef struct Pertag Pertag;

struct Monitor {
	char ltsymbol[16];
	int ltaxis[3];
	float mfact;
	int nmaster;
	int num;
	int by;               /* bar geometry */
	int eby;              /* extrabar geometry */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	int gappx;            /* gaps between windows */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	int showebar;
	int topbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
	Window barwin;
	Window ebarwin;
	const Layout *lt[2];
	Pertag *pertag;
};

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	int isfloating;
	int monitor;
} Rule;

/* function declarations */
static void applyrules(Client *c);
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void attach(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clientmessage(XEvent *e);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static Monitor *createmon(void);
static void destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
static Monitor *dirtomon(int dir);
static void drawbar(Monitor *m, int xpos);
static void drawbars(void);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void focus(Client *c);
static void focusin(XEvent *e);
static void focusmon(const Arg *arg);
static void focusstack(const Arg *arg);
static Atom getatomprop(Client *c, Atom prop);
static int getrootptr(int *x, int *y);
static long getstate(Window w);
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
static void incnmaster(const Arg *arg);
static void keypress(XEvent *e);
static void killclient(const Arg *arg);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void monocle(Monitor *m);
static void motionnotify(XEvent *e);
static void movemouse(const Arg *arg);
static Client *nexttiled(Client *c);
static void pop(Client *);
static void propertynotify(XEvent *e);
static void quit(const Arg *arg);
static Monitor *recttomon(int x, int y, int w, int h);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizemouse(const Arg *arg);
static void restack(Monitor *m);
static void run(void);
static void scan(void);
static int sendevent(Client *c, Atom proto);
static void sendmon(Client *c, Monitor *m);
static void setclientstate(Client *c, long state);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
static void setlayout(const Arg *arg);
static void setmfact(const Arg *arg);
static void setup(void);
static void seturgent(Client *c, int urg);
static void showhide(Client *c);
static void sigchld(int unused);
static void spawn(const Arg *arg);
static void tag(const Arg *arg);
static void tagmon(const Arg *arg);
static void tile(Monitor *);
static void togglebar(const Arg *arg);
static void togglefloating(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);
static void unfocus(Client *c, int setfocus);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(XEvent *e);
static void updatebarpos(Monitor *m);
static void updatebars(void);
static void updateclientlist(void);
static int updategeom(void);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatestatus(void);
static void updatetitle(Client *c);
static void updatewindowtype(Client *c);
static void updatewmhints(Client *c);
static void view(const Arg *arg);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static void zoom(const Arg *arg);

/* patch - function declarations */
static void drawebar(char *text, Monitor *m, int xpos);
static void drawtheme(int x, int s, int status, int theme);
static void drawtaggrid(Monitor *m, int *x_pos, unsigned int occ);
static void mirrorlayout(const Arg *arg);
static void rotatelayoutaxis(const Arg *arg);
static void setgaps(const Arg *arg);
static void setcfact(const Arg *arg);
static void switchtag(const Arg *arg);
static void toggleebar(const Arg *arg);
static void togglebars(const Arg *arg);
static void xinitvisual();

/* variables */
static const char broken[] = "broken";
static char stext[256];
static int screen;
static int sw, sh;								/* X display screen geometry width, height */
static int bh, blw, tgw, sep, gap;				/* bar geometry */
static int lrpad;								/* sum of left and right padding for text */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ClientMessage] = clientmessage,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};
static Atom wmatom[WMLast], netatom[NetLast];
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon;
static Window root, wmcheckwin;

/* patch - variables */
unsigned int fsep = 0, fblock = 0;			/* focused bar item */

static int depth;
static int useargb = 0;
static Colormap cmap;
static Visual *visual;

/* configuration, allows nested code to access above variables */
#include "config.h"

struct Pertag {
	int ltaxes[LENGTH(tags) + 1][3];
	unsigned int curtag, prevtag;				/* current and previous tag */
	int nmasters[LENGTH(tags) + 1];				/* number of windows in master area */
	float mfacts[LENGTH(tags) + 1];				/* mfacts per tag */
	unsigned int sellts[LENGTH(tags) + 1];		/* selected layouts */
	const Layout *ltidxs[LENGTH(tags) + 1][2];	/* matrix of tags and layouts indexes  */
	int showbars[LENGTH(tags) + 1];				/* display bar for the current tag */
	int showebars[LENGTH(tags) + 1];			/* display ebar for the current tag */
};

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };

/* function implementations */
void
applyrules(Client *c)
{
	const char *class, *instance;
	unsigned int i;
	const Rule *r;
	Monitor *m;
	XClassHint ch = { NULL, NULL };

	/* rule matching */
	c->isfloating = 0;
	c->tags = 0;
	XGetClassHint(dpy, c->win, &ch);
	class    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;

	for (i = 0; i < LENGTH(rules); i++) {
		r = &rules[i];
		if ((!r->title || strstr(c->name, r->title))
		&& (!r->class || strstr(class, r->class))
		&& (!r->instance || strstr(instance, r->instance)))
		{
			c->isfloating = r->isfloating;
			c->tags |= r->tags;
			for (m = mons; m && m->num != r->monitor; m = m->next);
			if (m)
				c->mon = m;
		}
	}
	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);
	c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
}

int
applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact)
{
	int baseismin;
	Monitor *m = c->mon;

	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);
	if (interact) {
		if (*x > sw)
			*x = sw - WIDTH(c);
		if (*y > sh)
			*y = sh - HEIGHT(c);
		if (*x + *w + 2 * c->bw < 0)
			*x = 0;
		if (*y + *h + 2 * c->bw < 0)
			*y = 0;
	} else {
		if (*x >= m->wx + m->ww)
			*x = m->wx + m->ww - WIDTH(c);
		if (*y >= m->wy + m->wh)
			*y = m->wy + m->wh - HEIGHT(c);
		if (*x + *w + 2 * c->bw <= m->wx)
			*x = m->wx;
		if (*y + *h + 2 * c->bw <= m->wy)
			*y = m->wy;
	}
	if (*h < bh)
		*h = bh;
	if (*w < bh)
		*w = bh;
	if (resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;
		if (!baseismin) { /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for aspect limits */
		if (c->mina > 0 && c->maxa > 0) {
			if (c->maxa < (float)*w / *h)
				*w = *h * c->maxa + 0.5;
			else if (c->mina < (float)*h / *w)
				*h = *w * c->mina + 0.5;
		}
		if (baseismin) { /* increment calculation requires this */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for increment value */
		if (c->incw)
			*w -= *w % c->incw;
		if (c->inch)
			*h -= *h % c->inch;
		/* restore base dimensions */
		*w = MAX(*w + c->basew, c->minw);
		*h = MAX(*h + c->baseh, c->minh);
		if (c->maxw)
			*w = MIN(*w, c->maxw);
		if (c->maxh)
			*h = MIN(*h, c->maxh);
	}
	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void
arrange(Monitor *m)
{
	if (m)
		showhide(m->stack);
	else for (m = mons; m; m = m->next)
		showhide(m->stack);
	if (m) {
		arrangemon(m);
		restack(m);
	} else for (m = mons; m; m = m->next)
		arrangemon(m);
}

void
arrangemon(Monitor *m)
{
	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
	if (m->lt[m->sellt]->arrange)
		m->lt[m->sellt]->arrange(m);
}

void
attach(Client *c)
{
	c->next = c->mon->clients;
	c->mon->clients = c;
}

void
attachstack(Client *c)
{
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

/* buttonpress-functions */

int buttontag(Monitor *m, int x, int xpos, int ypos, int click, Arg *arg) {
	Client *c;
	unsigned int columns, i = 0, occ = 0;

	columns = LENGTH(tags) / tagrows + ((LENGTH(tags) % tagrows > 0) ? 1 : 0);

	if (drawtagmask & DRAWCLASSICTAGS) {
		for (c = m->clients; c; c = c->next)
			occ |= c->tags == 255 ? 0 : c->tags;
		do {
			/* do not reserve space for vacant tags */
			if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i))
				continue;
			x += TEXTW(tags[i]);
		} while (xpos > x && ++i < LENGTH(tags));
		if(i < LENGTH(tags)) {
			click = ClkTagBar;
			arg->ui = 1 << i;
		}
	} else if (xpos < x + columns * bh / tagrows && (drawtagmask & DRAWTAGGRID)) {
		click = ClkTagBar;
		i = (xpos - x) / (bh / tagrows);
		i = i + columns * (ypos / (bh / tagrows));
		if (i >= LENGTH(tags))
			i = LENGTH(tags) - 1;
		arg->ui = 1 << i;
	}
	return click;
}

void
buttonpress(XEvent *e)
{
	Arg arg = {0};
	Client *c;
	Monitor *m;
	XButtonPressedEvent *ev = &e->xbutton;

	/* focus monitor if necessary */
	if ((m = wintomon(ev->window)) && m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
		focus(NULL);
	}
	int len, i, lr = 0, l = 0, r = m->ww, set = 0, pos = 1;
	int click = ClkRootWin;

	if (ev->window == selmon->barwin) {
		len = sizeof(barorder)/sizeof(barorder[0]);

		for (i = 0; set == 0 && i < len && i >= 0; pos == 1 ? i++ : i--) {
			if (strcmp ("bartab", barorder[i]) == 0) {
				if (pos == 1) {pos = -1; i = len - 1; l = lr; lr = r; }
				else {r = lr; break;}
			}
			if (strcmp ("tagbar", barorder[i]) == 0) {
				if (pos * ev->x < pos * (lr + pos * tgw)) {click = buttontag(m, lr - (pos < 0 ? tgw : 0), ev->x, ev->y, click, &arg); set = 1; }
				else lr = lr + pos * tgw;
			} else if (strcmp ("ltsymbol", barorder[i]) == 0) {
				if (pos * ev->x < pos * (lr + pos * blw)) { click = ClkLtSymbol; set = 1; }
				else lr = lr + pos * blw;
			} else if (strcmp ("seperator", barorder[i]) == 0) {
				if (pos * ev->x < pos * (lr + pos * sep)) return;
				else lr = lr + pos * sep;
			} else if (strcmp ("gap", barorder[i]) == 0) {
				if (pos * ev->x < pos * (lr + pos * gap)) return;
				else lr = lr + pos * gap;
			} else if (strcmp ("sepgap", barorder[i]) == 0) {
				if (pos * ev->x < pos * (lr + pos * gap)) return;
				else lr = lr + pos * gap;
			}
		}
		if (set == 0 && ev->x > l && ev->x < r)
			click = ClkWinTitle;

	} else if (ev->window == selmon->ebarwin) {
		len = sizeof(ebarorder)/sizeof(ebarorder[0]);
		for (i = 0; set == 0 && i < len && i >= 0; pos == 1 ? i++ : i--) {
			if (strcmp ("status", ebarorder[i]) == 0) {
				if (pos == 1) {pos = -1; i = len - 1; l = lr; lr = r; }
				else {r = lr; break;}
			}
			if (strcmp ("tagbar", ebarorder[i]) == 0) {
				if (pos * ev->x < pos * (lr + pos * tgw)) {click = buttontag(m, lr - (pos < 0 ? tgw : 0), ev->x, ev->y, click, &arg); set = 1; }
				else lr = lr + pos * tgw;
			} else if (strcmp ("ltsymbol", ebarorder[i]) == 0) {
				if (pos * ev->x < pos * (lr + pos * blw)) { click = ClkLtSymbol; set = 1; }
				else lr = lr + pos * blw;
			} else if (strcmp ("seperator", ebarorder[i]) == 0) {
				if (pos * ev->x < pos * (lr + pos * sep)) return;
				else lr = lr + pos * sep;
			} else if (strcmp ("gap", ebarorder[i]) == 0) {
				if (pos * ev->x < pos * (lr + pos * gap)) return;
				else lr = lr + pos * gap;
			} else if (strcmp ("sepgap", ebarorder[i]) == 0) {
				if (pos * ev->x < pos * (lr + pos * gap)) return;
				else lr = lr + pos * gap;
			}
		}
		if (set == 0 && ev->x > l && ev->x < r)
			click = ClkStatusText;

	} else if ((c = wintoclient(ev->window))) {
		focus(c);
		restack(selmon);
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
	}
	for (i = 0; i < LENGTH(buttons); i++)
		if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
		&& CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
}

void
checkotherwm(void)
{
	xerrorxlib = XSetErrorHandler(xerrorstart);
	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
cleanup(void)
{
	Arg a = {.ui = ~0};
	Layout foo = { "", NULL };
	Monitor *m;
	size_t i;

	view(&a);
	selmon->lt[selmon->sellt] = &foo;
	for (m = mons; m; m = m->next)
		while (m->stack)
			unmanage(m->stack, 0);
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	XUnmapWindow(dpy, m->ebarwin);
	XDestroyWindow(dpy, m->ebarwin);
	while (mons)
		cleanupmon(mons);
	for (i = 0; i < CurLast; i++)
		drw_cur_free(drw, cursor[i]);
	for (i = 0; i < LENGTH(colors) + 1; i++)
		free(scheme[i]);
	free(scheme);
	XDestroyWindow(dpy, wmcheckwin);
	drw_free(drw);
	XSync(dpy, False);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}

void
cleanupmon(Monitor *mon)
{
	Monitor *m;

	if (mon == mons)
		mons = mons->next;
	else {
		for (m = mons; m && m->next != mon; m = m->next);
		m->next = mon->next;
	}
	XUnmapWindow(dpy, mon->barwin);
	XUnmapWindow(dpy, mon->ebarwin);
	XDestroyWindow(dpy, mon->barwin);
	XDestroyWindow(dpy, mon->ebarwin);
	free(mon->pertag);
	free(mon);
}

void
clientmessage(XEvent *e)
{
	XClientMessageEvent *cme = &e->xclient;
	Client *c = wintoclient(cme->window);

	if (!c)
		return;
	if (cme->message_type == netatom[NetWMState]) {
		if (cme->data.l[1] == netatom[NetWMFullscreen]
		|| cme->data.l[2] == netatom[NetWMFullscreen])
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
				|| (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));
	} else if (cme->message_type == netatom[NetActiveWindow]) {
		if (c != selmon->sel && !c->isurgent)
			seturgent(c, 1);
	}
}

void
configure(Client *c)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void
configurenotify(XEvent *e)
{
	Monitor *m;
	Client *c;
	XConfigureEvent *ev = &e->xconfigure;
	int dirty;

	/* TODO: updategeom handling sucks, needs to be simplified */
	if (ev->window == root) {
		dirty = (sw != ev->width || sh != ev->height);
		sw = ev->width;
		sh = ev->height;
		if (updategeom() || dirty) {
			drw_resize(drw, sw, bh);
			updatebars();
			for (m = mons; m; m = m->next) {
				for (c = m->clients; c; c = c->next)
					if (c->isfullscreen)
						resizeclient(c, m->mx, m->my, m->mw, m->mh);
				XMoveResizeWindow(dpy, m->barwin, m->wx, m->by, m->ww, bh);
				XMoveResizeWindow(dpy, m->ebarwin, m->wx, m->eby, m->ww, bh);
			}
			focus(NULL);
			arrange(NULL);
		}
	}
}

void
configurerequest(XEvent *e)
{
	Client *c;
	Monitor *m;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if ((c = wintoclient(ev->window))) {
		if (ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
			m = c->mon;
			if (ev->value_mask & CWX) {
				c->oldx = c->x;
				c->x = m->mx + ev->x;
			}
			if (ev->value_mask & CWY) {
				c->oldy = c->y;
				c->y = m->my + ev->y;
			}
			if (ev->value_mask & CWWidth) {
				c->oldw = c->w;
				c->w = ev->width;
			}
			if (ev->value_mask & CWHeight) {
				c->oldh = c->h;
				c->h = ev->height;
			}
			if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
				c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
			if ((c->y + c->h) > m->my + m->mh && c->isfloating)
				c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
			if ((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
				configure(c);
			if (ISVISIBLE(c))
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		} else
			configure(c);
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}
	XSync(dpy, False);
}

Monitor *
createmon(void)
{
	Monitor *m;
	unsigned int i;

	m = ecalloc(1, sizeof(Monitor));
	m->tagset[0] = m->tagset[1] = 1;
	m->mfact = mfact;
	m->nmaster = nmaster;
	m->showbar = showbar;
	m->showebar = showebar;
	m->topbar = topbar;
	m->gappx = gappx;
	m->lt[0] = &layouts[0];
	m->lt[1] = &layouts[1 % LENGTH(layouts)];
	strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
	m->ltaxis[0] = layoutaxis[0];
	m->ltaxis[1] = layoutaxis[1];
	m->ltaxis[2] = layoutaxis[2];
	m->pertag = ecalloc(1, sizeof(Pertag));
	m->pertag->curtag = m->pertag->prevtag = 1;
	/* init tags, bars, layouts, axes, nmasters, and mfacts */
	for (i = 0; i <= LENGTH(tags); i++) {
		m->pertag->nmasters[i] = m->nmaster;
		m->pertag->mfacts[i] = m->mfact;
		m->pertag->ltaxes[i][0] = m->ltaxis[0];
		m->pertag->ltaxes[i][1] = m->ltaxis[1];
		m->pertag->ltaxes[i][2] = m->ltaxis[2];
		m->pertag->ltidxs[i][0] = m->lt[0];
		m->pertag->ltidxs[i][1] = m->lt[1];
		m->pertag->sellts[i] = m->sellt;
		m->pertag->showbars[i] = m->showbar;
		m->pertag->showebars[i] = m->showebar;
	}
	return m;
}

void
destroynotify(XEvent *e)
{
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if ((c = wintoclient(ev->window)))
		unmanage(c, 1);
}

void
detach(Client *c)
{
	Client **tc;

	for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
	*tc = c->next;
}

void
detachstack(Client *c)
{
	Client **tc, *t;

	for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;

	if (c == c->mon->sel) {
		for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext);
		c->mon->sel = t;
	}
}

Monitor *
dirtomon(int dir)
{
	Monitor *m = NULL;

	if (dir > 0) {
		if (!(m = selmon->next))
			m = mons;
	} else if (selmon == mons)
		for (m = mons; m->next; m = m->next);
	else
		for (m = mons; m->next != selmon; m = m->next);
	return m;
}

/* drawbar-functions */

int drawsep(Monitor *m, int lr, int p, int xpos, int s) {
	int len, sp, dot = 0;

	dot = seppad < 0 ? 1 : 0;
	sp = dot ? (seppad < -lrpad/2 ? lrpad/2 : -seppad) : (seppad > bh/2 ? bh/2 : seppad);

	if (s < 2)
		len = sep = dot ? sp : 1;
	else
		len = gap = lrpad/2 + (s == 3 ? dot ? sp : 1 : 0);

	int x = p ? m->ww - len - lr : lr;

	if (xpos && xpos > x && xpos <= x + len) {
		fsep = x;
		fblock = len;
	}
	if (s) {
		XSetForeground(drw->dpy, drw->gc, scheme[SchemeBar][bartheme ? ColFloat : ColBg].pixel);
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, 0, len, bh);
	}
	XSetForeground(drw->dpy, drw->gc, scheme[SchemeBar][bartheme ? ColBg : ColFloat].pixel);
	if (s != 2) {
		if (dot)
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + (s == 3 ? lrpad/4 : s == 0 ? -sp/2 : 0), bh/2 - sp/2, sp, sp);
		else
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + (s == 3 ? lrpad/4 : s == 0 ? -sp/2 : 0), sp, 1, bh - 2 * sp);
	}

	return lr + len;
}

int drawtag(Monitor *m, int lr, int p, int xpos) {
	tgw = lr;
	int indn, i, x, w = 0;
	unsigned int occ = 0, urg = 0;
	Client *c;

	for (c = m->clients; c; c = c->next) {
		occ |= c->tags == 255 ? 0 : c->tags;
		if (c->isurgent)
			urg |= c->tags;
	}
	if (drawtagmask & DRAWCLASSICTAGS) {
		for (i = p ? LENGTH(tags) - 1 : 0; p ? i >= 0 : i < LENGTH(tags); p ? i-- : i++) {
			/* do not draw vacant tags */
			if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i))
				continue;
			indn = 0;
			w = TEXTW(tags[i]);
			x = p ? m->ww - lr - w : lr;

			if (xpos && xpos > x && xpos <= x + w) {
				fsep = x;
				fblock = w;
			}
			if (m->tagset[m->seltags] & 1 << i)
				drawtheme(0,0,3,tagtheme);
			else if (x == fsep && w == fblock && w)
				drawtheme(0,0,2,tagtheme);
			else
				drawtheme(0,0,1,tagtheme);
			int yy = ((m->tagset[m->seltags] & 1 << i) || (x == fsep && w == fblock)) ? 0 : (bartheme && tagtheme) ? -1 : 0;
			drw_text(drw, x, yy, w, bh, lrpad / 2, tags[i], urg & 1 << i);
			if (bartheme) {
				if(m->tagset[m->seltags] & 1 << i)
					drawtheme(x, w, 3, tagtheme);
				else if (x != fsep || w != fblock)
					drawtheme(x, w, 1, tagtheme);
				else
					drawtheme(x, w, 2, tagtheme);
			}
			for (c = m->clients; c; c = c->next) {
				if ((c->tags & (1 << i)) && (indn * 3 + 2 < bh)) {
					drw_rect(drw, x + ((bartheme && tagtheme > 1) ? 2 : 1), indn * 3 + 2 + yy, selmon->sel == c ? 5 : 2, 2, 1, urg & 1 << i);
					indn++;
				}
			}
			lr = lr + w;
		}
	}
	if (drawtagmask & DRAWTAGGRID) {
		w = bh / tagrows * (LENGTH(tags) / tagrows + ((LENGTH(tags) % tagrows > 0) ? 1 : 0));
		x = p ? m->ww - lr - w : lr;
		if (xpos && xpos > x && xpos <= x + w) {
			fsep = x;
			fblock = w;
		}
		drawtaggrid(m,&x,occ);
		lr = lr + w;
	}
	tgw = lr - tgw;
	return lr;
}

void drawstatus(char* stext, Monitor *m, int l, int r, int xpos) {
	int saw = TEXTW(stext) - lrpad + 2; /* 2px right padding */
	int x = l;
	int w = w = m->ww - l - r;

	if (xpos && xpos > x && xpos <= x + saw) {
		fsep = x;
		fblock = saw;
	}
	if (m == selmon) { /* status is only drawn on selected monitor */
		if (x == fsep && saw == fblock && saw)
			drawtheme(0,0,2,statustheme);
		else
			drawtheme(0,0,0,0);
		drw_text(drw, x, 0, saw, bh, 0, stext, 0);
		drawtheme(0,0,0,0);
		drw_rect(drw, x + saw, 0, w - saw, bh, 1, 1);
	}
}

int drawltsymbol(Monitor *m, int lr, int p, int xpos) {
	blw = TEXTW(m->ltsymbol);
	int x = p ? m->ww - blw - lr : lr;

	if (xpos && xpos > x && xpos <= x + blw) {
		fsep = x;
		fblock = blw;
	}
	if (x == fsep && blw == fblock && blw)
		drawtheme(0,0,2,tagtheme);
	else
		drawtheme(0,0,0,0);
	drw_text(drw, x, 0, blw, bh, lrpad / 2, m->ltsymbol, 0);

	return lr + blw;
}

void drawbartab(Monitor *m, int l, int r, int xpos) {
	int w, x = l;
	int boxs = drw->fonts->h / 9;
	int boxw = drw->fonts->h / 6 + 2;

	if ((w = m->ww - l - r) > bh) {
		if (m->sel) {
			if (m == selmon)
				drawtheme(0,0,3,tabbartheme);
			else
				drawtheme(0,0,0,0);
			drw_text(drw, x, 0, w, bh, lrpad / 2, m->sel->name, 0);
			if (m->sel->isfloating)
				drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
		} else {
			drawtheme(0,0,0,0);
			drw_rect(drw, x, 0, w, bh, 1, 1);
		}
	}
	if (xpos && xpos > x && xpos <= x + w) {
		fsep = x;
		fblock = w;
	}
}

void
drawbar(Monitor *m, int xpos)
{
	int i, len, l = 0, r = 0, lr = 0, pos = 0;

	len = sizeof(barorder)/sizeof(barorder[0]);

	if (!m->showbar)
		return;

	for (i = 0; i < len && i >= 0; pos == 1 ? i-- : i++) {
		if (strcmp ("bartab", barorder[i]) == 0) {
			if (pos == 0) {pos = 1; i = len - 1; l = lr; lr = 0; }
			else {r = lr; break;}
		}
		if (strcmp ("tagbar", barorder[i]) == 0)
			lr = drawtag(m, lr, pos, xpos);
		if (strcmp ("ltsymbol", barorder[i]) == 0)
			lr = drawltsymbol(m, lr, pos, xpos);
		if (strcmp ("sepgap", barorder[i]) == 0)
			lr = drawsep(m, lr, pos, xpos, 3);
		if (strcmp ("gap", barorder[i]) == 0)
			lr = drawsep(m, lr, pos, xpos, 2);
		if (strcmp ("seperator", barorder[i]) == 0)
			lr = drawsep(m, lr, pos, xpos, 1);
	}
	drawbartab(m, l, r, xpos);
	drw_map(drw, m->barwin, 0, 0, m->ww, bh);
}

void
drawebar(char* stext, Monitor *m, int xpos)
{
	int i, len, l = 0, r = 0, lr = 0, pos = 0;

	len = sizeof(ebarorder)/sizeof(ebarorder[0]);

	if (!m->showebar)
		return;

	for (i = 0; i < len && i >= 0; pos == 1 ? i-- : i++) {
		if (strcmp ("status", ebarorder[i]) == 0) {
			if (pos == 0) {pos = 1; i = len - 1; l = lr; lr = 0; }
			else {r = lr; break;}
		}
		if (strcmp ("tagbar", ebarorder[i]) == 0)
			lr = drawtag(m, lr, pos, xpos);
		if (strcmp ("ltsymbol", ebarorder[i]) == 0)
			lr = drawltsymbol(m, lr, pos, xpos);
		if (strcmp ("sepgap", ebarorder[i]) == 0)
			lr = drawsep(m, lr, pos, xpos, 3);
		if (strcmp ("gap", ebarorder[i]) == 0)
			lr = drawsep(m, lr, pos, xpos, 2);
		if (strcmp ("seperator", ebarorder[i]) == 0)
			lr = drawsep(m, lr, pos, xpos, 1);
	}
	drawstatus(stext, m, l, r, xpos);
	drw_map(drw, m->ebarwin, 0, 0, m->ww, bh);
}

void
drawbars(void)
{
	Monitor *m;

	for (m = mons; m; m = m->next) {
		drawbar(m, 0);
		drawebar(stext, m, 0);
	}
}

void
enternotify(XEvent *e)
{
	Client *c;
	Monitor *m;
	XCrossingEvent *ev = &e->xcrossing;

	fblock = fsep = 0;
	if (ev->window == selmon->barwin)
		drawebar(stext, selmon, 0);
	else if (ev->window == selmon->ebarwin)
		drawbar(selmon, 0);
	else {
		drawebar(stext, selmon, 0);
		drawbar(selmon, 0);
	}

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	c = wintoclient(ev->window);
	m = c ? c->mon : wintomon(ev->window);
	if (m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
	} else if (!c || c == selmon->sel)
		return;
	focus(c);
}

void
expose(XEvent *e)
{
	Monitor *m;
	XExposeEvent *ev = &e->xexpose;

	if (ev->count == 0 && (m = wintomon(ev->window))) {
		drawbar(m, 0);
		drawebar(stext, m, 0);
	}
}

void
focus(Client *c)
{
	if (!c || !ISVISIBLE(c))
		for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext);
	if (selmon->sel && selmon->sel != c)
		unfocus(selmon->sel, 0);
	if (c) {
		if (c->mon != selmon)
			selmon = c->mon;
		if (c->isurgent)
			seturgent(c, 0);
		detachstack(c);
		attachstack(c);
		grabbuttons(c, 1);
		XSetWindowBorder(dpy, c->win, scheme[SchemeBorder][ColFg].pixel);
		setfocus(c);
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
	selmon->sel = c;
	if (selmon->lt[selmon->sellt]->arrange == tile || selmon->lt[selmon->sellt]->arrange == monocle)
		arrangemon(selmon);
	drawbars();
}

/* there are some broken focus acquiring clients needing extra handling */
void
focusin(XEvent *e)
{
	XFocusChangeEvent *ev = &e->xfocus;

	if (selmon->sel && ev->window != selmon->sel->win)
		setfocus(selmon->sel);
}

void
focusmon(const Arg *arg)
{
	Monitor *m;

	if (!mons->next)
		return;
	if ((m = dirtomon(arg->i)) == selmon)
		return;
	unfocus(selmon->sel, 0);
	selmon = m;
	focus(NULL);
}

void
focusstack(const Arg *arg)
{
	Client *c = NULL, *i;

	if (!selmon->sel || (selmon->sel->isfullscreen && lockfullscreen))
		return;
	if (arg->i > 0) {
		for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
		if (!c)
			for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
	} else {
		for (i = selmon->clients; i != selmon->sel; i = i->next)
			if (ISVISIBLE(i))
				c = i;
		if (!c)
			for (; i; i = i->next)
				if (ISVISIBLE(i))
					c = i;
	}
	if (c) {
		focus(c);
		restack(selmon);
	}
}

Atom
getatomprop(Client *c, Atom prop)
{
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM,
		&da, &di, &dl, &dl, &p) == Success && p) {
		atom = *(Atom *)p;
		XFree(p);
	}
	return atom;
}

int
getrootptr(int *x, int *y)
{
	int di;
	unsigned int dui;
	Window dummy;

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long
getstate(Window w)
{
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
		&real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return -1;
	if (n != 0)
		result = *p;
	XFree(p);
	return result;
}

int
gettextprop(Window w, Atom atom, char *text, unsigned int size)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	if (!text || size == 0)
		return 0;
	text[0] = '\0';
	if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems)
		return 0;
	if (name.encoding == XA_STRING)
		strncpy(text, (char *)name.value, size - 1);
	else {
		if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
			strncpy(text, *list, size - 1);
			XFreeStringList(list);
		}
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return 1;
}

void
grabbuttons(Client *c, int focused)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if (!focused)
			XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
				BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
		for (i = 0; i < LENGTH(buttons); i++)
			if (buttons[i].click == ClkClientWin)
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabButton(dpy, buttons[i].button,
						buttons[i].mask | modifiers[j],
						c->win, False, BUTTONMASK,
						GrabModeAsync, GrabModeSync, None, None);
	}
}

void
grabkeys(void)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		KeyCode code;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		for (i = 0; i < LENGTH(keys); i++)
			if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
						True, GrabModeAsync, GrabModeAsync);
	}
}

void
incnmaster(const Arg *arg)
{
	unsigned int n;
	Client *c;

	for(n = 0, c = nexttiled(selmon->clients); c; c = nexttiled(c->next), n++);
	if(!arg || !selmon->lt[selmon->sellt]->arrange || selmon->nmaster + arg->i < 1 || selmon->nmaster + arg->i > n)
		return;
	selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag] = MAX(selmon->nmaster + arg->i, 0);
	arrange(selmon);
}

#ifdef XINERAMA
static int
isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
	while (n--)
		if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
		&& unique[n].width == info->width && unique[n].height == info->height)
			return 0;
	return 1;
}
#endif /* XINERAMA */

void
keypress(XEvent *e)
{
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;

	if (fblock || fsep) {
		fblock = fsep = 0;
		drawbar(selmon, 0);
		drawebar(stext, selmon, 0);
	}
	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for (i = 0; i < LENGTH(keys); i++)
		if (keysym == keys[i].keysym
		&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
		&& keys[i].func)
			keys[i].func(&(keys[i].arg));
}

void
killclient(const Arg *arg)
{
	if (!selmon->sel)
		return;
	if (!sendevent(selmon->sel, wmatom[WMDelete])) {
		XGrabServer(dpy);
		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, selmon->sel->win);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
}

void
manage(Window w, XWindowAttributes *wa)
{
	Client *c, *t = NULL;
	Window trans = None;
	XWindowChanges wc;

	c = ecalloc(1, sizeof(Client));
	c->win = w;
	/* geometry */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;
	c->cfact = 1.0;

	updatetitle(c);
	if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
		c->mon = t->mon;
		c->tags = t->tags;
	} else {
		c->mon = selmon;
		applyrules(c);
	}

	if (c->x + WIDTH(c) > c->mon->mx + c->mon->mw)
		c->x = c->mon->mx + c->mon->mw - WIDTH(c);
	if (c->y + HEIGHT(c) > c->mon->my + c->mon->mh)
		c->y = c->mon->my + c->mon->mh - HEIGHT(c);
	c->x = MAX(c->x, c->mon->mx);
	/* only fix client y-offset, if the client center might cover the bar */
	c->y = MAX(c->y, ((c->mon->by == c->mon->my) && (c->x + (c->w / 2) >= c->mon->wx)
		&& (c->x + (c->w / 2) < c->mon->wx + c->mon->ww)) ? bh : c->mon->my);
	c->bw = borderpx;

	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, scheme[SchemeBorder][c->isfloating ? ColFloat : ColBorder].pixel);
	configure(c); /* propagates border_width, if size doesn't change */
	updatewindowtype(c);
	updatesizehints(c);
	updatewmhints(c);
	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grabbuttons(c, 0);
	if (!c->isfloating)
		c->isfloating = c->oldstate = t || c->isfixed;
	if (c->isfloating) {
		XRaiseWindow(dpy, c->win);
		XSetWindowBorder(dpy, w, scheme[SchemeBorder][ColFloat].pixel);
	}
	attach(c);
	attachstack(c);
	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
		(unsigned char *) &(c->win), 1);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
	setclientstate(c, NormalState);
	if (c->mon == selmon)
		unfocus(selmon->sel, 0);
	c->mon->sel = c;
	arrange(c->mon);
	XMapWindow(dpy, c->win);
	focus(NULL);
}

void
mappingnotify(XEvent *e)
{
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard)
		grabkeys();
}

void
maprequest(XEvent *e)
{
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &e->xmaprequest;

	if (!XGetWindowAttributes(dpy, ev->window, &wa))
		return;
	if (wa.override_redirect)
		return;
	if (!wintoclient(ev->window))
		manage(ev->window, &wa);
}

void
monocle(Monitor *m)
{
	unsigned int n = 0;
	Client *c;

	for (c = m->clients; c; c = c->next)
		if (ISVISIBLE(c))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (c = m->stack; c && (!ISVISIBLE(c) || c->isfloating); c = c->snext);
	if (c && !c->isfloating) {
		XMoveWindow(dpy, c->win, m->wx + m->gappx - c->bw, m->wy + m->gappx - (abs(m->showbar) == 0 ? c->bw : 0));
		resize(c, m->wx + m->gappx - c->bw, m->wy + m->gappx - (abs(m->showbar) == 0 ? c->bw : 0), m->ww - 2 * m->gappx, m->wh - 2 * m->gappx, 0);
		c = c->snext;
	}
	for (; c; c = c->snext)
		if (!c->isfloating && ISVISIBLE(c))
			XMoveWindow(dpy, c->win, - 2 * WIDTH(c), c->y);
}

void
motionnotify(XEvent *e)
{
	static Monitor *mon = NULL;
	Monitor *m;
	XMotionEvent *ev = &e->xmotion;

	if (ev->window == selmon->ebarwin || ev->window == selmon->barwin) {
		if (ev->x < fsep || ev->x > fsep + fblock) {
			fblock = fsep = 0;
			if (ev->window == selmon->ebarwin)
				drawebar(stext, selmon, ev->x);
			else
				drawbar(selmon, ev->x);
		} else
			return;
	}

	if (ev->window != root)
		return;
	if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
		unfocus(selmon->sel, 1);
		selmon = m;
		focus(NULL);
	}
	mon = m;
}

void
movemouse(const Arg *arg)
{
	int x, y, ocx, ocy, nx, ny;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;
	if (c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if (abs(selmon->wx - nx) < snap)
				nx = selmon->wx;
			else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
				nx = selmon->wx + selmon->ww - WIDTH(c);
			if (abs(selmon->wy - ny) < snap)
				ny = selmon->wy;
			else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
				ny = selmon->wy + selmon->wh - HEIGHT(c);
			if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
			&& (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
				togglefloating(NULL);
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, nx, ny, c->w, c->h, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

Client *
nexttiled(Client *c)
{
	for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
	return c;
}

void
pop(Client *c)
{
	detach(c);
	attach(c);
	focus(c);
	arrange(c->mon);
}

void
propertynotify(XEvent *e)
{
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if ((ev->window == root) && (ev->atom == XA_WM_NAME))
		updatestatus();
	else if (ev->state == PropertyDelete)
		return; /* ignore */
	else if ((c = wintoclient(ev->window))) {
		switch(ev->atom) {
		default: break;
		case XA_WM_TRANSIENT_FOR:
			if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
				(c->isfloating = (wintoclient(trans)) != NULL))
				arrange(c->mon);
			break;
		case XA_WM_NORMAL_HINTS:
			updatesizehints(c);
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			drawbars();
			break;
		}
		if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if (c == c->mon->sel)
				drawbar(c->mon, 0);
		}
		if (ev->atom == netatom[NetWMWindowType])
			updatewindowtype(c);
	}
}

void
quit(const Arg *arg)
{
	running = 0;
}

Monitor *
recttomon(int x, int y, int w, int h)
{
	Monitor *m, *r = selmon;
	int a, area = 0;

	for (m = mons; m; m = m->next)
		if ((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}
	return r;
}

void
resize(Client *c, int x, int y, int w, int h, int interact)
{
	if (applysizehints(c, &x, &y, &w, &h, interact))
		resizeclient(c, x, y, w, h);
}

void
resizeclient(Client *c, int x, int y, int w, int h)
{
	XWindowChanges wc;

	c->oldx = c->x; c->x = wc.x = x;
	c->oldy = c->y; c->y = wc.y = y;
	c->oldw = c->w; c->w = wc.width = w;
	c->oldh = c->h; c->h = wc.height = h;
	wc.border_width = c->bw;
	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}

void
resizemouse(const Arg *arg)
{
	int ocx, ocy, nw, nh;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;
	if (c->isfullscreen) /* no support resizing fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
			if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
			&& c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
			{
				if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
				&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

void
restack(Monitor *m)
{
	Client *c;
	XEvent ev;
	XWindowChanges wc;

	drawbar(m, 0);
	drawebar(stext, m, 0);
	if (!m->sel)
		return;
	if (m->sel->isfloating || !m->lt[m->sellt]->arrange)
		XRaiseWindow(dpy, m->sel->win);
	if (m->lt[m->sellt]->arrange) {
		wc.stack_mode = Below;
		wc.sibling = m->barwin;
		for (c = m->stack; c; c = c->snext)
			if (!c->isfloating && ISVISIBLE(c)) {
				XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
				wc.sibling = c->win;
			}
	}
	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
run(void)
{
	XEvent ev;
	/* main event loop */
	XSync(dpy, False);
	while (running && !XNextEvent(dpy, &ev))
		if (handler[ev.type])
			handler[ev.type](&ev); /* call handler */
}

void
scan(void)
{
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
				manage(wins[i], &wa);
		}
		for (i = 0; i < num; i++) { /* now the transients */
			if (!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if (XGetTransientForHint(dpy, wins[i], &d1)
			&& (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
				manage(wins[i], &wa);
		}
		if (wins)
			XFree(wins);
	}
}

void
sendmon(Client *c, Monitor *m)
{
	if (c->mon == m)
		return;
	unfocus(c, 1);
	detach(c);
	detachstack(c);
	c->mon = m;
	c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
	attach(c);
	attachstack(c);
	focus(NULL);
	arrange(NULL);
}

void
setclientstate(Client *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
		PropModeReplace, (unsigned char *)data, 2);
}

int
sendevent(Client *c, Atom proto)
{
	int n;
	Atom *protocols;
	int exists = 0;
	XEvent ev;

	if (XGetWMProtocols(dpy, c->win, &protocols, &n)) {
		while (!exists && n--)
			exists = protocols[n] == proto;
		XFree(protocols);
	}
	if (exists) {
		ev.type = ClientMessage;
		ev.xclient.window = c->win;
		ev.xclient.message_type = wmatom[WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, c->win, False, NoEventMask, &ev);
	}
	return exists;
}

void
setfocus(Client *c)
{
	if (!c->neverfocus) {
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dpy, root, netatom[NetActiveWindow],
			XA_WINDOW, 32, PropModeReplace,
			(unsigned char *) &(c->win), 1);
	}
	sendevent(c, wmatom[WMTakeFocus]);
}

void
setfullscreen(Client *c, int fullscreen)
{
	if (fullscreen && !c->isfullscreen) {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
			PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);
		c->isfullscreen = 1;
		c->oldstate = c->isfloating;
		c->oldbw = c->bw;
		c->bw = 0;
		c->isfloating = 1;
		resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
		XRaiseWindow(dpy, c->win);
	} else if (!fullscreen && c->isfullscreen){
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
			PropModeReplace, (unsigned char*)0, 0);
		c->isfullscreen = 0;
		c->isfloating = c->oldstate;
		c->bw = c->oldbw;
		c->x = c->oldx;
		c->y = c->oldy;
		c->w = c->oldw;
		c->h = c->oldh;
		resizeclient(c, c->x, c->y, c->w, c->h);
		arrange(c->mon);
	}
}

void
setlayout(const Arg *arg)
{
	if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
		selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag] ^= 1;
	if (arg && arg->v)
		selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt] = (Layout *)arg->v;
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
	if (selmon->sel)
		arrange(selmon);
	else {
		drawbar(selmon, 0);
		drawebar(stext, selmon, 0);
	}
	arrangemon(selmon);
}

/* arg > 1.0 will set mfact absolutely */
void
setmfact(const Arg *arg)
{
	float f;

	if (!arg || !selmon->lt[selmon->sellt]->arrange)
		return;
	f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
	if (f < 0.05 || f > 0.95)
		return;
	selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag] = f;
	arrange(selmon);
}

void
setup(void)
{
	int i;
	XSetWindowAttributes wa;
	Atom utf8string;

	/* clean up any zombies immediately */
	sigchld(0);

	/* init screen */
	screen = DefaultScreen(dpy);
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	root = RootWindow(dpy, screen);
	xinitvisual();
	drw = drw_create(dpy, screen, root, sw, sh, visual, depth, cmap);
	if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");
	lrpad = drw->fonts->h;
	bh = drw->fonts->h + 2;
	updategeom();
	/* init atoms */
	utf8string = XInternAtom(dpy, "UTF8_STRING", False);
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
	netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	/* init cursors */
	cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
	cursor[CurResize] = drw_cur_create(drw, XC_sizing);
	cursor[CurMove] = drw_cur_create(drw, XC_fleur);
	/* init appearance */
	scheme = ecalloc(LENGTH(colors) + 1, sizeof(Clr *));
	scheme[LENGTH(colors)] = drw_scm_create(drw, colors[0], alphas[0], 4);
	for (i = 0; i < LENGTH(colors); i++)
		scheme[i] = drw_scm_create(drw, colors[i], alphas[i], 4);
	/* init bars */
	updatebars();
	updatestatus();
	/* supporting window for NetWMCheck */
	wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
		PropModeReplace, (unsigned char *) "dwm", 3);
	XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) netatom, NetLast);
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	/* select events */
	wa.cursor = cursor[CurNormal]->cursor;
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
		|ButtonPressMask|PointerMotionMask|EnterWindowMask
		|LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);
	grabkeys();
	focus(NULL);
}


void
seturgent(Client *c, int urg)
{
	XWMHints *wmh;

	c->isurgent = urg;
	if (!(wmh = XGetWMHints(dpy, c->win)))
		return;
	wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
	XSetWMHints(dpy, c->win, wmh);
	XFree(wmh);
}

void
showhide(Client *c)
{
	if (!c)
		return;
	if (ISVISIBLE(c)) {
		/* show clients top down */
		XMoveWindow(dpy, c->win, c->x, c->y);
		if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && !c->isfullscreen)
			resize(c, c->x, c->y, c->w, c->h, 0);
		showhide(c->snext);
	} else {
		/* hide clients bottom up */
		showhide(c->snext);
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
	}
}

void
sigchld(int unused)
{
	if (signal(SIGCHLD, sigchld) == SIG_ERR)
		die("can't install SIGCHLD handler:");
	while (0 < waitpid(-1, NULL, WNOHANG));
}

void
spawn(const Arg *arg)
{
	if (arg->v == dmenucmd)
		dmenumon[0] = '0' + selmon->num;
	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(((char **)arg->v)[0], (char **)arg->v);
		fprintf(stderr, "dwm: execvp %s", ((char **)arg->v)[0]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}
}

void
tag(const Arg *arg)
{
	if (selmon->sel && arg->ui & TAGMASK) {
		selmon->sel->tags = arg->ui & TAGMASK;
		focus(NULL);
		arrange(selmon);
	}
}

void
tagmon(const Arg *arg)
{
	if (!selmon->sel || !mons->next)
		return;
	sendmon(selmon->sel, dirtomon(arg->i));
}

void
tile(Monitor *m)
{
	char sym1 = 61, sym2 = 93, sym3 = 61, sym;
	int x1 = m->wx + m->gappx, y1 = m->wy + m->gappx, h1 = m->wh - m->gappx, w1 = m->ww - m->gappx, X1 = x1 + w1, Y1 = y1 + h1;
	int x2 = m->wx + m->gappx, y2 = m->wy + m->gappx, h2 = m->wh - m->gappx, w2 = m->ww - m->gappx, X2 = x2 + w2, Y2 = y2 + h2;
	unsigned int i, n, n1, n2, bw;
	Client *c, *s, *d, *t, *o;
	float mfacts = 0, sfacts = 0;

	bw = borderpx;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++) {
		if (n < m->nmaster)
			mfacts += c->cfact;
		else
			sfacts += c->cfact;
	}
	if(m->nmaster > n)
		m->nmaster = (n == 0) ? 1 : n;
	/* layout symbol */
	if(abs(m->ltaxis[0]) == m->ltaxis[1])    /* explicitly: ((abs(m->ltaxis[0]) == 1 && m->ltaxis[1] == 1) || (abs(m->ltaxis[0]) == 2 && m->ltaxis[1] == 2)) */
		sym1 = 124;
	if(abs(m->ltaxis[0]) == m->ltaxis[2])
		sym3 = 124;
	if(m->ltaxis[1] == 3)
		sym1 = (n == 0) ? 0 : m->nmaster;
	if(m->ltaxis[2] == 3)
		sym3 = (n == 0) ? 0 : n - m->nmaster;
	if(m->ltaxis[0] < 0) {
		sym = sym1;
		sym1 = sym3;
		sym2 = 91;
		sym3 = sym;
	}
	if(m->nmaster == 1) {
		if(m->ltaxis[0] > 0)
			sym1 = 91;
		else
			sym3 = 93;
	}
	if(m->nmaster > 1 && m->ltaxis[1] == 3 && m->ltaxis[2] == 3)
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "%d%c%d", sym1, sym2, sym3);
	else if((m->nmaster > 1 && m->ltaxis[1] == 3 && m->ltaxis[0] > 0) || (m->ltaxis[2] == 3 && m->ltaxis[0] < 0))
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "%d%c%c", sym1, sym2, sym3);
	else if((m->ltaxis[2] == 3 && m->ltaxis[0] > 0) || (m->nmaster > 1 && m->ltaxis[1] == 3 && m->ltaxis[0] < 0))
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "%c%c%d", sym1, sym2, sym3);
	else
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "%c%c%c", sym1, sym2, sym3);
	if (n == 0)
		return;

	/* master and stack area */
	if(abs(m->ltaxis[0]) == 1 && n > m->nmaster) {
		w1 *= m->mfact;
		w2 -= w1;
		x1 += (m->ltaxis[0] < 0) ? w2 : 0;
		x2 += (m->ltaxis[0] < 0) ? 0 : w1;
	} else if(abs(m->ltaxis[0]) == 2 && n > m->nmaster) {
		h1 *= m->mfact;
		h2 -= h1;
		y1 += (m->ltaxis[0] < 0) ? h2 : 0;
		y2 += (m->ltaxis[0] < 0) ? 0 : h1;
	}

	if(m->gappx == 0) {
		if(abs(m->showbar) + abs(m->showebar) == 0) {
			y1 -= topbar ? borderpx : 0;	h1 += borderpx;
			y2 -= topbar ? borderpx : 0;	h2 += borderpx;
		}
		if(abs(m->ltaxis[0]) == 1 && n > m->nmaster) {
			h1 += borderpx;			h2 += borderpx;
			w1 += borderpx;			w2 += borderpx;
			if(m->ltaxis[0] < 0)	x2 -= borderpx;
			else					x1 -= borderpx;
			if(m->topbar == 0) {	y1 -= borderpx;	y2 -= borderpx; }
		}
		if(abs(m->ltaxis[0]) == 2 && n > m->nmaster) {
			w1 += 2 * borderpx;		w2 += 2 * borderpx;
			x1 -= borderpx;			x2 -= borderpx;
			if(m->topbar == 0) {	h1 += borderpx;	y1 -= borderpx; }
			else					h2 += borderpx;
		}
		if(n == 1) {
			h1 += borderpx;			h2 += borderpx;
			w1 += 2 * borderpx;		w2 += 2 * borderpx;
			x2 -= borderpx;			x1 -= borderpx;
			if(m->topbar == 0) {	y1 -= borderpx;	y2 -= borderpx; }
		}
	}

	X1 = x1 + w1; X2 = x2 + w2; Y1 = y1 + h1; Y2 = y2 + h2;

	/* master */
	n1 = (m->ltaxis[1] != 1 || w1 < (bh + m->gappx + 2 * borderpx) * (m->nmaster + 1)) ? 1 : m->nmaster;
	n2 = (m->ltaxis[1] != 2 || h1 < (bh + m->gappx + 2 * borderpx) * (m->nmaster + 1)) ? 1 : m->nmaster;
	for(i = 0, o = c = nexttiled(m->clients); i < m->nmaster; o = c = nexttiled(c->next), i++) {
		resize(c, x1, y1,
			(m->ltaxis[1] == 1 && i + 1 == m->nmaster) ? X1 - x1 - 2 * bw - m->gappx : w1 * (n1 > 1 ? (c->cfact / mfacts) : 1) - 2 * bw - m->gappx,
			(m->ltaxis[1] == 2 && i + 1 == m->nmaster) ? Y1 - y1 - 2 * bw - m->gappx : h1 * (n2 > 1 ? (c->cfact / mfacts) : 1) - 2 * bw - m->gappx,
			0);
		if(n1 > 1)
			x1 = c->x + WIDTH(c) + m->gappx;
		if(n2 > 1)
			y1 = c->y + HEIGHT(c) + m->gappx;
	}
	if(m->ltaxis[1] == 3) {
		for(i = 0, o = d = nexttiled(m->clients); i < m->nmaster; o = d = nexttiled(d->next), i++) {
			XMoveWindow(dpy, d->win, WIDTH(d) * -2, d->y);
		}
		for (t = m->stack; t; t = t->snext) {
			if (!ISVISIBLE(t) || t->isfloating)
				continue;
			for (i = 0, d = nexttiled(m->clients); d && d != t; d = nexttiled(d->next), i++);
			if (i >= m->nmaster)
				continue;
			XMoveWindow(dpy, t->win, x1, y1);
//			resize(t, x1, y1, w1 - 2 * bw - m->gappx, h1 - 2 * bw - m->gappx, 0);
			break;
		}
	}

	/* stack */
	if(n > m->nmaster) {
		n1 = (m->ltaxis[2] != 1 || w2 < (bh + m->gappx + 2 * borderpx) * (n - m->nmaster + 1)) ? 1 : n - m->nmaster;
		n2 = (m->ltaxis[2] != 2 || h2 < (bh + m->gappx + 2 * borderpx) * (n - m->nmaster + 1)) ? 1 : n - m->nmaster;
			for(i = 0, c = o; c; c = nexttiled(c->next), i++) {
				resize(c, x2, y2, 
					(m->ltaxis[2] == 1 && i + 1 == n - m->nmaster) ? X2 - x2 - 2 * bw - m->gappx : w2 * (n1 > 1 ? (c->cfact / sfacts) : 1) - 2 * bw - m->gappx, 
					(m->ltaxis[2] == 2 && i + 1 == n - m->nmaster) ? Y2 - y2 - 2 * bw - m->gappx : h2 * (n2 > 1 ? (c->cfact / sfacts) : 1) - 2 * bw - m->gappx,
					0);
				if(n1 > 1)
					x2 = c->x + WIDTH(c) + m->gappx;
				if(n2 > 1)
					y2 = c->y + HEIGHT(c) + m->gappx;
			}
		if(m->ltaxis[2] == 3) {
			for(i = 0, c = o; c; c = nexttiled(c->next), i++)
				XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
			for (s = m->stack; s; s = s->snext) {
				if (!ISVISIBLE(s) || s->isfloating)
					continue;
				for (i = 0, c = nexttiled(m->clients); c && c != s; c = nexttiled(c->next), i++);
				if (i < m->nmaster)
					continue;
				resize(c, x2, y2, w2 - 2 * bw - m->gappx, h2 - 2 * bw - m->gappx, 0);
				XMoveWindow(dpy, c->win, x2, y2);
				break;
			}
		}
	}
}

void
togglebar(const Arg *arg)
{
	selmon->showbar = selmon->pertag->showbars[selmon->pertag->curtag] = !selmon->showbar;
	updatebarpos(selmon);
	XMoveWindow(dpy, selmon->barwin, selmon->wx, selmon->by);
	XMoveWindow(dpy, selmon->ebarwin, selmon->wx, selmon->eby);
	arrange(selmon);
}

void
toggleebar(const Arg *arg)
{
	selmon->showebar = selmon->pertag->showebars[selmon->pertag->curtag] = !selmon->showebar;
    updatebarpos(selmon);
	XMoveWindow(dpy, selmon->barwin, selmon->wx, selmon->by);
	XMoveWindow(dpy, selmon->ebarwin, selmon->wx, selmon->eby);
    arrange(selmon);
}

void
togglebars(const Arg *arg)
{
	if((selmon->showbar + selmon->showebar == 2))
		toggleebar(NULL);
	else if((selmon->showbar + selmon->showebar == 1))
		togglebar(NULL);
	else if((selmon->showbar + selmon->showebar == 0)) {
		togglebar(NULL);
		toggleebar(NULL);
	}
}

void
togglefloating(const Arg *arg)
{
	if (!selmon->sel)
		return;
	if (selmon->sel->isfullscreen) /* no support for fullscreen windows */
		return;
	selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
	if (selmon->sel->isfloating)
		resize(selmon->sel, selmon->sel->x, selmon->sel->y,
			selmon->sel->w, selmon->sel->h, 0);
	arrange(selmon);
	arrangemon(selmon);
}

void
toggletag(const Arg *arg)
{
	unsigned int i, newtags;

	if (!selmon->sel)
		return;
	newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
	if (newtags) {
		selmon->sel->tags = newtags;
		if(newtags == ~0) {
			selmon->pertag->prevtag = selmon->pertag->curtag;
			selmon->pertag->curtag = 0;
		}
		if(!(newtags & 1 << (selmon->pertag->curtag - 1))) {
			selmon->pertag->prevtag = selmon->pertag->curtag;
			for (i = 0; !(newtags & 1 << i); i++); /* get first new tag */
			selmon->pertag->curtag = i + 1;
		}
		selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
		selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];
		selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
		selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
		selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
		selmon->ltaxis[0] = selmon->pertag->ltaxes[selmon->pertag->curtag][0];
		selmon->ltaxis[1] = selmon->pertag->ltaxes[selmon->pertag->curtag][1];
		selmon->ltaxis[2] = selmon->pertag->ltaxes[selmon->pertag->curtag][2];
		if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
			togglebar(NULL);
		focus(NULL);
		arrange(selmon);
	}
}

void
toggleview(const Arg *arg)
{
	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);
	int i;

	if (newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;
		if (newtagset == ~0) {
			selmon->pertag->prevtag = selmon->pertag->curtag;
			selmon->pertag->curtag = 0;
		}
		/* test if the user did not select the same tag */
		if (!(newtagset & 1 << (selmon->pertag->curtag - 1))) {
			selmon->pertag->prevtag = selmon->pertag->curtag;
			for (i = 0; !(newtagset & 1 << i); i++) ;
			selmon->pertag->curtag = i + 1;
		}
		/* apply settings for this view */
		selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
		selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
		selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
		selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
		selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];
		if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
			togglebar(NULL);
		if (selmon->showebar != selmon->pertag->showebars[selmon->pertag->curtag])
			toggleebar(NULL);
		focus(NULL);
		arrange(selmon);
	}
}

void
unfocus(Client *c, int setfocus)
{
	if (!c)
		return;
	grabbuttons(c, 0);
	XSetWindowBorder(dpy, c->win, scheme[SchemeBorder][c->isfloating ? ColFloat : ColBorder].pixel);
	if (setfocus) {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
}

void
unmanage(Client *c, int destroyed)
{
	Monitor *m = c->mon;
	XWindowChanges wc;

	detach(c);
	detachstack(c);
	if (!destroyed) {
		wc.border_width = c->oldbw;
		XGrabServer(dpy); /* avoid race conditions */
		XSetErrorHandler(xerrordummy);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
	free(c);
	focus(NULL);
	updateclientlist();
	arrange(m);
}

void
unmapnotify(XEvent *e)
{
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if ((c = wintoclient(ev->window))) {
		if (ev->send_event)
			setclientstate(c, WithdrawnState);
		else
			unmanage(c, 0);
	}
}

void
updatebars(void)
{
	Monitor *m;
	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixel = 0,
		.border_pixel = 0,
		.colormap = cmap,
		.event_mask = ButtonPressMask|ExposureMask|PointerMotionMask|EnterWindowMask
	};
	XClassHint ch = {"dwm", "dwm"};
	for (m = mons; m; m = m->next) {
		if (!m->barwin) {
			m->barwin = XCreateWindow(dpy, root, m->wx, m->by, m->ww, bh, 0, depth,
									InputOutput, visual,
									CWOverrideRedirect|CWBackPixel|CWBorderPixel|CWColormap|CWEventMask, &wa);
			XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
			XMapRaised(dpy, m->barwin);
			XSetClassHint(dpy, m->barwin, &ch);
		}
        if (!m->ebarwin) {
			m->ebarwin = XCreateWindow(dpy, root, m->wx, m->eby, m->ww, bh, 0, depth,
									InputOutput, visual,
									CWOverrideRedirect|CWBackPixel|CWBorderPixel|CWColormap|CWEventMask, &wa);
			XDefineCursor(dpy, m->ebarwin, cursor[CurNormal]->cursor);
			XMapRaised(dpy, m->ebarwin);
			XSetClassHint(dpy, m->ebarwin, &ch);
		}
	}
}

void
updatebarpos(Monitor *m)
{
	m->wy = m->my;
	m->wh = m->mh;
	if ((m->showbar) && (m->showebar)){
		m->wh -= 2 * bh;
		m->wy = m->topbar ? m->wy + 2 * bh : m->wy;
		m->by = m->topbar ? m->wy - bh : m->wy + m->wh;
		m->eby =  m->topbar ? m->wy - 2 * bh : m->wy + m->wh + bh;
	} else if ((m->showbar) && !(m->showebar)) {
		m->wh -= bh;
		m->wy = m->topbar ? m->wy + bh: m->wy;
		m->by = m->topbar ? m->wy - bh : m->wy + m->wh;
		m->eby = - bh;
	} else if (!(m->showbar) && (m->showebar)) {
		m->wh -= bh;
		m->wy = m->topbar ? m->wy + bh: m->wy;
		m->eby = m->topbar ? m->wy - bh : m->wy + m->wh;
		m->by = - bh;
	} else {
		m->eby = - bh;
		m->by = - bh;
    }
}

void
updateclientlist()
{
	Client *c;
	Monitor *m;

	XDeleteProperty(dpy, root, netatom[NetClientList]);
	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			XChangeProperty(dpy, root, netatom[NetClientList],
				XA_WINDOW, 32, PropModeAppend,
				(unsigned char *) &(c->win), 1);
}

int
updategeom(void)
{
	int dirty = 0;

#ifdef XINERAMA
	if (XineramaIsActive(dpy)) {
		int i, j, n, nn;
		Client *c;
		Monitor *m;
		XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
		XineramaScreenInfo *unique = NULL;

		for (n = 0, m = mons; m; m = m->next, n++);
		/* only consider unique geometries as separate screens */
		unique = ecalloc(nn, sizeof(XineramaScreenInfo));
		for (i = 0, j = 0; i < nn; i++)
			if (isuniquegeom(unique, j, &info[i]))
				memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
		XFree(info);
		nn = j;
		if (n <= nn) { /* new monitors available */
			for (i = 0; i < (nn - n); i++) {
				for (m = mons; m && m->next; m = m->next);
				if (m)
					m->next = createmon();
				else
					mons = createmon();
			}
			for (i = 0, m = mons; i < nn && m; m = m->next, i++)
				if (i >= n
				|| unique[i].x_org != m->mx || unique[i].y_org != m->my
				|| unique[i].width != m->mw || unique[i].height != m->mh)
				{
					dirty = 1;
					m->num = i;
					m->mx = m->wx = unique[i].x_org;
					m->my = m->wy = unique[i].y_org;
					m->mw = m->ww = unique[i].width;
					m->mh = m->wh = unique[i].height;
					updatebarpos(m);
				}
		} else { /* less monitors available nn < n */
			for (i = nn; i < n; i++) {
				for (m = mons; m && m->next; m = m->next);
				while ((c = m->clients)) {
					dirty = 1;
					m->clients = c->next;
					detachstack(c);
					c->mon = mons;
					attach(c);
					attachstack(c);
				}
				if (m == selmon)
					selmon = mons;
				cleanupmon(m);
			}
		}
		free(unique);
	} else
#endif /* XINERAMA */
	{ /* default monitor setup */
		if (!mons)
			mons = createmon();
		if (mons->mw != sw || mons->mh != sh) {
			dirty = 1;
			mons->mw = mons->ww = sw;
			mons->mh = mons->wh = sh;
			updatebarpos(mons);
		}
	}
	if (dirty) {
		selmon = mons;
		selmon = wintomon(root);
	}
	return dirty;
}

void
updatenumlockmask(void)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
				== XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

void
updatesizehints(Client *c)
{
	long msize;
	XSizeHints size;

	if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	if (size.flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	} else if (size.flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	} else
		c->basew = c->baseh = 0;
	if (size.flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	} else
		c->incw = c->inch = 0;
	if (size.flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	} else
		c->maxw = c->maxh = 0;
	if (size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	} else if (size.flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	} else
		c->minw = c->minh = 0;
	if (size.flags & PAspect) {
		c->mina = (float)size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
	} else
		c->maxa = c->mina = 0.0;
	c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
}

void
updatestatus(void)
{
	if (!gettextprop(root, XA_WM_NAME, stext, sizeof(stext)))
		strcpy(stext, "dwm-"VERSION);
	drawebar(stext, selmon, 0);
}

void
updatetitle(Client *c)
{
	if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
	if (c->name[0] == '\0') /* hack to mark broken clients */
		strcpy(c->name, broken);
}

void
updatewindowtype(Client *c)
{
	Atom state = getatomprop(c, netatom[NetWMState]);
	Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

	if (state == netatom[NetWMFullscreen])
		setfullscreen(c, 1);
	if (wtype == netatom[NetWMWindowTypeDialog])
		c->isfloating = 1;
}

void
updatewmhints(Client *c)
{
	XWMHints *wmh;

	if ((wmh = XGetWMHints(dpy, c->win))) {
		if (c == selmon->sel && wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
		} else
			c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
		if (wmh->flags & InputHint)
			c->neverfocus = !wmh->input;
		else
			c->neverfocus = 0;
		XFree(wmh);
	}
}

void
view(const Arg *arg)
{
	int i;
	unsigned int tmptag;

	if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
		return;
	selmon->seltags ^= 1; /* toggle sel tagset */
	if (arg->ui & TAGMASK) {
		selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		if (arg->ui == ~0)
			selmon->pertag->curtag = 0;
		else {
			for (i = 0; !(arg->ui & 1 << i); i++);
			selmon->pertag->curtag = i + 1;
		}
	} else {
		tmptag = selmon->pertag->prevtag;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		selmon->pertag->curtag = tmptag;
	}
	selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
	selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
	selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
	selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
	selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];
	selmon->ltaxis[0] = selmon->pertag->ltaxes[selmon->pertag->curtag][0];
	selmon->ltaxis[1] = selmon->pertag->ltaxes[selmon->pertag->curtag][1];
	selmon->ltaxis[2] = selmon->pertag->ltaxes[selmon->pertag->curtag][2];
	if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
		togglebar(NULL);
	if (selmon->showebar != selmon->pertag->showebars[selmon->pertag->curtag])
		toggleebar(NULL);
	focus(NULL);
	arrange(selmon);
}

Client *
wintoclient(Window w)
{
	Client *c;
	Monitor *m;

	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			if (c->win == w)
				return c;
	return NULL;
}

Monitor *
wintomon(Window w)
{
	int x, y;
	Client *c;
	Monitor *m;

	if (w == root && getrootptr(&x, &y))
		return recttomon(x, y, 1, 1);
	for (m = mons; m; m = m->next)
		if (w == m->barwin)
			return m;
	if ((c = wintoclient(w)))
		return c->mon;
	return selmon;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int
xerror(Display *dpy, XErrorEvent *ee)
{
	if (ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dpy, XErrorEvent *ee)
{
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dpy, XErrorEvent *ee)
{
	die("dwm: another window manager is already running");
	return -1;
}

void
zoom(const Arg *arg)
{
	Client *c = selmon->sel;

	if (!selmon->lt[selmon->sellt]->arrange
	|| (selmon->sel && selmon->sel->isfloating))
		return;
	if (c == nexttiled(selmon->clients))
		if (!c || !(c = nexttiled(c->next)))
			return;
	pop(c);
}

/* patch - function implementations */

void
drawtheme(int x, int s, int status, int theme) {
	/* STATUS 		 THEME
	 * unfocus 	= 1  default 	= 0
	 * focus 	= 2  button 	= 1
	 * select 	= 3  floater 	= 2	*/

	if (x + s == 0) {
		drw_setscheme(drw, scheme[LENGTH(colors)]);
		if (status == 3) {
				drw->scheme[ColFg] = scheme[SchemeSelect][ColFg];
				drw->scheme[ColBg] = scheme[SchemeSelect][ColBg];
		}
		if (status == 2) {
				drw->scheme[ColFg] = scheme[bartheme ? SchemeFocus : SchemeUnfocus][ColFg];
				drw->scheme[ColBg] = scheme[bartheme ? SchemeFocus : SchemeUnfocus][ColBg];
		}
		if (status == 1) {
			if (bartheme) {
				drw->scheme[ColFg] = scheme[theme ? SchemeUnfocus : SchemeBar][ColFg];
				drw->scheme[ColBg] = scheme[theme ? SchemeUnfocus : SchemeBar][theme ? ColBg : ColFloat];
			} else {
				drw->scheme[ColFg] = scheme[SchemeBar][ColFg];
				drw->scheme[ColBg] = scheme[SchemeBar][ColBg];
			}
		}
		if (status == 0) {
				drw->scheme[ColFg] = scheme[(bartheme && theme) ? SchemeFocus : SchemeBar][(bartheme && theme) ? ColBg : ColFg];
				drw->scheme[ColBg] = scheme[SchemeBar][bartheme ? ColFloat : ColBg];
		}
	return;
	}

	if (theme == 0 || bartheme == 0)
		return;
	if (theme == 2) {
		if (status == 1) {
			XSetForeground(drw->dpy, drw->gc, scheme[SchemeBar][ColFloat].pixel);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, 0, 2, bh);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, bh - 2, s, 2);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + s - 2, 0, 2, bh);
			XSetForeground(drw->dpy, drw->gc, scheme[SchemeUnfocus][ColFloat].pixel);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + s - 2, 2, 2, bh - 2);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + 4, bh - 2, s - 4, 2);
		}
		if (status == 2) {
			XSetForeground(drw->dpy, drw->gc, scheme[SchemeBar][ColFloat].pixel);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, 0, 2, bh);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + s - 2, 0, 2, bh);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, 0, s, 1);
			XSetForeground(drw->dpy, drw->gc, scheme[SchemeFocus][ColFloat].pixel);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + 2, bh - 1, s - 3, 1);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + s - 2, 1, 1, bh - 1);
		}
		if (status == 3) {
			XSetForeground(drw->dpy, drw->gc, scheme[SchemeBar][ColFloat].pixel);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, 0, 1, bh);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + s - 1, 0, 1, bh);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, bh - 1, s, 1);
			XSetForeground(drw->dpy, drw->gc, scheme[SchemeSelect][ColBorder].pixel);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + 2, 0, s - 3, 1);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + 1, 0, 1, bh - 1);
		}
	}
	if (theme == 1) {
		if (status == 1 || status == 2) {
			XSetForeground(drw->dpy, drw->gc,scheme[SchemeUnfocus][ColBorder].pixel);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, 0, 1, bh - 1);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, 0, s - 1, 1);
			XSetForeground(drw->dpy, drw->gc,scheme[SchemeUnfocus][ColFloat].pixel);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + s - 1, 0, 1, bh - 1);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + s - 2, bh - 2, 1, 1);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, bh - 1, s, 1);
		}
		if (status == 3) {
			XSetForeground(drw->dpy, drw->gc, scheme[SchemeBar][ColFloat].pixel);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x + s - 1, 0, 1, bh);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, bh - 1, s, 1);
			XSetForeground(drw->dpy, drw->gc, scheme[SchemeSelect][ColBorder].pixel);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, 0, s - 1, 1);
			XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, 0, 1, bh - 1);
		}
	}
}

void
drawtaggrid(Monitor *m, int *x_pos, unsigned int occ)
{
	unsigned int x, y, h, max_x, columns;
	int invert, i,j, k;
	
	h = bh / tagrows;
	x = max_x = *x_pos;
	y = 0;
	columns = LENGTH(tags) / tagrows + ((LENGTH(tags) % tagrows > 0) ? 1 : 0);
	
	/* Firstly we will fill the borders of squares */
	
	XSetForeground(drw->dpy, drw->gc, scheme[SchemeTag][ColBorder].pixel);
	XFillRectangle(dpy, drw->drawable, drw->gc, x, y, h*columns + 1, bh);
	
	/* We will draw LENGTH(tags) squares in tagraws raws. */
	for(j = 0,  i= 0; j < tagrows; j++) {
		x = *x_pos;
		for (k = 0; k < columns && i < LENGTH(tags); k++, i++) {
			invert = m->tagset[m->seltags] & 1 << i ? 0 : 1;
			
			/* Select active color for current square */
			XSetForeground(drw->dpy, drw->gc, !invert ? scheme[SchemeTag][ColFg].pixel : scheme[SchemeTag][ColBg].pixel);
			XFillRectangle(dpy, drw->drawable, drw->gc, x+1, y+1, h-1, h-1);
			
			/* Mark square if tag has client */
			if (occ & 1 << i) {
				XSetForeground(drw->dpy, drw->gc, !invert ? scheme[SchemeTag][ColBg].pixel : scheme[SchemeTag][ColFloat].pixel);
				XFillRectangle(dpy, drw->drawable, drw->gc, x + 1, y + 1, h / 2, h / 2);
			}
			x += h;
			if (x > max_x)
				max_x = x;
		}
		y += h;
	}
	*x_pos = max_x + 1;
}

void
mirrorlayout(const Arg *arg) {
	if(!selmon->lt[selmon->sellt]->arrange)
		return;
	selmon->ltaxis[0] *= -1;
	selmon->pertag->ltaxes[selmon->pertag->curtag][0] = selmon->ltaxis[0];
	arrange(selmon);
}

void
rotatelayoutaxis(const Arg *arg) {
	if(!selmon->lt[selmon->sellt]->arrange)
		return;
	if(arg->i == 0) {
		if(selmon->ltaxis[0] > 0)
			selmon->ltaxis[0] = selmon->ltaxis[0] + 1 > 2 ? 1 : selmon->ltaxis[0] + 1;
		else
			selmon->ltaxis[0] = selmon->ltaxis[0] - 1 < -2 ? -1 : selmon->ltaxis[0] - 1;
		if(selmon->ltaxis[1] == abs(selmon->ltaxis[0]))
			selmon->ltaxis[1] = abs(selmon->ltaxis[0]) + 1 > 2 ? 1 : abs(selmon->ltaxis[0]) + 1;
		if(selmon->ltaxis[2] == abs(selmon->ltaxis[0]))
			selmon->ltaxis[2] = abs(selmon->ltaxis[0]) + 1 > 2 ? 1 : abs(selmon->ltaxis[0]) + 1;
	}
	else
		selmon->ltaxis[arg->i] = selmon->ltaxis[arg->i] + 1 > 3 ? 1 : selmon->ltaxis[arg->i] + 1;
	selmon->pertag->ltaxes[selmon->pertag->curtag][arg->i] = selmon->ltaxis[arg->i];
	arrange(selmon);
}

void setcfact(const Arg *arg) {
	float f;
	Client *c;

	c = selmon->sel;

	if(!arg || !c || !selmon->lt[selmon->sellt]->arrange)
		return;
	f = arg->f + c->cfact;
	if(arg->f == 0.0)
		f = 1.0;
	else if(f < 0.25 || f > 4.0)
		return;
	c->cfact = f;
	arrange(selmon);
}

void
setgaps(const Arg *arg)
{
	if ((arg->i == 0) || (selmon->gappx + arg->i < 0))
		selmon->gappx = 0;
	else if (selmon->gappx + arg->i < 50)
		selmon->gappx += arg->i;
	arrangemon(selmon);
}

void
switchtag(const Arg *arg)
{
	unsigned int columns;
	unsigned int new_tagset = 0;
	unsigned int pos, i;
	int col, row;
	Arg new_arg;
	
	columns = (drawtagmask & DRAWCLASSICTAGS) ? LENGTH(tags) : LENGTH(tags) / tagrows + ((LENGTH(tags) % tagrows > 0) ? 1 : 0);
	
	for (i = 0; i < LENGTH(tags); ++i) {
		if (!(selmon->tagset[selmon->seltags] & 1 << i))
			continue;
		pos = i;
		row = pos / columns;
		col = pos % columns;
		if (arg->ui & SWITCHTAG_UP) {     /* UP */
			row --;
			if (row < 0)
				row = tagrows - 1;
			do {
				pos = row * columns + col;
				row --;
			} while (pos >= LENGTH(tags));
		}
		if (arg->ui & SWITCHTAG_DOWN) {     /* DOWN */
			row ++;
			if (row >= tagrows)
				row = 0;
			pos = row * columns + col;
			if (pos >= LENGTH(tags))
				row = 0;
			pos = row * columns + col;
		}
		if (arg->ui & SWITCHTAG_LEFT) {     /* LEFT */
			col --;
			if (col < 0)
				col = columns - 1;
			do {
				pos = row * columns + col;
				col --;
			} while (pos >= LENGTH(tags));
		}
		if (arg->ui & SWITCHTAG_RIGHT) {     /* RIGHT */
			col ++;
			if (col >= columns)
				col = 0;
			pos = row * columns + col;
			if (pos >= LENGTH(tags)) {
				col = 0;
				pos = row * columns + col;
			}
		}
		new_tagset |= 1 << pos;
	}
	new_arg.ui = new_tagset;
	if (arg->ui & SWITCHTAG_TOGGLETAG)
		toggletag(&new_arg);
	if (arg->ui & SWITCHTAG_TAG)
		tag(&new_arg);
	if (arg->ui & SWITCHTAG_VIEW)
		view (&new_arg);
	if (arg->ui & SWITCHTAG_TOGGLEVIEW)
		toggleview (&new_arg);
}

void
xinitvisual()
{
	XVisualInfo *infos;
	XRenderPictFormat *fmt;
	int nitems;
	int i;

	XVisualInfo tpl = {
		.screen = screen,
		.depth = 32,
		.class = TrueColor
	};
	long masks = VisualScreenMask | VisualDepthMask | VisualClassMask;

	infos = XGetVisualInfo(dpy, masks, &tpl, &nitems);
	visual = NULL;
	for(i = 0; i < nitems; i ++) {
		fmt = XRenderFindVisualFormat(dpy, infos[i].visual);
		if (fmt->type == PictTypeDirect && fmt->direct.alphaMask) {
			visual = infos[i].visual;
			depth = infos[i].depth;
			cmap = XCreateColormap(dpy, root, visual, AllocNone);
			useargb = 1;
			break;
		}
	}

	XFree(infos);

	if (! visual) {
		visual = DefaultVisual(dpy, screen);
		depth = DefaultDepth(dpy, screen);
		cmap = DefaultColormap(dpy, screen);
	}
}

int
main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-"VERSION);
	else if (argc != 1)
		die("usage: dwm [-v]");
	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if (!(dpy = XOpenDisplay(NULL)))
		die("dwm: cannot open display");
	checkotherwm();
	setup();
#ifdef __OpenBSD__
	if (pledge("stdio rpath proc exec", NULL) == -1)
		die("pledge");
#endif /* __OpenBSD__ */
	scan();
	run();
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}
