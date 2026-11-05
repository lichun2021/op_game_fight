#include "ParaCurve.h"
#include <cmath>

namespace OnePiece {
	ParaCurve::ParaCurve(){}
	ParaCurve::~ParaCurve(){}

	ParaCurve::ParaCurve(float x1, float y1, float x2, float y2, float heightIndex)
	{
		ParaCurve::getABC(x1, y1, x2, y2, heightIndex, &a, &b, &c);
	}

	float ParaCurve::getYByX(float x)
	{
		return ParaCurve::getYByX(x, a, b, c);
	}

	float ParaCurve::getSlope(float x)
	{
		return 2*a*x + b;
	}

    void ParaCurve::initABCForCannon(float x1, float y1, float x2, float y2, float heightIndex)
    {
        if (x1 == x2) {
            a = 0, b = 0, c = 0;
        }
        else
        {
            float delt = x1 - x2;
            delt = delt > 0 ? delt : -delt;
            unsigned short x3 = (x1 + x2) / 2;
            unsigned short y3 = (y1 + y2) / 2 + heightIndex*delt / 7;

            a = (y1 - y2) / ((x1 - x3)*(x1 - x2)) - (y2 - y3) / ((x1 - x3)*(x2 - x3));
            b = (y1 - y2)*(x2 + x3) / ((x1 - x2)*(x3 - x1)) - (y2 - y3)*(x1 + x2) / ((x2 - x3)*(x3 - x1));
            c = y1 - (a)*x1*x1 - (b)*x1;
        }
    }

	void ParaCurve::getABC(float x1, float y1, float x2, float y2, float heightIndex, float* a, float* b, float* c)
	{
		if (x1 == x2) {
			*a = 0, *b = 0, *c = 0;
		}
		else if (std::abs(x1-x2)<200)
		{
			*a = 0;//y=c
			*b = 0;
			*c = (y1+y2)/2;
		}
		else if (heightIndex<0.2)
		{
			*a = 0;//y=bx+c
			*b = (y1 - y2) / (x1 - x2);
			*c = y1 - x1*(y1 - y2) / (x1 - x2);
		}
		else 
		{
			float delt = x1 - x2;
			delt = delt > 0 ? delt : -delt;
			unsigned short x3 = (x1 + x2) / 2;
			unsigned short y3 = (y1 + y2) / 2 + heightIndex*delt / 7;

			//a = y1 / ((x1 - x2)*(x1 - x3)) + y2 / ((x2 - x1)*(x2 - x3)) + y3 / ((x3 - x1) *(x3 - x2));
			//b = -y1 *(x2 + x3) / ((x1 - x2)*(x1 - x3)) - y2*(x1 + x3) / ((x2 - x1)*(x2 - x3)) - y3 *(x1 + x2) / ((x3 - x1) *(x3 - x2));
			//c = y1*x2*x3 / ((x1 - x2)*(x1 - x3)) + y2*x1*x3 / ((x2 - x1)*(x2 - x3)) + y3*x1*x2 / ((x3 - x1) *(x3 - x2));
			*a = (y1 - y2) / ((x1 - x3)*(x1 - x2)) - (y2 - y3) / ((x1 - x3)*(x2 - x3));
			*b = (y1 - y2)*(x2 + x3) / ((x1 - x2)*(x3 - x1)) - (y2 - y3)*(x1 + x2) / ((x2 - x3)*(x3 - x1));
			*c = y1 - (*a)*x1*x1 - (*b)*x1;
		}
	}

	float ParaCurve::getYByX(float x, float a, float b, float c)
	{
		//("ParaCurve getYByX: %f, %f", x, (a*x*x + b*x + c));
		return (a*x*x + b*x + c);
	}
}
