#ifndef FROM_DWM_C
#include "dwm.h"
#endif /* COMPILING */

/* See LICENSE file for copyright and license details. */

#ifdef __clang__
#pragma clang diagnostic ignored "-Wundefined-internal"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

/* appearance */
#define SHOULD_DRAW_CHRISTMAS_LIGHTS(month) (month == 11 || month == 0)

static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "monospace:size=13" };
static const char dmenufont[]       = "monospace:size=13";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const char scratchpadname[] = "scratchpad";
static const char float_terminal_name[] = "floaty";

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class     instance  title           tags mask  isfloating  isterminal  noswallow  monitor    float x,y,w,h       floatborderpx*/
	{ "firefox",  NULL,     NULL,           1 << 1,    0,          0,           0,        -1,       -1,-1,-1,-1,         -1},
	{ "kitty",    NULL,     NULL,           0,         0,          1,           0,        -1,       -1,-1,-1,-1,         -1},
	{ "kitty",    NULL,     scratchpadname, 0,         1,          1,           0,        -1,       -1,-1,960,540,       -1},
	{ "obsidian", NULL,     NULL,           1 << 6,    0,          0,           0,        -1,       -1,-1,-1,-1,         -1},
	{ "discord",  NULL,     NULL,           1 << 3,    0,          0,           0,        -1,       -1,-1,-1,-1,         -1},
	{ NULL,       NULL,     "Event Tester", 0,         0,          0,           1,        -1,       -1,-1,-1,-1,         -1}, /* xev */
	{ "kitty",    NULL,float_terminal_name, 0,         1,          1,           0,        -1,       480,270,960,540,     -1},
};

/* status bar for the statuscmd patch */
#define STATUSBAR "dwmblocks"

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

static Workspace workspaces[4];

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

#define REFRESH_DWMBLOCKS_KEYBOARD_CODE "13"
#define KEYBOARD_LAYOUTS "us,il,ru"

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }
#define SET_KEYBOARD_LAYOUT(layout) SHCMD("[[ $(xkb-switch -l | wc -l) -le 1 ]] && setxkbmap -layout " KEYBOARD_LAYOUTS " ; xkb-switch -s " layout " && pkill -RTMIN+" REFRESH_DWMBLOCKS_KEYBOARD_CODE " dwmblocks")

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "kitty", NULL };
static const char *screenshotcmd[] = { "screenshot-window", NULL };
static const char *screenshotallcmd[] = { "screenshot", NULL };
static const char *scratchpadcmd[] = { "kitty", "-T", scratchpadname, NULL };
static const char *float_terminal_cmd[] = { "kitty", "-T", float_terminal_name, NULL };
static const char *suspendcmd[] = {"systemctl", "suspend", NULL };
static const char *brightnessdowncmd[] = {"brillo", "-q", "-U", "1", NULL };
static const char *brightnessupcmd[] = {"brillo", "-q", "-A", "1", NULL };
static const char *poweroffcmd[] = { "shutdown-prompt", NULL };

static const Key keys[] = {
	/* modifier                     key        function        argument */
    { 0,                            XK_Print,  spawn,          {.v = screenshotcmd } },
    { MODKEY,                       XK_Print,  spawn,          {.v = screenshotallcmd } },
    { MODKEY|ShiftMask,             XK_r,      spawn,          SET_KEYBOARD_LAYOUT("ru") },
    { MODKEY|ShiftMask,             XK_e,      spawn,          SET_KEYBOARD_LAYOUT("us") },
    { MODKEY|ShiftMask,             XK_h,      spawn,          SET_KEYBOARD_LAYOUT("il") },
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
    { MODKEY,                       XK_s,      spawn,          {.v = suspendcmd } },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
    { MODKEY,                       XK_grave,  togglescratch,  {.v = scratchpadcmd } },
    { MODKEY|ShiftMask,             XK_grave,  spawn,          {.v = float_terminal_cmd } },
    { MODKEY|ShiftMask,             XK_j,      nextws,         {0} },
	{ MODKEY|ShiftMask,             XK_k,      prevws,         {0} },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
    { MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
    { MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
    { MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY|ShiftMask,             XK_space,  setlayout,      {0} },
	{ MODKEY,                       XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },

    { 0,                            XF86XK_MonBrightnessDown, spawn, {.v = brightnessdowncmd}},
    { 0,                            XF86XK_MonBrightnessUp,   spawn, {.v = brightnessupcmd}},

    { 0,                            XF86XK_PowerOff,          spawn, {.v = poweroffcmd} },

	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quitprompt,    {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
    { ClkStatusText,        0,              Button1,        sigstatusbar,   {.i = 1} },
	{ ClkStatusText,        0,              Button2,        sigstatusbar,   {.i = 2} },
	{ ClkStatusText,        0,              Button3,        sigstatusbar,   {.i = 3} },
	{ ClkStatusText,        ShiftMask,      Button1,        sigstatusbar,   {.i = 4} },
	{ ClkStatusText,        ShiftMask,      Button2,        sigstatusbar,   {.i = 5} },
	{ ClkStatusText,        ShiftMask,      Button3,        sigstatusbar,   {.i = 6} },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

