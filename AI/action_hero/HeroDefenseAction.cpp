//
//  HeroDefenseAction.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "HeroDefenseAction.h"
#include "HeroActionController.h"

namespace OnePiece {
	void HeroDefenseAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
	{
		if (aiCharacter.currentAction != DEFENSE_ACTION) return;
		HeroActionController::actionSelector(aiCharacter, aiContext);
	}
}