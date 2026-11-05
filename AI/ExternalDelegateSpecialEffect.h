//
//  ExternalDelegateSpecialEffect.h
//  OnePiece
//
//  Created by Beileng Shan on 3/11/14.
//
//

#ifndef __OnePiece__ExternalDelegateSpecialEffect__
#define __OnePiece__ExternalDelegateSpecialEffect__

namespace OnePiece {

	class CharacterData;
	class AIContext;
	class AIProjectile;

	class ExternalDelegateSpecialEffect
	{
	public:
		//执行入口
        virtual void executeExec(AIContext& aiContext, int parentExecID, int sefID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos, int altLinkEffLinkTick) = 0;
		virtual void notify(AIContext& aiContext, int parentExecID) = 0;
	};

}

#endif /* defined(__OnePiece__ExternalDelegateSpecialEffect__) */
