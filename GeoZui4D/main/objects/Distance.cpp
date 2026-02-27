#include "objects/Distance.h"
#include <iostream>
#include <math.h>

using namespace std;

Distance::Distance(int s)
{
    int i;
    
    size = s;
    
    table = new double[size];
    
    for(i = 0; i < size; i++)
    {
        table[i]=sqrt(1+(((double)i)/((double)size))*(((double)i)/((double)size)));
    }
}

double Distance::GetDistance(double x, double y)
{
    double min, max;
    
    if( x<0.0)
    {
        x =-x;
    }

    if( y<0.0)
    {
        y =-y;
    }
    
    if(x < y)
    {
        min = x;
        max = y;
    }
    else
    {
        min = y;
        max = x;
    }
    
    if(max == 0.0)
    {
        return 0.0;
    }
    else
    {
//        cerr << "min/max " << min/max << " *size " << (min/max)*size << " as int " << int((min/max)*size) << "\n";
        return table[int((min/max)*size)]*max;
    }
}
