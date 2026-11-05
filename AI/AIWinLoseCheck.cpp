//
//  AIWinLoseCheck.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/29/14.
//
//

#include "AIWinLoseCheck.h"
#include <cfloat>
#include "AIContext.h"
#include "AICannonKing.h"
#include "action_hero/HeroActionController.h"

namespace OnePiece {

    void CheckFuncGetClusterAliveInfo(AIContext& aiContext, int side, int clusterNum, int& outAliveCnt, int& outDeathTriggerCnt)
    {
        int aliveCnt = 0;
        int deathTriggerCnt = 0;
        auto cluster = aiContext.getClusterOrigin(TeamSide(side), clusterNum);
        for (auto iter = cluster->begin(); iter != cluster->end(); ++iter)
        {
            auto pCharacter = *iter;
            if (pCharacter->isCaptain())
                continue;
            if (pCharacter->isSummon() && pCharacter->summonProperty && pCharacter->summonProperty->affectWinlose == 0)
                continue;
            if (!HeroActionController::isDeadAction(pCharacter->currentAction))
            {
                aliveCnt++;
                //if (ActionType::DEATH_TRIGGERED_ATTACK_ACTION == (*iter)->currentAction)
                if (HeroActionController::isDeadAttacking(pCharacter))
                    deathTriggerCnt++;
            }
            else
            {
                if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *pCharacter, EffectType::eEFFECT_REVIVE, pCharacter->getHPTop()) > 0)
                {
                    aliveCnt++;
                }
            }
        }

        outAliveCnt = aliveCnt;
        outDeathTriggerCnt = deathTriggerCnt;
    }

	AIWinLoseCheck* AIWinLoseCheck::getInstance()
	{
		static AIWinLoseCheck* isme = NULL;
		if (isme == NULL) isme = new AIWinLoseCheck();
		return isme;
	}

	AIWinLoseCheck::AIWinLoseCheck() :
        conditionList(nullptr),
        timeElapseCluster(0),
        timeElapseBattle(0)
	{
	}

	AIWinLoseCheck::~AIWinLoseCheck()
	{}

    void AIWinLoseCheck::resetCluster(float *conditionList)
	{
        timeElapseCluster = 0;
		this->conditionList = conditionList;
	}

    void AIWinLoseCheck::resetBattleTimeElapse()
    {
        timeElapseBattle = 0;
    }

    float AIWinLoseCheck::getTimeElapseConditon() const
    {
        if (conditionList)
            return conditionList[0];
        return 0;
    }

    ////ongoing: 0, win:1, lose:2
    //int AIWinLoseCheck::check(AIContext& aiContext, float dt)
    //{
    //    // TODO: optimise
    //    int friend_alive_count = 0;
    //    int death_trigger_count = 0; // 处于死亡触发技状态
    //    std::list<CharacterData*>::iterator iter;
    //    std::list<CharacterData*>* ourCluster = aiContext.getClusterOrigin(TeamSide::kOur, aiContext.current_ClusterNumOur);
    //    for (iter = ourCluster->begin(); iter != ourCluster->end(); ++iter)
    //    {
    //        CharacterData* character = (*iter);
    //        if (!HeroActionController::isDeadAction(character->currentAction))
    //        {
    //            friend_alive_count++;
    //            if (ActionType::DEATH_TRIGGERED_ATTACK_ACTION == character->currentAction)
    //                death_trigger_count++;
    //        }
    //        //if (tmp_count > 0) break;
    //    }
    //    if (friend_alive_count == 0)
    //        return WinLoseType::kLose;

    //    int tmp_count = 0;
    //    std::list<CharacterData*>* currentRivals = aiContext.getClusterOrigin(TeamSide::kEnemy, aiContext.current_ClusterNumEnemy);
    //    if (currentRivals != nullptr)
    //    {
    //        std::list<CharacterData*>::iterator iter;
    //        for (iter = currentRivals->begin(); iter != currentRivals->end(); ++iter)
    //        {
    //            CharacterData* character = (*iter);
    //            if (!HeroActionController::isDeadAction(character->currentAction)) tmp_count++;
    //            if (tmp_count > 0) break;
    //        }
    //    }
    //    if (tmp_count == 0)
    //    {
    //        if (friend_alive_count == death_trigger_count)
    //            return WinLoseType::kDeterminedByServer;
    //
    //        return WinLoseType::kWin;
    //    }
    //
    //    if (!conditionList)
    //        return WinLoseType::kOnGoing;
    //    timeElapse += dt;
    //    if (timeElapse > conditionList[0])
    //        return WinLoseType::kDeterminedByServer;
    //
    //    return WinLoseType::kOnGoing;
    //}

    int AIWinLoseCheck::check(AIContext& aiContext, float dt)
    {
        int aliveCntOur = 0;
        int deathTriggerCntOur = 0;
        CheckFuncGetClusterAliveInfo(aiContext, TeamSide::kOur, aiContext.current_ClusterNumOur, aliveCntOur, deathTriggerCntOur);
        aiContext.setTeamAliveInfo(TeamSide::kOur, aiContext.current_ClusterNumOur, aliveCntOur, deathTriggerCntOur);

        int aliveCntEnemy = 0;
        int deathTriggerCntEnemy = 0;
        CheckFuncGetClusterAliveInfo(aiContext, TeamSide::kEnemy, aiContext.current_ClusterNumEnemy, aliveCntEnemy, deathTriggerCntEnemy);
        aiContext.setTeamAliveInfo(TeamSide::kEnemy, aiContext.current_ClusterNumEnemy, aliveCntEnemy, deathTriggerCntEnemy);

        if (BusinessType::CANNON_KING != aiContext.businessType)
        {
            if (0 == aliveCntOur && 0 == aliveCntEnemy)
                return WinLoseType::kDeuce;

            if (0 == aliveCntOur && 0 == deathTriggerCntEnemy)
                return WinLoseType::kLose;

            if (0 == aliveCntEnemy && 0 == deathTriggerCntOur)
                return WinLoseType::kWin;
        }
        // TODO: ptr safe check
        else if (aiContext.getCannonKing()->cannonEndTick > 0)
        {
            if (aiContext.getCannonKing()->cannonRemainCount <= 0 && aiContext.currentTick > aiContext.getCannonKing()->cannonEndTick)
                return WinLoseType::kDeterminedByServer;
        }

        timeElapseCluster += dt;
        timeElapseBattle += dt;

        float timeElapse = timeElapseCluster;
        if (BusinessType::TOWER_DEFENCE_TIME == aiContext.businessType ||
            BusinessType::PVE_GONGHUI == aiContext.businessType ||
            BusinessType::CANNON_KING == aiContext.businessType)
            timeElapse = timeElapseBattle;

        if (!conditionList || int(conditionList[0]) <= 0 || timeElapse <= conditionList[0])
            return WinLoseType::kOnGoing;



        // 时间到
        if (BusinessType::WHEELWAR_SOLO == aiContext.businessType ||
            BusinessType::WHEELWAR_GROUP == aiContext.businessType)
        {
            bool isOurLastCluster = aiContext.isCurrentLastClusterForTeam(TeamSide::kOur);
            bool isEnemyLastCluster = aiContext.isCurrentLastClusterForTeam(TeamSide::kEnemy);

            // 如果交战双方都还有替补，那么本场的双方参战角色全部出局
            if (!isOurLastCluster && !isEnemyLastCluster)
                return WinLoseType::kDeuce;

            // 如果双方都已经没有替补，那么当前存活队员数多的一方获胜
            // 当前存活队员数相同，那么防守方获胜
            if (isOurLastCluster && isEnemyLastCluster)
                return aliveCntOur <= aliveCntEnemy ? WinLoseType::kLose : WinLoseType::kWin;

            // 如果只有其中一方还有替补，那么有替补的一方获胜
            return isOurLastCluster ? WinLoseType::kLose : WinLoseType::kWin;
        }
        else if (BusinessType::TOWER_DEFENCE_TIME == aiContext.businessType)
        {
            return (0 != aliveCntOur ? WinLoseType::kWin : WinLoseType::kLose);
        }
        //else if (BusinessType::PVE_GONGHUI == aiContext.businessType)
        //    return WinLoseType::kDeuce;
        else
            return WinLoseType::kDeterminedByServer;
            
        return WinLoseType::kOnGoing;
    }
}

