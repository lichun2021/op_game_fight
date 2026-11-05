//
//  AIProjectile.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__AIProjectileForLanding__
#define __OnePiece__AIProjectileForLanding__

#include "../AIProjectile.h"
#include "../AIContext.h"
#include "ParaCurve.h"
#include <vector>
#include <list>

namespace OnePiece {
	class CharacterData;
	class AIContext;

	class ProjectileForLanding : public AIProjectile
	{
	public:
		virtual ~ProjectileForLanding();
		ProjectileForLanding(AIContext& aiContext, int projectileID, CharacterData* origin, CharacterData* target, int targetX, int targetY, int targetZ, int skillID, int skillLevel);
		int targetX, targetY, targetZ;
		AIContext& aiContext;

	public:
		virtual void update(AIContext& aiContext);
		virtual float getYByX(float x);
		virtual float getSlope(float x);
        virtual int getDirection();
		virtual int getRealChangeValue();
		virtual float getSpeedX() override;

	private:
		CharacterData* target;
		bool startedCompanion;
		ParaCurve* pCurve;
        int direction;
        bool isFirstRun;
		bool isReachPos12;

		int GetRealTargetX();
        int checkIfHitTarget(CharacterData *theTarget);
        void updateCollision(std::list<CharacterData *> *list);
        void updateCollisionNearest(std::list<CharacterData *> *list);
		bool isMatchDeadCond();
		//AIProjectile(const AIProjectile&);
		//AIProjectile(AIProjectile&&);
		//AIProjectile& operator=(const AIProjectile&);
	};

}

#endif /* defined(__OnePiece__AIProjectile__) */
