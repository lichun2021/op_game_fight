//
//  HeroCelebrateAction.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__HeroCelebrateAction__
#define __OnePiece__HeroCelebrateAction__

//#include "HeroOutofControlAction.h"
#include "HeroActionController.h"

namespace OnePiece {

	class HeroCelebrateAction// : public HeroOutofControlAction
	{
	public:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
        static void Enter(CharacterData& aiCharacter, AIContext& aiContext, unsigned short tickLength, ActionType actionType);
        static void Exit(CharacterData& aiCharacter, AIContext& aiContext);
	};

}

#endif /* defined(__OnePiece__HeroCelebrateAction__) */
