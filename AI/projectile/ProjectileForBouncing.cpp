//
//  AIProjectile.cpp
//  OnePiece
//
//
//

#include "ProjectileForBouncing.h"
#include "../HeroHurtcalculate.h"
#include "../AIContext.h"
#include "../action_hero/HeroActionController.h"
#include "../scriptbind/AIScriptBindCharacterData.hpp"
#include "vector"
#include "list"
#include "cocos2d.h"

namespace OnePiece {

    ProjectileForBouncing::ProjectileForBouncing(AIContext& aiContext, int projectileID, CharacterData* origin, CharacterData* target, int skillID, int skillLevel)
    :aiContext(aiContext), AIProjectile(origin, origin->clusterNum)
{
    myID = aiContext.local_incremential++;
    startedCompanion = false;

    this->projectileID = projectileID;
    this->projectileExec = AIDataHelper::getProjectileExec(projectileID);

    this->origin = origin;
    this->source = origin;
    this->target = target;

    this->currentX = origin->getX() + projectileExec->x* source->direction;
    this->currentY = origin->getY() + projectileExec->y;
    this->currentZ = origin->getZ();
    this->skillID = skillID;
    this->skillLevel = skillLevel;

    this->targetX = target->getX();
    this->targetY = target->getY();
    this->targetZ = target->getZ();

    direction = this->targetX > this->currentX ? 1 : -1;

    pCurve = new ParaCurve(currentX, currentY, this->targetX, this->targetY + projectileExec->y, projectileExec->orbitType);

	this->maxTrack.tickMax = getRealChangeValue(); // projectileExec->deadParam;

    this->startTick = aiContext.currentTick + 1;
    this->localTick = aiContext.currentTick;
    this->totalTick = 0;
	if (projectileExec->deadType == EProjectileDeadType::eProjectileTimeDead)
	{
		this->totalTick = getRealChangeValue(); // projectileExec->deadParam;
	}

    aiContext.projectileList.push_back(this);
}


ProjectileForBouncing::~ProjectileForBouncing()
{
    aiContext.projectileList.remove(this);
    delete pCurve;
}

float ProjectileForBouncing::getYByX(float x)
{
    return pCurve->getYByX(x);
}

float ProjectileForBouncing::getSlope(float x)
{
    return pCurve->getSlope(x);
}

int ProjectileForBouncing::getDirection()
{
    return direction;
}

CharacterData* ProjectileForBouncing::getNextTarget(CharacterData* curTarget)
{
    if (!curTarget)
        return nullptr;

    CharacterData* nextTarget = nullptr;
    std::list<CharacterData*> targets;
    //auto allTargets = curTarget->getCurrentFriendsList();
    auto allTargets = aiContext.getClusterOrigin(curTarget->teamSide, curTarget->clusterNum);
    for (auto iter = allTargets->begin(); iter != allTargets->end(); ++iter)
    {
        auto role = (*iter);
        if (role->myID == curTarget->myID || !role->canBeAttacked(source, false, origin) || !luahelper_characterdata_canbeattackbyskill(aiContext.getLuaState(), role, source, skillID, this))
            continue;
        targets.push_back(role);
    }

    switch (projectileExec->throwType)
    {
        // 可相同目标
    case EProjectileTargetType::eProjectileTarget_6:
    case EProjectileTargetType::eProjectileTarget_7:
        if (targets.size() == 0 && curTarget->canBeAttacked(source, false, origin) && luahelper_characterdata_canbeattackbyskill(aiContext.getLuaState(), curTarget, source, skillID, this))
            nextTarget = curTarget;
        break;

    default:
        break;
    }

    if (!nextTarget && targets.size() > 0)
        nextTarget = AIActionController::getRandomCharacter(aiContext, *source, &targets, false);

    if ((AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *origin, eEFFECT_FORCE_ATTACK_SELF, 0) > 0))
    {
        if (skillID == origin->normalSkill.first)
        {
            if (projectileExec->throwType == eProjectileTarget_6 || projectileExec->throwType == eProjectileTarget_7)
                nextTarget = origin;
            else
                nextTarget = nullptr;
        }
    }

    return nextTarget;
}

bool ProjectileForBouncing::isCurStateMatchDeadCondition()
{
    if ((deadTrack.distance > 3000) || ((localTick - startTick) > 200))//|| (currentX - targetX)*origin->direction >= -1 * projectileExec->dx/2))
        return true;

    bool result = false;

    switch (projectileExec->deadType)
    {
    case EProjectileDeadType::eProjectileTimeDead:
        result = (localTick - startTick) >= maxTrack.tickMax;
        break;
    case EProjectileDeadType::eProjectileDropDead:
        result = currentY < origin->getY();
        break;
    case EProjectileDeadType::eProjectileCollideDead:
        result = deadTrack.collisionNum >= maxTrack.collisionMax;
        break;
    case EProjectileDeadType::eProjectileDistanceDead:
        result = deadTrack.distance > maxTrack.distanceMax;
        break;
    default:
        break;
    }

    return result;
}

bool ProjectileForBouncing::isMatchCollideDeadCondition(bool isCurTargetHitted, CharacterData* nextTarget)
{
    if (EProjectileDeadType::eProjectileCollideDead != projectileExec->deadType)
        return false;

    if (!isCurTargetHitted)
        return deadTrack.collisionNum >= maxTrack.collisionMax;

    return deadTrack.collisionNum >= maxTrack.collisionMax || nextTarget == nullptr;
}

void ProjectileForBouncing::doDeadExec()
{
    //add dead (ui, audio) effect to context list
    if (projectileExec->deadSpecialEffect1>0)
        AIActionController::executeExec(aiContext, projectileExec->deadSpecialEffect1, skillID, skillLevel, source, this, target, 0);
    if (projectileExec->deadSpecialEffect2>0)
        AIActionController::executeExec(aiContext, projectileExec->deadSpecialEffect2, skillID, skillLevel, source, this, target, 0);
    if (projectileExec->deadSpecialEffect3>0)
        AIActionController::executeExec(aiContext, projectileExec->deadSpecialEffect3, skillID, skillLevel, source, this, target, 0);
    if (projectileExec->deadSpecialEffect4>0)
        AIActionController::executeExec(aiContext, projectileExec->deadSpecialEffect4, skillID, skillLevel, source, this, target, 0);
    if (projectileExec->deadSpecialEffect5>0)
        AIActionController::executeExec(aiContext, projectileExec->deadSpecialEffect5, skillID, skillLevel, source, this, target, 0);
    if (projectileExec->deadExec>0)
        AIActionController::executeExec(aiContext, projectileExec->deadExec, skillID, skillLevel, source, this, target, 0);
    if (projectileExec->deadAudio>0)
        AIActionController::executeExec(aiContext, projectileExec->deadAudio, skillID, skillLevel, source, this, target, 0);
}

void ProjectileForBouncing::update(AIContext& aiContext)
{
    AINode::update(aiContext);
    if (isFrozen)
        return;

    if (!startedCompanion)
    {
        if (projectileExec->followingExec1>0)
            AIActionController::executeExec(aiContext, projectileExec->followingExec1, skillID, skillLevel, source, this, nullptr, 0);
        if (projectileExec->followingExec2>0)
            AIActionController::executeExec(aiContext, projectileExec->followingExec2, skillID, skillLevel, source, this, nullptr, 0);

        // 10.0 add: 弹射连接特效
        for (auto aBouncingExecId : projectileExec->bouncingExecs)
        {
            if (IsSpecialUIEffectExec(aBouncingExecId) && !IsLogicSpecialUIEffectExec(aBouncingExecId))
            {
                int theAltLinkEffTick = 0;
                auto theExec = AIDataHelper::getSpecialUIEffectExec(aBouncingExecId);
                if (theExec->duration2 > 0 && projectileExec->dx != 0)
                {
                    theAltLinkEffTick = (target->getX() - currentX) / projectileExec->dx;
                    theAltLinkEffTick = abs(theAltLinkEffTick);
                    if (theAltLinkEffTick <= 0)
                        theAltLinkEffTick = 1;
                }

                AIActionController::executeSpecialEffect(aiContext, 0, aBouncingExecId, skillLevel, skillID, origin, nullptr, target, 0, false, theAltLinkEffTick);
            }
            else if (aBouncingExecId > 0)
            {
                AIActionController::executeExec(aiContext, aBouncingExecId, skillID, skillLevel, this->origin, nullptr, target, 0);
            }
        }

        startedCompanion = true;
    }

    bool isHitted = false;
    CharacterData* nextBouncingTarget = nullptr;

    if (target->checkIfHit(currentX, currentY, projectileExec->dx, projectileExec->dy, direction) == 0)
    {
	//	deadTrack.collisionNum++;
	//	assert(!HeroActionController::isDeadAction(target->currentAction));
        if (target->canBeAttacked(source, false, origin) && luahelper_characterdata_canbeattackbyskill(aiContext.getLuaState(), target, source, skillID, this))
		{
			// clair_wang 大熊弹射飞行道具丢失目标后无效的bug
			deadTrack.collisionNum++;

            //AIActionController::executeExec(aiContext, projectileExec->hitExec, skillID, skillLevel, source, this, target, 0);
			for (int idx = 0; idx < projectileExec->hitExecVec.size(); idx++)
			{
				int execID = projectileExec->hitExecVec[idx];
				/*ExecExcuteInfo info(execID, skillID, skillLevel, source, this, character, 0);
				aiContext.scheduleExcuteExec(info);*/
				AIActionController::executeExec(aiContext, execID, skillID, skillLevel, source, this, target, 0);
			}
		}

        isHitted = true;

        if (EProjectileDeadType::eProjectileCollideDead == projectileExec->deadType && deadTrack.collisionNum < maxTrack.collisionMax)
            nextBouncingTarget = getNextTarget(target);
    }

    if (isCurStateMatchDeadCondition() || isMatchCollideDeadCondition(isHitted, nextBouncingTarget))
    {
        doDeadExec();

        delete this;
        return;
    }

    if (isHitted && deadTrack.collisionNum < maxTrack.collisionMax && nextBouncingTarget)
    {
        for (auto aBouncingExecId : projectileExec->bouncingExecs)
        {
            if (IsSpecialUIEffectExec(aBouncingExecId) && !IsLogicSpecialUIEffectExec(aBouncingExecId))
            {
                int theAltLinkEffTick = 0;
                auto theExec = AIDataHelper::getSpecialUIEffectExec(aBouncingExecId);
                if (theExec->duration2 > 0 && projectileExec->dx != 0)
                {
                    theAltLinkEffTick = (nextBouncingTarget->getX() - currentX) / projectileExec->dx;
                    theAltLinkEffTick = abs(theAltLinkEffTick);
                    if (theAltLinkEffTick <= 0)
                        theAltLinkEffTick = 1;
                }

                AIActionController::executeSpecialEffect(aiContext, 0, aBouncingExecId, skillLevel, skillID, target, nullptr, nextBouncingTarget, 0, false, theAltLinkEffTick);
            }
            else if (aBouncingExecId > 0)
            {
                AIActionController::executeExec(aiContext, aBouncingExecId, skillID, skillLevel, this->target, nullptr, nextBouncingTarget, 0);
            }
        }
        this->target = nextBouncingTarget;
        this->targetX = this->target->getX();
        this->targetY = this->target->getY();
        this->targetZ = this->target->getZ();

        nextBouncingTarget = nullptr;

        delete pCurve;
        pCurve = new ParaCurve(this->currentX, this->currentY, this->targetX, this->targetY + this->projectileExec->y, this->projectileExec->orbitType);
    }

    //distance calculate
    deadTrack.distance += projectileExec->dx;

    direction = target->getX() > currentX ? 1 : -1;

    currentX += projectileExec->dx * direction;
    currentY = pCurve->getYByX(currentX);
    //currentZ += 0;
    slope = pCurve->getSlope(currentX);

    if (isHitted)
        distanceFromLastHit = 0;
    distanceFromLastHit += projectileExec->dx;
}

int ProjectileForBouncing::getRealChangeValue()
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

}
