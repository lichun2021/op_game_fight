#ifndef __OnePiece__ParaCurve__
#define __OnePiece__ParaCurve__

namespace OnePiece {


	class ParaCurve
	{
	public:
		~ParaCurve();

	public:
		ParaCurve();
		ParaCurve(const ParaCurve&);
		ParaCurve(ParaCurve&);
		ParaCurve& operator=(ParaCurve&);

		ParaCurve(float x1, float y1, float x2, float y2, float heightIndex);//heightIndex:0-9
		float getYByX(float x);
		float getSlope(float x);

		static void getABC(float x1, float y1, float x2, float y2, float heightIndex, float* a, float* b, float* c);//heightIndex:0-9
		static float getYByX(float x, float a, float b, float c);

        void initABCForCannon(float x1, float y1, float x2, float y2, float heightIndex);

	private:
		// y = ax^2+bx+c;
		float a;
		float b;
		float c;

	};

}

#endif
