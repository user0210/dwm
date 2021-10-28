//	██████╗ ██╗    ██╗███╗   ███╗
//	██╔══██╗██║    ██║████╗ ████║
//	██║  ██║██║ █╗ ██║██╔████╔██║
//	██║  ██║██║███╗██║██║╚██╔╝██║
//	██████╔╝╚███╔███╔╝██║ ╚═╝ ██║
//	╚═════╝  ╚══╝╚══╝ ╚═╝     ╚═╝

/* See LICENSE file for copyright and license details. */


/* bartabgroups */
#define BARTABGROUPS_FUZZPX				 5		/* pixels cutoff between bartab groups to merge (e.g. max gaps px) */
#define BARTABGROUPS_TAGSINDICATOR		 1		/* 0 = disable, 1 = enable when >1 client or view tag, 2 = enable always */
#define BARTABGROUPS_TAGSPX				 5		/* pixels for tag grid boxes */
#define BARTABGROUPS_INDICATORSPADPX	 2		/* pixels from l/r to pad tags indicators */

/* grid of tags */
#define DRAWCLASSICTAGS				1 << 0
#define DRAWTAGGRID					1 << 1

#define SWITCHTAG_UP				1 << 0
#define SWITCHTAG_DOWN				1 << 1
#define SWITCHTAG_LEFT				1 << 2
#define SWITCHTAG_RIGHT				1 << 3
#define SWITCHTAG_TOGGLETAG			1 << 4
#define SWITCHTAG_TAG				1 << 5
#define SWITCHTAG_VIEW				1 << 6
#define SWITCHTAG_TOGGLEVIEW		1 << 7

static const int drawtagmask		= DRAWCLASSICTAGS; /* | DRAWCLASSICTAGS | DRAWTAGGRID | */
static const int tagrows			= 2;

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6" };


/* barorder and theme */
static const int seppad 			= 5;		/* top and bottom padding of seperator (if "> bh" = dot or "< 0" dotradius) */
static const int statussep			= 1;		/* 0 = off, 1 = on, 2 = hide on focus */
static const int tabbarsep			= 2;		/* 0 = off, 1 = on, 2 = hide on focus */
static const int bartheme			= 1;		/* 0 = off, 1 = on */
static const int statustheme		= 2;		/* 0 = classic, 1 = button-theme, 2 = float-theme */
static const int tabbartheme		= 2;		/* 0 = classic, 1 = button-theme, 2 = float-theme */
static const int tagtheme			= 0;		/* 0 = classic, 1 = button-theme, 2 = float-theme */

static const char *barorder[]		= {
//	"sepgap",									/*	[|]	*/
//	"gap",										/*	[ ]	*/
//	"seperator",								/*	 |	*/
	"tagbar",
	"ltsymbol",
	"bartab", 									/* FIXED ON "EBAR" - sepparates left and right allignment */
	"systray",
""};

static const char *ebarorder[] = {
	"status", 									/* FIXED ON "EBAR" - sepparates left and right allignment */
""};


/* appearance */
static const char dmenufont[]		= "monospace:pixelsize=16";
static const char *fonts[]			= { dmenufont };
static const char istatusprefix[]	= "msg: ";			/* prefix for important status messages */
static const char istatusclose[]	= "msg:close";		/* prefix to close messages */
static const char icommandprefix[]	= "dwm:cmd ";		/* prefix for commands */
static const char slopspawnstyle[]	= "-t 0 -l -c 0.92,0.85,0.69,0.3 -o";	/* do NOT define -f (format) here */
static const char slopresizestyle[]	= "-t 0 -l -c 0.92,0.85,0.69,0.3 -o";	/* do NOT define -f (format) here */

static const int attachdirection	= 2;		/* 0 default, 1 above, 2 aside, 3 below, 4 bottom, 5 top */
static const int bargap				= 1;		/* bar padding on/off */
static const int borderpx			= 1;		/* border pixel of windows */
static const int tagborderpx		= borderpx;	/* border pixel of tagpreview */
static const int gappx				= 4;		/* gaps between windows */
static const int iconpad			= 1;		/* icon padding to barborders */
static const int iconspacing		= 5;		/* space between icon and title */
static const int istatustimeout		= 5;		/* max timeout before displaying regular status after istatus */
static const int oneclientdimmer	= 1;		/* 1 makes tab for one client in unfocused color... */
static const int riodraw_borders	= 0;        /* 0 or 1, indicates whether the area drawn using slop includes the window borders */
static const int riodraw_matchpid	= 1;        /* 0 or 1, indicates whether to match the PID of the client that was spawned with riospawn */
static const int riodraw_spawnasync	= 0;        /* 0 spawned after selection, 1 application initialised in background while selection is made */
static const int scalepreview		= 4;		/* Tag preview scaling */
static const int snap				= 32;		/* snap pixel */
static const int showbar			= 1;		/* 0 means no bar */
static const int showebar           = 1;        /* 0 means no extra bar */
static const int statuscenter		= 0;		/* center status elements */
static const int swallowfloating	= 1;		/* 1 means swallow floating windows by default */
static const int systraypinning 	= 0;		/* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const int systrayspacing 	= 2;		/* systray spacing */
static const int systraypinningfailfirst = 1;	/* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray		= 1;		/* 0 means no systray */
static const int tileswitch			= 2;		/* gapps <= tileswitch = borderpx on, resizehints off */
static const int borderswitch		= 1;		/* 1: switch border on/off with tileswitch */
static const int topbar				= 1;		/* 0 means bottom bar */

static char bar_fg[]			= "#bbbbbb";
static char bar_bg[]			= "#222222";
static char bar_brd[]			= "#444444";
static char bar_flo[]			= "#444444";
static char sel_fg[]			= "#eeeeee";
static char sel_bg[]			= "#005577";
static char sel_brd[]			= "#005577";
static char sel_flo[]			= "#bbbbbb";
static char brd_fg[]			= "#eeeeee";
static char brd_bg[]			= "#222222";
static char brd_brd[]			= "#444444";
static char brd_flo[]			= "#444444";
static char foc_fg[]			= "#222222";
static char foc_bg[]			= "#eeeeee";
static char foc_brd[]			= "#444444";
static char foc_flo[]			= "#bbbbbb";
static char unf_fg[]			= "#eeeeee";
static char unf_bg[]			= "#444444";
static char unf_brd[]			= "#222222";
static char unf_flo[]			= "#222222";
static char tag_fg[]			= "#005577";
static char tag_bg[]			= "#eeeeee";
static char tag_brd[]			= "#222222";
static char tag_flo[]			= "#222222";

static const float transp    = 0.0;
static const float semitr    = 0.8;

static char *colors[][4]		= {
	/*					fg			bg			border		float   */
	[SchemeBar]		= { bar_fg,		bar_bg,		bar_brd,	bar_flo	},
	[SchemeSelect]	= { sel_fg,		sel_bg,		sel_brd,	sel_flo	},
	[SchemeBorder]	= { brd_fg,		brd_bg,		brd_brd,	brd_flo },
	[SchemeFocus]	= { foc_fg,		foc_bg,		foc_brd,	foc_flo },
	[SchemeUnfocus]	= { unf_fg,		unf_bg,		unf_brd,	unf_flo },
	[SchemeTag]		= { tag_fg,		tag_bg,		tag_brd,	tag_flo },
};

static const float alphas[][4] = {
    /*					fg			bg			border		float	*/
    [SchemeBar]		= { OPAQUE,		OPAQUE,		OPAQUE,		OPAQUE	},
    [SchemeSelect]	= { OPAQUE,		OPAQUE,		OPAQUE,		OPAQUE	},
    [SchemeBorder]	= { OPAQUE,		OPAQUE,		OPAQUE,		OPAQUE	},
    [SchemeFocus]	= { OPAQUE,		OPAQUE,		OPAQUE,		OPAQUE	},
    [SchemeUnfocus] = { OPAQUE,		OPAQUE,		OPAQUE,		OPAQUE	},
    [SchemeTag]		= { OPAQUE,		OPAQUE,		OPAQUE,		OPAQUE	},
};


/* layout(s) */
static const float mfact			= 0.55;		/* factor of master area size [0.05..0.95] */
static const int nmaster			= 1;		/* number of clients in master area */
static const int resizehints		= 1;		/* 1 respect size hints in tiled resizals */
static const int lockfullscreen 	= 1;		/* 1 will force focus on fullscreen window */

static const Layout layouts[]		= {
	/* symbol		arrange function */
	{ "[]=",		tile },						/* first entry is default */
	{ "><>",		NULL },						/* no layout function means floating behavior */
	{ "[M]",		monocle },
};

static const int  layoutaxis[] = {
	1,    /* layout axis: 1 = x, 2 = y; negative values mirror the layout */
	2,    /* master axis: 1 = x (left to right), 2 = y (top to bottom), 3 = z (monocle) */
	2,    /* stack  axis: 1 = x (left to right), 2 = y (top to bottom), 3 = z (monocle) */
};

/* rules */
static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING)	= instance, class
	 *	WM_NAME(STRING)		= title
	 *	switchtag			= 0 default, 1 switch to tag, 2 add application-tag, 3 as (1) and revert, 4 as (2) and revert
	 *	border 				= -1 dont set border, 0 zero border, NUMBER thickness
	 */
	/* class			instance		title		tags mask	switchtag	isfloating	fakefullsc.	isterminal	noswallow	monitor 	border		*/
	{ "Gimp",			NULL,			NULL,		0,			1,			1,			0,			0,			0,			-1, 		-1			},
	{ "Firefox",		NULL,			NULL,		1 << 8,		1,			0,			1,			0,			-1,			-1, 		-1			},
	{ "st",				NULL,			NULL,		0,			0,			0,			0,			1,			-1,			-1, 		-1			},
	{ NULL,				NULL,	"Event Tester",		0,			0,			1,			0,			0,			1,			-1, 		-1			},	/* xev */
};


/* key definitions */
#define MODKEY Mod4Mask
#define ALT Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,							KEY,		view,				{.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,				KEY,		toggleview,			{.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,					KEY,		tag,				{.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask,		KEY,		toggletag,			{.ui = 1 << TAG} },


/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }
static const char notifymenu[]	= "cat /tmp/notify | sed 's/^\\^........\\^//; s/\\^d\\^//' | dmenu -ix -l 10 | sort -r | xargs -I {} sed -i '{}d' /tmp/notify && kill -48 $(pidof dwmblocks)";


/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static char dmenugap[16] = "0";
static char dmenulen[16] = "0";
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-x", dmenugap, "-y", dmenugap, "-z", dmenulen, "-nb", bar_bg, "-nf", bar_fg, "-sb", foc_bg, "-sf", foc_fg, NULL };
static const char *termcmd[]  = { "st", NULL };


/* xsetroot fake commands */
static const Command commands[]		= {
	/* command		function		argument */
	{ "xrdb",		xrdb,			{.v = NULL }	},
	{ "kill",		killclient,		{0}  			},
};


/* keymap */
static Key keys[] = {
	/* modifier						key					function				argument */
	{ MODKEY|ControlMask|ShiftMask,	XK_Return,			spawn,					SHCMD(notifymenu) },
	{ MODKEY,						XK_Return,			spawn,					{.v = dmenucmd } },
	{ MODKEY|ShiftMask,				XK_Return,			spawn,					{.v = termcmd } },
	{ MODKEY|ControlMask,			XK_s,				riospawn,				{.v = termcmd } },
	{ MODKEY,						XK_s,				rioresize,				{0} },
	{ MODKEY,						XK_b,				togglebars,				{0} },
	{ MODKEY|ControlMask,			XK_b,				togglebar,				{0} },
	{ MODKEY|ControlMask|ShiftMask,	XK_b,				toggleebar,				{0} },
	{ MODKEY,						XK_n,				focusstack,				{.i = +1 } },
	{ MODKEY,						XK_p,				focusstack,				{.i = -1 } },
	{ MODKEY|ShiftMask,				XK_n,				inplacerotate,			{.i = +1} },
	{ MODKEY|ShiftMask,				XK_p,				inplacerotate,			{.i = -1} },
	{ MODKEY|ControlMask,			XK_n,				inplacerotate,			{.i = +2} },
	{ MODKEY|ControlMask,			XK_p,				inplacerotate,			{.i = -2} },
	{ MODKEY|ControlMask,			XK_i,				incnmaster,				{.i = +1 } },
	{ MODKEY|ControlMask,			XK_d,				incnmaster,				{.i = -1 } },
	{ MODKEY|ControlMask,			XK_h,				setmfact,				{.f = -0.05} },
	{ MODKEY|ControlMask,			XK_l,				setmfact,				{.f = +0.05} },
	{ MODKEY|ControlMask,			XK_k,				setcfact,				{.f = +0.25} },
	{ MODKEY|ControlMask,			XK_j,				setcfact,				{.f = -0.25} },
	{ MODKEY|ControlMask,			XK_o,				setcfact,				{.f =  0.00} },
	{ MODKEY|ControlMask,			XK_z,				zoom,					{0} },
	{ MODKEY|ShiftMask,				XK_z,				transfer,				{0} },
	{ MODKEY|ControlMask,			XK_Tab,				switchcol,				{0} },
	{ MODKEY,						XK_Tab,				view,					{0} },
	{ MODKEY|ShiftMask,				XK_q,				killclient,				{0} },
	{ MODKEY,						XK_t,				setlayout,				{.v = &layouts[0]} },
	{ MODKEY,						XK_f,				setlayout,				{.v = &layouts[1]} },
	{ MODKEY,						XK_m,				setlayout,				{.v = &layouts[2]} },
	{ MODKEY|ControlMask,			XK_space,			setlayout,				{0} },
	{ MODKEY|ControlMask|ShiftMask,	XK_t,				rotatelayoutaxis,		{.i = 0} },			/* 0 = layout axis */
	{ MODKEY|ShiftMask,				XK_t,				rotatelayoutaxis,		{.i = 1} },			/* 1 = master axis */
	{ MODKEY|ControlMask,			XK_t,				rotatelayoutaxis,		{.i = 2} },			/* 2 = stack axis */
	{ MODKEY|ControlMask,			XK_m,				mirrorlayout,			{0} },
	{ MODKEY,						XK_space,			togglefloating,			{0} },
	{ MODKEY,						XK_0,				view,					{.ui = ~0 } },
	{ MODKEY|ShiftMask,				XK_0,				tag,					{.ui = ~0 } },
	{ MODKEY,          				XK_comma,			focusmon,				{.i = -1 } },
	{ MODKEY,          				XK_period,			focusmon,				{.i = +1 } },
	{ MODKEY|ShiftMask,				XK_comma,			tagmon,					{.i = -1 } },
	{ MODKEY|ShiftMask,				XK_period,			tagmon,					{.i = +1 } },
	{ MODKEY|ControlMask,			XK_f,				togglefullscreen,		{0} },
	{ MODKEY|ShiftMask,				XK_f,				togglefakefullscreen,	{0} },
	{ MODKEY,						XK_minus,			setgaps,				{.i = -2 } },
	{ MODKEY,						XK_plus,			setgaps,				{.i = +2 } },
	{ MODKEY|ShiftMask,				XK_o,				setgaps,				{.i = 0  } },
	{ MODKEY|ControlMask|ShiftMask,	XK_q,				quit,					{0} },
	{ MODKEY|ShiftMask,				XK_r,				xrdb,					{.v = NULL } },

    { MODKEY|ALT,					XK_k,				switchtag,				{ .ui = SWITCHTAG_UP     | SWITCHTAG_VIEW } },
    { MODKEY|ALT,					XK_j,				switchtag,				{ .ui = SWITCHTAG_DOWN   | SWITCHTAG_VIEW } },
    { MODKEY|ALT,					XK_l,				switchtag,				{ .ui = SWITCHTAG_RIGHT  | SWITCHTAG_VIEW } },
    { MODKEY|ALT,					XK_h,				switchtag,				{ .ui = SWITCHTAG_LEFT   | SWITCHTAG_VIEW } },
    { MODKEY|ControlMask|ALT,		XK_k,				switchtag,				{ .ui = SWITCHTAG_UP     | SWITCHTAG_TAG | SWITCHTAG_VIEW } },
    { MODKEY|ControlMask|ALT,		XK_j,				switchtag,				{ .ui = SWITCHTAG_DOWN   | SWITCHTAG_TAG | SWITCHTAG_VIEW } },
    { MODKEY|ControlMask|ALT,		XK_l,				switchtag,				{ .ui = SWITCHTAG_RIGHT  | SWITCHTAG_TAG | SWITCHTAG_VIEW } },
    { MODKEY|ControlMask|ALT,		XK_h,				switchtag,				{ .ui = SWITCHTAG_LEFT   | SWITCHTAG_TAG | SWITCHTAG_VIEW } },

	TAGKEYS(						XK_1,										0)
	TAGKEYS(						XK_2,										1)
	TAGKEYS(						XK_3,										2)
	TAGKEYS(						XK_4,										3)
	TAGKEYS(						XK_5,										4)
	TAGKEYS(						XK_6,										5)
	TAGKEYS(						XK_7,										6)
	TAGKEYS(						XK_8,										7)
	TAGKEYS(						XK_9,										8)
};


/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click				event mask		button			function			argument */
	{ ClkLtSymbol,			0,				Button1,		setlayout,			{0} },
	{ ClkLtSymbol,			0,				Button3,		setlayout,			{.v = &layouts[2]} },
	{ ClkStatusText,		0,				Button1,		sigdwmblocks,		{.i = 1 } },
	{ ClkStatusText,		0,				Button2,		sigdwmblocks,		{.i = 2 } },
	{ ClkStatusText,		0,				Button3,		sigdwmblocks,		{.i = 3 } },
	{ ClkStatusText,		0,				Button4,		sigdwmblocks,		{.i = 4 } },
	{ ClkStatusText,		0,				Button5,		sigdwmblocks,		{.i = 5 } },
	{ ClkClientWin,			MODKEY,			Button1,		movemouse,			{0} },
	{ ClkClientWin,			MODKEY,			Button2,		togglefloating,		{0} },
	{ ClkClientWin,			MODKEY,			Button3,		resizemouse,		{0} },
	{ ClkTagBar,			0,				Button1,		view,				{0} },
	{ ClkTagBar,			0,				Button3,		toggleview,			{0} },
	{ ClkTagBar,			MODKEY,			Button1,		tag,				{0} },
	{ ClkTagBar,			MODKEY,			Button3,		toggletag,			{0} },
	{ ClkNotifyText,		0,				Button1,		notifyhandler,		{.i = 1 } },
	{ ClkRootWin,			0,				Button1,		dragfact,			{0} },
};

