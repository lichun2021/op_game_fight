//
//  HeroMoveAction.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__HeroMoveAction__
#define __OnePiece__HeroMoveAction__

#include "../AIContext.h"

namespace OnePiece {

	class HeroMoveAction
	{
		friend class HeroActionController;
	public:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
		static void Enter(CharacterData&aiCharacter, AIContext& aiContext);
	};

}

#endif /* defined(__OnePiece__HeroMoveAction__) */
