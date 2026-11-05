#include "HeroStruckHoverAction.h"
#include "HeroActionController.h"
#include "HeroIdleAction.h"
#include "HeroAttackAction.h"
#include "../AIContinuousEffectCaster.h"
#include "cocos2d.h"

namespace OnePiece {
    
    void HeroStruckHoverAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
    {
        if (aiCharacter.totalTick == 0)
        {
            aiCharacter.setY(aiCharacter.oldY);
            HeroIdleAction::Enter(aiCharacter, aiContext, 4, IDLE_ACTION, aiCharacter);
        }
        else if (aiCharacter.totalTick < 0)
        {
            aiCharacter.totalTick++;
        }
        else
        {
            // HQ: 7.0修改
            // 因现在黑胡子的黑洞会把目标吸起来改变Y的速度,这里直接使用固定的移动速度,防止出现除0错误
            int speedY = 10 * aiCharacter.staticData->damping; //HeroActionController::getFlySpeedY(aiCharacter.actionAtt);
            if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_STRUCK_HOVER, 0))
            {
                int offsetY = AIContinuousEffectCaster::getEffectValue(&aiCharacter, EffectType::eEFFECT_STRUCK_HOVER);
                int flyTick = offsetY / speedY;
                int stayTicks = aiCharacter.totalTick - 2 * flyTick;

                int playedTick = aiCharacter.localTick - aiCharacter.startTick;
                if (playedTick >= aiCharacter.totalTick)
                {
                    aiCharacter.setY(aiCharacter.oldY);
                    aiCharacter.totalTick = 0;
                }
                else if (playedTick < flyTick)
                {
                    aiCharacter.setY(aiCharacter.getY() + speedY);
                }
                else if (playedTick - flyTick > stayTicks)
                {
                    aiCharacter.setY(aiCharacter.getY() - speedY);
                }

                if (aiCharacter.getY() <= aiCharacter.oldY) {
                    aiCharacter.setY(aiCharacter.oldY);
                    aiCharacter.totalTick = 0;
                }
            }
            else
            {
                aiCharacter.setY(aiCharacter.getY() - speedY);
                if (aiCharacter.getY() <= aiCharacter.oldY) {
                    aiCharacter.setY(aiCharacter.oldY);
                    aiCharacter.totalTick = 0;
                }
                HeroIdleAction::Enter(aiCharacter, aiContext, 4, IDLE_ACTION, aiCharacter);
            }
        }
    }

    void HeroStruckHoverAction::Enter(CharacterData& aiCharacter, AIContext& aiContext, int SpeedY, int totalTicks, CharacterData& aiAttacker)
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

        HeroActionController::exitAction(aiCharacter, aiContext);
        HeroActionController::cleanup(aiCharacter, aiContext);
        aiCharacter.currentAction = STRUCK_HOVER_ACTION;
        aiCharacter.startTick = aiCharacter.localTick + 1;
        aiCharacter.totalTick = totalTicks;
        aiCharacter.oldY = aiCharacter.getY();
        aiCharacter.actionCauserMyID = aiAttacker.myID;
        HeroActionController::setFlySpeedY(aiCharacter.actionAtt, SpeedY * aiCharacter.staticData->damping);

        HeroActionController::didEnterAction(aiCharacter, aiContext);
    }

    void HeroStruckHoverAction::Exit(CharacterData& aiCharacter, AIContext& aiContext)
    {
        aiCharacter.setY(aiCharacter.oldY);
    }

}
