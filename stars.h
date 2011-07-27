/*************************************************************************
 *                                                                       *
 * Copyright (c) 1992, 1993 Ronald Joe Record                           *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/
 /*
 *  The X Consortium, and any party obtaining a copy of these files from
 *  the X Consortium, directly or indirectly, is granted, free of charge,
 *  a full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 *  nonexclusive right and license to deal in this software and
 *  documentation files (the "Software"), including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons who receive
 *  copies from any such party to do so.  This license includes without
 *  limitation a license to do the foregoing actions under any patents of
 *  the party supplying this software to the X Consortium.
 */

/*
 *	Ported to X (SCO UNIX 3.2 ODT) by Ron Record 27 Sep 1990.
 */

#include <assert.h>
#include <stdio.h>
#include <values.h>
#include <X11/StringDefs.h> 
#include <X11/Intrinsic.h> 
#include <X11/keysym.h> 
#include <Xm/Xm.h> 
#include <Xm/Form.h> 
#include <Xm/PushB.h> 
#include <Xm/DrawingA.h> 
#include <Xm/ToggleB.h> 
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#define ABS(a)	(((a)<0) ? (0-(a)) : (a) )
#define Min(x,y) ((x < y)?x:y)
#define Max(x,y) ((x > y)?x:y)

int x_center, y_center, col=1;
int radius = 2;
int fflag  = 0;
int dflag  = 0;
int write_mode = 4;
int Fflag  = 0;
int fill_style = 1;
int icflag  = 1;
int cflag  = 0;
int lflag  = 0;
int rflag  = 0;
int pflag  = 0;
int uflag  = 0;
int NoShowButtons = 0;
int FullScreen = 0;
int bottom;
Screen*		screen;
Display*	dpy;
char*		displayname = 0;

extern long time();
extern int optind;
extern char *optarg;

Widget framework, canvas, button[4];
XtWorkProcId work_proc_id = (XtWorkProcId)NULL;

XtEventHandler     Getkey();
void     resize();
void     redisplay();
void     quit();
void     Warp();
void     start_iterate();
void     stop_iterate();
Boolean  mvstars();

typedef struct {
	int x, y, u, v;
} xy_t;

typedef struct {
	int left, right, top, bottom;
} lrtb_t;

/* size of buffers which collect lines, circles, rectangles, etc. */
#define MAXPOINTS 64
/* maximum number of colors allowed */
#define MAXCOLOR  256
int	numcolors=256;
/* Maximum star velocity */
#define MAXVEL 4
#define PROBFAR 6
#define PROBNEAR 500
/* default window size */
#define MIN_WIDTH 480
#define MIN_HEIGHT 360

struct{
   XPoint  coord[MAXCOLOR][MAXPOINTS];
   int     npoints[MAXCOLOR];
} Points;

struct image_data {
  int          ncolors;
  GC           gc;
  Dimension    width, height;
} *Data[MAXCOLOR];

extern char *malloc();

#define NUMSIZ 4
#define VEL_INC 8

int	turn=0, randcol=1, reverse=0, warp=1, initcolor;
int	xinitialv=0, yinitialv=0, bindex=0, showit=0, maxstars=512;
int	width=0, height=0, xflag=1, yflag=1;
xy_t	midpt, velocity;
xy_t	*newpos;
xy_t	*initstars; 
xy_t	*stars;
int	*starcolor;
int	*dist;
int	distance=0, numgen=1;
Boolean appDefaultsFileInstalled=False;
