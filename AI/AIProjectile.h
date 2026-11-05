//
//  AIProjectile.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__AIProjectile__
#define __OnePiece__AIProjectile__

#include <vector>
#include <list>
#include "CommonType.h"
#include "AIUICallback.h"
#include "AINode.h"

/*
#define SETGET_ATT(getX,setX,arr,i) \
int getX(){ return arr[i]; } const \
void setX(int val){ arr[i] = val; }
*/

namespace OnePiece {
	
	class AIContext;
	class CharacterData;

	class AIProjectile : public AINode
	{
	public:
		virtual ~AIProjectile();
		AIProjectile(CharacterData* origin, unsigned short clusterNum);
		int projectileID=-1;
		int currentX = 0;
		int currentY = 0;
		int currentZ = 0;
		int targetX = 0;
		int targetY = 0;
		int targetZ = 0;
		float slope;

		ProjectileExec* projectileExec = nullptr;

		CharacterData* origin = nullptr;
		CharacterData* source = nullptr; //changable dynamically, mainly used for retracing

		struct
		{
			int collisionNum = 0;
			int distance = 0;
		} deadTrack;

		union
		{
			int tickMax = 0;//from 0
			int collisionMax;
			int distanceMax;
		} maxTrack;

        int skillID = 0;
		int skillLevel = 1;
        bool shouldHideUI = false;

	public:
		virtual void update(AIContext& aiContext) = 0;
		void setUICompanion(AIUICallback* aiuiCallback);
		virtual float getYByX(float x)=0;
		virtual float getSlope(float x) = 0;
        virtual int getDirection() = 0;
		virtual int getRealChangeValue() = 0;
		virtual float getSpeedX();
        int getDistanceFromLastHit();

	protected:
		AIUICallback* aiuiCallback=nullptr;
        int distanceFromLastHit = 0;

	private:
		//AIProjectile(const AIProjectile&);
		//AIProjectile(AIProjectile&&);
		//AIProjectile& operator=(const AIProjectile&);
	};

}

#endif /* defined(__OnePiece__AIProjectile__) */
