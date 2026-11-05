//
//  ExternalDelegateAudio.h
//  OnePiece
//
//  Created by Beileng Shan on 3/11/14.
//
//

#ifndef __OnePiece__ExternalDelegateAudio__
#define __OnePiece__ExternalDelegateAudio__

namespace OnePiece {

	class CharacterData;
	class AIContext;
	class AIProjectile;

	class ExternalDelegateAudio
	{
	public:
		//执行入口
		virtual void executeExec(AIContext& aiContext, int audioID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen) = 0;
	};

}

#endif /* defined(__OnePiece__ExternalDelegateAudio__) */
