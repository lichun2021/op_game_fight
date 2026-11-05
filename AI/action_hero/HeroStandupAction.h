//
//  HeroStandupAction.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__HeroStandupAction__
#define __OnePiece__HeroStandupAction__

#include "HeroOutofControlAction.h"

namespace OnePiece {
	class AIContext;
	class CharacterData;

	class HeroStandupAction : public HeroOutofControlAction
	{
		friend class HeroActionController;
	public:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
	};

}

#endif /* defined(__OnePiece__HeroStandupAction__) */
