//
//  AIProjectile.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "ProjectileForCharacter.h"
#include <cmath>
#include "../AIContext.h"
#include "../HeroHurtcalculate.h"
#include "../action_hero/HeroActionController.h"
#include "../scriptbind/AIScriptBindCharacterData.hpp"

namespace OnePiece{

	ProjectileForCharacter::ProjectileForCharacter(AIContext& aiContext, int projectileID, CharacterData* origin, CharacterData* target, int skillID, int skillLevel) 
		: aiContext(aiContext), AIProjectile(origin, origin->clusterNum)
	{
		myID = aiContext.local_incremential++;

        startedCompanion = false;

		this->projectileID = projectileID;
		this->projectileExec = AIDataHelper::getProjectileExec(projectileID);

		this->origin = origin;
		this->source = origin;
		this->target = target;

		this->currentX = origin->getX() + projectileExec->x * source->direction;
		this->currentY = origin->getY() + projectileExec->y;
		this->currentZ = origin->getZ();
        this->skillID = skillID;
		this->skillLevel = skillLevel;

		pCurve = new ParaCurve(currentX, currentY, origin->getX(), origin->getY() + projectileExec->y, projectileExec->orbitType);

		this->maxTrack.tickMax = projectileExec->deadParam;

		this->startTick = aiContext.currentTick+1;
		this->localTick = aiContext.currentTick;
		this->totalTick = 0;
		if (projectileExec->deadType == EProjectileDeadType::eProjectileTimeDead)
            this->totalTick = projectileExec->deadParam;

		aiContext.projectileList.push_back(this);
	}

	ProjectileForCharacter::~ProjectileForCharacter()
	{
		aiContext.projectileList.remove(this);
		delete pCurve;
	}

	void ProjectileForCharacter::update(AIContext& aiContext)
	{
		AINode::update(aiContext);
		if (isFrozen) return;

        if (origin->isFrozen || origin->currentAction == EFFECT_ACTION)
            return;

        if ((origin->currentAction == ATTACK_ACTION && origin->hasTag("deadattacking")) || HeroActionController::isDeadAction(origin->currentAction))
        {
            delete this;
            return;
        }

        if (!startedCompanion)
        {
            //companion exec
            if (projectileExec->followingExec1>0)
                AIActionController::executeExec(aiContext, projectileExec->followingExec1, skillID, skillLevel, source, this, nullptr, 0);
            if (projectileExec->followingExec2>0)
                AIActionController::executeExec(aiContext, projectileExec->followingExec2, skillID, skillLevel, source, this, nullptr, 0);
            startedCompanion = true;
        }

        auto friendList = origin->getCurrentFriendsList();
        auto rivalList = origin->getCurrentRivalsList();

        std::list<CharacterData*> tmpFriends;
        std::list<CharacterData*> tmpRivals;
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *origin, eEFFECT_TAUNT_EXTRA, 0) > 0 || origin->hasTag("summonincharm253"))
        {
            tmpFriends.assign(rivalList->begin(), rivalList->end());
            tmpFriends.push_back(origin);

            for (auto tmpIter = friendList->begin(); tmpIter != friendList->end(); ++tmpIter)
            {
                auto theFriend = *tmpIter;
                if (theFriend->myID == origin->myID)
                    continue;
                tmpRivals.push_back(theFriend);
            }
            friendList = &tmpFriends;
            rivalList = &tmpRivals;
        }

        switch (projectileExec->hurtTrigger)
        {
        case EProjectileHurtTriggerType::eProjectileHurtTrigger_HitAny:
        case EProjectileHurtTriggerType::eProjectileHurtTrigger_HitFriend:
        case EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget:
            updateCollision(friendList);
            break;
        default:
            break;
        }

        switch (projectileExec->hurtTrigger)
        {
        case EProjectileHurtTriggerType::eProjectileHurtTrigger_HitAny:
        case EProjectileHurtTriggerType::eProjectileHurtTrigger_HitEnemy:
        case EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget:
            updateCollision(rivalList);
            break;
        default:
            break;
        }

        bool isProjectileDead = false;
        switch (projectileExec->deadType)
        {
        case eProjectileTimeDead:
            isProjectileDead = ((localTick - startTick) >= maxTrack.tickMax);
            break;
        case eProjectileDistanceDead:
            isProjectileDead = (deadTrack.distance > maxTrack.distanceMax);
            break;
        case eProjectileDropDead:
            isProjectileDead = (currentY < origin->getY());
            break;
        case eProjectileCollideDead:
            isProjectileDead = (deadTrack.collisionNum >= maxTrack.collisionMax);
            break;
        default:
            isProjectileDead = (deadTrack.distance > 3000 || (localTick - startTick) > 200);
            break;
        }

        if (isProjectileDead)
        {
			//add dead (ui, audio) effect to context list
			if (projectileExec->deadSpecialEffect1>0)
                AIActionController::executeExec(aiContext, projectileExec->deadSpecialEffect1, skillID, skillLevel, source, this, nullptr, 0);
			if (projectileExec->deadSpecialEffect2>0)
                AIActionController::executeExec(aiContext, projectileExec->deadSpecialEffect2, skillID, skillLevel, source, this, nullptr, 0);
			if (projectileExec->deadSpecialEffect3>0)
                AIActionController::executeExec(aiContext, projectileExec->deadSpecialEffect3, skillID, skillLevel, source, this, nullptr, 0);
			if (projectileExec->deadSpecialEffect4>0)
                AIActionController::executeExec(aiContext, projectileExec->deadSpecialEffect4, skillID, skillLevel, source, this, nullptr, 0);
			if (projectileExec->deadSpecialEffect5>0)
                AIActionController::executeExec(aiContext, projectileExec->deadSpecialEffect5, skillID, skillLevel, source, this, nullptr, 0);
			if (projectileExec->deadExec>0)
                AIActionController::executeExec(aiContext, projectileExec->deadExec, skillID, skillLevel, source, this, nullptr, 0);
			if (projectileExec->deadAudio>0)
                AIActionController::executeExec(aiContext, projectileExec->deadAudio, skillID, skillLevel, source, this, nullptr, 0);

			delete this;
			return;
		}

        // 虽然是跟随人物的飞行道具，计算距离依然使用飞行道具速度，简化处理
		deadTrack.distance += projectileExec->dx;

        int oldX = currentX;
        currentX = source->getX() + projectileExec->x * source->direction;
        currentY = source->getY() + projectileExec->y;
        currentZ = source->getZ();
		slope = pCurve->getSlope(currentX);
        distanceFromLastHit += std::abs(currentX - oldX);
	}

	float ProjectileForCharacter::getYByX(float x)
	{
		return pCurve->getYByX(x);
	}

	float ProjectileForCharacter::getSlope(float x)
	{
		return pCurve->getSlope(x);
	}

    int ProjectileForCharacter::getDirection()
    {
        return source->direction;
    }

	int ProjectileForCharacter::getRealChangeValue()
	{
		return -1;
	}

    void ProjectileForCharacter::updateCollision(CharacterDataListPtr targets)
    {
        // TODO: optimise in 9.0
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *origin, eEFFECT_FORCE_ATTACK_SELF, 0) > 0 && skillID == origin->normalSkill.first)
        {
            if (deadTrack.collisionNum > 1)
            {

            }
            else
            {
                deadTrack.collisionNum++;
                
				//AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, source, 0);
				for (int idx = 0; idx < projectileExec->hitExecVec.size(); idx++)
				{
					int execID = projectileExec->hitExecVec[idx];
					/*ExecExcuteInfo info(execID, skillID, skillLevel, source, this, character, 0);
					aiContext.scheduleExcuteExec(info);*/
					AIActionController::executeExec(aiContext, execID, skillID, skillLevel, source, this, source, 0);
				}
            }
        }
        else
        {
            for (auto iter = targets->begin(); iter != targets->end(); ++iter)
            {
                CharacterData* character = (*iter);
                if (character->canBeAttacked(source, false, origin) && luahelper_characterdata_canbeattackbyskill(aiContext.getLuaState(), character, source, skillID, this) && character->checkIfHit(currentX, currentY, projectileExec->dx, projectileExec->dy, getDirection()) == 0)
                {
                    if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget)
                    {
                        if (target == character)
                        {
                            deadTrack.collisionNum++;

                        //    AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, character, 0);
							for (int idx = 0; idx < projectileExec->hitExecVec.size(); idx++)
							{
								int execID = projectileExec->hitExecVec[idx];
								/*ExecExcuteInfo info(execID, skillID, skillLevel, source, this, character, 0);
								aiContext.scheduleExcuteExec(info);*/
								AIActionController::executeExec(aiContext, execID, skillID, skillLevel, source, this, character, 0);
							}
                            break;
                        }
                    }
                    else
                    {
                        if (_hitedCharacterMyId.find(character->myID) == _hitedCharacterMyId.end())
                        {
                            _hitedCharacterMyId.insert(character->myID);
                            deadTrack.collisionNum++;
                            
							//AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, character, 0);
							for (int idx = 0; idx < projectileExec->hitExecVec.size(); idx++)
							{
								int execID = projectileExec->hitExecVec[idx];
								/*ExecExcuteInfo info(execID, skillID, skillLevel, source, this, character, 0);
								aiContext.scheduleExcuteExec(info);*/
								AIActionController::executeExec(aiContext, execID, skillID, skillLevel, source, this, character, 0);
							}
                        }
                    }
                }
                if (deadTrack.collisionNum >= maxTrack.collisionMax && projectileExec->deadType == EProjectileDeadType::eProjectileCollideDead)
                    break;
            }
        }
    }

}

