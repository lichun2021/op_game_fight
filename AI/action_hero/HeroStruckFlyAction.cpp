#include "HeroStruckFlyAction.h"
#include "HeroActionController.h"
#include "HeroStandupAction.h"
#include "HeroAttackAction.h"
#include "../AIContinuousEffectCaster.h"


#include "cocos2d.h"
#define DELTA_SPEED_Y 10

namespace OnePiece {

	void HeroStruckFlyAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
	{
		if (aiCharacter.totalTick == 0)
		{
			aiCharacter.setY(aiCharacter.oldY);
			HeroStandupAction::Enter(aiCharacter, aiContext, 4, STANDUP_ACTION, aiCharacter);
		}
		else if (aiCharacter.totalTick < 0)
		{
			aiCharacter.totalTick++;

            bool isFirstOnFloorTick = false;
            if (aiCharacter.hitFlyExtendTick > 0)
                isFirstOnFloorTick = (aiCharacter.totalTick == -aiCharacter.hitFlyExtendTick);
            else
                isFirstOnFloorTick = (aiCharacter.totalTick == 0);

            if (isFirstOnFloorTick)
                AIExcuteOnHurtResultEffect::checkExcute(&aiCharacter);
		}
		else
		{
            aiCharacter.setX(aiContext.getAdjustedPosXWithinRange(aiCharacter.getX() + HeroActionController::getFlySpeedX(aiCharacter.actionAtt)));
			aiCharacter.setY(aiCharacter.getY() + HeroActionController::getFlySpeedY(aiCharacter.actionAtt));
			HeroActionController::setFlySpeedY(aiCharacter.actionAtt, HeroActionController::getFlySpeedY(aiCharacter.actionAtt) - DELTA_SPEED_Y);
			if (aiCharacter.getY() <= aiCharacter.oldY) {
				aiCharacter.setY(aiCharacter.oldY);
				aiCharacter.totalTick=-(10 + aiCharacter.hitFlyExtendTick);
			}
		}
	}

    void HeroStruckFlyAction::Enter(CharacterData& aiCharacter, AIContext& aiContext, short SpeedX, short SpeedY, int extendTick)
    {
        if (aiCharacter.currentAction == ActionType::ATTACK_ACTION)
        {
            if (aiCharacter.hasTag("attacknobrokenwhenenter"))
                return;
            if (HeroAttackAction::ForceEndAccumulate(aiCharacter, aiContext))
                return;
            HeroActionController::setSkillActionCD(aiCharacter.actionAtt, aiCharacter.getAInterval());
            //CCLOG("DDDDDDDD:%d--%d--%d--%d", aiCharacter.myID, aiCharacter.attributes[0], aiCharacter.actionAtt[1], aiCharacter.localTick);
        }

        int currentY = aiCharacter.getY();
        bool isFlyAfterFly = (aiCharacter.currentAction == ActionType::STRUCK_FLY_AWAY_ACTION);

        HeroActionController::exitAction(aiCharacter, aiContext);
		HeroActionController::cleanup(aiCharacter, aiContext);
		aiCharacter.currentAction = STRUCK_FLY_AWAY_ACTION;
		aiCharacter.startTick = aiCharacter.localTick+1;
		aiCharacter.totalTick = 1;
        aiCharacter.oldY = aiCharacter.getY();
		HeroActionController::setFlySpeedX(aiCharacter.actionAtt, SpeedX * aiCharacter.staticData->damping);
		HeroActionController::setFlySpeedY(aiCharacter.actionAtt, SpeedY * aiCharacter.staticData->damping);
        aiCharacter.hitFlyExtendTick = extendTick;

        // 击飞接击飞时，在原有高度继续击飞
        if (isFlyAfterFly)
            aiCharacter.setY(currentY);

        HeroActionController::didEnterAction(aiCharacter, aiContext);
	}

    void HeroStruckFlyAction::Exit(CharacterData& aiCharacter, AIContext& aiContext)
    {
        aiCharacter.eraseTag("hurtresultstack");
        aiCharacter.setY(aiCharacter.oldY);
        aiCharacter.hitFlyExtendTick = 0;
    }
}

