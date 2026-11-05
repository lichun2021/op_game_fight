//
//  AIProjectile.h
//  OnePiece
//
//
//

#ifndef __OnePiece__AIProjectileForBouncing__
#define __OnePiece__AIProjectileForBouncing__

#include "../AIProjectile.h"
#include "../AIContext.h"
#include "ParaCurve.h"
#include <list>
#include <vector>

namespace OnePiece {

    class ProjectileForBouncing : public AIProjectile
    {
    public:
        virtual ~ProjectileForBouncing();
        ProjectileForBouncing(AIContext& aiContext, int projectileID, CharacterData* origin, CharacterData* target, int skillID, int skillLevel);
        AIContext& aiContext;

    public:
        virtual void update(AIContext& aiContext);
        virtual float getYByX(float x);
        virtual float getSlope(float x);
        virtual int getDirection();
		virtual int getRealChangeValue();

    private:
        CharacterData* getNextTarget(CharacterData* curTarget);
        bool isCurStateMatchDeadCondition();
        bool isMatchCollideDeadCondition(bool isCurTargetHitted, CharacterData* nextTarget);
        void doDeadExec();
		CharacterData* getAnotherTarget();

    private:
        CharacterData* target;
        ParaCurve* pCurve;
        bool startedCompanion;
        int direction;
    };

}

#endif /* defined(__OnePiece__AIProjectileForBouncing__) */
