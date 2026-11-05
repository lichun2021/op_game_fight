#ifndef __OnePiece__HeroStruckHoverAction__
#define __OnePiece__HeroStruckHoverAction__

#include "../AIContext.h"

namespace OnePiece {

    class HeroStruckHoverAction
    {
        friend class HeroActionController;
    public:
        static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
        static void Enter(CharacterData& aiCharacter, AIContext& aiContext, int SpeedY, int totalTicks, CharacterData& aiAttacker);
        static void Exit(CharacterData& aiCharacter, AIContext& aiContext);
    };

}

#endif /* defined(__OnePiece__HeroStruckHoverAction__) */
