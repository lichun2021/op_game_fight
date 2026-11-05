//
//  AIProjectile.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__AIProjectileForCharacter__
#define __OnePiece__AIProjectileForCharacter__

#include "../AIProjectile.h"
#include "../AIContext.h"
#include "ParaCurve.h"
#include <list>
#include <vector>
#include <set>

namespace OnePiece {

	class ProjectileForCharacter : public AIProjectile
	{
	public:
		virtual ~ProjectileForCharacter();
		ProjectileForCharacter(AIContext& aiContext, int projectileID, CharacterData* origin, CharacterData* target, int skillID, int skillLevel);
		AIContext& aiContext;

	public:
		virtual void update(AIContext& aiContext);
		virtual float getYByX(float x);
		virtual float getSlope(float x);
        virtual int getDirection();
		virtual int getRealChangeValue();

	private:
		CharacterData* target;
		ParaCurve* pCurve;
        bool startedCompanion;
		//AIProjectile(const AIProjectile&);
		//AIProjectile(AIProjectile&&);
		//AIProjectile& operator=(const AIProjectile&);
        std::set<int> _hitedCharacterMyId;

    private:
        void updateCollision(CharacterDataListPtr targets);
	};

}

#endif /* defined(__OnePiece__AIProjectile__) */
