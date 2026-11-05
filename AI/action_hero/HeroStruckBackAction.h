#ifndef __OnePiece__HeroStruckBackAction__
#define __OnePiece__HeroStruckBackAction__

#include "../AIContext.h"

namespace OnePiece {

	class HeroStruckBackAction
	{
		friend class HeroActionController;
	public:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
		static void Enter(CharacterData&aiCharacter, AIContext& aiContext, unsigned short tickLength, short speed);
        static void Exit(CharacterData& aiCharacter, AIContext& aiContext);
	};

}

#endif /* defined(__OnePiece__HeroMoveAction__) */