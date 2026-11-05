//
//  ExternalDelegateAnimation.h
//  OnePiece
//
//  Created by Beileng Shan on 3/11/14.
//
//

#ifndef __OnePiece__ExternalDelegateAnimation__
#define __OnePiece__ExternalDelegateAnimation__

namespace OnePiece {

	class CharacterData;
	class AIContext;
	class AIProjectile;

	class ExternalDelegateAnimation
	{
	public:
		//执行入口
		virtual void executeExec(AIContext& aiContext, int audioID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen) = 0;

        virtual void resyncCharacter(AIContext& aiContext, CharacterData* hero) = 0;
        virtual void forceTurnCharacter(AIContext& aiContext, CharacterData* hero) = 0;
        virtual void reviveCharacter(AIContext& aiContext, CharacterData* hero) = 0;
        virtual void enterCopyCharacter(AIContext& aiContext, CharacterData* copyer, CharacterData* copySource) = 0;
        virtual void exitCopyCharacter(AIContext& aiContext, CharacterData* copyer) = 0;
        virtual void enterStateAnimation(AIContext& aiContext, CharacterData* target, int theStateAnimation, unsigned char r, unsigned char g, unsigned char b, int theShaderParam) = 0;
        virtual void exitStateAnimation(AIContext& aiContext, CharacterData* target, int theStateAnimation) = 0;
	};

}

#endif /* defined(__OnePiece__ExternalDelegateAnimation__) */
