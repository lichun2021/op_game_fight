//
//  AIContinuousProjectileCaster.h
//  OnePiece
//
//  Created by Beileng Shan on 14/5/4.
//
//

#ifndef __OnePiece__AIContinuousProjectileCaster__
#define __OnePiece__AIContinuousProjectileCaster__

#include <vector>
#include <list>
#include "CommonType.h"
#include "AINode.h"

namespace OnePiece {
	class CharacterData;
	class AIContext;

	class AIContinuousProjectileCaster : public AINode
	{
	public:
		int projectileID;
		ProjectileExec* staticData;
		CharacterData* origin;
        int skillID;
		int skillLevel;
	public:
		AIContinuousProjectileCaster(int projectileID, CharacterData* origin, AIContext& aiContext, int skillID, int skillLevel);
		~AIContinuousProjectileCaster();
        void Excute(AIContext& aiContext);
        void implThrowProjectile(AIContext& aiContext, bool shouldTurn, bool shouldThrow);
        void implThrowProjectileNative(AIContext& aiContext, bool shouldTurn, bool shouldThrow);
        void throwBySearchOld(AIContext &aiContext, bool shouldTurn, bool shouldThrow);
        void throwBySearchNew(AIContext &aiContext, bool shouldTurn, bool shouldThrow);

	private:
		int getRealEndPos(ProjectileExec* def, int initPos,bool isY = false);
		//AIContinuousProjectileCaster(const AIContinuousProjectileCaster&);
		//AIContinuousProjectileCaster(AIContinuousProjectileCaster&&);
		//AIContinuousProjectileCaster& operator=(const AIContinuousProjectileCaster&);
        
        int _currentLazyTick = 0;
	};

}

#endif /* defined(__OnePiece__AIContinuousProjectileCaster__) */
