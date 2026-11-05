//
//  HeroEffectAction.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__HeroEffectAction__
#define __OnePiece__HeroEffectAction__

#include "HeroEffectAction.h"

namespace OnePiece {
	class AIContext;
	class CharacterData;

	class HeroEffectAction
	{
		friend class HeroActionController;
	public:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
        static bool Enter(CharacterData&aiCharacter, AIContext& aiContext, int effectID, int tickLength, ActionType oldActionType, int oldTotalTick, int oldTick, bool canEffBeDebuff, CharacterData& aiAttacker);
        static void Exit(CharacterData& aiCharacter, AIContext& aiContext);
        static bool ExtendDuration(CharacterData& aiCharacter, int effectID, int extendTick, bool canEffBeDebuff, CharacterData& aiAttacker, AIContext& aiContext);
	};
}

#endif /* defined(__OnePiece__HeroEffectAction__) */
