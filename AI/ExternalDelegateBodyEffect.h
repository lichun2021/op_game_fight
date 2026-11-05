//
//  ExternalDelegateBodyEffect.h
//  OnePiece
//
//  Created by Huang Qing on Nov/24/15.
//
//

#ifndef __OnePiece__ExternalDelegateBodyEffect__
#define __OnePiece__ExternalDelegateBodyEffect__

namespace OnePiece {

	class CharacterData;
	class AIContext;

    class ExternalDelegateBodyEffect
	{
	public:
		virtual void executeExec(AIContext& aiContext, int parentExecID, int bodyEffectID, CharacterData* target) = 0;
		virtual void notify(AIContext& aiContext, int parentExecID) = 0;
        virtual void clear() = 0;
	};

}

#endif /* defined(__OnePiece__ExternalDelegateBodyEffect__) */
