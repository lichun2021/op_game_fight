#ifndef __OnePiece__HeroTurnAction__
#define __OnePiece__HeroTurnAction__

#include "../AIContext.h"
#include "HeroActionController.h"

namespace OnePiece {

	class HeroTurnAction
	{
		//friend class HeroTurnAction;
	public:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
		static void Enter(CharacterData&aiCharacter, AIContext& aiContext);
	};

}

#endif