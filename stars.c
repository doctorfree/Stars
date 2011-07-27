/*************************************************************************
 *                                                                       *
 * Copyright (c) 1991, 1992, 1993 Ronald Joe Record                           *
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

/* Written by Ron Record (rr@ronrecord.com) 09 June 1991 */

#include "stars.h"

main(ac, av)
	int ac;
	char **av;
{
	Widget toplevel;
	static int i;
	unsigned char wname[256];
	Arg wargs[1];
	static XtResource resource[] = {
	{ "appDefaultsFileInstalled" , "AppDefaultsFileInstalled", XtRBoolean,
		sizeof(Boolean), 0, XtRString, "False" } };

	parseargs(ac, av);
	toplevel = XtInitialize(av[0], "Stars", NULL, 0, &ac, av);
	framework = XtCreateManagedWidget("framework", 
                         xmFormWidgetClass, toplevel, NULL, 0);
	/*
 	* Create the canvas widget to display the star portal 
 	*/
	canvas = XtCreateManagedWidget("drawing_canvas", 
                   xmDrawingAreaWidgetClass, framework, NULL, 0);
    	dpy = XtDisplay(canvas);
    	screen = XtScreen(canvas);
	if (FullScreen) {
        	width = XDisplayWidth(dpy, DefaultScreen(dpy));
        	height = XDisplayHeight(dpy, DefaultScreen(dpy));
	}
	/* 
 	* Create the pushbutton widgets. 
 	*/ 
	button[0] =  XtCreateManagedWidget("go_button",
                                xmPushButtonWidgetClass,
                                framework, NULL, 0);
	button[1] =  XtCreateManagedWidget("stop_button",
                                xmPushButtonWidgetClass,
                                framework, NULL, 0);
	button[2] =  XtCreateManagedWidget("quit_button",
                                xmPushButtonWidgetClass,
                                framework, NULL, 0);
	button[3] =  XtCreateManagedWidget("warp_button",
                                xmPushButtonWidgetClass,
                                framework, NULL, 0);
	init_data();
	init_canvas();
	x_center = width / 2;
	y_center = height / 2;
	if (xflag)
		midpt.x = x_center;
	if (yflag)
		midpt.y = y_center;
	initsky();
	srand((int)time(0));
	if (Fflag)
	    for (i=0; i<=numcolors; i++)
		XSetFillStyle(dpy, Data[i]->gc, FillTiled);
	if (fflag)
	    for (i=0; i<=numcolors; i++)
		XSetFunction(dpy, Data[i]->gc, write_mode);
	/*  
 	* Add callbacks. 
 	*/ 
	XtAddCallback(button[0],XmNactivateCallback,start_iterate,NULL);
	XtAddCallback(button[1],XmNactivateCallback,stop_iterate,NULL);
	XtAddCallback(button[2], XmNactivateCallback, quit, NULL);
	XtAddCallback(button[3], XmNactivateCallback, Warp, NULL);
	XtRealizeWidget(toplevel);
	/* Title */
	sprintf((char *) wname, "Stars by Ron Record");
 	XChangeProperty(dpy, XtWindow(toplevel), XA_WM_NAME, XA_STRING, 8, 
 		PropModeReplace, wname, strlen(wname));
	if (NoShowButtons) {
		for (i=0;i<4;i++)
			XtUnrealizeWidget(button[i]);
		XtSetArg(wargs[0], XmNbottomPosition, &bottom);
		XtGetValues(canvas, wargs, 1);
		XtSetArg(wargs[0], XmNbottomPosition, 99);
		XtSetValues(canvas, wargs, 1);
	}
	XtAddCallback(canvas, XmNresizeCallback, resize, Data); 
	XtAddEventHandler(canvas, KeyPressMask,
			 FALSE, (XtEventHandler)Getkey, NULL);
	XtAddEventHandler(button[0], KeyPressMask,
			 FALSE, (XtEventHandler)Getkey, NULL);
	XtAddEventHandler(button[1], KeyPressMask,
			 FALSE, (XtEventHandler)Getkey, NULL);
	XtAddEventHandler(button[2], KeyPressMask,
			 FALSE, (XtEventHandler)Getkey, NULL);
	XtAddEventHandler(button[3], KeyPressMask,
			 FALSE, (XtEventHandler)Getkey, NULL);
	Clear();
	if (icflag)
		initcolor = WhitePixelOfScreen(screen);
  	work_proc_id = XtAddWorkProc(mvstars, canvas);
	XtGetApplicationResources(toplevel, &appDefaultsFileInstalled, resource,
			1, NULL, 0);
	if (!appDefaultsFileInstalled)
		fprintf(stderr, 
				"Warning: the Stars app-defaults file is not installed\n");
	XtMainLoop();
}

initsky()
{
	register i;
	static int revinit;

	newpos = (xy_t *)malloc(sizeof(xy_t));
	initstars = (xy_t *)malloc((maxstars+1)*sizeof(xy_t));
	stars = (xy_t *)malloc((maxstars+1)*sizeof(xy_t));
	starcolor = (int *)malloc((maxstars+1)*sizeof(int));
	dist = (int *)malloc((maxstars+1)*sizeof(int));
	velocity.x = xinitialv;
	velocity.y = yinitialv;
	srand(getpid());
	revinit=0;
	if (reverse) {
		revinit=1;
		reverse = (!reverse);
	}
	for (i=0;i<maxstars;i++) {
		if (rand() % PROBFAR)
			initstars[i].v = (rand() % MAXVEL) + 1;
		else
			initstars[i].v = 0;
		newstar(i);
	}
	if (revinit)
		reverse = (!reverse);
}

Boolean
mvstars(w, data, call_data)
Widget	w;
struct image_data	*data[];
XmDrawingAreaCallbackStruct	*call_data;
{
	register i;
	extern xy_t *mv_line();

	for (i=0;i<maxstars;i++) {
		if ((warp) && (initstars[i].u))
	 		sendpoint(BlackPixel(dpy,XDefaultScreen(dpy)), stars[i]);
		mv_line(initstars[i], midpt, dist[i], newpos);
		stars[i].x = newpos[0].x;
		stars[i].y = newpos[0].y;
		if (reverse) {
			dist[i] += initstars[i].u;
			if ((dist[i] > ABS(midpt.x-initstars[i].x)) &&
			    (dist[i] > ABS(midpt.y-initstars[i].y)))
				newstar(i);
		}
		else {
			dist[i] -= initstars[i].u;
		}
		if ((stars[i].x >= width) || (stars[i].y >= height) ||
			(stars[i].x < 0) || (stars[i].y < 0))
				newstar(i);
		sendpoint(starcolor[i], stars[i]);
		if (initstars[i].u == 0)
			if ((rand() % PROBNEAR) == 1)
				initstars[i].u = (rand() % MAXVEL) + 1;
	}
	if (warp && showit)
		sendpoint(BlackPixel(dpy,XDefaultScreen(dpy)), midpt);
	if (turn)
		turnmidpt();
	if (numgen++ == distance)
		exit(0);
	midpt.x += velocity.x;
	midpt.y += velocity.y;
	if (midpt.x >= width) {
		midpt.x = width;; 
		velocity.x *= -1;
	}
	if (midpt.x <= 0) {
		midpt.x = 0; 
		velocity.x *= -1;
	}
	if (midpt.y >= height) {
		midpt.y = height;
		velocity.y *= -1;
	}
	if (midpt.y <= 0) {
		midpt.y = 0;
		velocity.y *= -1;
	}
	if (showit)
		sendpoint(WhitePixel(dpy,XDefaultScreen(dpy)), midpt);
	return FALSE;
}

init_data()
{
	static int i;

	for (i=0; i<MAXCOLOR; i++) {
		if((Data[i]=(struct image_data *)malloc(sizeof(struct image_data)))==
						NULL) {
			fprintf(stderr,"Error malloc'ing Data[%d].\n",i);
			exit(-1);
		}
    	Data[i]->ncolors = XDisplayCells(dpy, XDefaultScreen(dpy));
		Data[i]->width = 0;
		Data[i]->height = 0;
	}
	numcolors = Min(Data[0]->ncolors, MAXCOLOR) - 1;
}

init_canvas()
{
	Arg wargs[4];
	XGCValues gcv;
	XmString label;
	static int i;

	/*
 	* Set the size of the drawing areas.
 	*/
	if (!width)
		width = MIN_WIDTH;
	if (!height)
		height = MIN_HEIGHT;
	XtSetArg(wargs[0], XtNwidth, width);
	XtSetArg(wargs[1], XtNheight, height);
	XtSetValues(framework, wargs,2);
	/*
 	* create default, writable, graphics contexts for the canvas.
 	*/
	for (i=0; i<numcolors; i++) {
	    Data[i]->gc = XCreateGC(dpy, DefaultRootWindow(dpy), 
					0, (XGCValues *)NULL); 
	    Data[i]->width = width;
	    Data[i]->height = height;
	    /* set the background to black */
	    XSetBackground(dpy,Data[i]->gc,BlackPixel(dpy,XDefaultScreen(dpy)));
	    /* set the foreground of the ith context to i */
	    XSetForeground(dpy, Data[i]->gc, i);
	}
	/* label the buttons correctly */
	if (reverse) {
	    label = XmStringCreate("REVERSE",XmSTRING_DEFAULT_CHARSET);
	    XtSetArg(wargs[0], XmNlabelString, label);
	    XtSetValues(button[1], wargs, 1);
	}
	else {
	    label = XmStringCreate("FORWARD",XmSTRING_DEFAULT_CHARSET);
	    XtSetArg(wargs[0], XmNlabelString, label);
	    XtSetValues(button[1], wargs, 1);
	}
	if (warp) {
	    label = XmStringCreate("IMPULSE ON",XmSTRING_DEFAULT_CHARSET);
	    XtSetArg(wargs[0], XmNlabelString, label);
	    XtSetValues(button[3], wargs, 1);
	}
	else {
	    label = XmStringCreate("WARP ON",XmSTRING_DEFAULT_CHARSET);
	    XtSetArg(wargs[0], XmNlabelString, label);
	    XtSetValues(button[3], wargs, 1);
	}
	/* zero the point buffer */
	init_buffer();
}

void 
resize(w, data, call_data)
Widget         w;
struct image_data    *data[];
caddr_t        call_data;
{
	static int i;
	Arg wargs[10];
	/*  
 	*   Get the new window size.
 	*/   
	for (i=0;i<numcolors;i++) {
		XtSetArg(wargs[0], XtNwidth,  &Data[i]->width);
		XtSetArg(wargs[1], XtNheight, &Data[i]->height);
		XtGetValues(w, wargs, 2);
	}
	/*
 	* Clear the window.
 	*/
 	if(XtIsRealized(canvas))
     		XClearArea(dpy, XtWindow(canvas), 0, 0, 0, 0, TRUE);
	/* reset the buffers containing the lines, circles, boxes, etc. */
	init_buffer();
	/* recalculate offsets */
	width = Data[0]->width; height = Data[0]->height;
	midpt.x = x_center = width / 2;
	midpt.y = y_center = height / 2;
	for (i=0;i<maxstars;i++)
		newstar(i);
}

parseargs(ac, av)
int ac;
char **av;
{
	register int c;
	extern int optind;
	extern char *optarg;

	while ((c = getopt(ac, av, "AMFSXfruwB:W:H:c:a:b:D:n:x:y:")) != EOF) {
		switch (c) {
		case 'B':	bindex=atoi(optarg); 
				if ((bindex > NUMSIZ) || (bindex < 0))
					usage();
				break;
		case 'M':	randcol--; break;
		case 'F':	FullScreen=1; break;
		case 'H':	height=atoi(optarg); break;
		case 'S':	showit++; break;
		case 'W':	width=atoi(optarg); break;
		case 'X':	turn++; break;
		case 'f':	NoShowButtons=1; break;
		case 'r':	reverse++; break;
		case 'w':	warp--; break;
		case 'c':	icflag=0; initcolor=atoi(optarg); break;
		case 'a':	xinitialv=atoi(optarg); break;
		case 'b':	yinitialv=atoi(optarg); break;
		case 'D':	distance=atoi(optarg); break;
		case 'n':	maxstars=atoi(optarg); break;
		case 'x':	midpt.x=atoi(optarg); xflag=0; break;
		case 'y':	midpt.y=atoi(optarg); yflag=0; break;
		case 'u':
		case '?':	usage(); break;
		}
	}
}

usage()
{
	fprintf(stderr,"stars [-AMXruw][-B[0123]][-a#][-b#][-c#][-d#][-x xmid][-y ymid][-H#][-W#]\n");
	fprintf(stderr,"\tWhere: -M indicates use monochrome\n");
	fprintf(stderr,"\t       -S indicates show midpt\n");
	fprintf(stderr,"\t       -X indicates randomly walk the midpt\n");
	fprintf(stderr,"\t       -r indicates put the ship in reverse\n");
	fprintf(stderr,"\t       -u displays this message\n");
	fprintf(stderr,"\t       -w indicates put the ship in warp\n");
	fprintf(stderr,"\t       -B# specifies the initial star size\n");
	fprintf(stderr,"\t       -H# specifies the initial window height\n");
	fprintf(stderr,"\t       -W# specifies the initial window width\n");
	fprintf(stderr,"\t       -a# specifies the inital 'x' velocity\n");
	fprintf(stderr,"\t       -b# specifies the inital 'y' velocity\n");
	fprintf(stderr,"\t       -c# specifies the star color\n");
	fprintf(stderr,"\t       -d# specifies the duration of display\n");
	fprintf(stderr,"\tDuring display :\n");
	fprintf(stderr,"\t  [hjklHJKL] rogue-like control of midpt movement\n");
	fprintf(stderr,"\t        '.' stops the motion of the midpt\n");
	fprintf(stderr,"\t        '+' resets the midpt to screen center\n");
	fprintf(stderr,"\t  '>'or '<' increases/decreases the delay (text mode)\n");
	fprintf(stderr,"\t     b or B selects star size\n");
	fprintf(stderr,"\t     c or C toggles all/one color (graphics mode only)");
	fprintf(stderr,"\n\t     t or T toggles random walk of midpt\n");
	fprintf(stderr,"\t     x or X clears the screen\n");
	fprintf(stderr,"\t     r or R toggles reverse/forward\n");
	fprintf(stderr,"\t     s or S toggles midpt display\n");
	fprintf(stderr,"\t     w or W toggles warp/normal\n");
	fprintf(stderr,"\t     q or Q exits\n");
	exit(1);
}

init_buffer()
{
	int i;

	for(i=0;i<MAXCOLOR;i++) {
		Points.npoints[i] = 0;
	}
}

buffer_bit(color, x , y)
int         color, x,y;
{
	static int i;

	if(Points.npoints[color] == MAXPOINTS - 1){
  	/*
   	* If the buffer is full, set the foreground color
   	* of the graphics context and draw the points in the window.
   	*/
	    for (i=0; i<=numcolors; i++) {
		  if (Points.npoints[i]) {
  			XDrawPoints (dpy, XtWindow(canvas), Data[i]->gc, 
               			Points.coord[i], Points.npoints[i], CoordModeOrigin);
  			/*
   			* Reset the buffer.
   			*/
  			Points.npoints[i] = 0;
		  }
	    }
	}
	/*
 	* Store the point in the buffer according to its color.
 	*/
	Points.coord[color][Points.npoints[color]].x = x;
	Points.coord[color][Points.npoints[color]].y = y;
	Points.npoints[color]++;
}

void
start_iterate(w, cv, call_data)
Widget w, cv;
XmAnyCallbackStruct *call_data;
{
	XmString label;
	Arg wargs[1];

	if(work_proc_id) {
	    XtRemoveWorkProc(work_proc_id);
	    work_proc_id = (XtWorkProcId)NULL; 
	    if (warp) {
	      label = XmStringCreate("RESUME IMPULSE",XmSTRING_DEFAULT_CHARSET);
	      XtSetArg(wargs[0], XmNlabelString, label);
	      XtSetValues(button[0], wargs, 1);
	    }
	    else {
	      label = XmStringCreate("RESUME WARP",XmSTRING_DEFAULT_CHARSET);
	      XtSetArg(wargs[0], XmNlabelString, label);
	      XtSetValues(button[0], wargs, 1);
	    }
	    label = XmStringCreate("ENGINES OFF",XmSTRING_DEFAULT_CHARSET);
	    XtSetArg(wargs[0], XmNlabelString, label);
	    XtSetValues(button[3], wargs, 1);
	}
	else {
		/*
 		* Register mvstars() as a WorkProc.
 		*/
		work_proc_id = XtAddWorkProc((XtWorkProc)mvstars, cv);
		label = XmStringCreate("STOP",XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[0], XmNlabelString, label);
		XtSetValues(button[0], wargs, 1);
		if (warp) {
		  label = XmStringCreate("IMPULSE ON",XmSTRING_DEFAULT_CHARSET);
		  XtSetArg(wargs[0], XmNlabelString, label);
		  XtSetValues(button[3], wargs, 1);
		}
		else {
		  label = XmStringCreate("WARP ON",XmSTRING_DEFAULT_CHARSET);
		  XtSetArg(wargs[0], XmNlabelString, label);
		  XtSetValues(button[3], wargs, 1);
		}
	}
}

void 
stop_iterate(w, cv, call_data)
Widget                w, cv;
XmAnyCallbackStruct  *call_data;
{
	XmString label;
	Arg wargs[1];

	reverse = (!reverse);
	if (reverse) {
		label = XmStringCreate("REVERSE",XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[0], XmNlabelString, label);
		XtSetValues(button[1], wargs, 1);
	}
	else {
		label = XmStringCreate("FORWARD",XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[0], XmNlabelString, label);
		XtSetValues(button[1], wargs, 1);
	}
}

void
Warp(w, call_value)
Widget w;
XmAnyCallbackStruct *call_value;
{
	XmString label;
	Arg wargs[1];

	warp = (!warp);
	if (warp) {
	    Clear();
	      if (work_proc_id) {
		label = XmStringCreate("IMPULSE ON",XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[0], XmNlabelString, label);
		XtSetValues(button[3], wargs, 1);
	      }
	      else {
	        label=XmStringCreate("ENGINES OFF",XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[0], XmNlabelString, label);
		XtSetValues(button[3], wargs, 1);
		label=XmStringCreate("RESUME IMPULSE",XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[0], XmNlabelString, label);
		XtSetValues(button[0], wargs, 1);
	      }
	}
	else {
	    if (work_proc_id) {
		label = XmStringCreate("WARP ON",XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[0], XmNlabelString, label);
		XtSetValues(button[3], wargs, 1);
	    }
	    else {
		label=XmStringCreate("ENGINES OFF",XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[0], XmNlabelString, label);
		XtSetValues(button[0], wargs, 1);
		label=XmStringCreate("RESUME WARP",XmSTRING_DEFAULT_CHARSET);
		XtSetArg(wargs[0], XmNlabelString, label);
		XtSetValues(button[0], wargs, 1);
	    }
	}
}

void
quit(w, call_value)
Widget w;
XmAnyCallbackStruct *call_value;
{
	exit(0);
}

turnmidpt()
{

	if (rand() % 2)
		midpt.x++;
	else
		midpt.x--;
	if (rand() % 2)
		midpt.y++;
	else
		midpt.y--;
	if (midpt.x >= width-1)
		midpt.x = width - 2; 
	if (midpt.x <= 1)
		midpt.x = 2; 
	if (midpt.y >= height-1)
		midpt.y = height - 2;
	if (midpt.y <= 1)
		midpt.y = 2;
}

XtEventHandler
Getkey(w, client_data, event)
Widget w;
caddr_t client_data;
XKeyEvent *event;
{
	Arg wargs[1];
	static int i;

	switch (XLookupKeysym(event, 0)) {
	case XK_h: velocity.x--; break;
	case XK_j: velocity.y--; break;
	case XK_k: velocity.y++; break;
	case XK_l: velocity.x++; break;
	case XK_H: velocity.x -= VEL_INC; break;
	case XK_J: velocity.y -= VEL_INC; break;
	case XK_K: velocity.y += VEL_INC; break;
	case XK_L: velocity.x += VEL_INC; break;
	case XK_plus:
	case XK_equal: midpt.x = width/2; midpt.y = height/2; Clear();
	case XK_period: velocity.x = velocity.y = 0; break;
	case XK_B: bindex=0; 
		  Clear();
		  break;
	case XK_b: ++bindex; bindex=bindex % (NUMSIZ+1);
		  Clear(); break;
	case XK_f:
	case XK_F: NoShowButtons = (!NoShowButtons);
		   if (NoShowButtons) {
			for (i=0;i<4;i++)
				XtUnrealizeWidget(button[i]);
			XtSetArg(wargs[0], XmNbottomPosition, &bottom);
			XtGetValues(canvas, wargs, 1);
			XtSetArg(wargs[0], XmNbottomPosition, 99);
			XtSetValues(canvas, wargs, 1);
		   }
		   else {
			XtSetArg(wargs[0], XmNbottomPosition, bottom);
			XtSetValues(canvas, wargs, 1);
			for (i=0;i<4;i++) {
				XtManageChild(button[i]);
				XtRealizeWidget(button[i]);
			}
		   }
		   break;
	case XK_C:
	case XK_c: randcol = (!randcol); break;
	case XK_r:
	case XK_R: reverse = (!reverse); break;
	case XK_s:
	case XK_S: showit = (!showit); break;
	case XK_t:
	case XK_T: turn = (!turn); break;
	case XK_X:
	case XK_x: Clear(); break;
	case XK_w:
	case XK_W: warp = (!warp); 
		   if (warp)
			Clear();
		   break;
	case XK_q:
	case XK_Q: exit(0); break;
	default:  break;
	}
}

/* a star is born
 */
newstar(i)
{
	if (reverse) {
		if (rand() % 2) {
			initstars[i].y = stars[i].y= rand()%(height-1);
			if (rand() % 2)
				initstars[i].x = stars[i].x=1;
			else
				initstars[i].x = stars[i].x=width-1;
		}
		else {
			initstars[i].x = stars[i].x= rand()%(width-1);
			if (rand() % 2)
				initstars[i].y = stars[i].y=1;
			else
				initstars[i].y = stars[i].y=height-1;
		}
	}
	else {
		initstars[i].y = stars[i].y= rand()%(height-1);
		initstars[i].x = stars[i].x= rand()%(width-1);
	}
	if (randcol)
		starcolor[i] = (rand() % (numcolors-2)) + 2;
	else
		starcolor[i] = initcolor;
	dist[i] = 0;
	initstars[i].u = initstars[i].v;
}

xy_t points[NUMSIZ][16] = {
/* ptsize[0] */	{ {-1,-1},{1,-1},{-1,1},{1,1},{0,0},{0,0},{0,0},{0,0},
		{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} },
/* ptsize[1] */	{ {0,-3},{-2,-2},{0,-2},{2,-2},{-1,-1},{0,-1},{1,-1},{-1,1},
		{0,1},{1,1},{-2,2},{0,2},{2,2},{0,3},{0,0},{0,0} },
/* ptsize[2] */	{ {0,-3},{0,-2},{-1,-1},{0,-1},{1,-1},{-2,0},{-1,0},{1,0},
		{2,0},{-1,1},{0,1},{1,1},{0,2},{0,3},{0,0},{0,0} },
/* ptsize[3] */	{ {-4,-4},{0,-4},{4,-4},{-2,-2},{0,-2},{2,-2},{0,-6},{0,8},
		{0,-8},{0,6},{-2,2},{0,2},{2,2},{-4,4},{0,4},{4,4} }
};

int ptsize[NUMSIZ] = { 4, 14, 14, 16 };

sendpoint(color, point)
int color;
xy_t point;
{
	static int i;
	xy_t newp;

	if (bindex) {
		for (i=0; i < ptsize[bindex-1]; i++) {
			newp.x = point.x + points[bindex-1][i].x;
			newp.y = point.y + points[bindex-1][i].y;
			if (newp.x >= width) newp.x = width-1;
			if (newp.x < 0) newp.x = 0;
			if (newp.y >= height) newp.y = height-1;
			if (newp.y < 0) newp.y = 0;
			buffer_bit(color, newp.x, newp.y);
		}
		buffer_bit(color, point.x, point.y);
	}
	else
		buffer_bit(color, point.x, point.y);
}

Clear() {
	static int i;

 	if(XtIsRealized(canvas))
     	XClearArea(dpy, XtWindow(canvas), 0, 0, 0, 0, TRUE);
	for (i=0; i<=numcolors; i++)
  		Points.npoints[i] = 0;
}
