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

#define ABSOLUTE_VALUE(a)	(((a)<0) ? (0-(a)) : (a) )

typedef struct {
	int x, y, u, v;
} xy_t;

/*
 * mv_line() takes a pair of points, a distance, and an xy_t pointer
 * and returns the xy_t pointer pointing to a point whose coordinates
 * are those along the line from 'p0' to 'p1' distance 'dir'. 
 * The distance can be negative. 
 */

xy_t *
mv_line(p0, p1, dir, newpt)
int dir;
xy_t p0, p1;
xy_t *newpt;
{
	xy_t delta;
	xy_t *move_vert_line();
	xy_t *move_horz_line();
	xy_t *move_steep_line();
	xy_t *move_shallow_line();

	delta.x = p0.x - p1.x;
	delta.y = p0.y - p1.y;
	if(0==delta.x)
		move_vert_line(p0, p1, dir, newpt);
	else if(0==delta.y)
		move_horz_line(p0, p1, dir, newpt);
	else if(ABSOLUTE_VALUE(delta.y) > ABSOLUTE_VALUE(delta.x))
		move_steep_line(p0, p1, delta, dir, newpt);
	else
		move_shallow_line(p0, p1, delta, dir, newpt);
	return(newpt);
}

xy_t *
move_steep_line(p0, p1, delta, dir, p2)
int dir;
xy_t p0, p1, delta;
xy_t *p2;
{
	float slope;

	p2[0].x = p0.x;
	p2[0].y = p0.y;
	if(p0.y > p1.y)
		p2[0].y -= dir;
	else
		p2[0].y += dir;
	slope = (float)delta.y / (float)delta.x;
	p2[0].x = (((float)(p2[0].y - p1.y))/slope) + (float)p1.x;
	return(p2);
}

xy_t *
move_shallow_line(p0, p1, delta, dir, p2)
int dir;
xy_t p0, p1, delta;
xy_t *p2;
{
	float slope;

	p2[0].x = p0.x;
	p2[0].y = p0.y;
	if(p0.x > p1.x)
		p2[0].x -= dir;
	else
		p2[0].x += dir;
	slope = (float)delta.y / (float)delta.x;
	p2[0].y = (((float)(p2[0].x - p1.x)) * slope) + p1.y;
	return(p2);
}

xy_t *
move_horz_line(p0, p1, dir, p2)
int dir;
xy_t p0, p1;
xy_t *p2;
{
	p2[0].x = p0.x;
	p2[0].y = p0.y;
	if (p0.x > p1.x)
		p2[0].x -= dir;
	else
		p2[0].x += dir;
	return(p2);
}

xy_t *
move_vert_line(p0, p1, dir, p2)
int dir;
xy_t p0, p1;
xy_t *p2;
{
	p2[0].x = p0.x;
	p2[0].y = p0.y;
	if (p0.y > p1.y)
		p2[0].y -= dir;
	else
		p2[0].y += dir;
	return(p2);
}
