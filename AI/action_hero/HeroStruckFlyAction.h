#ifndef __OnePiece__HeroStruckFlyAction__
#define __OnePiece__HeroStruckFlyAction__

#include "../AIContext.h"

namespace OnePiece {

	class HeroStruckFlyAction
	{
		friend class HeroActionController;
	public:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
		static void Enter(CharacterData&aiCharacter, AIContext& aiContext, short SpeedX, short SpeedY, int extendTick);
        static void Exit(CharacterData& aiCharacter, AIContext& aiContext);
	};

}

#endif /* defined(__OnePiece__HeroMoveAction__) */