//
//  ProjectileForWarship.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__AIProjectileForWarship__
#define __OnePiece__AIProjectileForWarship__

#include "../AIProjectile.h"
#include "../AIContext.h"
#include "ParaCurve.h"
#include <vector>
#include <list>

namespace OnePiece {
	class CharacterData;
	class AIContext;

	class ProjectileForWarship : public AIProjectile
	{
	public:
		virtual ~ProjectileForWarship();
		ProjectileForWarship(AIContext& aiContext, int projectileID, CharacterData* origin, int sourceX, int sourceY, int sourceZ, int targetX, int targetY, int targetZ, int skillID, int skillLevel);
		AIContext& aiContext;

	public:
		virtual void update(AIContext& aiContext);
		virtual float getYByX(float x);
		virtual float getSlope(float x);
        virtual int getDirection();
		virtual int getRealChangeValue();

	private:
		bool startedCompanion;
		ParaCurve* pCurve;
		//AIProjectile(const AIProjectile&);
		//AIProjectile(AIProjectile&&);
		//AIProjectile& operator=(const AIProjectile&);
	};

}

#endif /* defined(__OnePiece__AIProjectile__) */
