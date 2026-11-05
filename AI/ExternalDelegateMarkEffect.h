//
//  ExternalDelegateMarkEffect.h
//  OnePiece
//
//  Created by h7ing on Mar/16/2015.
//
//

#ifndef __OnePiece__ExternalDelegateMarkEffect__
#define __OnePiece__ExternalDelegateMarkEffect__

namespace OnePiece {

    class CharacterData;
    class AIContext;

    class ExternalDelegateMarkEffect
    {
    public:
        virtual void executeExec(AIContext& aiContext, CharacterData* source, int markID, int markLevel) = 0;
        virtual void notify(AIContext& aiContext, CharacterData* source, int markID) = 0;
    };

} // namespace OnePiece

#endif
