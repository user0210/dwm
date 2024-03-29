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

static int drawtagmask		= DRAWCLASSICTAGS; /* | DRAWCLASSICTAGS | DRAWTAGGRID | */
static int tagrows			= 2;

/* tagging */
static char *tags[] = { "1", "2", "3", "4", "5", "6" };


/* barorder and theme */
static int seppad 			= 5;		/* top and bottom padding of seperator (if "> bh" = dot or "< 0" dotradius) */
static int statussep		= 1;		/* 0 = off, 1 = on, 2 = hide on focus */
static int tabbarsep		= 2;		/* 0 = off, 1 = on, 2 = hide on focus */
static int statustheme		= 2;		/* 0 = classic, 1 = button-theme, 2 = float-theme */
static int tabbartheme		= 2;		/* 0 = classic, 1 = button-theme, 2 = float-theme */
static int tagtheme			= 0;		/* 0 = classic, 1 = button-theme, 2 = float-theme */

static char *barorder[]		= {
//	"sepgap",									/*	[|]	*/
//	"gap",										/*	[ ]	*/
//	"seperator",								/*	 |	*/
	"tagbar",
	"ltsymbol",
	"bartab", 									/* FIXED ON "EBAR" - sepparates left and right allignment */
	"systray",
""};

static char *ebarorder[] = {
	"status", 									/* FIXED ON "EBAR" - sepparates left and right allignment */
""};


/* appearance */
static char font[]				= "monospace:pixelsize=16";
static char *fonts[]			= { font };
static char istatusprefix[]		= "msg: ";			/* prefix for important status messages */
static char istatusclose[]		= "msg:close";		/* prefix to close messages */
static char icommandprefix[]	= "dwm:cmd ";		/* prefix for commands */
static char slopspawnstyle[]	= "-t 0 -l -c 0.92,0.85,0.69,0.3 -o";	/* do NOT define -f (format) here */
static char slopresizestyle[]	= "-t 0 -l -c 0.92,0.85,0.69,0.3 -o";	/* do NOT define -f (format) here */

static int fontshadow			= 1;		/* 1/0 -- on/off */
static int attachdirection		= 2;		/* 0 default, 1 above, 2 aside, 3 below, 4 bottom, 5 top */
static int bargap				= 1;		/* bar padding on/off */
static int barborder			= 1;		/* size of barborders */
static int barsep				= 1;		/* size of barseperator */
static int barhighlight			= 1;		/* highlight border when focused */
static int borderpx				= 1;		/* border pixel of windows */
static int tagborderpx			= 1;		/* border pixel of tagpreview */
static int floatposgrid_x		= 5;		/* float grid columns */
static int floatposgrid_y		= 5;		/* float grid rows */
static int gappx				= 4;		/* gaps between windows */
static int iconpad				= 1;		/* icon padding to barborders */
static int iconspacing			= 5;		/* space between icon and title */
static int istatustimeout		= 5;		/* max timeout before displaying regular status after istatus */
static int oneclientdimmer		= 1;		/* 1 makes tab for one client in unfocused color... */
static int riodraw_borders		= 0;        /* 0 or 1, indicates whether the area drawn using slop includes the window borders */
static int riodraw_matchpid		= 1;        /* 0 or 1, indicates whether to match the PID of the client that was spawned with riospawn */
static int riodraw_spawnasync	= 0;        /* 0 spawned after selection, 1 application initialised in background while selection is made */
static int scalepreview			= 4;		/* Tag preview scaling */
static int snap					= 32;		/* snap pixel */
static int showbar				= 1;		/* 0 means no bar */
static int showebar         	= 1;        /* 0 means no extra bar */
static int statuscenter			= 0;		/* center status elements */
static int statusspan			= 1;		/* span status over space */
static int titlecenter			= 2;		/* 1 - center title, 2 center title with icon */
static int swallowfloating		= 1;		/* 1 means swallow floating windows by default */
static int systraypinning 		= 0;		/* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static int systrayspacing 		= 2;		/* systray spacing */
static int systraypinningfailfirst = 1;	/* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static int showsystray			= 1;		/* 0 means no systray */
static int tileswitch			= 2;		/* gapps <= tileswitch = resizehints off */
static int tileswitchcolor		= 1;		/* switch to alt-colors on tileswich */
static int borderswitch			= 1;		/* 1: switch border on/off with tileswitch */
static int topbar				= 1;		/* 0 means bottom bar */

static char bar_fg_a[]			= "1.0";
static char bar_bg_a[]			= "1.0";
static char bar_brd_a[]			= "1.0";
static char bar_flo_a[]			= "1.0";
static char bar_sha_a[]			= "1.0";
static char sel_fg_a[]			= "1.0";
static char sel_bg_a[]			= "1.0";
static char sel_brd_a[]			= "1.0";
static char sel_flo_a[]			= "1.0";
static char sel_sha_a[]			= "1.0";
static char brd_fg_a[]			= "1.0";
static char brd_bg_a[]			= "1.0";
static char brd_brd_a[]			= "1.0";
static char brd_flo_a[]			= "1.0";
static char brd_sha_a[]			= "1.0";
static char foc_fg_a[]			= "1.0";
static char foc_bg_a[]			= "1.0";
static char foc_brd_a[]			= "1.0";
static char foc_flo_a[]			= "1.0";
static char foc_sha_a[]			= "1.0";
static char unf_fg_a[]			= "1.0";
static char unf_bg_a[]			= "1.0";
static char unf_brd_a[]			= "1.0";
static char unf_flo_a[]			= "1.0";
static char unf_sha_a[]			= "1.0";
static char tag_fg_a[]			= "1.0";
static char tag_bg_a[]			= "1.0";
static char tag_brd_a[]			= "1.0";
static char tag_flo_a[]			= "1.0";
static char tag_sha_a[]			= "1.0";
static char alt_fg_a[]			= "1.0";
static char alt_bg_a[]			= "1.0";
static char alt_brd_a[]			= "1.0";
static char alt_flo_a[]			= "1.0";
static char alt_sha_a[]			= "1.0";

static char bar_fg[]			= "#bbbbbb";
static char bar_bg[]			= "#222222";
static char bar_brd[]			= "#444444";
static char bar_flo[]			= "#444444";
static char bar_sha[]			= "#000000";
static char sel_fg[]			= "#eeeeee";
static char sel_bg[]			= "#005577";
static char sel_brd[]			= "#005577";
static char sel_flo[]			= "#bbbbbb";
static char sel_sha[]			= "#666666";
static char brd_fg[]			= "#eeeeee";
static char brd_bg[]			= "#222222";
static char brd_brd[]			= "#444444";
static char brd_flo[]			= "#444444";
static char brd_sha[]			= "#222222";
static char foc_fg[]			= "#222222";
static char foc_bg[]			= "#eeeeee";
static char foc_brd[]			= "#444444";
static char foc_flo[]			= "#bbbbbb";
static char foc_sha[]			= "#444444";
static char unf_fg[]			= "#eeeeee";
static char unf_bg[]			= "#444444";
static char unf_brd[]			= "#222222";
static char unf_flo[]			= "#222222";
static char unf_sha[]			= "#222222";
static char tag_fg[]			= "#005577";
static char tag_bg[]			= "#eeeeee";
static char tag_brd[]			= "#222222";
static char tag_flo[]			= "#222222";
static char tag_sha[]			= "#222222";
static char alt_fg[]			= "#444444";
static char alt_bg[]			= "#bbbbbb";
static char alt_brd[]			= "#bbbbbb";
static char alt_flo[]			= "#444444";
static char alt_sha[]			= "#222222";

static char *colors[][5] = {
	/*					fg			bg			border		float 		shadow 	*/
	[SchemeBar]		= { bar_fg,		bar_bg,		bar_brd,	bar_flo,	bar_sha },
	[SchemeSelect]	= { sel_fg,		sel_bg,		sel_brd,	sel_flo,	sel_sha },
	[SchemeBorder]	= { brd_fg,		brd_bg,		brd_brd,	brd_flo,	brd_sha },
	[SchemeFocus]	= { foc_fg,		foc_bg,		foc_brd,	foc_flo,	foc_sha },
	[SchemeUnfocus]	= { unf_fg,		unf_bg,		unf_brd,	unf_flo,	unf_sha },
	[SchemeTag]		= { tag_fg,		tag_bg,		tag_brd,	tag_flo,	tag_sha },
	[SchemeAlt]		= { alt_fg,		alt_bg,		alt_brd,	alt_flo,	alt_sha },
};

static char *alphas[][5] = {
    /*					fg			bg			border		float 		shadow	*/
	[SchemeBar]		= { bar_fg_a,	bar_bg_a,	bar_brd_a,	bar_flo_a,	bar_sha_a },
	[SchemeSelect]	= { sel_fg_a,	sel_bg_a,	sel_brd_a,	sel_flo_a,	sel_sha_a },
	[SchemeBorder]	= { brd_fg_a,	brd_bg_a,	brd_brd_a,	brd_flo_a,	brd_sha_a },
	[SchemeFocus]	= { foc_fg_a,	foc_bg_a,	foc_brd_a,	foc_flo_a,	foc_sha_a },
	[SchemeUnfocus]	= { unf_fg_a,	unf_bg_a,	unf_brd_a,	unf_flo_a,	unf_sha_a },
	[SchemeTag]		= { tag_fg_a,	tag_bg_a,	tag_brd_a,	tag_flo_a,	tag_sha_a },
	[SchemeAlt]		= { alt_fg_a,	alt_bg_a,	alt_brd_a,	alt_flo_a,	alt_sha_a },
};


/* layout(s) */
static float mfact			= 0.55;		/* factor of master area size [0.05..0.95] */
static int nmaster			= 1;		/* number of clients in master area */
static int resizehints		= 1;		/* 1 respect size hints in tiled resizals */
static int lockfullscreen 	= 1;		/* 1 will force focus on fullscreen window */

static Layout layouts[]		= {
	/* symbol		arrange function */
	{ "[]=",		tile },						/* first entry is default */
	{ "><>",		NULL },						/* no layout function means floating behavior */
	{ "[M]",		monocle },
};

static int  layoutaxis[] = {
	1,    /* layout axis: 1 = x, 2 = y; negative values mirror the layout */
	2,    /* master axis: 1 = x (left to right), 2 = y (top to bottom), 3 = z (monocle) */
	2,    /* stack  axis: 1 = x (left to right), 2 = y (top to bottom), 3 = z (monocle) */
};

/* rules */
static Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING)	= instance, class
	 *	WM_NAME(STRING)		= title
	 *	switchtag			= 0 default, 1 switch to tag, 2 add application-tag, 3 as (1) and revert, 4 as (2) and revert
	 *	border 				= -1 dont set border, 0 zero border, NUMBER thickness
	 *
	 *  floatpos values - Coordinates char meaning
	 *  A - absolute position in the drawable area (this spans all monitors)
	 *  a - absolute position relative to the client's current position
	 *  X - position relative to monitor
	 *  x - position relative to client
	 *  Y - position relative to monitor
	 *  y - position relative to client
	 *  m - position relative to mouse cursor
	 *  M - as above
	 *  % - specifies the client's mid-point in percentage relative to the window area height / width
	 *  S - the client's current x / y position is fixed (sticky)
	 *  	if value is -1 then width / height indicators determines the size of the client
	 *  	otherwise the value indicates the next position, determining the size of the client
	 *  	this can, as an example, be used to "maximize" a client from it's current position to the edge of the screen
	 *  C - as above, but the client's mid-point (center) is fixed (sticky)
	 *  	this is typically used when resizing clients, making them expand or contract in all directions
	 *  Z - as above, but the opposite side of the client is fixed (sticky) (i.e. client position + size)
	 *  G - indicates that the monitor is split into a grid on which points the client can be positioned
	 *  	the value defines the size of the grid, if value is 0 then grid dimensions from config is used
	 *  floatpos values - Dimensions char meaning
	 *  A - absolute height / width
	 *  a - relative height / width
	 *  H - defines height of client, but position can take precedence
	 *  h - defines height relative to client size
	 *  W - defines width of client, but position can take precedence
	 *  w - defines width relative to client size
	 *  % - size determined in percentage of the monitor's window area
	 *  P - absolute grid position
	 *  p - relative grid position
	 */
	/* class			instance		title		tags mask	switchtag	isfloating	fakefullsc.	isterminal	noswallow	monitor 	ignoresize	border		scratch		floatpos			*/
	{ "Gimp",			NULL,			NULL,		0,			1,			1,			0,			0,			0,			-1, 		0,			-1,			0,			NULL				},
	{ "Firefox",		NULL,			NULL,		1 << 8,		1,			0,			1,			0,			-1,			-1, 		0,			-1,			0,			NULL				},
	{ NULL,				NULL,	"scratchpad",		0,			0,			1,			0,			1,			0,			-1,			0,			 4,			's',		"50% 50% 50% 50%"	},	/* before st ! */
	{ "St",				NULL,			NULL,		0,			0,			0,			0,			1,			-1,			-1, 		1,			-1,			0,			NULL				},
	{ NULL,				NULL,	"Event Tester",		0,			0,			1,			0,			0,			1,			-1, 		0,			-1,			0,			NULL				},	/* xev */
	{ "qutebrowser",	NULL,			NULL,		0,			0,			0,			1,			0,			0,			-1,			0,			-1,			0,			NULL				},
	{ "Onboard",		NULL,			NULL,		0,			0,			1,			0,			0,			0,			-1,			0,			 0,			0,			NULL				},
	{ NULL,				"wallman",		NULL,		0,			0,			1,			0,			0,			0,			-1,			0,			 0,			0,			"0% 0a 100% 110H"	},
	/* dwmblocks modules */
	{ "dbar",			"alsam",		NULL,		0,			0,			1,			0,			0,			0,			-1,			0,			 4,			0,			"0m 0y 400W 500H"	},
	{ "dbar",			"pulsm",		NULL,		0,			0,			1,			0,			0,			0,			-1,			0,			 4,			0,			"0m 0y 600W 600H"	},
	{ "dbar",			"psmem",		NULL,		0,			0,			1,			0,			0,			0,			-1,			0,			 4,			0,			"0m 0y 610W 650H"	},
	{ "dbar",			"calen",		NULL,		0,			0,			1,			0,			0,			0,			-1,			0,			 4,			0,			"0m 0y 170W 160H"	},
	{ "dbar",			"calcurse",		NULL,		0,			0,			1,			0,			0,			0,			-1,			0,			 4,			0,			"0m 0y 600W 600H"	},
	{ "dbar",			"htop",			NULL,		0,			0,			1,			0,			0,			0,			-1,			0,			 4,			0,			"0m 0y 1010W 650H"	},
	{ NULL,				NULL,			NULL,		0,			0,			0,			0,			0,			0,			-1,			0,			-1,			0,			"50% 50% 0w% 0w%"	},	/* default (last!)*/
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
static char notifymenu[]	= "cat /tmp/notify | sed 's/^\\^........\\^//; s/\\^d\\^//' | dmenu -ix -l 10 | sort -r | xargs -I {} sed -i '{}d' /tmp/notify && kill -48 $(pidof dwmblocks)";


/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static char dmenugap[16] = "0";
static char dmenulen[16] = "0";
static char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", font, "-x", dmenugap, "-y", dmenugap, "-z", dmenulen, NULL };
static char *termcmd[]  = { "st", "-u", "-e", "bash", "--rcfile", "~/.config/tmux/shell/stmux/.bashrc", NULL };
static char *scratchpadcmd[] = { "s", "st", "-t", "scratchpad", "-u", "-e", "bash", "--rcfile", "~/.config/tmux/shell/scratchpad/.bashrc", NULL };
static char *browser[] = { "qutebrowser", NULL };
static char *alsam[] = { "st", "-c", "dbar", "-n", "alsam", "-e", "alsamixer", NULL };
static char *pulsm[] = { "pavucontrol", "--class", "dbar", "--name", "pulsm", NULL };


/* xsetroot fake commands */
static Command commands[]		= {
	/* command		function			argument */
	{ "xrdb",		resource_reload,	{.v = NULL }	},
	{ "kill",		killclient,			{0}  			},
	{ "alsam",		spawn,				{.v = alsam }	},
	{ "pulsm",		spawn,				{.v = pulsm }	},
};


/* keymap */
#include <X11/XF86keysym.h>

static float mset			= 0.05;		/* resize m-fact */
static float cset			= 0.25;		/* resize c-fact */
static char pospx[16]		= "25";		/* change in pixel for floatpos */
static Key keys[] = {
	/* modifier						key					function				argument */
    { 0,	XF86XK_AudioLowerVolume,					spawn,					SHCMD("/usr/bin/amixer -q sset Master 5%-; kill -35 $(pidof dwmblocks)") },
	{ 0,	XF86XK_AudioRaiseVolume,					spawn,					SHCMD("/usr/bin/amixer -q sset Master 5%+; kill -35 $(pidof dwmblocks)") },
//	{ 0,	XF86XK_AudioMute,							spawn,					SHCMD("/usr/bin/amixer set Master toggle; kill -35 $(pidof dwmblocks)") },
	{ 0,	XF86XK_AudioMute,							spawn,					SHCMD("kill -35 $(pidof dwmblocks)") },
//	{ 0,	XF86XK_AudioMicMute,						spawn,					SHCMD("/usr/bin/amixer set Capture toggle") },
    { 0,	XF86XK_AudioPlay,							spawn,					SHCMD("playerctl play-pause") },
    { 0,	XF86XK_AudioNext,							spawn,					SHCMD("playerctl next") },
    { 0,	XF86XK_AudioPrev,							spawn,					SHCMD("playerctl previous") },
    { 0,	XF86XK_MonBrightnessUp,						spawn,					SHCMD("sleep 0.1; kill -36 $(pidof dwmblocks)") },
    { 0,	XF86XK_MonBrightnessDown,					spawn,					SHCMD("sleep 0.1; kill -36 $(pidof dwmblocks)") },
    { 0,	XF86XK_Launch5,								spawn,					SHCMD("thinkpad-rotate flip") },
    { 0,	XF86XK_Launch6,								spawn,					SHCMD("thinkpad-rotate left") },
	{ 0,	XF86XK_Launch1,								spawn,					{.v = dmenucmd } },

	{ MODKEY|ControlMask,			XK_w,				spawn,					{.v = browser } },
	{ MODKEY|ControlMask|ShiftMask,	XK_Return,			spawn,					SHCMD(notifymenu) },
	{ MODKEY,						XK_Return,			spawn,					{.v = dmenucmd } },
	{ MODKEY|ShiftMask,				XK_Return,			spawn,					{.v = termcmd } },
	{ MODKEY|ControlMask,			XK_Return,			togglescratch,			{.v = scratchpadcmd } },
	{ MODKEY|ALT,					XK_minus,			removescratch,			{.v = scratchpadcmd } },
	{ MODKEY|ALT,					XK_plus,			setscratch,				{.v = scratchpadcmd } },
	{ MODKEY|ControlMask,			XK_s,				riospawn,				{.v = termcmd } },
	{ MODKEY,						XK_s,				rioresize,				{0} },
	{ MODKEY,						XK_b,				togglebars,				{0} },
	{ MODKEY|ControlMask,			XK_b,				togglebar,				{0} },
	{ MODKEY|ControlMask|ShiftMask,	XK_b,				toggleebar,				{0} },
	{ MODKEY,						XK_h,				focusdir,				{.i = 0 } },		/* left */
	{ MODKEY,						XK_l,				focusdir,				{.i = 1 } },		/* right */
	{ MODKEY,						XK_k,				focusdir,				{.i = 2 } },		/* up */
	{ MODKEY,						XK_j,				focusdir,				{.i = 3 } },		/* down */
	{ MODKEY,						XK_n,				focusstack,				{.i = +1 } },
	{ MODKEY,						XK_p,				focusstack,				{.i = -1 } },
	{ MODKEY|ShiftMask,				XK_n,				inplacerotate,			{.i = +1} },
	{ MODKEY|ShiftMask,				XK_p,				inplacerotate,			{.i = -1} },
	{ MODKEY|ControlMask,			XK_n,				inplacerotate,			{.i = +2} },
	{ MODKEY|ControlMask,			XK_p,				inplacerotate,			{.i = -2} },
	{ MODKEY|ControlMask,			XK_i,				incnmaster,				{.i = +1 } },
	{ MODKEY|ControlMask,			XK_d,				incnmaster,				{.i = -1 } },
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
	{ MODKEY|ShiftMask,				XK_r,				resource_reload,		{.v = NULL } },

	{ MODKEY|ALT,					XK_n,				shiftviewclients,		{ .i = +1 } },
	{ MODKEY|ALT,					XK_p,				shiftviewclients,		{ .i = -1 } },
    { MODKEY|ALT,					XK_k,				switchtag,				{ .ui = SWITCHTAG_UP     | SWITCHTAG_VIEW } },
    { MODKEY|ALT,					XK_j,				switchtag,				{ .ui = SWITCHTAG_DOWN   | SWITCHTAG_VIEW } },
    { MODKEY|ALT,					XK_l,				switchtag,				{ .ui = SWITCHTAG_RIGHT  | SWITCHTAG_VIEW } },
    { MODKEY|ALT,					XK_h,				switchtag,				{ .ui = SWITCHTAG_LEFT   | SWITCHTAG_VIEW } },
    { MODKEY|ControlMask|ALT,		XK_k,				switchtag,				{ .ui = SWITCHTAG_UP     | SWITCHTAG_TAG | SWITCHTAG_VIEW } },
    { MODKEY|ControlMask|ALT,		XK_j,				switchtag,				{ .ui = SWITCHTAG_DOWN   | SWITCHTAG_TAG | SWITCHTAG_VIEW } },
    { MODKEY|ControlMask|ALT,		XK_l,				switchtag,				{ .ui = SWITCHTAG_RIGHT  | SWITCHTAG_TAG | SWITCHTAG_VIEW } },
    { MODKEY|ControlMask|ALT,		XK_h,				switchtag,				{ .ui = SWITCHTAG_LEFT   | SWITCHTAG_TAG | SWITCHTAG_VIEW } },

	{ MODKEY|ControlMask,			XK_h,				windowsize,				{.i = 0 } },		/* left */
	{ MODKEY|ControlMask,			XK_l,				windowsize,				{.i = 1 } },		/* right */
	{ MODKEY|ControlMask,			XK_k,				windowsize,				{.i = 2 } },		/* up */
	{ MODKEY|ControlMask,			XK_j,				windowsize,				{.i = 3 } },		/* down */
	{ MODKEY|ShiftMask,				XK_h,				windowmove,				{.i = 0 } },		/* left */
	{ MODKEY|ShiftMask,				XK_l,				windowmove,				{.i = 1 } },		/* right */
	{ MODKEY|ShiftMask,				XK_k,				windowmove,				{.i = 2 } },		/* up */
	{ MODKEY|ShiftMask,				XK_j,				windowmove,				{.i = 3 } },		/* down */
	{ MODKEY|ControlMask|ShiftMask,	XK_Up,				floatpos,				{.v = " 0x  0Z   0%   0%" } },	// ↑
	{ MODKEY|ControlMask|ShiftMask,	XK_Left,			floatpos,				{.v = " 0Z  0y   0%   0%" } },	// ←
	{ MODKEY|ControlMask|ShiftMask,	XK_Right,			floatpos,				{.v = "-1S  0y 100%   0%" } },	// →
	{ MODKEY|ControlMask|ShiftMask,	XK_Down,			floatpos,				{.v = " 0x -1S   0% 100%" } },	// ↓

//	/* maximize in direction																						*/
//	{ MODKEY|ControlMask|ShiftMask,	XK_Up,				floatpos,				{.v = " 0x  0Z   0%   0%" } },	// ↑
//	/* a|a) absolute position, x|y) no bar overlap and only this monitor		        a/x  a/y					*/
//	{ MODKEY|ShiftMask,				XK_Up,				floatpos,				{.v = "  0a -26a" } },			// ↑
//	/* w|h) Resize client, W|H) absolute size											w|W  h|H					*/
//	{ MODKEY|ControlMask,			XK_Up,				floatpos,				{.v = "  0w -26h" } },			// ↑
//	/* move one position up in grid... 																				*/
//	{ ???,							???,				floatpos,				{.v = " 0p -1p" } },			// ↑
//	/* corner and center positioning... (center)																	*/
//	{ ???,							???,				floatpos,				{.v = " 50% 50%" } },			// ↑
//	/* use 3x3 grid und take third position...																		*/
//	{ ???,							???,				floatpos,				{.v = "3G 3G 3P 1P" } },		// ↗

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
/* placemouse options: 0) tiled position is relative to mouse cursor 1) tiled postiion is relative to window center 2) mouse pointer warps to window center
 * ...The moveorplace uses movemouse or placemouse depending on the floating state of the selected client. Set up individual keybindings for the two if you want
 * to control these separately (i.e. to retain the feature to move a tiled window into a floating position).
 */
static Button buttons[] = {
	/* click				event mask		button			function			argument */
	{ ClkLtSymbol,			0,				Button1,		setlayout,			{0} },
	{ ClkLtSymbol,			0,				Button3,		setlayout,			{.v = &layouts[2]} },
	{ ClkStatusText,		0,				Button1,		sigdwmblocks,		{.i = 1 } },
	{ ClkStatusText,		0,				Button2,		sigdwmblocks,		{.i = 2 } },
	{ ClkStatusText,		0,				Button3,		sigdwmblocks,		{.i = 3 } },
	{ ClkStatusText,		0,				Button4,		sigdwmblocks,		{.i = 4 } },
	{ ClkStatusText,		0,				Button5,		sigdwmblocks,		{.i = 5 } },
//	{ ClkClientWin,			MODKEY,			Button1,		movemouse,			{0} },
	{ ClkClientWin,			MODKEY,			Button1,		moveorplace,		{.i = 1} },
	{ ClkClientWin,			MODKEY,			Button2,		togglefloating,		{0} },
	{ ClkClientWin,			MODKEY,			Button3,		resizemouse,		{0} },
	{ ClkTagBar,			0,				Button1,		view,				{0} },
	{ ClkTagBar,			0,				Button3,		toggleview,			{0} },
	{ ClkTagBar,			MODKEY,			Button1,		tag,				{0} },
	{ ClkTagBar,			MODKEY,			Button3,		toggletag,			{0} },
	{ ClkNotifyText,		0,				Button1,		notifyhandler,		{.i = 1 } },
	{ ClkRootWin,			0,				Button1,		dragfact,			{0} },
	{ ClkRootWin,			0,				Button3,		spawn,				SHCMD("xmenu.sh") },
};

ResourcePref resources[] = {
		{ "font",					STRING,		&font },
		{ "fontshadow",				INTEGER,	&fontshadow },

		{ "bar_fg",					STRING,		&bar_fg },
		{ "bar_bg",					STRING,		&bar_bg },
		{ "bar_brd",				STRING,		&bar_brd },
		{ "bar_flo",				STRING,		&bar_flo },
		{ "bar_sha",				STRING,		&bar_sha },
		{ "sel_fg",					STRING,		&sel_fg },
		{ "sel_bg",					STRING,		&sel_bg },
		{ "sel_brd",				STRING,		&sel_brd },
		{ "sel_flo",				STRING,		&sel_flo },
		{ "sel_sha",				STRING,		&sel_sha },
		{ "brd_fg",					STRING,		&brd_fg },
		{ "brd_bg",					STRING,		&brd_bg },
		{ "brd_brd",				STRING,		&brd_brd },
		{ "brd_flo",				STRING,		&brd_flo },
		{ "brd_sha",				STRING,		&brd_sha },
		{ "foc_fg",					STRING,		&foc_fg },
		{ "foc_bg",					STRING,		&foc_bg },
		{ "foc_brd",				STRING,		&foc_brd },
		{ "foc_flo",				STRING,		&foc_flo },
		{ "foc_sha",				STRING,		&foc_sha },
		{ "unf_fg",					STRING,		&unf_fg },
		{ "unf_bg",					STRING,		&unf_bg },
		{ "unf_brd",				STRING,		&unf_brd },
		{ "unf_flo",				STRING,		&unf_flo },
		{ "unf_sha",				STRING,		&unf_sha },
		{ "tag_fg",					STRING,		&tag_fg },
		{ "tag_bg",					STRING,		&tag_bg },
		{ "tag_brd",				STRING,		&tag_brd },
		{ "tag_flo",				STRING,		&tag_flo },
		{ "tag_sha",				STRING,		&tag_sha },
		{ "alt_fg",					STRING,		&alt_fg },
		{ "alt_bg",					STRING,		&alt_bg },
		{ "alt_brd",				STRING,		&alt_brd },
		{ "alt_flo",				STRING,		&alt_flo },
		{ "alt_sha",				STRING,		&alt_sha },

		{ "bar_fg_a",				STRING,		&bar_fg_a },
		{ "bar_bg_a",				STRING,		&bar_bg_a },
		{ "bar_brd_a",				STRING,		&bar_brd_a },
		{ "bar_flo_a",				STRING,		&bar_flo_a },
		{ "bar_sha_a",				STRING,		&bar_sha_a },
		{ "sel_fg_a",				STRING,		&sel_fg_a },
		{ "sel_bg_a",				STRING,		&sel_bg_a },
		{ "sel_brd_a",				STRING,		&sel_brd_a },
		{ "sel_flo_a",				STRING,		&sel_flo_a },
		{ "sel_sha_a",				STRING,		&sel_sha_a },
		{ "brd_fg_a",				STRING,		&brd_fg_a },
		{ "brd_bg_a",				STRING,		&brd_bg_a },
		{ "brd_brd_a",				STRING,		&brd_brd_a },
		{ "brd_flo_a",				STRING,		&brd_flo_a },
		{ "brd_sha_a",				STRING,		&brd_sha_a },
		{ "foc_fg_a",				STRING,		&foc_fg_a },
		{ "foc_bg_a",				STRING,		&foc_bg_a },
		{ "foc_brd_a",				STRING,		&foc_brd_a },
		{ "foc_flo_a",				STRING,		&foc_flo_a },
		{ "foc_sha_a",				STRING,		&foc_sha_a },
		{ "unf_fg_a",				STRING,		&unf_fg_a },
		{ "unf_bg_a",				STRING,		&unf_bg_a },
		{ "unf_brd_a",				STRING,		&unf_brd_a },
		{ "unf_flo_a",				STRING,		&unf_flo_a },
		{ "unf_sha_a",				STRING,		&unf_sha_a },
		{ "tag_fg_a",				STRING,		&tag_fg_a },
		{ "tag_bg_a",				STRING,		&tag_bg_a },
		{ "tag_brd_a",				STRING,		&tag_brd_a },
		{ "tag_flo_a",				STRING,		&tag_flo_a },
		{ "tag_sha_a",				STRING,		&tag_sha_a },
		{ "alt_fg_a",				STRING,		&alt_fg_a },
		{ "alt_bg_a",				STRING,		&alt_bg_a },
		{ "alt_brd_a",				STRING,		&alt_brd_a },
		{ "alt_flo_a",				STRING,		&alt_flo_a },
		{ "alt_sha_a",				STRING,		&alt_sha_a },

		{ "seppad",					INTEGER,	&seppad },
		{ "statussep",				INTEGER,	&statussep },
		{ "tabbarsep",				INTEGER,	&tabbarsep },
		{ "statustheme",			INTEGER,	&statustheme },
		{ "tabbartheme",			INTEGER,	&tabbartheme },
		{ "tagtheme",				INTEGER,	&tagtheme },
		{ "barborder",				INTEGER,	&barborder },
		{ "barsep",					INTEGER,	&barsep },
		{ "barhighlight",			INTEGER,	&barhighlight },
		{ "attachdirection",		INTEGER,	&attachdirection },
		{ "bargap",					INTEGER,	&bargap },
		{ "borderpx",				INTEGER,	&borderpx },
		{ "tagborderpx",			INTEGER,	&tagborderpx },
		{ "floatposgrid_x",			INTEGER,	&floatposgrid_x },
		{ "floatposgrid_y",			INTEGER,	&floatposgrid_y },
		{ "gappx",					INTEGER,	&gappx },
		{ "iconpad",				INTEGER,	&iconpad },
		{ "iconspacing",			INTEGER,	&iconspacing },
		{ "oneclientdimmer",		INTEGER,	&oneclientdimmer },
		{ "scalepreview",			INTEGER,	&scalepreview },
		{ "snap",					INTEGER,	&snap },
		{ "showbar",				INTEGER,	&showbar },
		{ "showebar",				INTEGER,	&showebar },
		{ "statuscenter",			INTEGER,	&statuscenter },
		{ "titlecenter",			INTEGER,	&titlecenter },
		{ "swallowfloating",		INTEGER,	&swallowfloating },
		{ "tileswitch",				INTEGER,	&tileswitch },
		{ "tileswitchcolor",		INTEGER,	&tileswitchcolor },
		{ "borderswitch",			INTEGER,	&borderswitch },
		{ "topbar",					INTEGER,	&topbar },
		{ "mfact",					FLOAT,		&mfact },
		{ "nmaster",				INTEGER,	&nmaster },
		{ "resizehints",			INTEGER,	&resizehints },
		{ "lockfullscreen",			INTEGER,	&lockfullscreen },
};
