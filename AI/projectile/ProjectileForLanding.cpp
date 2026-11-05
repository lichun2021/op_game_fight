//
//  AIProjectile.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "ProjectileForLanding.h"
#include "../HeroHurtcalculate.h"
#include "../AIContext.h"
#include "../action_hero/HeroActionController.h"
#include "../scriptbind/AIScriptBindCharacterData.hpp"
#include "vector"
#include "list"
#include "cocos2d.h"

namespace OnePiece{

	ProjectileForLanding::ProjectileForLanding(AIContext& aiContext, int projectileID, CharacterData* origin, CharacterData* target, int targetX, int targetY, int targetZ, int skillID, int skillLevel)
		: aiContext(aiContext), AIProjectile(origin, origin->clusterNum)
	{
		myID = aiContext.local_incremential++;
		startedCompanion = false;

		this->projectileID = projectileID;
		this->projectileExec = AIDataHelper::getProjectileExec(projectileID);
		
		this->origin = origin;
		this->source = origin;
		this->target = target;

		this->targetX = targetX;
		this->targetY = targetY;
		this->targetZ = targetZ;
		this->currentX = origin->getX() + projectileExec->x* source->direction;
		this->currentY = origin->getY() + projectileExec->y;
		this->currentZ = origin->getZ();

        this->isFirstRun = true;
		this->isReachPos12 = false;

        direction = source->direction;
        if (projectileExec->throwDirectionType == 1)
            direction = this->targetX > this->currentX ? 1 : -1;

		pCurve = new ParaCurve(currentX, currentY, targetX, targetY + projectileExec->y, projectileExec->orbitType);

		if (projectileExec->deadType == EProjectileDeadType::eProjectileTimeDead)
			this->maxTrack.tickMax = projectileExec->deadParam;
		else if (projectileExec->deadType == EProjectileDeadType::eProjectileDistanceDead)
			this->maxTrack.distanceMax = projectileExec->deadParam;
		if (projectileExec->deadType == EProjectileDeadType::eProjectileCollideDead)
			this->maxTrack.collisionMax = projectileExec->deadParam;

		this->startTick = aiContext.currentTick+1;
		this->localTick = aiContext.currentTick;
        this->skillID = skillID;
		this->skillLevel = skillLevel;

		//totalTick = 0;
		//if (projectileExec->deadType == EProjectileDeadType::eProjectileTimeDead) totalTick = projectileExec->deadParam;

		aiContext.projectileList.push_back(this);
	}
	
	ProjectileForLanding::~ProjectileForLanding()
	{
		aiContext.projectileList.remove(this);
		delete pCurve;
	}

	float ProjectileForLanding::getYByX(float x)
	{
		return pCurve->getYByX(x);
	}

	float ProjectileForLanding::getSlope(float x)
	{
		return pCurve->getSlope(x);
	}

    int ProjectileForLanding::getDirection()
    {
        return direction;
    }

	void ProjectileForLanding::update(AIContext& aiContext)
	{
		AINode::update(aiContext);
		if (isFrozen) return;

		if (!startedCompanion)
		{
			//companion exec
			if (projectileExec->followingExec1>0)
				AIActionController::executeExec(aiContext, projectileExec->followingExec1, skillID, skillLevel, source, this, nullptr, 0);
			if (projectileExec->followingExec2>0)
				AIActionController::executeExec(aiContext, projectileExec->followingExec2, skillID, skillLevel, source, this, nullptr, 0);
			startedCompanion = true;
		}

		if (projectileExec->throwType == eProjectileTargetPosition && isReachPos12)
		{
		}
        else if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, eEFFECT_FORCE_ATTACK_SELF, 0) > 0 && skillID == source->normalSkill.first)
        {
            CharacterData* character = source;
            
            if (character->canBeAttacked(source, false, origin) && luahelper_characterdata_canbeattackbyskill(aiContext.getLuaState(), character, source, skillID, this) && checkIfHitTarget(character) == 0)
            {
                if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget)
                {
                    if (target == character)
                    {
                        deadTrack.collisionNum++;

                     //   AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, character, 0);
						for (int idx = 0; idx < projectileExec->hitExecVec.size(); idx++)
						{
							int execID = projectileExec->hitExecVec[idx];
							/*ExecExcuteInfo info(execID, skillID, skillLevel, source, this, character, 0);
							aiContext.scheduleExcuteExec(info);*/
							AIActionController::executeExec(aiContext, execID, skillID, skillLevel, source, this, character, 0);
						}
						
                    }
                }
                else
                {
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
        else
        {
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
            /*
            std::list<CharacterData*>::iterator iter;
            if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitAny
                || projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitFriend
                || projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget)
            {
                for (iter = friendList->begin(); iter != friendList->end(); ++iter)
                {
                    CharacterData* character = (*iter);

                    if (character->canBeAttacked(source) && luahelper_characterdata_canbeattackbyskill(aiContext.getLuaState(), character, source, skillID, this) && checkIfHitTarget(character) == 0)
                    {
                        if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget)
                        {
                            if (target == character)
                            {
                                deadTrack.collisionNum++;
                                AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, character, 0);
                                break;
                            }
                        }
                        else
                        {
                            deadTrack.collisionNum++;
                            AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, character, 0);
                        }
                    }
                    if (deadTrack.collisionNum >= maxTrack.collisionMax && projectileExec->deadType == EProjectileDeadType::eProjectileCollideDead)
                        break;
                }
            }
            if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitAny
                || projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitEnemy
                || projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget)
            {
                
                for (iter = rivalList->begin(); iter != rivalList->end(); ++iter)
                {
                    CharacterData* character = (*iter);

                    if (character->canBeAttacked(source) && luahelper_characterdata_canbeattackbyskill(aiContext.getLuaState(), character, source, skillID, this) && checkIfHitTarget(character) == 0)
                    {
                        if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget)
                        {
                            if (target == character)
                            {
                                deadTrack.collisionNum++;
                                AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, character, 0);
                                break;
                            }
                        }
                        else
                        {
                            deadTrack.collisionNum++;
                            AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, character, 0);
                        }
                    }
                    if (deadTrack.collisionNum >= maxTrack.collisionMax && projectileExec->deadType == EProjectileDeadType::eProjectileCollideDead)
                        break;
                }
                
            }
            */

            if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitFriendNearest)
            {
                updateCollisionNearest(friendList);
            }
            else if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitEnemyNearest)
            {
                updateCollisionNearest(rivalList);
            }
            else
            {
                if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitAny
                    || projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitFriend
                    || projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget)
                {
                    updateCollision(friendList);
                }

                if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitAny
                    || projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitEnemy
                    || projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget)
                {
                    updateCollision(rivalList);
                }
            }
        }

        if (isFirstRun)
            isFirstRun = false;

		if (projectileExec->throwType == eProjectileTargetPosition && !isReachPos12)
		{
			// 到达指定坐标后在原地停留
			if ((currentX - GetRealTargetX())*getDirection() >= -1 * projectileExec->dx / 2)
			{
				isReachPos12 = true;
				// [a,b,c,d,e,f...] 第六位f开始：到达指定坐标后触发执行
				for (int i = 5; i < projectileExec->throwParams.size(); i++)
				{
					int execId = projectileExec->throwParams[i];
					if (execId > 0)
						AIActionController::executeExec(aiContext, execId, skillID, skillLevel, source, this, nullptr, 0, false, 0, true);
				}
			}
		}

		//if ((deadTrack.distance > 3000 || (localTick - startTick)>200 || (currentX - GetRealTargetX())*getDirection() >= -1 * projectileExec->dx / 2)
		//	|| ((localTick - startTick) >= maxTrack.tickMax && projectileExec->deadType == EProjectileDeadType::eProjectileTimeDead)
		//	|| (currentY < origin->getY() && projectileExec->deadType == EProjectileDeadType::eProjectileDropDead)
		//	|| (deadTrack.collisionNum >= maxTrack.collisionMax && projectileExec->deadType == EProjectileDeadType::eProjectileCollideDead)
		//	|| (deadTrack.distance > maxTrack.distanceMax && projectileExec->deadType == EProjectileDeadType::eProjectileDistanceDead))
		if (isMatchDeadCond())
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

		//distance calculate
		do
		{
			if (projectileExec->throwType == eProjectileTargetPosition)
			{
				// 到达指定坐标后在原地停留
				if (isReachPos12)
					break;
			}

			///	int speed = (targetX - currentX)*getDirection();
			// clair_wang 2016.03.07 修改触发类型3，,目标target的X坐标有左右移动时，计算出speed为0，不准的情况~
			// 解决bug 飞行道具飞出去后，因target被击飞击倒X坐标偏移，而无法击中target的状况~
			int speed = (GetRealTargetX() - currentX)*getDirection();
			if (speed > projectileExec->dx) speed = projectileExec->dx;
			if (speed < 0) speed = 0;

			deadTrack.distance += speed;
			currentX += speed * getDirection();
			currentY = pCurve->getYByX(currentX);
			currentZ += 0;
			slope = pCurve->getSlope(currentX);
			distanceFromLastHit += speed;
		} while (false);
	}


	int  ProjectileForLanding::GetRealTargetX()
	{
		if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget && target)
		{
			return target->getX();
		}
		else
		{
			return targetX;
		}
	}


	int ProjectileForLanding::getRealChangeValue()
	{
		int _returnValue = 0;
		float nTmpValue = 0;

		int lowerLimit = (projectileExec->limitValLowerBase + projectileExec->limitValLowerGrowth * skillLevel) / 100;
		int upperLimit = (projectileExec->limitValUpperBase + projectileExec->limitValUpperGrowth * skillLevel) / 100;


		nTmpValue = projectileExec->deadParam + (projectileExec->levelup / 100 * skillLevel);


		if (projectileExec->limitOnOff == 0)
		{
			_returnValue = nTmpValue;
		}
		else if (projectileExec->limitOnOff == 1)
		{
			if (nTmpValue >= lowerLimit && nTmpValue <= upperLimit)
			{
				_returnValue = nTmpValue;
			}
			else
			{
_returnValue = (abs(nTmpValue - lowerLimit) > abs(nTmpValue - upperLimit)) ? upperLimit : lowerLimit;
			}
		}

		return _returnValue;
	}

	int ProjectileForLanding::checkIfHitTarget(CharacterData *theTarget)
	{
		int hitResult = theTarget->checkIfHit(currentX, currentY, projectileExec->dx, projectileExec->dy, getDirection());
		if (isFirstRun && hitResult != 0 && projectileExec->x > 0 && ((projectileExec->x + projectileExec->x) < projectileExec->dx))
		{
			int halfX = projectileExec->x / 2;
			hitResult = theTarget->checkIfHit(currentX - halfX, currentY, projectileExec->x, projectileExec->dy, getDirection());
		}

		return hitResult;
	}

	void ProjectileForLanding::updateCollision(std::list<CharacterData *> *list)
	{
		for (auto character : *list)
		{
			if (character->canBeAttacked(source, false, origin) && luahelper_characterdata_canbeattackbyskill(aiContext.getLuaState(), character, source, skillID, this) && checkIfHitTarget(character) == 0)
			{
				if (projectileExec->hurtTrigger == EProjectileHurtTriggerType::eProjectileHurtTrigger_HitTarget)
				{
					if (target == character)
					{
						deadTrack.collisionNum++;

						for (int idx = 0; idx < projectileExec->hitExecVec.size(); idx++)
						{
							int execID = projectileExec->hitExecVec[idx];
							/*ExecExcuteInfo info(execID, skillID, skillLevel, source, this, character, 0);
							aiContext.scheduleExcuteExec(info);*/
							AIActionController::executeExec(aiContext, execID, skillID, skillLevel, source, this, character, 0);
						}
						/*AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, character, 0);*/
						break;
					}
				}
				else
				{
					deadTrack.collisionNum++;

					for (int idx = 0; idx < projectileExec->hitExecVec.size(); idx++)
					{
						int execID = projectileExec->hitExecVec[idx];
						/*ExecExcuteInfo info(execID, skillID, skillLevel, source, this, character, 0);
						aiContext.scheduleExcuteExec(info);*/
						AIActionController::executeExec(aiContext, execID, skillID, skillLevel, source, this, character, 0);
					}
					//AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, character, 0);
				}
			}
			if (deadTrack.collisionNum >= maxTrack.collisionMax && projectileExec->deadType == EProjectileDeadType::eProjectileCollideDead)
				break;
		}
	}

	void ProjectileForLanding::updateCollisionNearest(std::list<CharacterData *> *list)
	{
		CharacterData *t = nullptr;
		int distNearest = 0;

		for (auto character : *list)
		{
			if (character->canBeAttacked(source, false, origin) && luahelper_characterdata_canbeattackbyskill(aiContext.getLuaState(), character, source, skillID, this) && checkIfHitTarget(character) == 0)
			{
				int aDist = character->getX() - currentX;
				aDist = abs(aDist);
				if (!t || distNearest < aDist)
				{
					distNearest = aDist;
					t = character;
				}
			}
		}

		if (t)
		{
			deadTrack.collisionNum++;

			for (int idx = 0; idx < projectileExec->hitExecVec.size(); idx++)
			{
				int execID = projectileExec->hitExecVec[idx];
				/*ExecExcuteInfo info(execID, skillID, skillLevel, source, this, character, 0);
				aiContext.scheduleExcuteExec(info);*/
				AIActionController::executeExec(aiContext, execID, skillID, skillLevel, source, this, t, 0);
			}
			//AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, t, 0);
		}
	}

	bool ProjectileForLanding::isMatchDeadCond()
	{
		if (projectileExec->throwType == eProjectileTargetPosition)
		{
			if (projectileExec->deadType == EProjectileDeadType::eProjectileTimeDead)
			{
				if ((localTick - startTick) >= maxTrack.tickMax)
					return true;
				return false;
			}
			else if (projectileExec->deadType == EProjectileDeadType::eProjectilePositionDead)
			{
				if (isReachPos12)
					return true;
				return false;
			}
		}

		if ((deadTrack.distance > 3000 || (localTick - startTick) > 200 || (currentX - GetRealTargetX())*getDirection() >= -1 * projectileExec->dx / 2)
			|| ((localTick - startTick) >= maxTrack.tickMax && projectileExec->deadType == EProjectileDeadType::eProjectileTimeDead)
			|| (currentY < origin->getY() && projectileExec->deadType == EProjectileDeadType::eProjectileDropDead)
			|| (deadTrack.collisionNum >= maxTrack.collisionMax && projectileExec->deadType == EProjectileDeadType::eProjectileCollideDead)
			|| (deadTrack.distance > maxTrack.distanceMax && projectileExec->deadType == EProjectileDeadType::eProjectileDistanceDead))
		{
			return true;
		}
		return false;
	}

	float ProjectileForLanding::getSpeedX()
	{
		if (projectileExec->throwType == eProjectileTargetPosition && isReachPos12)
		{
			return 0;
		}
		return projectileExec->dx;
	}
}