//
//  ExternalDelegateTransform.h
//  op_game
//
//  Created by h7ing on 15/9/24.
//
//

#ifndef __op_game__ExternalDelegateTransform__
#define __op_game__ExternalDelegateTransform__

namespace OnePiece {
    
    class CharacterData;
    class AIContext;
    
    class ExternalDelegateTransform
    {
    public:
        virtual void createTransformCreature(AIContext& aiContext, CharacterData* source, int transformAniCharID) = 0;
        virtual void transformToCreature(AIContext& aiContext, CharacterData* source, int transformAniCharID) = 0;
        virtual void clearTransformer(int transformerUID) = 0;
        virtual void enterTransformToCharacter(AIContext& aiContext, CharacterData* source, int aniCharId) = 0;
        virtual void exitTransform(CharacterData* source) = 0;
        virtual void clear() = 0;
    };
    
}

#endif /* defined(__op_game__ExternalDelegateTransform__) */
