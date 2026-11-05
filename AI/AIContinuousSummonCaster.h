//
//  AIContinuousSummonCaster.h
//  op_game
//
//  Created by Ping Wang on 15/5/5.
//
//

#ifndef __op_game__AIContinuousSummonCaster__
#define __op_game__AIContinuousSummonCaster__

#include <vector>
#include <list>
#include "CommonType.h"
#include "AINode.h"

namespace OnePiece {
    class CharacterData;
    class AIContext;
    
    class AIContinuousSummonCaster : public AINode
    {
    public:
		SummonExec* summonExec;
		SummonProperty *summonProperty;
        CharacterData* origin;
        CharacterData* target;
        AIContext& aiContext;
		std::list<CharacterData*> m_unActiveSummonList;
        int skillLevel;

    public:
		AIContinuousSummonCaster(AIContext& aiContext, CharacterData* origin, CharacterData* target, SummonExec* pSummonExec, int skillLv, int innerExecIndex);
        ~AIContinuousSummonCaster();
        void Excute(AIContext& aiContext);
        
        std::list<int> summonUIDList; //characterMyID
        
        //void showSummons();
        void removeSummons();
        //int removeSummonByID(int characterID, int removeNum);
        int removeSummon(int summonID, int num);

        int getActiveCount();
        int getSummonID();

        static void sleepCasterSummon(CharacterData *target, int summonID, int coverCnt, int excludeCasterMyID);
        static void sleepGroupSummon(int groupID, int groupWeight, CharacterData *target, int excludeCasterMyID, int sleepNum);
        static int getGroupSummonAvailableCount(int casterMyID, CharacterData *target, int execID);
        
        static bool couldHurtSummon(CharacterData* pSource, HurtExec* pHurtExec, CharacterData* pTarget, int skillID);
        static void checkExcuteDeadRun(AIContext &aiContext, CharacterData *pSummon, int runCondition);
		//void beforeShowSummon();
        static void initSummonPool(AIContinuousSummonCaster *caster, CharacterData *summoner, int summonId, int poolType);
        
        void outCharm253();

    private:
		int _innerExecIndex;
        //bool _shouldDeleteSelf = false;
        int _summonID;
        std::list<CharacterData *> _summonPool;
        std::map<int, std::list<int>> _summonExecTodo; // <interval,[execid]>
        
        //void awakeSummons(const SummonProperty* pSummonProperty,const SummonInnerExec& summonInnerExec);
		//void beforeAwakeSummons(const SummonProperty* pSummonProperty, const SummonInnerExec& summonInnerExec);
        //bool isTargetSummon(const SummonProperty* pSummonProperty,CharacterData* pCharacterData);
        
        //void setSummonPos(const SummonProperty* pSummonProperty,CharacterData* pSummon);
        void initSummonPos(CharacterData *theSummon);
        void initSummonExecTodo(SummonInnerExec &theExec);
        void summonDoExec(int execID);
    };
    
}

#endif /* defined(__op_game__AIContinuousSummonCaster__) */
