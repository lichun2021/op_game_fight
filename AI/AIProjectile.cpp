//
//  AIProjectile.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "AIProjectile.h"
#include "AIContext.h"

namespace OnePiece{

	AIProjectile::AIProjectile(CharacterData* origin, unsigned short cNum)
		: AINode(origin->myID,cNum)
	{
		deadTrack.collisionNum = 0;
		deadTrack.distance = 0;
		maxTrack.tickMax = 0;
		slope = 0;
        shouldHideUI = origin->hasTag("invisible2");
	}

	AIProjectile::~AIProjectile()
	{
		if (aiuiCallback)
			aiuiCallback->detach();
	}

	void AIProjectile::setUICompanion(AIUICallback* aiuiCallback)
	{
		this->aiuiCallback = aiuiCallback;
	}

    int AIProjectile::getDistanceFromLastHit()
    {
        return distanceFromLastHit;
    }

	float AIProjectile::getSpeedX()
	{
		return projectileExec->dx;
	}

}