//
//  ExternalDelegateSpecialExec.h
//  OnePiece
//
//  Created by Beileng Shan on 3/11/14.
//
//

#ifndef __OnePiece__ExternalDelegateSpecialExec__
#define __OnePiece__ExternalDelegateSpecialExec__

namespace OnePiece {

	class CharacterData;
	class AIContext;
	class AIProjectile;

	class ExternalDelegateSpecialExec
	{
	public:
		//执行入口
		virtual void executeExec(AIContext& aiContext, int audioID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen) = 0;
        virtual void stopScreenExec(AIContext& aiContext, CharacterData *target) = 0;
	};

}

#endif /* defined(__OnePiece__ExternalDelegateSpecialExec__) */
