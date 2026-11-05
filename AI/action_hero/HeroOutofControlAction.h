//
//  HeroStunAction.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__HeroOutofControlAction__
#define __OnePiece__HeroOutofControlAction__

#include "HeroActionController.h"

namespace OnePiece {
	class AIContext;

	class HeroOutofControlAction
	{
		friend class HeroActionController;
	public:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
		static bool Enter(CharacterData&aiCharacter, AIContext& aiContext, unsigned short tickLength, ActionType actionType, CharacterData& aiAttacker);
        static void Exit(CharacterData& aiCharacter, AIContext& aiContext);
        static void ExitNative(CharacterData& aiCharacter, AIContext& aiContext);
	};

}

#endif /* defined(__OnePiece__HeroOutofControlAction__) */
