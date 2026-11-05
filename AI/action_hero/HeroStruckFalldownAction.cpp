
#include "HeroStruckFalldownAction.h"
#include "HeroStandupAction.h"

namespace OnePiece {

	void HeroStruckFalldownAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
	{
		if (aiCharacter.localTick < aiCharacter.startTick + aiCharacter.totalTick)
		{
            //aiCharacter.setX(aiContext.getAdjustedPosXWithinRange(aiCharacter.getX() + HeroActionController::getFlySpeedX(aiCharacter.actionAtt)));
		}
		else
		{
			HeroStandupAction::Enter(aiCharacter, aiContext, 4, STANDUP_ACTION, aiCharacter);
		}
	}
}

