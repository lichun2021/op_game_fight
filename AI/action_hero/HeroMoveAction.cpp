//
//  HeroMoveAction.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "HeroMoveAction.h"
#include "HeroAttackAction.h"
#include "HeroIdleAction.h"
#include "HeroActionController.h"
#include "HeroSkillController.h"
#include "HeroTurnAction.h"
#include "../scriptbind/AIScriptBindActionController.hpp"

#include "cocos2d.h"
using namespace std;

namespace OnePiece {
	void HeroMoveAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
	{
        bool cannotTurn = (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_NO_TURN_ACTION, 0) > 0);
		aiCharacter.oldY = aiCharacter.getY();
		if (aiCharacter.needAdjusting)
		{
            int speed = aiCharacter.getSpeed();
            if (aiCharacter.adjustingSpeedX) 
            {
                speed = aiCharacter.adjustingSpeedX;
            }
			if (aiCharacter.adjustingX != 100000000)
			{
                if ((aiCharacter.adjustingX - aiCharacter.getX())*aiCharacter.direction<0 && !cannotTurn)
				{
					HeroTurnAction::Enter(aiCharacter, aiContext);
					return;
				}
				if (aiCharacter.adjustingX >= aiCharacter.getX())
				{
					if (aiCharacter.adjustingX - aiCharacter.getX() > speed)
                        aiCharacter.setX(aiCharacter.getX() + speed);
					else
					{
						aiCharacter.setX(aiCharacter.adjustingX);
						aiCharacter.needAdjusting = false;
					}
				}
				else
				{
					if (aiCharacter.getX() - aiCharacter.adjustingX > speed)
                        aiCharacter.setX(aiCharacter.getX() - speed);
					else
					{
						aiCharacter.setX(aiCharacter.adjustingX);
						aiCharacter.needAdjusting = false;
					}
				}
			}

			if (aiCharacter.adjustingY != 100000000)
			{
				int speedY = 4;// aiCharacter.getSpeed() / 2;
				int yy = aiCharacter.getY();
				if (aiCharacter.adjustingY >= aiCharacter.getY())
				{
					if (aiCharacter.adjustingY - aiCharacter.getY() > speedY)
					{
						aiCharacter.setY(aiCharacter.getY() + speedY);
						aiCharacter.oldY = aiCharacter.getY();
						aiCharacter.setZ(aiCharacter.getY());
					}
					else
					{
						aiCharacter.setY(aiCharacter.adjustingY);
						aiCharacter.oldY = aiCharacter.getY();
						aiCharacter.setZ(aiCharacter.adjustingY);
						aiCharacter.needAdjusting = false;
					}
				}
				else
				{
					if (aiCharacter.getY() - aiCharacter.adjustingY > speedY)
					{
						aiCharacter.setY(aiCharacter.getY() - speedY);
						aiCharacter.oldY = aiCharacter.getY();
						aiCharacter.setZ(aiCharacter.getY());
					}
					else
					{
						aiCharacter.setY(aiCharacter.adjustingY);
						aiCharacter.oldY = aiCharacter.getY();
						aiCharacter.setZ(aiCharacter.adjustingY);
						aiCharacter.needAdjusting = false;
					}
				}
			}

            if (!aiCharacter.needAdjusting) {
                aiCharacter.adjustingSpeedX = 0;
                HeroActionController::actionSelector(aiCharacter, aiContext);
            }
			return;
		}

		aiCharacter.needAdjusting = false;

        // 场外角色先回场内
        bool isOutRange = false;
        bool shouldTurn = false;
        
        if (aiContext.monsterCanMove && AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_LIMIT_MOVE_RANGE, 0) > 0) {
            int posX = aiCharacter.getX();
            bool shouldMirror = (aiCharacter.teamSide == kEnemy);
            if (shouldMirror)
                posX = 2 * aiContext.battleFieldCenterX - posX;

            if (posX > AILimitMoveRangeEffect::getLimitMoveEffectUpper(&aiCharacter)) {
                isOutRange = true;
                shouldTurn = (aiCharacter.direction == (shouldMirror ? -1 : 1));
            }
            else if (posX < AILimitMoveRangeEffect::getLimitMoveEffectLower(&aiCharacter)) {
                isOutRange = true;
                shouldTurn = (aiCharacter.direction == (shouldMirror ? 1 : -1));
            }
        }

        if (aiContext.monsterCanMove && !isOutRange && !aiContext.getOwnerIsInVaildBarrier(&aiCharacter)) {
            isOutRange = true;
            if (aiCharacter.getX() < aiContext.getBattleFieldXMostLeft())
                shouldTurn = (aiCharacter.direction == -1);
            else if (aiCharacter.getX() > aiContext.getBattleFieldXMostRight())
                shouldTurn = (aiCharacter.direction == 1);
        }

        bool isMoved = false;
        if (isOutRange) {
            if (shouldTurn) {
                if (cannotTurn)
                    aiCharacter.setX(aiCharacter.getX() - aiCharacter.getSpeed() * aiCharacter.direction);
                else {
                    HeroTurnAction::Enter(aiCharacter, aiContext);
                    return;
                }
            }
            else {
                aiCharacter.setX(aiCharacter.getX() + aiCharacter.getSpeed() * aiCharacter.direction);
            }
            isMoved = true;
        }

		aiCharacter.target = aiCharacter.lockTarget(aiContext);
        auto targetForDirection = aiCharacter.target;
        //if (aiCharacter.specialFoundSkillTarget)
        //    targetForDirection = aiCharacter.specialFoundSkillTarget;

		if (aiCharacter.target == nullptr)
		{
			HeroIdleAction::Enter(aiCharacter, aiContext, 1, IDLE_ACTION, aiCharacter);
			return;
		}

        bool isTargetBehind = false;
        if ((targetForDirection->getX() - aiCharacter.getX())*aiCharacter.direction<0)
		{
            isTargetBehind = true;
            if (!cannotTurn)
            {
                //HeroTurnAction::Enter(aiCharacter, aiContext);
                luahelper_actioncontroller_on_moveaction_turntotarget(aiContext.getLuaState(), &aiCharacter);
                return;
            }
		}
		
		// clair_wang 
		if ((!isMoved) && aiCharacter.currentAction == MOVE_ACTION)
		{
			for (auto iterr = aiCharacter.moveCasts.begin(); iterr != aiCharacter.moveCasts.end(); iterr++)
			{
				if ((*iterr)->moveType == AIContinuousMoveCaster::MoveType::kTrackTargetMove)
				{
					isMoved = true;
					break;
				}
			}
		}

        if (!isMoved) {
            int s = aiCharacter.getSpeed();
            int offsetX = abs(aiCharacter.target->getX() - aiCharacter.getX());
            if (offsetX <= s)
            {
                offsetX /= 2;
            }
            else
            {
                offsetX = s;
            }

            offsetX *= aiCharacter.direction;
            if (isTargetBehind && cannotTurn)
                offsetX = -offsetX;
            aiCharacter.setX(aiCharacter.getX() + offsetX);
        }

        //if ((HeroActionController::getSkillID(aiCharacter.actionAtt)>0
        //    && HeroSkillController::isTargetInSkilllRange(&aiCharacter, HeroActionController::getSkillID(aiCharacter.actionAtt), aiCharacter.target))
        //    //&& aiContext.getOwnerIsInVaildBarrier(&aiCharacter)
        //    )
        //{
        //    HeroActionController::actionSelector(aiCharacter, aiContext);
        //}
        //else
        //{
        //    // HQ test from 8.0
        //    // 纵向在一直线上，且距离超出技能范围时，调整Y轴站位
        //    int xdist = abs(aiCharacter.target->getX() - aiCharacter.getX());
        //    if (xdist <= 1) {
        //        aiCharacter.setY(aiCharacter.getY() + (aiCharacter.target->getY() > aiCharacter.getY() ? 4 : -4));
        //    }
        //}

        int nextSkillId = HeroActionController::getNextSkillID(aiCharacter.actionAtt);
        if (nextSkillId > 0)
        {
            if (HeroSkillController::isTargetInSkilllRange(&aiCharacter, nextSkillId, aiCharacter.target))
            {
                HeroActionController::actionSelector(aiCharacter, aiContext);
            }
            else
            {
                // 纵向在一直线上，且距离超出技能范围时，调整Y轴站位
                int xdist = abs(aiCharacter.target->getX() - aiCharacter.getX());
                if (xdist <= 1) {
                    aiCharacter.setY(aiCharacter.getY() + (aiCharacter.target->getY() > aiCharacter.getY() ? 4 : -4));
                }
            }
        }
        else
        {
            HeroActionController::actionSelector(aiCharacter, aiContext);
        }
	}

	void HeroMoveAction::Enter(CharacterData&aiCharacter, AIContext& aiContext)
	{
		if (aiCharacter.targetStuckBackward)
		{
			aiCharacter.targetStuckBackward = false;
			HeroIdleAction::Enter(aiCharacter, aiContext, 11, IDLE_ACTION, aiCharacter);
			return;
		}

		if (aiCharacter.currentAction == ActionType::ATTACK_ACTION)
		{
            if (HeroAttackAction::ForceEndAccumulate(aiCharacter, aiContext))
                return;
			HeroActionController::setSkillActionCD(aiCharacter.actionAtt, aiCharacter.getAInterval());
			//CCLOG("DDDDDDDD:%d--%d--%d--%d", aiCharacter.myID, aiCharacter.attributes[0], aiCharacter.actionAtt[1], aiCharacter.localTick);
		}

        HeroActionController::exitAction(aiCharacter, aiContext);
		HeroActionController::cleanup(aiCharacter, aiContext);
		aiCharacter.currentAction = MOVE_ACTION;
        aiCharacter.actionCauserMyID = aiCharacter.myID;

        HeroActionController::didEnterAction(aiCharacter, aiContext);
	}
}

