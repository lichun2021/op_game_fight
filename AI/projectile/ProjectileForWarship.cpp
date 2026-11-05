//
//  AIProjectile.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "ProjectileForWarship.h"
#include "../HeroHurtcalculate.h"
#include "../AIContext.h"
#include "../AICannonKing.h"
#include "../action_hero/HeroSkillController.h"
#include "vector"
#include "list"

namespace OnePiece{

	ProjectileForWarship::ProjectileForWarship(AIContext& aiContext, int projectileID, CharacterData* origin, int sourceX, int sourceY, int sourceZ, int targetX, int targetY, int targetZ, int skillID, int skillLevel)
		: aiContext(aiContext), AIProjectile(origin, origin->clusterNum)
	{
		myID = aiContext.local_incremential++;
		startedCompanion = false;

		this->projectileID = projectileID;
		this->projectileExec = AIDataHelper::getProjectileExec(projectileID);
		
		this->origin = origin;
		this->source = origin;
		origin->currentAction = DEAD_ACTION1;
		origin->setHP(0);

		this->targetX = targetX;
		this->targetY = targetY;
		this->targetZ = targetZ;
		this->currentX = sourceX;
		this->currentY = sourceY;
		this->currentZ = sourceZ;

		pCurve = new ParaCurve(currentX, currentY, targetX, targetY + projectileExec->y, projectileExec->orbitType);
        pCurve->initABCForCannon(currentX, currentY, targetX, targetY + projectileExec->y, projectileExec->orbitType);

		this->startTick = aiContext.currentTick+1;
		this->localTick = aiContext.currentTick;
        this->skillID = skillID;
		this->skillLevel = skillLevel;

		aiContext.projectileList.push_back(this);
	}
	
	ProjectileForWarship::~ProjectileForWarship()
	{
		aiContext.projectileList.remove(this);
		delete pCurve;
	}

	float ProjectileForWarship::getYByX(float x)
	{
		return pCurve->getYByX(x);
	}

	float ProjectileForWarship::getSlope(float x)
	{
		return pCurve->getSlope(x);
	}

    int ProjectileForWarship::getDirection()
    {
        return source->direction;
    }

	int ProjectileForWarship::getRealChangeValue()
	{
		return -1;
	}

	void ProjectileForWarship::update(AIContext& aiContext)
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

		////distance calculate
		//int speed = (targetX - currentX)*origin->direction;
		//if (speed > projectileExec->dx) speed = projectileExec->dx;
		//if (speed<0) speed = 0;

		//deadTrack.distance += speed;
		//currentX += speed * source->direction;;
		//currentY = pCurve->getYByX(currentX);
		//currentZ += 0;
		//slope = pCurve->getSlope(currentX);

		/*
		deadTrack.distance += projectileExec->dx;
		currentX += projectileExec->dx * source->direction;;
		if (source->direction > 0 &&  currentX >= targetX){
			currentX = targetX;
		}
		if (source->direction < 0 && currentX <= targetX){
			currentX = targetX;
		}		
		currentY = pCurve->getYByX(currentX);
		currentZ = currentY;
		*/


		if (deadTrack.distance > 3000 || (localTick - startTick)>200 || currentX == targetX)
		{
			//ranged hurt, hitExec
			currentX = targetX;
			currentY = targetY;
			currentZ = targetY;
            std::vector<int> execs;
            /*if (projectileExec->hitExec > 0)
                execs.push_back(projectileExec->hitExec);*/
			for (int indx = 0; indx < projectileExec->hitExecVec.size(); indx++)
			{
				int nExecID = projectileExec->hitExecVec[indx];
				if (nExecID > 0)
				{
					execs.push_back(nExecID);
				}
			}

			//add dead (ui, audio) effect to context list
            if (projectileExec->deadSpecialEffect1 > 0)
                execs.push_back(projectileExec->deadSpecialEffect1);
            if (projectileExec->deadSpecialEffect2 > 0)
                execs.push_back(projectileExec->deadSpecialEffect2);
            if (projectileExec->deadSpecialEffect3 > 0)
                execs.push_back(projectileExec->deadSpecialEffect3);
            if (projectileExec->deadSpecialEffect4 > 0)
                execs.push_back(projectileExec->deadSpecialEffect4);
            if (projectileExec->deadSpecialEffect5 > 0)
                execs.push_back(projectileExec->deadSpecialEffect5);
            if (projectileExec->deadExec > 0)
                execs.push_back(projectileExec->deadExec);
            if (projectileExec->deadAudio > 0)
                execs.push_back(projectileExec->deadAudio);

            int endTick = 0;
            for (auto execID : execs)
            {
                int execLen = HeroSkillController::calculateExecLength(execID);
                if (endTick < execLen)
                    endTick = execLen;
                AIActionController::executeExec(aiContext, execID, skillID, skillLevel, source, this, nullptr, 0);
            }
            endTick += 40; // 船炮效果结束后多2秒等待

            auto cannonKing = aiContext.getCannonKing();
            if (cannonKing)
                cannonKing->onCannonExploded(myID, endTick + aiContext.currentTick);

			delete this;
			return;
		}

        //distance calculate
        int speed = (targetX - currentX)*origin->direction;
        if (speed > projectileExec->dx) speed = projectileExec->dx;
        if (speed<0) speed = 0;

        deadTrack.distance += speed;
        currentX += speed * source->direction;;
        currentY = pCurve->getYByX(currentX);
        currentZ += 0;
        slope = pCurve->getSlope(currentX);
        distanceFromLastHit += speed;
	}

}