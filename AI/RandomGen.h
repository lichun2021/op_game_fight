//
//  RandomGen.h
//  Football
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__RandomGen__
#define __OnePiece__RandomGen__

#include<fstream>

inline void func_test(char *file, long line)
{
	std::ofstream outfream_ai;
	outfream_ai.open("AiLog.txt", std::ios::app | std::ios::ate);
	outfream_ai << "file: " << file;
	outfream_ai << "line: " << line << std::endl;
}


#define OutPutFileLine (1) //(func_test(__FILE__, __LINE__), 1)

namespace OnePiece {

	class RandomGen
	{
	private:
		unsigned int m_iSeed;

	public:
		RandomGen(const int& seed = 0) : m_iSeed(seed) { }

		int nextInt(int low = 0, int high = 65535)
		{
			//CCLOG("File = %s\nLine = %d\nFunc=%s\nDate=%s\nTime=%s\n", __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__);
			m_iSeed = (m_iSeed * 7 + 11) % 131071;
			return m_iSeed % (high - low) + low;
		}

		float nextFloat(float low = 0.0, float high = 1.0)
		{
			int tmp = nextInt();
			return ((float)tmp / (float)65535);
		}

		unsigned int getSeed(){ return m_iSeed; }
	};

}

#endif /* defined(__Football__RandomGen__) */
