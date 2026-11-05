//
//  HeroStandupAction.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "HeroStandupAction.h"
#include "HeroIdleAction.h"
#include "HeroActionController.h"

namespace OnePiece {

	void HeroStandupAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
	{
		if (aiCharacter.localTick > aiCharacter.startTick + aiCharacter.totalTick)
		{
			HeroIdleAction::Enter(aiCharacter, aiContext, 1, IDLE_ACTION, aiCharacter);
		}
	}
}

