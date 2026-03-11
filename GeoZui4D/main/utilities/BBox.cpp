// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include "SDV_types.h"
#include "utilities/BBox.h"
#include<iostream>

using namespace std;

BBox::BBox()
{
    reset();
}

void BBox::reset()
{
    n = e = s = w = t = b = start = end = 0.0;
}

int BBox::merge(BBox bbox)
{
	int ret = 0;
	
	if(!hasVolume())
	{
		n = bbox.n;
		e = bbox.e;
		s = bbox.s;
		w = bbox.w;
		t = bbox.t;
		b = bbox.b;
		ret = 1;
	}
	else
	{
    if(bbox.e > e)
    {
        e = bbox.e;
		  ret = 1;
    }
    if(bbox.w < w)
    {
        w = bbox.w;
		  ret = 1;
    }
    if(bbox.n > n)
    {
        n = bbox.n;
		  ret = 1;
    }
    if(bbox.s < s)
    {
        s = bbox.s;
		  ret = 1;
    }
    if(bbox.t > t)
    {
        t = bbox.t;
		  ret = 1;
    }
    if(bbox.b < b)
    {
        b = bbox.b;
		  ret = 1;
    }
	}
	if((bbox.start != 0) || (bbox.end != 0))
	{
		if((start != 0) || (end != 0))
		{
			if(bbox.end > end)
			{
				end = bbox.end;
				ret = 1;
			}
			if(bbox.start < start)
			{
				start = bbox.start;
				ret = 1;
			}
		}
		else
		{
			end = bbox.end;
			start = bbox.start;
			ret = 1;
		}
	}
	return ret;
}


/*---------------------------------------------------------------------------*/
// Makes the bounding box big enough to include the passed in point, if it
// doesn't already fit in the bbox
/*---------------------------------------------------------------------------*/
void BBox::expand(gz4d::Point<double> const &point)
{
    if(point[0] > e)
    {
        e = point[0];
    }
    if(point[0] < w)
    {
        w = point[0];
    }
    if(point[1] > n)
    {
        n = point[1];
    }
    if(point[1] < s)
    {
        s = point[1];
    }
    if(point[2] > t)
    {
        t = point[2];
    }
    if(point[2] < b)
    {
        b = point[2];
    }
}

void BBox::set(gz4d::Point<double> const &p)
{
	e = w = p[0];
	n = s = p[1];
	t = b = p[2];
}

void BBox::set(BBox bb)
{
	e = bb.e;
	w = bb.w;
	n = bb.n;
	s = bb.s;
	start = bb.start;
	end = bb.end;
	t = bb.t;
	b = bb.b;
}

int BBox::overlapps(BBox b)
{
	if(b.w < e && b.e > w && b.s < n && b.n > s)
	{
		return 1;
	}
	return 0;		
}

bool BBox::contains(gz4d::Point<double> const &p)
{
    return ( p[0] >= w && p[0] <= e && p[1] >= s && p[1] <= n && p[2] >= b && p[2] <= t);
}

void BBox::expandTime(double t)
{
	if(start == 0.0 && end == 0.0)
	{
		start = end = t;
	}
	else
	{
		if(start > t)
			start = t;
		if(end < t)
			end = t;
	}
}

void BBox::expandTime(double t1, double t2)
{
    if(start == 0 || start > t1)
        start = t1;
    if(end < t2)
        end = t2;
}

int BBox::hasVolume()
{
	if(e==w && n==s && t==b)
		return 0;
	return 1;
}

void BBox::debugPrint()
{
	cerr << "BBox: n,s,e,w,t,b,s,e " << n << ", " << s << ", " << e << ", " << w << ", " << t << ", " << b << ", " << start << ", " << end << endl;
}

int BBox::isInX(double x)
{
	return(x >= w && x <= e);
}

int BBox::isInY(double y)
{
	return(y >= s && y <= n);
}
int BBox::isInZ(double z)
{
	return(z >= b && z <= t);
}
