//
//  HeroDefenseAction.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__HeroDefenseAction__
#define __OnePiece__HeroDefenseAction__

#include "../AIContext.h"

namespace OnePiece {

	class HeroDefenseAction
	{
		friend class HeroActionController;
	public:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
	};

}

#endif /* defined(__OnePiece__HeroIdleAction__) */
