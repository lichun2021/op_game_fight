//
//  AIBattleStats.cpp
//  op_game
//
//  Created by h7ing on 8/10/15.
//
//

#include "AIBattleStats.h"
#include "AIContext.h"

namespace OnePiece {

    namespace StatsFunc {

        StatsContainer& GetAllStatsSingletonInstance()
        {
            static StatsContainer sc;
            return sc;
        }

        void ResetStats()
        {
            //CCLOG("++++ battle stats:");
            //auto& allStats = GetAllStatsSingletonInstance();
            //for (auto iter = allStats.begin(); iter != allStats.end(); ++iter)
            //{
            //    CCLOG("hero uid(%d) id(%d) team(%d) cluster(%d) hurts(%d) heals(%d)", 
            //        iter->first,
            //        iter->second->heroID, 
            //        iter->second->teamSide,
            //        iter->second->clusterNum,
            //        iter->second->totalHurt, 
            //        iter->second->totalHeal);
            //}

            GetAllStatsSingletonInstance().clear();
        }

        void AppendHeroHurt(CharacterData& hero, HurtType hurtType, int value, CharacterData& target)
        {
#ifdef SERVER_VERIFYING
            return;
#endif
            /// 12.0
            if (hero.myID == target.myID && HurtType::HEALING_HURT_TYPE != hurtType)
                return;
            ///
            CharacterData *pHero = &hero;
            if (hero.isSummon())
                pHero = hero.getSummonSource();
            if (!pHero)
                return;
            //if (hero.teamSide == target.teamSide && HurtType::HEALING_HURT_TYPE != hurtType)
            //    return;
            if (hero.teamSide == target.teamSide)
            {
                if (HurtType::HEALING_HURT_TYPE != hurtType)
                    return;
                if (value < 0)
                    return;
            }
            auto& allStats = GetAllStatsSingletonInstance();
            if (allStats.find(pHero->myID) == allStats.end()) {
                AIBattleStatsPtr stats = AIBattleStatsPtr(new AIBattleStats);
                stats->heroID = pHero->getID();
                stats->teamSide = pHero->teamSide;
                stats->clusterNum = pHero->clusterNum;

                allStats[pHero->myID] = stats;
            }

            if (HurtType::HEALING_HURT_TYPE == hurtType)
                allStats[pHero->myID]->totalHeal += value;
            else
                allStats[pHero->myID]->totalHurt += value;
        }
    } // namespace StatsFunc

} // namespace OnePiece
