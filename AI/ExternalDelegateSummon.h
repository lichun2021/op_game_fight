//
//  ExternalDelegateSummon.h
//  op_game
//
//  Created by Ping Wang on 15/5/5.
//
//

#ifndef __op_game__ExternalDelegateSummon__
#define __op_game__ExternalDelegateSummon__

namespace OnePiece {
    
    class CharacterData;
    class AIContext;
    
    class ExternalDelegateSummon
    {
    public:
        virtual void createSummon(AIContext& aiContext, CharacterData* source, int iSummonID) = 0;
        virtual void awakeSummon(AIContext& aiContext, CharacterData* source, int iSummonID) = 0;
        virtual void sleepSummon(AIContext& aiContext, CharacterData* source, int iSummonID) = 0;
        virtual void hideOrShowSource(AIContext& aiContext, CharacterData* source,bool bHide) = 0;
		virtual void notifyClearSummon(int iSummonId) = 0;
    };
    
}

#endif /* defined(__op_game__ExternalDelegateSummon__) */
