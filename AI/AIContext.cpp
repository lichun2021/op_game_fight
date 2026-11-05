//
//  AIContext.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "AIContext.h"
#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include <thread>
#include "cocos2d.h"
#include "lua.hpp"
#include "AIScriptBinder.hpp"
#include "scriptbind/AIScriptBindContext.hpp"
#include "scriptbind/AIScriptBindCharacterData.hpp"
#include "AIBattleStats.h"
#include "AIDataManager.h"
#include "AIWinLoseCheck.h"
#include "AICallback.h"
#include "AIProjectile.h"
#include "AIActionController.h"
#include "AISpecialEffectLabel.h"
#include "AICannonKing.h"
#include "action_hero/HeroActionController.h"
#include "action_hero/HeroDeadAction.h"
#include "action_hero/HeroCelebrateAction.h"
#include "action_hero/HeroIdleAction.h"
#include "action_hero/HeroAttackAction.h"
#include "action_hero/HeroSkillController.h"
#include "projectile/ProjectileForWarship.h"
#include "BattleLog.h"
#include "AITargetHelper.h"


using namespace std;

namespace OnePiece {

    int _CELEBRATE_PERIOD = 30;

    bool isBusinessPVP(int business) {
        switch (business) {
        case PVP_WANTED:
        case PVP_FRIEND:
            return true;
        }
        return false;
    }

    bool isBusinessPVE(int business) {
        switch (business) {
        case PVE_GONGHUI:
        case PVE_GUANQIA:
        case PVE_ISLAND_CRUISE:
        case PVE_CALENDAR:
            return true;
        }
        return false;
    }

    bool isBusinessTowerDefence(int business) {
        switch (business) {
        case TOWER_DEFENCE_TIME:
        case TOWER_DEFENCE_WAVE:
            return true;
        }
        return false;
    }
    
    int AIContext::initLua()
    {
        if (_luaState)
        {
            lua_close(_luaState);
        }
        
        _luaState = op_createluastate();
        AIScriptBinder::getInstance()->initConfig(_luaState);
        if (!_luaState) { return 1; } // error!
        
        return 0;
    }

    AIContext::AIContext() : myPlaySpeed(1), clearWave_Countdown(0),
        clusterAttrInheritInfos(nullptr), clusterAttrEnhanceInfos(nullptr),
        currentWaveCount(0),
        nextClusterNumEnemy(-1), 
        passedClusterCount(0),
        input_trackAccumulate(nullptr)
    {
        localRandom = nullptr;
        rewardRandom = nullptr;
        //currentOurCluster = new CharacterDataList();
        //ourClusterRealtime = new CharacterDataList();

        battleFieldCenterX = 0;
        adjustingDistance = 0;
        clusterRangeToCenterX = 0;

        //winlosecheck_clusters = new std::map<unsigned short, float*>();
        aiCallback = nullptr;
        animationProxy = nullptr;
        audioProxy = nullptr;
        seffProxy = nullptr;
        sexecProxy = nullptr;
        comboProxy = nullptr;
        markProxy = nullptr;
        summonProxy = nullptr;
        transformProxy = nullptr;
        bodyEffectProxy = nullptr;

        clearWave_Countdown = 0;
        winlose = 0;
        local_incremential = 0;
        current_ClusterNumEnemy = 0;//0=friends, 1=wave#1,2=wave#2,3=wave#3
        current_ClusterNumOur = 0;
        currentFriendK = 0;
        currentRivalK = 0;

        currentTick = 0;
        length = 0;
        deltaMaxY = 0;
        deltaMinY = 0;
        myPlaySpeed = 1;
        autoFight = 0;

        //monster can move
        monsterCanMove = false;
        guanqiaID = -1;

        //trackCannon[0] = -1;
        //trackCannon[10] = -1;
        //trackCannon[20] = -1;
        //trackCannon_index = 0;
        input_trackCannon_index = 0;
        //input_trackCannon = nullptr;
        input_trackUltra = nullptr;
        input_trackSetDistance = nullptr;
        input_trackAutofight = nullptr;
        input_trackCannons = nullptr;

        captainOur = nullptr;
        captainEnemy = nullptr;

        //cui_2023_3_14 暂停所有动画，包括逻辑帧
        //realStopConut = 0;
        
        AITargetHelper::init();
    }

    AIContext::~AIContext()
    {
        clear();
    }

    void AIContext::setOPInput(//std::vector<int>* trackCannon,
        InputTrackCannonPtr input_trackCannon,
        std::unordered_map<std::string, int>* input_trackUltra,
        std::vector<int>* input_trackSetDistance,
        std::unordered_map<int, int>* input_trackAutofight,
        InputTrackAccumulatePtr input_trackAccumulate,
        std::shared_ptr< std::vector<int> > input_trackCenterX)
    {
        this->input_trackCannons = input_trackCannon;
        this->input_trackUltra = input_trackUltra;
        this->input_trackSetDistance = input_trackSetDistance;
        this->input_trackAutofight = input_trackAutofight;
        this->input_trackAccumulate = input_trackAccumulate;
        this->input_trackCenterX = input_trackCenterX;
    }

    void AIContext::setServerVerifyClusterAttrInheritInfos(const AttrInheritInfosPtr& pInfos)
    {
        _serverVerifyAttrInheritInfos = pInfos;
    }

    void AIContext::setServerVerifyEnemyWaveInfos(const EnemyWaveInfosPtr& pInfos)
    {
        _serverVerifyEnemyWaveInfos = pInfos;
    }

    void AIContext::clear()
    {
        _isBeingFinalClear = true;

        StatsFunc::ResetStats();

        currentWaveCount = 0;
        pauseAIFalg = false;
        _hasWinlose = false;

        _beginSkillLazyTick = 0;
        _beginSkillToReleaseTick = 0;

        //cui_2023_3_14  暂停所有动画，包括逻辑帧
        //realStopConut = 0;

        _ignoreNoDieCharacters.clear();

        _hploseListeners.clear();
        _eventListeners.clear();
        _appendedExcuteExecs.clear();
        _scheduleExcuteExecs.clear();

        sceneAnims.clear();

        if (seffProxy) {
            for (auto iter = _scheduleNotifyUIEffs.begin(); iter != _scheduleNotifyUIEffs.end(); ++iter) {
                for (auto parentID : iter->second) {
                    seffProxy->notify(*this, parentID);
                }
            }
        }
        _scheduleNotifyUIEffs.clear();

        for (auto iter2 = reward_candidates.begin(); iter2 != reward_candidates.end(); iter2++)
        {
            for (std::vector<RewardCandidateItem*>::iterator iter3 = iter2->second->begin(); iter3 != iter2->second->end();)
            {
                for (std::vector<IARitem*>::iterator iter4 = (*iter3)->iarlist.begin(); iter4 != (*iter3)->iarlist.end();){
                    delete *(iter4++);
                }
                delete *(iter3++);
            }
            free(iter2->first);
            delete iter2->second;
        }
        reward_candidates.clear();

        //通知消除召唤物
        if (summonProxy)
        {
            for (CharacterDataList::iterator ite = m_summonCharacterList.begin(); ite != m_summonCharacterList.end(); ++ite)
            {
                if (*ite)
                    summonProxy->notifyClearSummon((*ite)->myID);
            }
            m_summonCharacterList.clear();
        }

        if (transformProxy)
            transformProxy->clear();

        if (bodyEffectProxy)
            bodyEffectProxy->clear();

        auto currentOurCluster = getClusterOrigin(TeamSide::kOur, current_ClusterNumOur);
        for (auto iter = currentOurCluster->begin(); iter != currentOurCluster->end(); ++iter)
        {
            //HeroActionController::cleanup(**iter, *this, true);
            HeroActionController::safeCleanupCasters(**iter, *this);
        }
        auto currentRivals = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);
        for (auto iter = currentRivals->begin(); iter != currentRivals->end(); ++iter)
        {
            //HeroActionController::cleanup(**iter, *this, true);
            HeroActionController::safeCleanupCasters(**iter, *this);
        }

        for (auto iter = clustersEnemy.begin(); iter != clustersEnemy.end(); ++iter)
            delete iter->second;
        clustersEnemy.clear();

        //for (auto iter = clustersRealEnemy.begin(); iter != clustersRealEnemy.end(); ++iter)
        //    delete iter->second;
        //clustersRealEnemy.clear();

        for (auto iter = clustersOur.begin(); iter != clustersOur.end(); ++iter)
            delete iter->second;
        clustersOur.clear();

        //for (auto iter = clustersRealOur.begin(); iter != clustersRealOur.end(); ++iter)
        //    delete iter->second;
        //clustersRealOur.clear();

        //_allFriendsRivals.clear();
        for (auto it1 = _allFriendsRivals.begin(); it1 != _allFriendsRivals.end(); ++it1)
        {
            for (auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
            {
                auto& dataList0 = std::get<0>(it2->second);
                dataList0.clear();

                auto& dataList1 = std::get<1>(it2->second);
                dataList1.clear();

                auto& dataList2 = std::get<2>(it2->second);
                dataList2.clear();

                auto& dataList3 = std::get<3>(it2->second);
                dataList3.clear();
            }
        }

        for (std::unordered_map<unsigned short, float* >::iterator iter = winlosecheck_clusters.begin(); iter != winlosecheck_clusters.end(); iter++)
            delete[] iter->second;
        winlosecheck_clusters.clear();

        //projectileList.clear();
        for (std::list<int*>::iterator iter = ultraFreezeList.begin(); iter != ultraFreezeList.end();)
        {
            delete[] * iter++;
        }
        ultraFreezeList.clear();

        AIActionController::executeExec(*this, 87773, 0, 1, nullptr, nullptr, nullptr, 0);
        AIActionController::executeExec(*this, 87771, 0, 1, nullptr, nullptr, nullptr, 0);

        //cui 2023_2_24 延时执行列表
        for (std::list<DelayAIActionData*>::iterator iter = dalayAIActionList.begin(); iter != dalayAIActionList.end();)
        {
            delete *iter++;
        }
        dalayAIActionList.clear();



        for (std::list<AIProjectile*>::iterator pProjectile = projectileList.begin(); pProjectile != projectileList.end();)
        {
            delete *pProjectile++;
        }
        projectileList.clear();

        cannon = nullptr;
        _pCannonKing = nullptr;
        captainOur = nullptr;
        captainEnemy = nullptr;

        // MARK: mode by h7ing
        // 现在角色数据从这里释放
        for (auto iter = allCharacters.begin(); iter != allCharacters.end(); ++iter)
        {
            if (iter->second)
                delete iter->second;
        }
        allCharacters.clear();

        _allFriendsRivals.clear();

        delete aiCallback;
        aiCallback = nullptr;
        
        AIContinuousHurtCaster::cleanRemainCaster();

        winlose = 0;
        local_incremential = 0;
        current_ClusterNumEnemy = 0;
        current_ClusterNumOur = 0;
        currentFriendK = 0;
        currentRivalK = 0;
        nextClusterNumEnemy = -1;
        passedClusterCount = 0;
        currentClusterHPTopOur = 0;
        currentClusterHPTopEnemy = 0;

        currentTick = 0;
        length = 0;
        deltaMaxY = 0;
        deltaMinY = 0;
        myPlaySpeed = 1;
        battleFieldCenterX = 0;

        _inputBattleFieldCenterX = 0;
        _toNextClusterCnt = 0;

        autoFight = 0;
        businessType = -1;

        _subjectiveTeamside = TeamSide::kOur;

        //monster can move
        monsterCanMove = false;
        guanqiaID = -1;

        _isBattleMirroredForPlay = false;
        _isUnitHealthBarColorMirrored = false;
        _isUnitHealthBarTypeMirrored = false;

        AIWinLoseCheck::getInstance()->resetBattleTimeElapse();

        //CCLOG("SHANBEILENG-trackCannon:%d-%d-%d-%d-%d-%d-%d-%d-%d", trackCannon[0], trackCannon[1], trackCannon[10], trackCannon[11], trackCannon[20], trackCannon[21], trackSetDistance[0], trackSetDistance[1], trackSetDistance[2]);
        //CCLOG("SHANBEILENG-trackUltra:%d", trackUltra.size());

        //trackCannon[0] = -1;
        //trackCannon[10] = -1;
        //trackCannon[20] = -1;
        //trackCannon_index = 0;
        trackCannons.clear();

        //if (input_trackCannon != nullptr) delete input_trackCannon;
        //input_trackCannon = nullptr;
        input_trackCannon_index = 0;
        input_trackCannons = nullptr;

        trackUltra.clear();
        if (input_trackUltra != nullptr)
        {
            delete input_trackUltra;
            input_trackUltra = nullptr;
        }

        trackAccumulate.clear();
        input_trackAccumulate = nullptr;

        trackAutofight.clear();
        if (input_trackAutofight != nullptr) delete input_trackAutofight;
        input_trackAutofight = nullptr;

        //trackSetDistance[0] = -1;
        //trackSetDistance[1] = -1;
        //trackSetDistance[2] = -1;
        //trackSetDistance[3] = -1;
        //trackSetDistance[4] = -1;
        //trackSetDistance[5] = -1;
        trackSetDistance.clear();
        if (input_trackSetDistance != nullptr) delete input_trackSetDistance;
        input_trackSetDistance = nullptr;

        trackCenterX.clear();
        input_trackCenterX = nullptr;

        clusterAttrInheritInfos.reset();
        clusterAttrEnhanceInfos.reset();

        _serverVerifyAttrInheritInfos = nullptr;
        _serverVerifyEnemyWaveInfos = nullptr;

        CharacterFunc::ClearAttrDataCache();

        if (_isLuaReady)
        {
            //lua_gc(_luaState, LUA_GCCOLLECT, 0);
            luahelper_characterdata_reset_register(_luaState);
            bool shouldResetLuaState = false;
#ifndef SERVER_VERIFYING
            shouldResetLuaState = luahelper_aicontext_clear_client(_luaState);
#else
            shouldResetLuaState = luahelper_aicontext_clear_server(_luaState);
#endif
            if (shouldResetLuaState)
            {
                op_closeluastate(_luaState);
                _luaState = nullptr;
                _isLuaReady = false;

                AIScriptBinder::getInstance()->resetDefaultConfig();
            }
        }

#ifndef SERVER_VERIFYING
        //客户端准备清理静态数据缓存
        AIDataHelper::unload();
        AIDataManager::unload();
#endif

        _isBeingFinalClear = false;
    }

    void AIContext::clearWave(bool bCelebrate)
    {
        for (std::list<int*>::iterator iter = ultraFreezeList.begin(); iter != ultraFreezeList.end();)
        {
            delete[] * iter++;
        }
        ultraFreezeList.clear();

        AIActionController::executeExec(*this, 87773, 0, 1, nullptr, nullptr, nullptr, 0);
        AIActionController::executeExec(*this, 87771, 0, 1, nullptr, nullptr, nullptr, 0);


        for (std::list<AIProjectile*>::iterator pProjectile = projectileList.begin(); pProjectile != projectileList.end();)
        {
            delete *pProjectile++;
        }
        projectileList.clear();

        clusterAttrInheritInfos.reset();
        clusterAttrEnhanceInfos.reset();

        nextClusterNumEnemy = -1;

        auto currentOurCluster = getClusterOrigin(TeamSide::kOur, current_ClusterNumOur);

        if (BusinessType::CANNON_KING == businessType)
        {
            auto rivals = getClusterOrigin(TeamSide::kEnemy, 0);
            for (auto iter = rivals->begin(); iter != rivals->end(); ++iter)
            {
                auto character = *iter;
                if (!HeroActionController::isDeadAction(character->currentAction))
                {
                    character->resetPos();
                    HeroIdleAction::Enter(*character, *this, _CELEBRATE_PERIOD - 6, IDLE_ACTION, *character);
                }
            }
        }
        else if (winlose == WinLoseType::kWin) //WIN
        {
            CharacterDataListPtr currentRivals = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);
            for (CharacterDataList::iterator iter = currentRivals->begin(); iter != currentRivals->end(); iter++)
            {
                CharacterData* character = (*iter);
                if (!HeroActionController::isDeadAction(character->currentAction)) HeroDeadAction::Enter(*character, *this, 1);
            }

            for (CharacterDataList::iterator iter = currentOurCluster->begin(); iter != currentOurCluster->end(); iter++)
            {
                CharacterData* character = (*iter);
                if (!HeroActionController::isDeadAction(character->currentAction))
                {
                    if (bCelebrate)
                        HeroCelebrateAction::Enter(*character, *this, _CELEBRATE_PERIOD - 6, CELEBRATE_ACTION);
                    else
                        HeroIdleAction::Enter(*character, *this, _CELEBRATE_PERIOD - 6, IDLE_ACTION, *character);
                }
            }
        }
        else if (winlose == WinLoseType::kLose) //LOSE
        {
            CharacterDataListPtr currentRivals = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);
            for (CharacterDataList::iterator iter = currentRivals->begin(); iter != currentRivals->end(); iter++)
            {
                CharacterData* character = (*iter);
                if (!HeroActionController::isDeadAction(character->currentAction))
                {
                    //if (bCelebrate)
                    //	HeroCelebrateAction::Enter(*character, *this, _CELEBRATE_PERIOD - 6, CELEBRATE_ACTION);
                    //else
                    HeroIdleAction::Enter(*character, *this, _CELEBRATE_PERIOD - 6, IDLE_ACTION, *character);
                }
            }

            for (CharacterDataList::iterator iter = currentOurCluster->begin(); iter != currentOurCluster->end(); iter++)
            {
                CharacterData* character = (*iter);
                if (!HeroActionController::isDeadAction(character->currentAction)) HeroDeadAction::Enter(*character, *this, 1);
            }
        }
        else if (winlose == WinLoseType::kDeuce) // 平局, 移除双方
        {
            for (CharacterDataList::iterator iter = currentOurCluster->begin(); iter != currentOurCluster->end(); iter++)
            {
                CharacterData* character = (*iter);
                if (!HeroActionController::isDeadAction(character->currentAction))
                {
                    HeroActionController::cleanup(*character, *this, true);
                    HeroDeadAction::Enter(*character, *this, 1);
                }
            }

            CharacterDataListPtr currentRivals = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);
            for (CharacterDataList::iterator iter = currentRivals->begin(); iter != currentRivals->end(); iter++)
            {
                CharacterData* character = (*iter);
                if (!HeroActionController::isDeadAction(character->currentAction))
                {
                    HeroActionController::cleanup(*character, *this, true);
                    HeroDeadAction::Enter(*character, *this, 1);
                }
            }
        }
    }

    AIContext* AIContext::isme = NULL;

    AIContext* AIContext::getInstance()
    {
        if (isme == NULL) isme = new AIContext();

        return isme;
    }

    void AIContext::fightAsyncUntilWinlose()
    {
        if (asyncMeleeCombatCnt > 0)
        {
            CCLOG("!!!ai:StartFightAsync error, aysnc threads cnt:%d", asyncMeleeCombatCnt);
            return;
        }
        asyncMeleeCombatCnt++;

        std::thread a_thread(fightAsyncHelper);
        a_thread.detach();
    }

    void AIContext::fightAsyncHelper()
    {
        AIContext *inst = AIContext::getInstance();
        do {
            inst->updateAIContext(0.05);
        } while (!inst->hasWinlose() && !inst->stopAsyncMeleeCombatFlag);

        inst->asyncMeleeCombatCnt--;
    }

    void AIContext::startFightAsync(const std::string &battlelog)
    {
        if (asyncMeleeCombatCnt > 0)
        {
            CCLOG("!!!ai:StartFightAsync error, aysnc threads cnt:%d", asyncMeleeCombatCnt);
            return;
        }
        asyncMeleeCombatCnt++;

        this->meleeCombatBattleInfo = std::unique_ptr<MeleeCombatBattleInfo>(new MeleeCombatBattleInfo);
        AIDataHelper::parseMeleeCombatBattleLog(battlelog, this->meleeCombatBattleInfo.get());

        std::thread a_thread(verifyMeleeCombatByClient);
        a_thread.detach();
    }

    void AIContext::verifyMeleeCombatByClient()
    {
        AIContext *inst = getInstance();
        MeleeCombatBattleInfo *battleinfo = inst->meleeCombatBattleInfo.get();

        /// 
        for (auto iter = battleinfo->conditions.begin(); iter != battleinfo->conditions.end(); ++iter)
        {
            float *conditionList = new float[2];
            conditionList[0] = iter->second;
            conditionList[1] = 0;

            inst->setWinLoseCheck(conditionList, iter->first);
        }

        /// 
        inst->setDeltaY(battleinfo->battleMinY, battleinfo->battleMaxY);

        /// addcannon
        //{
        //    int type = 1;
        //    int clusterNum = 0;
        //    int * attList = new int[ATT_NUM]{ 10001, 0, 31, 0, 0, 0, 19181, 9804, 5371, 9401, 5402, 0, 0, 15, 456, 254, 99, 58, 200, 200, 184, 144, 348, 177, 0, 0, 3, 3, 0, 0, 0, 0, 30, 200, 0, 0, 0, 0};
        //    std::list<std::pair<int, int> > skilllist;
        //    int cannonUID = inst->addHeroCharacters(attList, skilllist, 0, type, 100, 1, clusterNum, OnePiece::TeamSide::kInvalid, 0);
        //    CharacterData *cannon = inst->getCharacterByID(cannonUID);
        //    cannon->currentAction = OnePiece::DEAD_ACTION1;
        //}

        /// 
        for (int i = 0; i < battleinfo->allcharacters.size(); i++)
        {
            MeleeCombatCharacterInfo &a_cinfo = battleinfo->allcharacters[i];

            int *attList = new int[ATT_NUM];
            memset(attList, 0, ATT_NUM * sizeof(int));
            for (int j = 0; j < ATT_NUM; j++)
            {
                if (j < a_cinfo.attrs.size())
                    attList[j] = a_cinfo.attrs[j];
            }

            std::list<std::pair<int, int>> skilllist;
            for (int j = 0; j < a_cinfo.skills.size(); j++)
            {
                skilllist.push_back(a_cinfo.skills[j]);
            }

            int juexing = a_cinfo.juexing;
            int type = a_cinfo.type;
            short k = a_cinfo.k;
            short ifUltraLock = a_cinfo.ifUltraLock;
            int clusterNum = a_cinfo.clusterNum;
            int teamSide = a_cinfo.teamSide;
            int fp = a_cinfo.fp;

            int a_cmyid = inst->addHeroCharacters(attList, skilllist, juexing, type, k, ifUltraLock, clusterNum, OnePiece::TeamSide(teamSide), fp);
            CharacterData *c = inst->getCharacterByID(a_cmyid);
            c->setTagValue("_mcid", c->getID()); // 白刃战初始船员tag
            c->setX(a_cinfo.posX);
            c->setY(a_cinfo.posY);

            for (int j = 0; j < a_cinfo.bqsls.size(); j++)
            {
                std::pair<int, int> &a_bqsl = a_cinfo.bqsls[j];
                c->addTalent(a_bqsl.first, a_bqsl.second);
            }
        }

        inst->setAutoFight(1);
        inst->setRewardSeed(battleinfo->seed);
        inst->setSeed(battleinfo->seed, 1);
        inst->setBusinessType(battleinfo->businesstype);

        /// 
        inst->toNextCluster(0, 0);

        /// 
        int maxtick = battleinfo->maxticklimit;
        for (int i = 0; i < maxtick; i++)
        {
            inst->updateAIContext(0.05);
            if (inst->hasWinlose() || inst->stopAsyncMeleeCombatFlag)
            {
                break;
            }
        }

        /// 
        {
            inst->clearLastCombatResult();

            inst->lastCombatResult.winlose = inst->winlose;
            inst->lastCombatResult.totaltick = inst->currentTick;

            CharacterDataListPtr team = inst->getClusterOrigin(TeamSide::kOur, 1);
            for (auto iter = team->begin(); iter != team->end(); ++iter)
            {
                CharacterData *c = *iter;
                if (!c->isCannon() && !c->isSummon())
                {
                    MeleeCombatCharacterSimpleInfo c_simpinfo;
                    c_simpinfo.character_myid = c->myID;
                    c_simpinfo.character_dataid = c->getID();
                    c_simpinfo.hp = c->getHP();
                    c_simpinfo.mp = c->getEnergy();
                    c_simpinfo.init_x = c->getScriptOriginX();
                    c_simpinfo.init_y = c->getScriptOriginY();
                    c_simpinfo.action = c->currentAction;

                    inst->lastCombatResult.ourinfo.push_back(c_simpinfo);
                }
            }

            team = OnePiece::AIContext::getInstance()->getClusterOrigin(OnePiece::TeamSide::kEnemy, 1);
            for (auto iter = team->begin(); iter != team->end(); ++iter)
            {
                CharacterData *c = *iter;
                if (!c->isCannon() && !c->isSummon())
                {
                    MeleeCombatCharacterSimpleInfo c_simpinfo;
                    c_simpinfo.character_myid = c->myID;
                    c_simpinfo.character_dataid = c->getID();
                    c_simpinfo.hp = c->getHP();
                    c_simpinfo.mp = c->getEnergy();
                    c_simpinfo.init_x = c->getScriptOriginX();
                    c_simpinfo.init_y = c->getScriptOriginY();
                    c_simpinfo.action = c->currentAction;

                    inst->lastCombatResult.enemyinfo.push_back(c_simpinfo);
                }
            }
        }

        inst->asyncMeleeCombatCnt--;
    }

    int AIContext::calculateBattleFieldCenterX()
    {
        int X1 = -100000, X2 = 100000;
        short S1 = -1, S2 = -1;

        CharacterDataListPtr currentOurCluster = getClusterOrigin(TeamSide::kOur, current_ClusterNumOur); 
        CharacterDataListPtr rivals = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);
        if (rivals != nullptr)
        {
            CharacterDataList::iterator iter;
            for (iter = currentOurCluster->begin(); iter != currentOurCluster->end(); ++iter)
            {
                auto aCharacter = *iter;
                if (aCharacter->isSummon())
                    continue;
                if (!HeroActionController::isDeadAction(aCharacter->currentAction) && X1<aCharacter->getX())
                {
                    X1 = aCharacter->getX();
                    if (S1 == -1){
                        S1 = aCharacter->getSpeed();
                    }
                }
            }
            for (iter = rivals->begin(); iter != rivals->end(); ++iter)
            {
                auto aCharacter = *iter;
                if (aCharacter->isSummon())
                    continue;
                if (!HeroActionController::isDeadAction(aCharacter->currentAction) && X2>aCharacter->getX())
                {
                    X2 = aCharacter->getX();
                    if (S2 == -1){
                        S2 = aCharacter->getSpeed();
                    }
                }
            }

            // Nov/17/2015, Huang Qing
            // 战场中心不再-20（不知道原来为什么要-20）
            if (S1 == 0)
                return X1 + 200/* - 20*/;
            else if (S2 == 0)
                return X2 - 200/* - 20*/;
            else
                return (X1 + X2) / 2/* - 20*/;
        }
        return -1;
    }

    int AIContext::getAdjustedPosXWithinRange(int originPosX)
    {
        if (battleFieldCenterX > 0 && clusterRangeToCenterX > 0)
        {
            int rangeMaxX = battleFieldCenterX + clusterRangeToCenterX;
            if (originPosX > rangeMaxX)
                originPosX = rangeMaxX;

            int rangeMinX = battleFieldCenterX - clusterRangeToCenterX;
            if (originPosX < rangeMinX)
                originPosX = rangeMinX;
        }
        return originPosX;
    }
    
    int AIContext::getAdjustedPosYWithinRange(int originPosY)
    {
        if (originPosY > getDeltaMaxY())
            originPosY = getDeltaMaxY();
        if (originPosY < getDeltaMinY())
            originPosY = getDeltaMinY();
        return originPosY;
    }

    int AIContext::getBattleFieldXMostLeft()
    {
        return battleFieldCenterX - clusterRangeToCenterX;
    }

    int AIContext::getBattleFieldXMostRight()
    {
        return battleFieldCenterX + clusterRangeToCenterX;
    }

    CharacterDataListPtr AIContext::getClusterOrigin(TeamSide teamSide, int clusterNum)
    {
        // TODO: optimise
        Clusters* team = nullptr;
        if (TeamSide::kOur == teamSide)
            team = &clustersOur;
        else if (TeamSide::kEnemy == teamSide)
            team = &clustersEnemy;

        if (team)
        {
            if ((*team)[clusterNum] == nullptr)
                (*team)[clusterNum] = new CharacterDataList();
            return (*team)[clusterNum];
        }
        
        return nullptr;
    }

    //CharacterDataListPtr AIContext::getClusterReal(TeamSide teamSide, int clusterNum)
    //{
    //    Clusters* team = nullptr;
    //    if (TeamSide::kOur == teamSide)
    //        team = &clustersRealOur;
    //    else if (TeamSide::kEnemy == teamSide)
    //        team = &clustersRealEnemy;

    //    if (team)
    //    {
    //        if ((*team)[clusterNum] == nullptr)
    //            (*team)[clusterNum] = new CharacterDataList();
    //        return (*team)[clusterNum];
    //    }

    //    return nullptr;
    //}

    CharacterDataListPtr AIContext::getFriendsForCluster(TeamSide side, int clusterNum, bool excludeSummon)
    {
        auto& container = _allFriendsRivals[side][clusterNum];
        auto& dataList = excludeSummon ? std::get<kTupleIndexFriendsNoSummon>(container) : std::get<kTupleIndexFriends>(container);
        return &dataList;
    }

    CharacterDataListPtr AIContext::getRivalsForCluster(TeamSide side, int clusterNum, bool excludeSummon)
    {
        auto& container = _allFriendsRivals[side][clusterNum];
        auto& dataList = excludeSummon ? std::get<kTupleIndexRivalsNoSummon>(container) : std::get<kTupleIndexRivals>(container);
        return &dataList;
    }

    void AIContext::initRivalsRealForCluster(TeamSide side, int clusterNum, CharacterDataListPtr pOriginRivals)
    {
        auto pRivalsReal = getRivalsForCluster(side, clusterNum, false);
        pRivalsReal->clear();
        auto pRivalsRealNosummon = getRivalsForCluster(side, clusterNum, true);
        pRivalsRealNosummon->clear();
        if (!pOriginRivals)
            return;
        for (auto iter = pOriginRivals->begin(); iter != pOriginRivals->end(); ++iter)
        {
            pRivalsReal->push_back(*iter);
            if ((*iter)->isSummon())
                continue;
            pRivalsRealNosummon->push_back(*iter);
        }
    }

    void AIContext::updateCluster(CharacterDataListPtr pCharacterList)
    {
        if (!pCharacterList)
            return;
        int deadCnt = 0;
        int charmCnt = 0;
        for (auto iter = pCharacterList->begin(); iter != pCharacterList->end(); ++iter)
        {
            CharacterData* character = (*iter);
            character->update(*this);
            if (HeroActionController::isDeadAction(character->currentAction))
            {
                deadCnt++;
                continue;
            }

            if (!monsterCanMove && adjustingDistance <= 0)
            {
                monsterCanMove = true;
                if (fixCamera == 0)
                {
                    fixCamera = 1;
                    if (AIDataHelper::getAiCenterXMode() == AICENTERX_USE_INPUT)
                    {
                        if (battleFieldCenterX == 0)
                        {
                            int theXFromConfig = AIDataHelper::getAiCenterXForCluster(_toNextClusterCnt);
                            if (theXFromConfig)
                            {
                                battleFieldCenterX = theXFromConfig;
                            }
                            else
                            {
                                battleFieldCenterX = calculateBattleFieldCenterX();
                            }
                        }
                    }
                    else
                    {
                        battleFieldCenterX = calculateBattleFieldCenterX();
                    }
                    int captainY = (deltaMaxY + deltaMinY) / 2;
                    if (captainOur)
                    {
                        captainOur->setX(battleFieldCenterX);
                        captainOur->setY(captainY);
                    }
                    if (captainEnemy)
                    {
                        captainEnemy->setX(battleFieldCenterX);
                        captainEnemy->setY(captainY);
                    }

                    if (aiCallback)
                        aiCallback->fixCamera(battleFieldCenterX - 20, 0, 0); // 战场中心修改，不再-20，移动镜头补上以保持原来效果
                }
            }

            if (character->isInCharmEffect())
                charmCnt++;
        }

        // 场上一方队伍只剩被魅惑的角色时,立即恢复,不用等魅惑效果消失才能打
        if (charmCnt > 0 && (deadCnt + charmCnt) == pCharacterList->size())
        {
            for (auto character : *pCharacterList)
            {
                if (character->isInCharmEffect())
                    AIContinuousEffectCaster::zero_Special_Effect(*this, *character, eEFFECT_CHARM);
            }
        }
    }

    void AIContext::releaseSkillOnBegin(CharacterDataListPtr pCharacterList)
    {
        for (auto iter = pCharacterList->begin(); iter != pCharacterList->end(); ++iter) {
            CharacterData *character = *iter;
            character->setTagValue("skonbegin", 1);
            HeroActionController::actionSelector(*character, *this);
            for (auto skilliter = character->skillList.begin(); skilliter != character->skillList.end(); ++skilliter)
            {
                auto skillID = skilliter->first;
                auto skillLevel = skilliter->second;
                AISkill* aiSkill = AIDataHelper::getSkillData(skillID);
                if (aiSkill && aiSkill->castCondition == ESkillCastCondition::eSkillCastConditionOnFightBegin && !HeroActionController::isDeadAction(character->currentAction))
                    HeroSkillController::executeExecIDInTick(*this, skillID, skillLevel, 0, *character, *character->target);
            }
            character->eraseTag("skonbegin");
        }
    }
    
    void AIContext::releaseSkillOnBeginByProtectiveProperty(CharacterDataListPtr pCharacterList, bool isProtective, int lazyTick)
    {
        for (auto iter = pCharacterList->begin(); iter != pCharacterList->end(); ++iter) {
            CharacterData *character = *iter;
            character->setTagValue("skonbegin", isProtective ? 1 : 2);
            HeroActionController::actionSelector(*character, *this);
            for (auto skilliter = character->skillList.begin(); skilliter != character->skillList.end(); ++skilliter)
            {
                auto skillID = skilliter->first;
                auto skillLevel = skilliter->second;
                AISkill* aiSkill = AIDataHelper::getSkillData(skillID);
                if (aiSkill && aiSkill->castCondition == ESkillCastCondition::eSkillCastConditionOnFightBegin && !HeroActionController::isDeadAction(character->currentAction))
                {
                    HeroSkillController::executeBeginSkillExecIDInTick(*this, skillID, skillLevel, 0, character, character->target, aiSkill, isProtective, lazyTick);
                }
            }
            character->eraseTag("skonbegin");
        }
    }

    bool AIContext::isCurrentLastClusterForTeam(TeamSide side)
    {
        if (TeamSide::kOur == side)
            return (current_ClusterNumOur == clustersOur.size());
        else if (TeamSide::kEnemy == side)
            return (current_ClusterNumEnemy == clustersEnemy.size());
        return false;
    }

    //void AIContext::removeRivals(int clusterNum)
    //{
    //    // TODO: remove
    //    CharacterDataListPtr currentRivals = getRivalCluster(clusterNum);
    //    CharacterDataListPtr tmpCurrentRivals = getTmpRivalCluster(clusterNum);

    //    if (currentRivals != nullptr)
    //    {
    //        int *rivalIDs = new int[currentRivals->size()];
    //        int rivalCount = 0;
    //        for (CharacterDataList::iterator iter = currentRivals->begin(); iter != currentRivals->end(); ++iter)
    //        {
    //            CharacterData* character = (*iter);
    //            rivalIDs[rivalCount++] = character->myID;
    //        }

    //        currentRivals->clear();
    //        tmpCurrentRivals->clear();

    //        aiCallback->removeCharacters(rivalIDs);
    //        delete[] rivalIDs;
    //    }
    //}

    void AIContext::toNextCluster(int adjust, int clusterCenterX /*= 0*/)
    {
        if (!_isLuaReady)
        {
            if (initLua() == 0)
            {
                _isLuaReady = true;
            }
        }
        
        if (BusinessType::CANNON_KING == businessType && _pCannonKing)
        {
            if (_pCannonKing->spawnTickNormal <= 0)
                _pCannonKing->spawnTickNormal = 100;
            if (_pCannonKing->spawnTickLong <= 0)
                _pCannonKing->spawnTickLong = 120;
            if (_pCannonKing->cannonRemainCount <= 0)
                _pCannonKing->cannonRemainCount = 3;
            if (_pCannonKing->bossClusterNumInterval <= 0)
                _pCannonKing->bossClusterNumInterval = 5;
        }

		assert(businessType>0);
		//assert(current_ClusterNumEnemy<6);
        //trackSetDistance[currentWaveCount] = adjust;//distance for each wave
        trackSetDistance.push_back(adjust);
		adjustingDistance = adjust;
        currentWaveCount++;
        pauseAIFalg = false;
        _hasWinlose = true;

        trackCenterX.push_back(clusterCenterX);
        _inputBattleFieldCenterX = clusterCenterX;
        _toNextClusterCnt += 1;

        if (AIDataHelper::getAiCenterXMode() == AICENTERX_USE_INPUT)
        {
            battleFieldCenterX = _inputBattleFieldCenterX;
        }
        else
        {
            battleFieldCenterX = 0;
        }
        

        clusterRangeToCenterX = AIDataHelper::getBattleFieldRangeToCenterX();

        int lastWinlose = winlose;
        int lastClusterNumOur = current_ClusterNumOur;
        int lastClusterNumEnemy = current_ClusterNumEnemy;

        if (AIDataHelper::getAiUpdateModeAfterWinlose() == AIUPDATEMODE_STOP_FIGHT_AFTER_WINLOSE)
            afterwarCountdown = -1;
        else
		    afterwarCountdown = 15;
		monsterCanMove = false;
		winlose = 0;
		fixCamera = 0;
		isFighting = false;
        //battleFieldCenterX = 0;
		adjustTick = 0;
		friend_num[0] = 0, friend_num[1] = 0, friend_num[2] = 0;
		rival_num[0] = 0, rival_num[1] = 0, rival_num[2] = 0;
		clearWave_Countdown = 0;
		comboConstraints.clear();

        _ignoreNoDieCharacters.clear();
        _hploseListeners.clear();

        _appendedExcuteExecs.clear();
        _scheduleExcuteExecs.clear();

        if (WinLoseType::kWin != lastWinlose)
            current_ClusterNumOur++;

        if (WinLoseType::kLose != lastWinlose)
            current_ClusterNumEnemy++;

        // MARK: reuse enemy
        if (this->nextClusterNumEnemy > 0)
            current_ClusterNumEnemy = this->nextClusterNumEnemy;

        if (BusinessType::CANNON_KING == businessType && _pCannonKing)
        {
            int bossClusterNum = _pCannonKing->bossClusterNum;
            do {
                if (_pCannonKing->normalWaves.count(bossClusterNum) > 0)
                    _pCannonKing->normalWaves.erase(_pCannonKing->normalWaves.find(bossClusterNum));
                auto iter = _pCannonKing->availableWaves.find(bossClusterNum);
                if (iter != _pCannonKing->availableWaves.end())
                    _pCannonKing->availableWaves.erase(iter);
                _pCannonKing->bossWaves.insert(bossClusterNum);

                bossClusterNum--;
            } while (bossClusterNum > 0 && bossClusterNum > _pCannonKing->bossClusterNum - 3);

            current_ClusterNumEnemy = 0;
            _pCannonKing->nextSpawnTick = _pCannonKing->spawnTickNormal;

            if (0 == currentTick)
                spawnCannonKingEnemy();
        }

        if (_isLuaReady)
            luahelper_aicontext_on_to_nextcluster_init_cluster(getLuaState(), current_ClusterNumOur, current_ClusterNumEnemy);

        CharacterDataListPtr currentOurCluster = getClusterOrigin(TeamSide::kOur, current_ClusterNumOur);
        CharacterDataListPtr newRivals = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);

        initRivalsRealForCluster(TeamSide::kOur, current_ClusterNumOur, newRivals);
        initRivalsRealForCluster(TeamSide::kEnemy, current_ClusterNumEnemy, currentOurCluster);

        CharacterDataListPtr friendsRealOur = getFriendsForCluster(TeamSide::kOur, current_ClusterNumOur);
        CharacterDataListPtr friendsRealTheir = getFriendsForCluster(TeamSide::kEnemy, current_ClusterNumEnemy);

        CharacterDataListPtr rivalsRealOur = getRivalsForCluster(TeamSide::kOur, current_ClusterNumOur);
        CharacterDataListPtr rivalsRealTheir = getRivalsForCluster(TeamSide::kEnemy, current_ClusterNumEnemy);

		if (newRivals!=nullptr)
		{
            AIWinLoseCheck::getInstance()->resetCluster(winlosecheck_clusters[current_ClusterNumEnemy]);

			//if (guanqiaID > 0)
			//{
			//	pve_guanqia_character* guanqiaCharacter = AIDataHelper::get_pve_guanqia_character(guanqiaID, current_ClusterNumEnemy);
			//}

            if (cannon)
            {
                cannon->friends = currentOurCluster;
                cannon->m_pTmpFriends = friendsRealOur;
                cannon->rivals = newRivals;
                cannon->m_pTmpRivals = rivalsRealOur;
            }

            if (captainOur)
            {
                captainOur->friends = currentOurCluster;
                captainOur->m_pTmpFriends = friendsRealOur;
                captainOur->rivals = newRivals;
                captainOur->m_pTmpRivals = rivalsRealOur;
            }

            if (captainEnemy)
            {
                captainEnemy->friends = newRivals;
                captainEnemy->m_pTmpFriends = friendsRealTheir;
                captainEnemy->rivals = currentOurCluster;
                captainEnemy->m_pTmpRivals = rivalsRealTheir;
            }

			CharacterDataList::iterator iter;
            for (iter = currentOurCluster->begin(); iter != currentOurCluster->end(); ++iter)
			{
                auto aCharacter = *iter;
                currentFriendK = aCharacter->k;
                aCharacter->targetStuckBackward = false;
                aCharacter->oldY = aCharacter->getY();
                aCharacter->friends = currentOurCluster;
                aCharacter->m_pTmpFriends = friendsRealOur;
                aCharacter->rivals = newRivals;
                aCharacter->m_pTmpRivals = rivalsRealOur;
				
                HeroActionController::cleanup(*aCharacter, *this, true);
                aCharacter->resetEffectExec();
                aCharacter->removeMarks();
                aCharacter->resetClusterSpecialSkillCondition();
                aCharacter->resetTraversalTargetRecord();
                aCharacter->resetFlags();
                if (HeroActionController::isDeadAction(aCharacter->currentAction))
				{
					continue;
				}

                friend_num[aCharacter->staticData->confg.line]++;
                HeroIdleAction::Enter(*aCharacter, *this, 1, IDLE_ACTION, *aCharacter);
                aCharacter->target = nullptr;
                aCharacter->m_pTmpTarget = nullptr;
                aCharacter->tauntTarget = nullptr;
                aCharacter->attacker = nullptr;
                aCharacter->mAttackConfirmedTarget = false;
                aCharacter->specialFoundSkillTarget = nullptr;
                aCharacter->setZ(aCharacter->getY());
                aCharacter->adjustingX = 100000000;
                memset(aCharacter->actionAtt, 0, sizeof(aCharacter->actionAtt));

                if (isBusinessTowerDefence(businessType) || BusinessType::WHEELWAR_SOLO == businessType || BusinessType::CANNON_KING == businessType)
                    aCharacter->resetPos();

                HeroActionController::setSkillActionCD(aCharacter->actionAtt, 0);
                HeroActionController::setSkillSeq(aCharacter->actionAtt, -1);
                HeroActionController::setSkillID(aCharacter->actionAtt, HeroSkillController::calculateSkillID(*this, *aCharacter, 0));

                preAddTransformCharacter(aCharacter);
			}
			for (iter = newRivals->begin(); iter != newRivals->end(); ++iter)
			{
                auto aCharacter = *iter;
                currentRivalK = aCharacter->k;
                aCharacter->targetStuckBackward = false;
                aCharacter->oldY = aCharacter->getY();
                aCharacter->friends = newRivals;
                aCharacter->m_pTmpFriends = friendsRealTheir;
                aCharacter->rivals = currentOurCluster;
                aCharacter->m_pTmpRivals = rivalsRealTheir;

                HeroActionController::cleanup(*aCharacter, *this, true);
                aCharacter->resetEffectExec();
                aCharacter->removeMarks();
                aCharacter->resetClusterSpecialSkillCondition();
                aCharacter->resetTraversalTargetRecord();
                aCharacter->resetFlags();
                if (HeroActionController::isDeadAction(aCharacter->currentAction))
				{
                    if (isBusinessTowerDefence(businessType))
                    {
                        aCharacter->updateHP(aCharacter->getHPTop());
                        aCharacter->updateEnergy(0);

                        if (animationProxy)
                            animationProxy->resyncCharacter(*this, aCharacter);
                    }
                    else
                    {
                        continue;
                    }
				}
                rival_num[aCharacter->staticData->confg.line]++;
                HeroIdleAction::Enter(*aCharacter, *this, 1, IDLE_ACTION, *aCharacter);
                aCharacter->target = nullptr;
                aCharacter->m_pTmpTarget = nullptr;
                aCharacter->tauntTarget = nullptr;
                aCharacter->attacker = nullptr;
                aCharacter->mAttackConfirmedTarget = false;
                aCharacter->specialFoundSkillTarget = nullptr;
                aCharacter->setZ(aCharacter->getY());
                aCharacter->adjustingX = 100000000;
                memset(aCharacter->actionAtt, 0, sizeof(aCharacter->actionAtt));

                if (isBusinessTowerDefence(businessType) || BusinessType::WHEELWAR_SOLO == businessType || BusinessType::CANNON_KING == businessType)
                    aCharacter->resetPos();

                if (clusterAttrEnhanceInfos)
                {
                    enhanceHero(aCharacter);
                    if (animationProxy)
                        animationProxy->resyncCharacter(*this, aCharacter);
                }

                HeroActionController::setSkillActionCD(aCharacter->actionAtt, 0);
                HeroActionController::setSkillSeq(aCharacter->actionAtt, -1);
                HeroActionController::setSkillID(aCharacter->actionAtt, HeroSkillController::calculateSkillID(*this, *aCharacter, 0));

                preAddTransformCharacter(aCharacter);
			}

            if (clusterAttrInheritInfos)
            {
                inheritAttributes(TeamSide::kOur, lastClusterNumOur, current_ClusterNumOur);
                inheritAttributes(TeamSide::kEnemy, lastClusterNumEnemy, current_ClusterNumEnemy);
            }
			
            //if (isBusinessPVE(businessType) && businessType != BusinessType::PVE_ISLAND_CRUISE && current_ClusterNumEnemy>1)
            //    restoreEnergyAndHP(currentOurCluster);

            switch (businessType)
            {
            case BusinessType::PVE_GUANQIA:
            case BusinessType::PVE_CALENDAR:
            case BusinessType::TOWER_DEFENCE_TIME:
            case BusinessType::TOWER_DEFENCE_WAVE:
                if (current_ClusterNumEnemy > 1)
                    restoreEnergyAndHP(currentOurCluster);
                break;

            case BusinessType::WHEELWAR_SOLO:
            case BusinessType::WHEELWAR_GROUP:
                if (lastClusterNumOur == current_ClusterNumOur)
                    restoreEnergyAndHP(currentOurCluster);
                if (lastClusterNumEnemy == current_ClusterNumEnemy)
                    restoreEnergyAndHP(newRivals);
                break;

            default:
                break;
            }

            _hasWinlose = false;

			if (aiCallback)
                aiCallback->startSchedule();
			
            // our k 1
            // new rival k 1
            // our k 0
            // new rival k 0
            auto firstUpdateCluster = currentOurCluster;
            auto secondUpdateCluster = newRivals;
            auto firstUpdateCaptain = captainOur; // OP2
            auto secondUpdateCaptain = captainEnemy;
            if (currentFriendK == 0 && currentRivalK == 1) {
                firstUpdateCluster = newRivals;
                secondUpdateCluster = currentOurCluster;
                firstUpdateCaptain = captainEnemy;
                secondUpdateCaptain = captainOur;
            }

            //releaseSkillOnBegin(firstUpdateCluster);
            //releaseSkillOnBegin(secondUpdateCluster);

            // OP2
            if (firstUpdateCaptain)
                HeroActionController::releaseSkillOnBegin(*this, firstUpdateCaptain, true, 0);
            if (secondUpdateCaptain)
                HeroActionController::releaseSkillOnBegin(*this, secondUpdateCaptain, true, 0);
            
            // 12.0
            //releaseSkillOnBeginByProtectiveProperty(firstUpdateCluster, true, 0);
            //releaseSkillOnBeginByProtectiveProperty(secondUpdateCluster, true, 0);

            HeroActionController::teamReleaseSkillOnBegin(*this, firstUpdateCluster, true, 0);
            HeroActionController::teamReleaseSkillOnBegin(*this, secondUpdateCluster, true, 0);
            
            if (_isLuaReady)
                _beginSkillLazyTick = AIScriptBinder::getInstance()->getCommonConfigInteger(getLuaState(), "beginskill_lazytick");

            if (_beginSkillLazyTick == 0)
            {
                //releaseSkillOnBeginByProtectiveProperty(firstUpdateCluster, false, _beginSkillLazyTick);
                //releaseSkillOnBeginByProtectiveProperty(secondUpdateCluster, false, _beginSkillLazyTick);

                if (firstUpdateCaptain)
                    HeroActionController::releaseSkillOnBegin(*this, firstUpdateCaptain, false, 0);
                if (secondUpdateCaptain)
                    HeroActionController::releaseSkillOnBegin(*this, secondUpdateCaptain, false, 0);

                HeroActionController::teamReleaseSkillOnBegin(*this, firstUpdateCluster, false, 0);
                HeroActionController::teamReleaseSkillOnBegin(*this, secondUpdateCluster, false, 0);
            }
            else
            {
                _beginSkillToReleaseTick = currentTick + _beginSkillLazyTick;
            }

            /**
            auto calcTeamHPTopFunc = [](CharacterDataListPtr pCharacterList)->int{
                int totalhptop = 0;
                for (auto iter = pCharacterList->begin(); iter != pCharacterList->end(); ++iter) {
                    auto character = *iter;
                    if (character->isSummon())
                        continue;
                    totalhptop += character->getHPTop();
                }
                return totalhptop;
            };
            currentClusterHPTopOur = calcTeamHPTopFunc(currentOurCluster);
            currentClusterHPTopEnemy = calcTeamHPTopFunc(newRivals);
            */

            currentClusterHPTopOur = CharacterFunc::CalcTeamHPTopFunc(currentOurCluster);
            currentClusterHPTopEnemy = CharacterFunc::CalcTeamHPTopFunc(newRivals);
		}

        if (_isLuaReady)
        {
            luahelper_aicontext_to_nextcluster_end(getLuaState(), adjust, clusterCenterX);
        }
	}

	void AIContext::setSeed(int seed, int clusterNum)
	{
		if (localRandom != nullptr)
		{
			delete localRandom;
			localRandom = nullptr;
		}
		localRandom = new RandomGen(seed);
	}
    
    void AIContext::setRewardSeed(int seed)
    {
        if (rewardRandom)
        {
            delete rewardRandom;
            rewardRandom = nullptr;
        }
        rewardRandom = new RandomGen(seed);
    }

	void AIContext::setRewardCandidate(std::vector<RewardCandidateItem*>* rclist, char* thekey, int clusterNum)
	{
		/*
		std::map<char*, std::vector<RewardCandidateItem*>*>::iterator iter2 = reward_candidates.find(thekey);
		if( iter2 != reward_candidates.end())
		{
			for (std::vector<RewardCandidateItem*>::iterator iter3 = iter2->second->begin(); iter3 != iter2->second->end();)
			{
				delete *(iter3++);
			}
			delete rclist;
			return;
		}
		*/

		char* k = (char*)malloc(strlen(thekey) + 1);
		strcpy(k, thekey);
		reward_candidates[k] = rclist;

        // 12.0: 预先随机掉落结果
        // 修改时，需告知逻辑服务器，同步修改
		if (rclist) {
			for (auto a_reward : *rclist) {
                if (!a_reward)
                    continue;
				OutPutFileLine;
				if (rewardRandom->nextInt(0, 100) < a_reward->r) {
                    a_reward->isInReward = true;
					int rate = rewardRandom->nextInt(0, 100);
					int increasedRate = 0;
					if (a_reward == nullptr)
                        continue;
					for (auto iar : a_reward->iarlist) {
						increasedRate += iar->r;
						if (rate < increasedRate) {
                            iar->isInReward = true;
                            break;
                        }
					}
				}
			}
		}
	}

	void AIContext::setWinLoseCheck(float *conditionList, int clusterNum)
	{
		winlosecheck_clusters[clusterNum] = conditionList;
	}

	void AIContext::setCallback(AICallback &aiCallback)
	{
		if (this->aiCallback)
			delete this->aiCallback;

		this->aiCallback = &aiCallback;
	}

	void AIContext::playSpeed(int multiple)
	{
		myPlaySpeed = multiple;
		//aiCallback->playSpeed(multiple);
	}

	int AIContext::playSpeed()
	{
		return myPlaySpeed;
	}

	void AIContext::setLength(long length)
	{
		this->length = length;
	}

	void AIContext::setGuanqiaID(int guanqiaID)
	{
		this->guanqiaID = guanqiaID;
	}

	void AIContext::setDeltaY(long minY ,long maxY)
	{
		this->deltaMaxY = maxY;
        this->deltaMinY = minY;
	}
    
    long AIContext::getDeltaMinY(){
        return this->deltaMinY;
    }
    
    long AIContext::getDeltaMaxY(){
        return this->deltaMaxY;
    }

	void AIContext::restoreEnergyAndHP(CharacterDataListPtr heros)
	{
		if (heros == nullptr) return;

		int energy, hp;
		for (CharacterDataList::iterator iter = heros->begin(); iter != heros->end(); iter++)
		{
			CharacterData* character = (*iter);
            if (!HeroActionController::isDeadAction(character->currentAction))
			{
				if (character->getENAdd1()>0)
				{
                    int olden = character->getEnergy();
                    CharacterFunc::ApplyEnergyDelta(character, character->getENAdd1());
                    energy = character->getEnergy();
					if (energy != olden)
					{
                        int endelta = energy - olden;
						AISpecialEffectLabel::excute(100012, endelta, *this, character);
					}
				}

				if (character->getHPAdd2() > 0)
				{
					hp = character->getHP() + character->getHPAdd2();
					if (hp > character->getHPTop()) hp = character->getHPTop();
					if (hp != character->getHP())
					{
						AISpecialEffectLabel::excute(100011, character->getHPAdd2(), *this, character);
						character->updateHP(hp);
					}
				}
			}
		}
	}

    void AIContext::inheritAttributes(TeamSide side, int lastClusterNum, int currentClusterNum)
    {
        if (!clusterAttrInheritInfos)
            return;

        if (lastClusterNum == currentClusterNum || lastClusterNum <= 0 || currentClusterNum <= 0)
            return;

        for (auto iter = clusterAttrInheritInfos->begin(); iter != clusterAttrInheritInfos->end(); ++iter)
        {
            const auto& info = *iter;
            if (!info || !info->attribute)
                continue;
            
            auto lastCluster = getClusterOrigin(side, lastClusterNum);
            auto currentCluster = getClusterOrigin(side, currentClusterNum);

            if (!lastCluster || !lastCluster->size() || !currentCluster || !currentCluster->size())
                continue;

            auto lastHero = lastCluster->front();
            auto currentHero = currentCluster->front();

            if (lastHero && currentHero)
            {
                currentHero->attributes[info->attribute] += (float)lastHero->attributes[info->attribute] / 10000.0f * (float)info->ratio;

                int limitValue = 0;
                if (ECharacterAttr::ENG == info->attribute)
                    limitValue = AIDataHelper::getEnergyRecover()[0] * info->limit / 10000;
                else if (ECharacterAttr::HP == info->attribute)
                    limitValue = (float)currentHero->getHPTop() * (float)info->limit / 10000.0f;

                if (limitValue && currentHero->attributes[info->attribute] > limitValue)
                    currentHero->attributes[info->attribute] = limitValue;
            }
        }
    }

    void AIContext::enhanceHero(CharacterData* hero)
    {
        if (!clusterAttrEnhanceInfos)
            return;

        for (auto iter = clusterAttrEnhanceInfos->begin(); iter != clusterAttrEnhanceInfos->end(); ++iter)
        {
            if (BusinessType::CANNON_KING == businessType)
                hero->enhanceAttribute((*iter)->attribute, (*iter)->scaleRatio * (passedClusterCount + 1), true);
            else
                hero->enhanceAttribute((*iter)->attribute, (*iter)->scaleRatio);
        }
    }

    bool AIContext::isBattleFinished(int wl)
    {
        if (BusinessType::CANNON_KING == businessType)
            return true;

        if (wl != WinLoseType::kWin && current_ClusterNumOur == clustersOur.size())
            return true;

        if (BusinessType::TOWER_DEFENCE_WAVE == businessType)
        {
            float* conditions = winlosecheck_clusters[current_ClusterNumEnemy];
            if (!conditions)
                conditions = winlosecheck_clusters[1];
            if (conditions && conditions[1] > 0)
            {
                int totalWaves = conditions[1];
                return (passedClusterCount >= totalWaves);
            }
        }
        else if (BusinessType::TOWER_DEFENCE_TIME == businessType)
        {
            return (wl == WinLoseType::kWin && AIWinLoseCheck::getInstance()->getTimeElapseConditon() < AIWinLoseCheck::getInstance()->getBattleTimeElapse());
        }

        return ((wl == WinLoseType::kWin || wl == WinLoseType::kDeuce) && current_ClusterNumEnemy == clustersEnemy.size());
    }

	void AIContext::updateAIContext(float dt)
	{
	//	CCLOG("SHANBEILENG-updateAIContext: %d-%.2f", currentTick, dt);
    /*   if (audioProxy != nullptr) // cui_2023_3_14  服务端不做处理
        {
            if (realStopConut > 0) {
                realStopConut--;
                if (realStopConut == 0) {
                    AIActionController::executeExec(*this, 88100, 0, 1, nullptr, nullptr, nullptr, 0);
                }
                return;
            }
        }
       */
		if (currentTick == 0) {
            //设置元素之力标签cui_2023_7_3
            setChapterElementAtkLabel();
			if (input_trackAutofight != nullptr && input_trackAutofight->find(currentTick) != input_trackAutofight->end())
			{
				setAutoFight(input_trackAutofight->find(currentTick)->second);
			}
		}

		adjustingDistance -= 15;
		++currentTick;
		if (adjustTick > 0)
            adjustTick--;

		char buff[10];
		
		//WIN LOSE countdown
		if (clearWave_Countdown != 0)
		{
            if (clearWave_Countdown == 5) //time to call fightingComplete with WIN
            {
				if (aiCallback)
				{
					aiCallback->stopSchedule(0,0,0,0);

					std::vector<std::pair<int, int>>* rewardArray = nullptr;
                    if (winlose == WinLoseType::kWin && guanqiaID > 0)
					{
						sprintf(buff, "%d", current_ClusterNumEnemy);
                        auto iter = reward_candidates.find(buff);
						std::vector<RewardCandidateItem*>* rewards = nullptr;// reward_candidates.find(buff)->second;
						if (iter != reward_candidates.end())
							rewards = iter->second;
						if (rewards != nullptr)
						{
							rewardArray = getPveGuanqiaReward(rewards);
						}
					}

                    int callbackWinlose = winlose;
                    if (WinLoseType::kDeuce == winlose)
                        callbackWinlose = WinLoseType::kOnGoing;
                    aiCallback->fightingComplete(callbackWinlose, rewardArray);
					if (rewardArray != nullptr) delete rewardArray;

                    if (WinLoseType::kLose != winlose)
                        passedClusterCount++;

					CCLOG("SHANBEILENG-hahaha:%d-%d", current_ClusterNumEnemy, clustersEnemy.size());
                    //if ((winlose != WinLoseType::kWin && current_ClusterNumOur == clustersOur.size()) || 
                    //    (winlose == WinLoseType::kWin && current_ClusterNumEnemy == clustersEnemy.size()))
                    if (isBattleFinished(winlose))
					{
                        CCLOG("DEBUG-BATTLE:battle finished,winlose(%d)", winlose);
                        if (transformProxy)
                            transformProxy->clear();
						//trackCannon
						aiCallback->operationTrack(trackCannons, trackUltra, trackSetDistance, trackAutofight, trackAccumulate, trackCenterX);
						//CCLOG("SHANBEILENG-trackCannon:%d-%d-%d-%d-%d-%d-%d-%d-%d", trackCannon[0], trackCannon[1], trackCannon[10], trackCannon[11], trackCannon[20], trackCannon[21], trackSetDistance[0], trackSetDistance[1], trackSetDistance[2]);
						//CCLOG("SHANBEILENG-trackUltra:%d", trackUltra.size());
        
						//通知消除召唤物
						if (summonProxy)
						{
							for (CharacterDataList::iterator ite = m_summonCharacterList.begin(); ite != m_summonCharacterList.end(); ++ite)
							{
								if (*ite)
									summonProxy->sleepSummon(*this, (*ite)->getSummonSource(), (*ite)->myID);
							}
						}

                        if (_isLuaReady)
                            luahelper_aicontext_on_battle_finished(getLuaState());

						return;
					}

                    serverToNext = 1;
				}
			}
			else if (clearWave_Countdown == _CELEBRATE_PERIOD) //time to call clearWave
			{
				clearWave(true);
			}
			else if (clearWave_Countdown == -1) //time to call clearWave
			{
				clearWave_Countdown = _CELEBRATE_PERIOD;
				clearWave(false);
			}

			if(--clearWave_Countdown==0) return;
		}

        /// 12.0
        if (_beginSkillLazyTick && _beginSkillToReleaseTick == currentTick)
        {
            CharacterDataListPtr left = getClusterOrigin(TeamSide::kOur, current_ClusterNumOur);
            CharacterDataListPtr right = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);
            // our k 1
            // new rival k 1
            // our k 0
            // new rival k 0
            auto firstUpdateCluster = left;
            auto secondUpdateCluster = right;
            auto firstUpdateCaptain = captainOur;
            auto secondUpdateCaptain = captainEnemy;
            if (currentFriendK == 0 && currentRivalK == 1) {
                firstUpdateCluster = right;
                secondUpdateCluster = left;
                firstUpdateCaptain = captainEnemy;
                secondUpdateCaptain = captainOur;
            }
            //releaseSkillOnBeginByProtectiveProperty(firstUpdateCluster, false, _beginSkillLazyTick);
            //releaseSkillOnBeginByProtectiveProperty(secondUpdateCluster, false, _beginSkillLazyTick);

            if (firstUpdateCaptain)
                HeroActionController::releaseSkillOnBegin(*this, firstUpdateCaptain, false, _beginSkillLazyTick);
            if (secondUpdateCaptain)
                HeroActionController::releaseSkillOnBegin(*this, secondUpdateCaptain, false, _beginSkillLazyTick);

            HeroActionController::teamReleaseSkillOnBegin(*this, firstUpdateCluster, false, _beginSkillLazyTick);
            HeroActionController::teamReleaseSkillOnBegin(*this, secondUpdateCluster, false, _beginSkillLazyTick);
        }
        ///

        implUpdate();

		//check winlose: ongoing: 0, win:1, lose:2
		if (clearWave_Countdown == 0)
		{
			winlose = AIWinLoseCheck::getInstance()->check(*this, dt);
			if (winlose)
			{
#if BATTLE_LOG_TEST
                OPBattleLog("totaltime", AIWinLoseCheck::getInstance()->getClusterTimeElapse());
#endif
                _hasWinlose = true;

                if (winlose == WinLoseType::kDeterminedByServer)
                {
                    winlose = WinLoseType::kOnGoing;
                }
                    
				//if (businessType == BusinessType::PVE_ISLAND_CRUISE && winlose == 0) winlose = 1;

				if (afterwarCountdown-- < 0)
				{
					if (aiCallback)
					{
                        //if (current_ClusterNumEnemy == clustersEnemy.size())
                        if (isBattleFinished(winlose) && WinLoseType::kWin == winlose)
							clearWave_Countdown = _CELEBRATE_PERIOD;
						else
							clearWave_Countdown = -1;

                        if (businessType == BusinessType::PVE_ISLAND_CRUISE)
                            //restoreEnergyAndHP(winlose == WinLoseType::kWin ? currentOurCluster : currentRivals);
                            restoreEnergyAndHP(winlose == WinLoseType::kWin ? getClusterOrigin(TeamSide::kOur, current_ClusterNumOur) : getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy));
					}
				}
			}
		}

        AIActionController::removeInvalidCasters(*this);

//#ifndef SERVER_VERIFYING
        if (_isLuaReady)
            luahelper_aicontext_on_update_end(getLuaState());
//#endif

		//CCLOG("SHANBEILENG-input_trackCannon_index: %d, %d, %d", currentTick, input_trackCannon_index, input_trackCannon[input_trackCannon_index]);
            
        
#ifdef SERVER_VERIFYING
        if (input_trackCannons && input_trackCannon_index < input_trackCannons->size())
        {
            auto pInputTrack = input_trackCannons->at(input_trackCannon_index);
            if (pInputTrack && pInputTrack->trackTick == currentTick)
            {
                createProjectileForWarship(
                    pInputTrack->projectileID, 
                    pInputTrack->characterid, 
                    pInputTrack->sourceX, 
                    pInputTrack->sourceY, 
                    pInputTrack->sourceZ, 
                    pInputTrack->targetX, 
                    pInputTrack->targetY, 
                    pInputTrack->targetZ, 
                    pInputTrack->skillLevel
                    );
                input_trackCannon_index++;

                if (businessType == BusinessType::CANNON_KING && aiCallback)
                    aiCallback->onCreateProjectileForWarship();
            }
        }

		if (input_trackAutofight != nullptr && input_trackAutofight->find(currentTick) != input_trackAutofight->end())
		{
			setAutoFight(input_trackAutofight->find(currentTick)->second);
		}

        CharacterDataListPtr currentOurCluster = getClusterOrigin(TeamSide::kOur, current_ClusterNumOur);
        CharacterDataListPtr currentRivals = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);

		if ((input_trackUltra != nullptr || input_trackAccumulate != nullptr) && currentRivals != nullptr)
		{
			for (CharacterDataList::iterator iter = currentRivals->begin(); iter != currentRivals->end(); iter++)
			{
				CharacterData* character = (*iter);
				if (character->k == 0) break;
				else if (character->k == 100) continue;
				sprintf(buff, "%d-%d", currentTick, character->myID);
                if (input_trackUltra && input_trackUltra->find(buff) != input_trackUltra->end())
                {
                    character->castUltraSkill();
                    input_trackUltra->erase(buff);
                }
                if (input_trackAccumulate && input_trackAccumulate->find(buff) != input_trackAccumulate->end())
                {
                    character->releaseAccumulateSkill(true);
                    input_trackAccumulate->erase(buff);
                }
			}
		}
        if ((input_trackUltra != nullptr || input_trackAccumulate != nullptr) && currentOurCluster != nullptr)
		{
            for (CharacterDataList::iterator iter = currentOurCluster->begin(); iter != currentOurCluster->end(); iter++)
			{
				CharacterData* character = (*iter);
				if (character->k == 0) break;
				else if (character->k == 100) continue;
				sprintf(buff, "%d-%d", currentTick, character->myID);
                if (input_trackUltra && input_trackUltra->find(buff) != input_trackUltra->end())
                {
                    character->castUltraSkill();
                    input_trackUltra->erase(buff);
                }
                if (input_trackAccumulate && input_trackAccumulate->find(buff) != input_trackAccumulate->end())
                {
                    character->releaseAccumulateSkill(true);
                    input_trackAccumulate->erase(buff);
                }
			}
		}
        if ((input_trackUltra != nullptr || input_trackAccumulate != nullptr) && currentRivals != nullptr)
		{
			for (CharacterDataList::iterator iter = currentRivals->begin(); iter != currentRivals->end(); iter++)
			{
				CharacterData* character = (*iter);
				if (character->k == 1) break;
				else if (character->k == 100) continue;
				sprintf(buff, "%d-%d", currentTick, character->myID);
                if (input_trackUltra && input_trackUltra->find(buff) != input_trackUltra->end())
                {
                    character->castUltraSkill();
                    input_trackUltra->erase(buff);
                }
                if (input_trackAccumulate && input_trackAccumulate->find(buff) != input_trackAccumulate->end())
                {
                    character->releaseAccumulateSkill(true);
                    input_trackAccumulate->erase(buff);
                }
			}
		}
        if ((input_trackUltra != nullptr || input_trackAccumulate != nullptr) && currentOurCluster != nullptr)
		{
            for (CharacterDataList::iterator iter = currentOurCluster->begin(); iter != currentOurCluster->end(); iter++)
			{
				CharacterData* character = (*iter);
				if (character->k == 1) break;
				else if (character->k == 100) continue;
				sprintf(buff, "%d-%d", currentTick, character->myID);
                if (input_trackUltra && input_trackUltra->find(buff) != input_trackUltra->end())
                {
                    character->castUltraSkill();
                    input_trackUltra->erase(buff);
                }
                if (input_trackAccumulate && input_trackAccumulate->find(buff) != input_trackAccumulate->end())
                {
                    character->releaseAccumulateSkill(true);
                    input_trackAccumulate->erase(buff);
                }
			}
		}

		if (serverToNext == 1) {
            if (_serverVerifyAttrInheritInfos)
                setClusterAttrInheritInfos(_serverVerifyAttrInheritInfos);

            if (_serverVerifyEnemyWaveInfos && currentWaveCount < _serverVerifyEnemyWaveInfos->size())
            {
                auto& info = _serverVerifyEnemyWaveInfos->at(currentWaveCount);
                setNextEnemyClusterNumber(info->first);
                setClusterAttrEnhanceInfos(info->second);
            }

            int adjustDistance = -1;
            if (input_trackSetDistance && currentWaveCount < input_trackSetDistance->size())
                adjustDistance = (*input_trackSetDistance)[currentWaveCount];
            int nextCenterX = 0;
            if (input_trackCenterX && currentWaveCount < input_trackCenterX->size())
                nextCenterX = (*input_trackCenterX)[currentWaveCount];
            toNextCluster(adjustDistance, nextCenterX);
			serverToNext = -1;	
		}
#endif
	}

    void AIContext::implUpdate()
    {
        //update ultraSkill countdown
        //OpCountdown, EffCountdown, myID, myCluster, startGlobalTick, freezetime
        std::list<int*>::iterator thefirst = ultraFreezeList.begin();
        if (thefirst != ultraFreezeList.end())
        {
            int theCluster = (*thefirst)[3], theID = (*thefirst)[2];
            bool todeleteFirst = false;
            for (std::list<int*>::iterator iter = ultraFreezeList.begin(); iter != ultraFreezeList.end();)
            {
                std::list<int*>::iterator iterOld = iter;
                iter++;

                if ((*iterOld)[2] == theID
                    || (theCluster == (*iterOld)[3] && (*iterOld)[4] - (*thefirst)[4]<(*iterOld)[5]))
                {
                    ///////////////////////////////cui_2023_7_28 begin///////////////////////////////////////////// 
                    //判断是否是首帧下一帧开始执行
                    if ((*iterOld)[0] == (*iterOld)[5] - 1) {
                        if ((*iterOld)[9] != 0) {
                            //执行特殊效果
                            AIActionController::executeExec(*this, (*iterOld)[9], 0, 1, getCharacterByID((*iterOld)[2]), nullptr, nullptr, 0);
                        }
                        if ((*iterOld)[7] > 0) {
                            //开始黑屏
                            AIActionController::executeExec(*this, 87772, 0, 1, getCharacterByID((*iterOld)[2]), nullptr, nullptr, 0);
                        }
                        if ((*iterOld)[8] > 0) {
                            //开始虚化
                            AIActionController::executeExec(*this, 87770, 0, 1, getCharacterByID((*iterOld)[2]), nullptr, nullptr, 0);
                        }
                    }
                    --((*iterOld)[7]);
                    --((*iterOld)[8]);

                    if ((*iterOld)[7] == 0) {
                        //关闭黑屏
                        AIActionController::executeExec(*this, 87773, 0, 1, nullptr, nullptr, nullptr, 0);
                    }
                    if ((*iterOld)[8] == 0) {
                        //关闭虚化
                        AIActionController::executeExec(*this, 87771, 0, 1, nullptr, nullptr, nullptr, 0);
                    }
                    ///////////////////////////////cui_2023_7_28 end///////////////////////////////////////////// 
                   
                    int t = (*iterOld)[1];
                    --((*iterOld)[0]);
                    if (--((*iterOld)[1]) < 0)
                    {
                        if (*iterOld == *thefirst)
                        {
                            todeleteFirst = true;
                        }
                        else
                        {
                            delete[] * iterOld;
                            ultraFreezeList.erase(iterOld);
                        }
                        AIActionController::executeExec(*this, 87773, 0, 1, nullptr, nullptr, nullptr, 0);
                        AIActionController::executeExec(*this, 87771, 0, 1, nullptr, nullptr, nullptr, 0);
                    }

                }
            }

            if (todeleteFirst)
            {
                delete[] * thefirst;
                ultraFreezeList.erase(thefirst);
            }
        }
        /*
            int deleteTick = 0;
            int execID = 0;
            int skillID = 0;
            int skillLevel = 0;
            CharacterData* source = null;
            AIProjectile* projectile = null;
            CharacterData* target = null;
            int parentTicklen = null;
            bool useLogicPos = false;
            int markID = 0;
            bool isproj12reachexec = false;
            int delayTick = 0;
        */

        //cui  update delayAIAction
        for (std::list<DelayAIActionData*>::iterator delayActionIter = dalayAIActionList.begin(); delayActionIter != dalayAIActionList.end();)
        {
            (*delayActionIter)->delayTick--;
            if ((*delayActionIter)->delayTick <= 0) {
                AIActionController::executeExec(*this, (*delayActionIter)->execID,
                    (*delayActionIter)->skillID,
                    (*delayActionIter)->skillLevel,
                    (*delayActionIter)->source,
                    (*delayActionIter)->projectile,
                    (*delayActionIter)->target,
                    (*delayActionIter)->parentTicklen,
                    (*delayActionIter)->useLogicPos,
                    (*delayActionIter)->markID,
                    (*delayActionIter)->isproj12reachexec);
                delete *delayActionIter;

                dalayAIActionList.erase(delayActionIter++);
            }
            else {
                delayActionIter++;
            }
        }


        



        //update projectile
        for (std::list<AIProjectile*>::iterator pProjectile = projectileList.begin(); pProjectile != projectileList.end();)
        {
            AIProjectile* ap = *(pProjectile++);
            ap->update(*this);
        }

        if (cannon)
            cannon->update(*this);

        if (monsterCanMove && adjustingDistance <= 0)
        {
            // 船长顺序
            // rival k 1
            // friend k 1
            // rival k 0
            // friend k 0
            auto firstCaptain = captainEnemy;
            auto secondCaptain = captainOur;
            if (captainOur && captainEnemy && captainOur->k == 1 && captainEnemy->k == 0)
            {
                firstCaptain = captainOur;
                secondCaptain = captainEnemy;
            }
            if (firstCaptain)
                firstCaptain->update(*this);
            if (secondCaptain)
                secondCaptain->update(*this);
        }

        if (_scheduleNotifyUIEffs.size()) {
            auto scheduleIter = _scheduleNotifyUIEffs.find(currentTick);
            if (scheduleIter != _scheduleNotifyUIEffs.end()) {
                if (seffProxy) {
                    for (auto parentID : scheduleIter->second) {
                        seffProxy->notify(*this, parentID);
                    }
                }
                _scheduleNotifyUIEffs.erase(scheduleIter);
            }
        }

        // MARK:更新顺序
        // rival k 1
        // friend k 1
        // rival k 0
        // friend k 0
        CharacterDataListPtr currentOurCluster = getClusterOrigin(TeamSide::kOur, current_ClusterNumOur);
        CharacterDataListPtr currentRivals = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);
        auto firstUpdateCluster = currentRivals;
        auto secondUpdateCluster = currentOurCluster;
        if (1 == currentFriendK && 0 == currentRivalK)
        {
            firstUpdateCluster = currentOurCluster;
            secondUpdateCluster = currentRivals;
        }

        updateCluster(firstUpdateCluster);
        updateCluster(secondUpdateCluster);

        for (auto &&execInfo : _appendedExcuteExecs) {
            _scheduleExcuteExecs.push_back(execInfo);
        }
        _appendedExcuteExecs.clear();

        for (auto it = _scheduleExcuteExecs.begin(); it != _scheduleExcuteExecs.end();) {
            auto &&execInfo = *it;
            if (execInfo.waitTickLen <= 0) {
                AIActionController::executeExec(*this, execInfo.execID, execInfo.skillID, execInfo.skillLv, execInfo.pSource, execInfo.pProjectile, execInfo.pTarget, execInfo.parentTickLen, execInfo.useLogicPos);
                it = _scheduleExcuteExecs.erase(it);
            }
            else {
                execInfo.waitTickLen -= 1;
                ++it;
            }
        }

        if (sceneAnims.size() > 0)
        {
            if (aiCallback)
            {
                aiCallback->sceneAnimation(sceneAnims);
            }
            sceneAnims.clear();
        }

        if (!clearWave_Countdown && BusinessType::CANNON_KING == businessType && _pCannonKing)
        {
            if (_pCannonKing->nextSpawnTick > 0 && currentTick == _pCannonKing->nextSpawnTick)
            {
                passedClusterCount++;
                spawnCannonKingEnemy();

                auto pOriginRivalsCluster = getClusterOrigin(TeamSide::kEnemy, 0);
                initRivalsRealForCluster(TeamSide::kOur, current_ClusterNumOur, pOriginRivalsCluster);

                if (aiCallback)
                    aiCallback->spawnedEnemyCluster(passedClusterCount + 1);
            }
        }
    }

	std::vector<std::pair<int, int>>* AIContext::getPveGuanqiaReward(std::vector<RewardCandidateItem*>* rewards)
	{
		std::vector<std::pair<int, int>>* rewardArray = new std::vector<std::pair<int, int>>();

		if (rewards != nullptr)
		{
			for (std::vector<RewardCandidateItem*>::iterator iter = rewards->begin(); iter != rewards->end(); ++iter)
			{
				// OutPutFileLine;
				RewardCandidateItem* rewardItem = *iter;
                if (rewardItem == nullptr)
                    continue;
				// if (localRandom->nextInt(0, 100) < rewardItem->r)
                if (rewardItem->isInReward)
				{
					// int rate = localRandom->nextInt(0, 100);
					// int increasedRate = 0;
					// if (rewardItem == nullptr) continue;
					for (std::vector<IARitem*>::iterator iariter = rewardItem->iarlist.begin(); iariter != rewardItem->iarlist.end(); ++iariter)
					{
						IARitem* iar = *iariter;
						// increasedRate += iar->r;
						// if (rate < increasedRate)
                        if (iar->isInReward)
                        {
                            rewardArray->push_back(std::pair<int, int>(iar->i, (*iter)->a * iar->a));
                            break;
                        }
					}
				}
			}
		}

		return rewardArray;
	}

	AIProjectile* AIContext::getProjectileByID(int id)
	{
		if (id > 0)
		{
			std::list<AIProjectile*>::iterator pProjectile;

			for (pProjectile = projectileList.begin(); pProjectile != projectileList.end(); ++pProjectile)
			{
				if ((*pProjectile)->myID == id) return *pProjectile;
			}
		}

		return nullptr;
	}

	CharacterData* AIContext::getCharacterByID(int id)
	{
		if (id >= 0)
		{
            std::unordered_map<unsigned short, CharacterData*>::iterator iter = allCharacters.find(id);
			if (iter == allCharacters.end())
                return nullptr;
            else {
                return iter->second;
            }
                
		}

		return nullptr;
	}

    CharacterData* AIContext::getCharacterByTagValue(const std::string &tag, int tagval)
    {
        for (auto iter = allCharacters.begin(); iter != allCharacters.end(); ++iter) 
        {
            CharacterData *c = iter->second;
            if (c && c->hasTag(tag) && c->getTagValue(tag) == tagval)
            {
                return c;
            }
        }
        return nullptr;
    }
    
    /*
    bool AIContext::checkTargetSummonIsDead(CharacterData *pCharacterData,int iTargetSummonId)
    {
        if (!pCharacterData->isSummon()) {
            return false;
        }
        
        if (!pCharacterData->summonProperty) {
            return false;
        }
        
        for (CharacterDataList::iterator ite = pCharacterData->friends->begin(); ite != pCharacterData->friends->end(); ++ite) {
            if ((*ite) && (*ite)->isSummon() && (*ite)->summonProperty && (*ite)->summonProperty->summonId == iTargetSummonId)
            {
                if ((*ite)->getHP() > 0) {
                    return false;
                }
            }
        }
        
        
        return true;
        
    }
    */

    int AIContext::addHeroCharacters(int* attList, std::list<std::pair<int, int>>& skillList, int juexing, int type, short k, short ifUltraLock, int clusterNum, TeamSide teamSide, int fp)
	{
        if (!_isLuaReady)
        {
            if (initLua() == 0)
            {
                _isLuaReady = true;
            }
        }
        
        if (TeamSide::kInvalid == teamSide)
        {
            //CCLOG("old hero(%d):cluster(%d)", attList[0], clusterNum);
            if (clusterNum > 0)
            {
                teamSide = TeamSide::kEnemy;
            }
            else
            {
                teamSide = TeamSide::kOur;
                clusterNum = 1;
            }
        }
#if BATTLE_LOG_TEST
        OPBattleLog("AI:add hero(%d):(team-cluster)%d-%d", attList[0], teamSide, clusterNum);
#endif
        CharacterData* cd = new CharacterData(attList, skillList, juexing, *this, type, k, ifUltraLock, clusterNum, NULL, teamSide, RoleType::kNormalCharacter, fp);

        if (_isLuaReady)
            luahelper_aicontext_addcharacter(getLuaState(), cd);

        if (100 == k)
        {
            cannon = cd;
            return cd->myID;
        }

#if BATTLE_LOG_TEST
        OPBattleLog("addhero：", teamSide, clusterNum, cd->getID());
#endif
        
        CharacterDataListPtr sameClusterHeroes = getClusterOrigin(teamSide, clusterNum);
        CharacterDataListPtr sameClusterHeroesRealtime = getFriendsForCluster(teamSide, clusterNum, false);
        CharacterDataListPtr friendsRealtimeNosummon = getFriendsForCluster(teamSide, clusterNum, true);

        if (!cd->isCaptain())
        {
            sameClusterHeroes->push_back(cd);
            sameClusterHeroesRealtime->push_back(cd);
            friendsRealtimeNosummon->push_back(cd);
        }
        else
        {
            if (teamSide == TeamSide::kOur)
                captainOur = cd;
            else if (teamSide == TeamSide::kEnemy)
                captainEnemy = cd;
        }

        cd->friends = sameClusterHeroes;
        cd->m_pTmpFriends = sameClusterHeroesRealtime;

        if (BusinessType::CANNON_KING == businessType && TeamSide::kEnemy == teamSide)
        {
            if (!_pCannonKing)
                _pCannonKing = AICannonKingPtr(new AICannonKing);

            _pCannonKing->normalWaves.insert(clusterNum);
            _pCannonKing->availableWaves[clusterNum] = 0;

            // 最后一波怪作为特殊波
            if (_pCannonKing->bossClusterNum < clusterNum)
                _pCannonKing->bossClusterNum = clusterNum;
        }
#if BATTLE_LOG_TEST
        OPBattleLog("add hero:cluster(%d), staticID(%d)", clusterNum, cd->attributes[0]);
#endif
        checkAddHeroSummonCharacter(cd, type, teamSide);
        
		return cd->myID;
	}

    int AIContext::addCaptain(int* attList, std::list<CaptainSeqSkillInfo> &seqList, int juexing, int type, short k, short ifUltraLock, int clusterNum, TeamSide teamSide)
    {
        if (!_isLuaReady)
        {
            if (initLua() == 0)
            {
                _isLuaReady = true;
            }
        }
        
        if (TeamSide::kInvalid == teamSide)
        {
            if (clusterNum > 0)
            {
                teamSide = TeamSide::kEnemy;
            }
            else
            {
                teamSide = TeamSide::kOur;
                clusterNum = 1;
            }
        }

        CCLOG("AI:add captain(%d):(team-cluster)%d-%d", attList[0], teamSide, clusterNum);

        std::list<std::pair<int, int>> skillList;
        for (auto iter = seqList.begin(); iter != seqList.end(); ++iter)
            skillList.push_back( std::make_pair(iter->skillID, iter->skillLevel) );
        CharacterData* cd = new CharacterData(attList, skillList, juexing, *this, type, k, ifUltraLock, clusterNum, NULL, teamSide, RoleType::kCaptain, 0);
        cd->initCaptainSeq(seqList);

        CharacterDataListPtr sameClusterHeroes = getClusterOrigin(teamSide, clusterNum);
        CharacterDataListPtr sameClusterHeroesRealtime = getFriendsForCluster(teamSide, clusterNum, false);
        //CharacterDataListPtr friendsRealtimeNosummon = getFriendsForCluster(teamSide, clusterNum, true);

        if (teamSide == TeamSide::kOur)
            captainOur = cd;
        else if (teamSide == TeamSide::kEnemy)
            captainEnemy = cd;

        cd->friends = sameClusterHeroes;
        cd->m_pTmpFriends = sameClusterHeroesRealtime;

        if (_isLuaReady)
            luahelper_aicontext_addcaptain(getLuaState(), cd);

        checkAddHeroSummonCharacter(cd, type, teamSide);

        return cd->myID;
    }
    
    void AIContext::checkAddHeroSummonCharacter(CharacterData* hero, int type, TeamSide teamSide)
    {
        if (!hero)
            return;

        //要准备增加召唤物
        //检查角色的技能中是否有
        std::vector<SummonInfo*> vSummonCharacterIds;
        if (!checkHeroCharacterHaveSummonSkill(hero, vSummonCharacterIds))
            return;

        //CharacterDataListPtr sameClusterHeroes = getClusterOrigin(teamSide, hero->clusterNum);
        //CharacterDataListPtr sameClusterHeroesRealtime = getFriendsForCluster(teamSide, hero->clusterNum, false);

        for (std::vector<SummonInfo*>::iterator ite = vSummonCharacterIds.begin(); ite != vSummonCharacterIds.end(); ++ite) {
            //准备将角色创建出来并加入相应的序列中 设定为召唤物 并通知前端
            SummonInfo *pInfo = *ite;
            //int characterId,int iLv,int juexingLv,int jinjie,std::list<std::pair<int, int>>& skillList
            int* summonAttList = initSummonCharacterAttrFromConfig(pInfo->characterId, pInfo->iLv, 0, 0, pInfo->skillList);
            //if (pInfo->pSummonProperty && pInfo->pSummonProperty->attrInheritBase > 0)
            //{
            //    std::vector<int> inheritAttrs = {
            //        ECharacterAttr::MAX_HP,
            //        ECharacterAttr::PHY,
            //        ECharacterAttr::AGILE,
            //        ECharacterAttr::INT,
            //        ECharacterAttr::ACCURATE,
            //        ECharacterAttr::SPIRIT,
            //        ECharacterAttr::PHY_AT,
            //        ECharacterAttr::MAG_AT,
            //        ECharacterAttr::PHY_CRIT,
            //        ECharacterAttr::MAG_CRIT,
            //        ECharacterAttr::PHY_CRIT_MUL,
            //        ECharacterAttr::MAG_CRIT_MUL,
            //        ECharacterAttr::PHY_HIT,
            //        ECharacterAttr::MAG_HIT,
            //        ECharacterAttr::PHY_DE,
            //        ECharacterAttr::MAG_DE,
            //        ECharacterAttr::PHY_PE,
            //        ECharacterAttr::MAG_PE,
            //        ECharacterAttr::DODGE_RATE,
            //        ECharacterAttr::RESIST_RATE,
            //        ECharacterAttr::BLOOD_SUCK,
            //        ECharacterAttr::CURE
            //    };
            //    int inheritRatio = pInfo->pSummonProperty->attrInheritBase;
            //    if (pInfo->pSummonProperty->attrInheritGrow > 0)
            //        inheritRatio += pInfo->pSummonProperty->attrInheritGrow * pInfo->iLv;
            //    float ratio = (float)inheritRatio / 10000.0f;
            //    for (auto attr : inheritAttrs)
            //        summonAttList[attr] = (float)hero->attributes[attr] * ratio;
            //    summonAttList[ECharacterAttr::HP] = summonAttList[ECharacterAttr::MAX_HP];
            //}

            CharacterFunc::CheckInheritSummonAttr(pInfo->pSummonProperty, pInfo->iLv, summonAttList, hero);

            if (summonAttList) {
                for (int i = 0; i < pInfo->num; ++i) {
                    int *tmpAttList = new int[ATT_NUM];
                    memcpy(tmpAttList, summonAttList, sizeof(int)* ATT_NUM);

                    CharacterData* pSummon = new CharacterData(tmpAttList, pInfo->skillList, 0, *this, type, hero->k, hero->ifUltraLock, hero->clusterNum, hero, teamSide, RoleType::kSummonCharacter, 0);
                    pSummon->summonProperty = pInfo->pSummonProperty;

                    //sameClusterHeroes->push_back(pSummon);
                    //sameClusterHeroesRealtime->push_back(pSummon);

                    //pSummon->friends = sameClusterHeroes;
                    //pSummon->m_pTmpFriends = sameClusterHeroesRealtime;

                    hero->getSummonList()->push_back(pSummon);

                    //准备通知前端
                    if (summonProxy)
                    {
                        summonProxy->createSummon(*this, hero, pSummon->myID);

                        m_summonCharacterList.push_back(pSummon);
                    }

                    if (_isLuaReady)
                    {
                        luahelper_aicontext_addcharacter(getLuaState(), pSummon);
                    }
                }
                delete[] summonAttList;

            }

        }


        //清空vSummonCharacterIds
        for (std::vector<SummonInfo*>::iterator ite = vSummonCharacterIds.begin(); ite != vSummonCharacterIds.end(); ++ite) {
            if (*ite)
            {
                delete *ite;
            }
        }
        vSummonCharacterIds.clear();
    }

    bool AIContext::checkHeroCharacterHaveSummonSkill(CharacterData *pCharacterData,std::vector<SummonInfo*>& vSummonCharacterIds)
    {
        auto addSummonFunc = [pCharacterData, &vSummonCharacterIds, this](int execID, int skillLevel)->int
        {
            if (!IsSummonExec(execID))
                return 0;
            SummonExec* pExec = AIDataHelper::getSummonExec(execID);
            if (!pExec)
                return 0;
            int summonCount = 0;
            for (const auto& summon : pExec->execs)
            {
                int maxSummonNum = summon.maxSummonNum;
                int summonId = summon.summonId;
                if (summonId <= 0)
                    continue;
                SummonProperty *pSummonProperty = AIDataHelper::getSummonProperty(summonId);
                if (!pSummonProperty)
                    continue;

                if (pSummonProperty->target == 0)
                {
                    //复制释放技能者本身
                    SummonInfo *pInfo = new SummonInfo;
                    pInfo->characterId = pCharacterData->attributes[0];
                    pInfo->iLv = skillLevel;
                    pInfo->num = maxSummonNum;
                    pInfo->pSummonProperty = pSummonProperty;

                    getSummonSkillList(pSummonProperty, pCharacterData, pInfo->skillList, skillLevel);

                    vSummonCharacterIds.push_back(pInfo);
                    summonCount++;
                }
                else
                {
                    SummonInfo *pInfo = new SummonInfo;
                    pInfo->characterId = pSummonProperty->targetId;
                    pInfo->iLv = skillLevel;
                    pInfo->num = maxSummonNum;
                    pInfo->pSummonProperty = pSummonProperty;
                    getSummonSkillList(pSummonProperty, pCharacterData, pInfo->skillList, skillLevel);
                    vSummonCharacterIds.push_back(pInfo);
                    summonCount++;
                }
            }

            return summonCount;
        };

        int totalSummonCount = 0;
        for (auto iter = pCharacterData->skillList.begin(); iter != pCharacterData->skillList.end(); ++iter)
        {
            auto skill = AIDataHelper::getSkillData(iter->first);
            if (!skill)
                continue;
            int skillLevel = iter->second;
            for (const auto& exec : skill->skillExec)
            {
                if (IsHurtExec(exec.execId))
                {
                    auto hurt = AIDataHelper::getHurtExec(exec.execId);
                    //totalSummonCount += addSummonFunc(hurt->hitExec1, skillLevel);
                    //totalSummonCount += addSummonFunc(hurt->hitExec2, skillLevel);
                    //totalSummonCount += addSummonFunc(hurt->hitExec3, skillLevel);
                    if (hurt)
                    {
                        for (auto hitExecID : hurt->hitExecs)
                        {
                            totalSummonCount += addSummonFunc(hitExecID, skillLevel);
                        }
                    }
                }
                else
                    totalSummonCount += addSummonFunc(exec.execId, skillLevel);
            }
        }

        return (totalSummonCount > 0);
    }
    
    void AIContext::getSummonSkillList(SummonProperty * pSummonProperty,CharacterData *pSource,std::list<std::pair<int,int>>& skillList,int summonSkillLv)
    {
        if (!pSummonProperty) {
            return;
        }
        
        
        for(std::vector<SummonSkillDetail>::iterator ite = pSummonProperty->summonSkillInfos.begin();ite != pSummonProperty->summonSkillInfos.end(); ++ite)
        {
            if((*ite).skillId > 0)
            {
                //0-无条件解锁，1-随施法者角色等级X解锁，2-随施法者技能X的解锁而解锁
                if ((*ite).param0 == 0) {
                    skillList.push_back(std::make_pair((*ite).skillId, summonSkillLv));
                }else if((*ite).param0 == 1)
                {
                    if(pSource->getLv() >= (*ite).param1)
                    {
                        skillList.push_back(std::make_pair((*ite).skillId, summonSkillLv));
                    }
                }else if((*ite).param0 == 2)
                {
                    
                    for (std::vector<std::pair<int, int>>::iterator sourceSkillIter = pSource->skillList.begin();sourceSkillIter != pSource->skillList.end(); ++sourceSkillIter) {
                        if (sourceSkillIter->second > 0 && (*ite).param1 == sourceSkillIter->first) {
                            skillList.push_back(std::make_pair((*ite).skillId, sourceSkillIter->second));
                            break;
                        }
                    }

                }
            }
        }
    }
    
    
    
    void AIContext::getSkillAttr(std::list<std::pair<int, int>>& skillList,int* tskill_attr)
    {
        //int tskill_attr[ATT_NUM] = {0};
        for (std::list<std::pair<int, int>>::iterator ite = skillList.begin(); ite != skillList.end(); ++ite) {
            if (ite->second > 0) {
                //技能等级 > 0
                AISkill* pSkill = AIDataHelper::getSkillData(ite->first);
                if(pSkill)
                {
                    int xiaoguoId = pSkill->skillExec[0].execId;
                    if (xiaoguoId == 0)
                        continue;
                    
                    EffectExec* pEffect = AIDataHelper::getEffectExec(xiaoguoId);
                    if (!pEffect)
                        continue;
                    //约定好属性加成只用前3个
                    for (std::vector<InnerEffectExec>::iterator effectExecIter = pEffect->execs.begin(); effectExecIter != pEffect->execs.end(); ++effectExecIter) {
                        int shuxingId = (*effectExecIter).ID;
                        if (shuxingId > 33 || shuxingId < 1) {
                            CCLOG("shuxingid NaN %d" ,shuxingId);
                            continue;
                        }
                        
                        
                        int shuxingtype = (*effectExecIter).changeType;
                        int shuxingvalue = (*effectExecIter).valueBase/100;
                        int shuxingincrement = (*effectExecIter).levelup/100;
                        
                        if (0 == shuxingtype) {
                            tskill_attr[shuxingId] += shuxingvalue + shuxingincrement * ite->second;
                        }else if(1 == shuxingtype)
                        {
                            tskill_attr[shuxingId] += shuxingvalue * (1 + (shuxingincrement * ite->second) / 100);
                        }
                        CCLOG("0 shuxingid: %d, xiaoguoid: %d,shuxingtype :%d, shuxingvalue : %d, shuxingincrement:%d", shuxingId , xiaoguoId , shuxingtype,shuxingvalue,shuxingincrement);
                        
                        
                    }

                }
            }
        }
        
        
        if (tskill_attr[ECharacterAttr::HP]) {
            tskill_attr[ECharacterAttr::MAX_HP] = tskill_attr[ECharacterAttr::HP];
        }
        
    }
    
    
    int* AIContext::initSummonCharacterAttrFromConfig(int characterId,int iLv,int juexingLv,int jinjie,std::list<std::pair<int, int>>& skillList)
    {
        CharData *pData = AIDataHelper::getCharacterData(characterId);
        if(pData)
        {
            
            int skillAttributes[ATT_NUM] = {0};
            
            getSkillAttr(skillList,skillAttributes);
            
            
            
            
            int *arrList = new int[ATT_NUM];
            memset(arrList, 0, ATT_NUM*sizeof(int));
            arrList[ECharacterAttr::ID] = characterId;
            
            arrList[ECharacterAttr::LV] = iLv;
            memcpy(&arrList[ECharacterAttr::PHY], &(pData->att.strenght), sizeof(pData->att));
            //觉醒进阶都不考虑
            int grow[5] = {0};
            for (int i = 0; i < 5; ++i) {
                grow[i] = pData->juexing[juexingLv * 5 + i]/100;
            }
            
            //目前是19阶为最高
            if(jinjie > 0 && jinjie <= 19)
            {
                int levelUpUnit = 21;
                
                for (int i = 0; i < jinjie; ++i) {
                    int startIndex = levelUpUnit * i;
                    for (int j = 0; j < levelUpUnit; ++j) {
                        arrList[ECharacterAttr::HP + j] += pData->levelup[startIndex + j];
                    }
                    
                }
            }
            
            
            for (int i = 0; i < 5; ++i) {
                arrList[i+ECharacterAttr::PHY] = arrList[i+ECharacterAttr::PHY] + iLv * grow[i];
            }
            
            for (int i = 0; i < ATT_NUM; ++i) {
                arrList[i] += skillAttributes[i];
            }
            
            AIDataHelper::loadConverAttrListTable();
            
            for (int i = 0; i < AIDataHelper::getConvertAttrVec().size(); ++i) {
                ConvertAttrParams& convertAttrParams = AIDataHelper::getConvertAttrVec()[i];
                
                int tmp = arrList[convertAttrParams.param0];
                arrList[convertAttrParams.param1] += tmp * convertAttrParams.param2;
            }

            
            
            arrList[ECharacterAttr::MAX_HP] = arrList[ECharacterAttr::HP];
            arrList[ECharacterAttr::LV] = iLv;
            
            return arrList;
        }
        return NULL;
    }

    void AIContext::preAddTransformCharacter(CharacterData* hero)
    {
        if (!transformProxy)
            return;

        auto checkAddFunc = [hero, this](int execID){
            if (IsTransformExec(execID))
            {
                auto transform = AIDataHelper::getTransformExec(execID);
                if (transform && transform->transform.animation)
                    transformProxy->createTransformCreature(*this, hero, transform->transform.animation);
            }
        };

        for (auto skillID : hero->staticData->sk)
        {
            if (skillID <= 0)
                continue;
            auto skill = AIDataHelper::getSkillData(skillID);
            if (!skill)
                continue;
            for (auto&& exec : skill->skillExec)
            {
                auto execID = exec.execId;
                if (IsHurtExec(execID))
                {
                    auto hurt = AIDataHelper::getHurtExec(execID);
                    //checkAddFunc(hurt->hitExec1);
                    //checkAddFunc(hurt->hitExec2);
                    //checkAddFunc(hurt->hitExec3);
                    if (hurt)
                    {
                        for (auto hitExecID : hurt->hitExecs)
                        {
                            checkAddFunc(hitExecID);
                        }
                    }
                }
                else
                    checkAddFunc(exec.execId);
            }
        }
    }
    
    void AIContext::setClusterAttrInheritInfos(const AttrInheritInfosPtr& pInfos)
    {
        clusterAttrInheritInfos = pInfos;
    }

    void AIContext::setClusterAttrEnhanceInfos(const AttrEnhanceInfosPtr& pInfos)
    {
        clusterAttrEnhanceInfos = pInfos;
    }

    void AIContext::setNextEnemyClusterNumber(int nextEnemyClusterNum)
    {
        this->nextClusterNumEnemy = nextEnemyClusterNum;
    }

    StatsContainer& AIContext::getBattleStats()
    {
        return StatsFunc::GetAllStatsSingletonInstance();
    }

	//send back to server to validate
	const char *jsonString()
	{
		return NULL;
	}

	void AIContext::printAIContext()
	{
		std::cout << currentTick << std::endl;
		std::cout << std::endl;

        CharacterDataListPtr currentOurCluster = getClusterOrigin(TeamSide::kOur, current_ClusterNumOur);
		CharacterDataList::iterator iter;
        for (iter = currentOurCluster->begin(); iter != currentOurCluster->end(); ++iter)
		{
			(*iter)->printCharacter();
		}

        CharacterDataList* currentRivals = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);
		if (currentRivals != nullptr)
		{
			for (iter = currentRivals->begin(); iter != currentRivals->end(); ++iter)
			{
				(*iter)->printCharacter();
			}
		}
	}

	void AIContext::createProjectileForWarship(int projectileID, int characterid, int sourceX, int sourceY, int sourceZ, int targetX, int targetY, int targetZ, int skillLevel)
	{
        //if (trackCannon_index < 30)
        //{
        //    trackCannon[trackCannon_index] = currentTick;
        //    trackCannon[trackCannon_index + 1] = projectileID;
        //    trackCannon[trackCannon_index + 2] = characterid;
        //    trackCannon[trackCannon_index + 3] = sourceX;
        //    trackCannon[trackCannon_index + 4] = sourceY;
        //    trackCannon[trackCannon_index + 5] = sourceZ;
        //    trackCannon[trackCannon_index + 6] = targetX;
        //    trackCannon[trackCannon_index + 7] = targetY;
        //    trackCannon[trackCannon_index + 8] = targetZ;
        //    trackCannon[trackCannon_index + 9] = skillLevel;
        //    trackCannon_index += 10;
        //}

        TrackCannonDataPtr pTrack = TrackCannonDataPtr(new TrackCannonData);
        pTrack->trackTick = currentTick;
        pTrack->projectileID = projectileID;
        pTrack->characterid = characterid;
        pTrack->sourceX = sourceX;
        pTrack->sourceY = sourceY;
        pTrack->sourceZ = sourceZ;
        pTrack->targetX = targetX;
        pTrack->targetY = targetY;
        pTrack->targetZ = targetZ;
        pTrack->skillLevel = skillLevel;

        trackCannons.push_back(pTrack);

		//assert(trackCannon_index <= 30);

        //CCLOG("SHANBEILENG-createProjectileForWarship:%d-%d", characterid, allCharacters.size());
        //for (std::unordered_map<unsigned short, CharacterData*>::iterator iter = allCharacters.begin(); iter != allCharacters.end(); iter++)
        //{
        //    CCLOG("SHANBEILENG-createProjectileForWarship iterator:%d-%d", iter->first, iter->second);
        //}
		ProjectileForWarship * projectile = new  ProjectileForWarship(*this, projectileID, this->getCharacterByID(characterid), sourceX, sourceY, sourceZ, targetX, targetY, targetZ, 0, skillLevel);
	}

	bool AIContext::getOwnerIsInVaildBarrier(CharacterData* theOwner)
	{
        if (clusterRangeToCenterX <= 0 || !theOwner)
            return true;

        if (AIContinuousEffectCaster::check_exert_Special_Effect(*this, *theOwner, eEFFECT_LIMIT_MOVE_RANGE, 0) > 0) {
            int posX = theOwner->getX();
            if (theOwner->teamSide == kEnemy)
                posX = 2 * battleFieldCenterX - posX;

            if (posX > AILimitMoveRangeEffect::getLimitMoveEffectUpper(theOwner) || posX < AILimitMoveRangeEffect::getLimitMoveEffectLower(theOwner))
                return false;
        }

        int distCenter = abs(battleFieldCenterX - theOwner->getX());
        return distCenter < clusterRangeToCenterX;
	}


    void AIContext::spawnCannonKingEnemy()
    {
        _pCannonKing->lastSpawnTick = currentTick;

        auto currentOurCluster = getClusterOrigin(TeamSide::kOur, 1);
        auto ourClusterRealtime = getFriendsForCluster(TeamSide::kOur, 1, false); //getClusterReal(TeamSide::kOur, 1);

        int nextClusterNum = 1;
        
        if (OPFuncCannonKing::isBossWave(passedClusterCount + 1, _pCannonKing->bossClusterNumInterval))
        {
            auto count = _pCannonKing->bossWaves.size();
            if (count > 1)
            {
                int index = localRandom->nextInt(0, 100) % count;
                for (auto iter = _pCannonKing->bossWaves.begin(); iter != _pCannonKing->bossWaves.end(); ++iter)
                {
                    index--;
                    if (index < 0)
                    {
                        nextClusterNum = *iter;
                        break;
                    }
                }
            }
            else
            {
                nextClusterNum = _pCannonKing->bossClusterNum;
            }
            _pCannonKing->nextSpawnTick = _pCannonKing->lastSpawnTick + _pCannonKing->spawnTickLong;
        }
        else
        {
            _pCannonKing->nextSpawnTick = _pCannonKing->lastSpawnTick + _pCannonKing->spawnTickNormal;
            auto count = _pCannonKing->availableWaves.size();
            if (count > 0)
            {
                int index = 0;
                if (count > 1)
                    int index = localRandom->nextInt(0, 100) % count;
                for (auto iter = _pCannonKing->availableWaves.begin(); iter != _pCannonKing->availableWaves.end(); ++iter)
                {
                    index--;
                    if (index < 0)
                    {
                        nextClusterNum = iter->first;
                        _pCannonKing->availableWaves.erase(iter);
                        break;
                    }
                }
            }
            else
            {
                for (auto iter = _pCannonKing->normalWaves.begin(); iter != _pCannonKing->normalWaves.end(); ++iter)
                    _pCannonKing->availableWaves[*iter] = 0;
            }
        }

        //CCLOG("spawn cluster:%d", nextClusterNum);

        auto nextWave = getClusterOrigin(TeamSide::kEnemy, nextClusterNum);
        auto currentEnemy = getClusterOrigin(TeamSide::kEnemy, 0);
        //auto currentEnemyTmp = NULL;// getClusterReal(TeamSide::kEnemy, 0); // TODO: aaa
        auto friendsRealForEnemy = getFriendsForCluster(TeamSide::kEnemy, 0);
        auto friendsRealNosummonForEnemy = getFriendsForCluster(TeamSide::kEnemy, 0, true);
        if (!nextWave)
            return;
        for (auto hero : *nextWave)
        {
            hero->resetPos();
            hero->targetStuckBackward = false;
            hero->oldY = hero->getY();
            hero->rivals = currentOurCluster;
            hero->m_pTmpRivals = ourClusterRealtime;

            HeroActionController::cleanup(*hero, *this, true);
            hero->resetEffectExec();
            hero->removeMarks();
            hero->resetSpecialSkillConditon();
            hero->resetTraversalTargetRecord();
            hero->resetFlags();
            //rival_num[hero->staticData->confg.line]++; // TODO: pos problem
            HeroIdleAction::Enter(*hero, *this, 1, IDLE_ACTION, *hero);
            hero->target = nullptr;
            hero->m_pTmpTarget = nullptr;
            hero->tauntTarget = nullptr;
            hero->attacker = nullptr;
            hero->mAttackConfirmedTarget = false;
            hero->specialFoundSkillTarget = nullptr;
            hero->setZ(hero->getY());
            hero->adjustingX = 100000000;
            memset(hero->actionAtt, 0, sizeof(hero->actionAtt));

            if (clusterAttrEnhanceInfos)
                enhanceHero(hero);
            hero->updateHP(hero->getHPTop());
            hero->updateEnergy(0);
            if (animationProxy)
                animationProxy->resyncCharacter(*this, hero);

            HeroActionController::setSkillActionCD(hero->actionAtt, 0);
            HeroActionController::setSkillSeq(hero->actionAtt, -1);
            HeroActionController::setSkillID(hero->actionAtt, HeroSkillController::calculateSkillID(*this, *hero, 0));

            preAddTransformCharacter(hero);

            currentEnemy->remove(hero);
            currentEnemy->push_back(hero);

            friendsRealForEnemy->remove(hero);
            friendsRealForEnemy->push_back(hero);

            
            if (!hero->isSummon())
            {
                friendsRealNosummonForEnemy->remove(hero);
                friendsRealNosummonForEnemy->push_back(hero);
            }

            hero->friends = currentEnemy;
            hero->m_pTmpFriends = friendsRealForEnemy;

            _pCannonKing->characterWaveNumMap[hero->myID] = passedClusterCount + 1;
        }
    }

    void AIContext::setCannonCount(int count)
    {
        if (!_pCannonKing)
            _pCannonKing = AICannonKingPtr(new AICannonKing);
        _pCannonKing->cannonRemainCount = count;
    }

    void AIContext::setCannonKingConfig(int normalWaveTick, int bossWaveTick, int bossWaveClusterNumInterval)
    {
        if (!_pCannonKing)
            _pCannonKing = AICannonKingPtr(new AICannonKing);
        _pCannonKing->spawnTickNormal = normalWaveTick;
        _pCannonKing->spawnTickLong = bossWaveTick;
        _pCannonKing->bossClusterNumInterval = bossWaveClusterNumInterval;
    }

    int AIContext::getCharacterWaveNum(int uid)
    {
        if (!_pCannonKing)
            return 0;

        if (_pCannonKing->characterWaveNumMap.find(uid) == _pCannonKing->characterWaveNumMap.end())
            return passedClusterCount + 1;
        return _pCannonKing->characterWaveNumMap[uid];
    }

    int AIContext::getFriendOrRivalAliveCount(TeamSide side, int clusterNum, bool isFriend, bool excludeSummon)
    {
        CharacterDataListPtr pCharacterList = nullptr;
        if (isFriend)
            pCharacterList = getFriendsForCluster(side, clusterNum, excludeSummon);
        else
            pCharacterList = getRivalsForCluster(side, clusterNum, excludeSummon);
        if (!pCharacterList)
            return 0;
        int count = 0;
        for (auto character : *pCharacterList)
        {
            if (!character || HeroActionController::isDeadAction(character->currentAction))
                continue;
            if (excludeSummon && character->isSummon())
                continue;
            count++;
        }
        return count;
    }

    void AIContext::scheduleNotifyUIEff(int scheduleTick, int parentExecMyID)
    {
        _scheduleNotifyUIEffs[scheduleTick].push_back(parentExecMyID);
    }

    void AIContext::scheduleExcuteExec(const ExecExcuteInfo &execInfo)
    {
        _appendedExcuteExecs.push_back(execInfo);
    }

    void AIContext::scheduleExcute(
        int theExecID,
        int theSkillID,
        int theSkillLevel,
        CharacterData* theSource,
        AIProjectile* theProjectile,
        CharacterData* theTarget,
        int theParentTicklen,
        bool theUseLogicPos,
        int theWaitTickLen)
    {
        ExecExcuteInfo execInfo(theExecID, theSkillID, theSkillLevel, theSource, theProjectile, theTarget, theParentTicklen, theUseLogicPos, theWaitTickLen);
        scheduleExcuteExec(execInfo);
    }

	// 某一角色死亡，借此判断整个角色列表中是否存在标记（标记执行释放条件：达到层数，有人死亡触发。）
	void AIContext::onMarkReleaseFun()
	{
		CharacterDataListPtr currentOurCluster = getClusterOrigin(TeamSide::kOur, current_ClusterNumOur);
		CharacterDataListPtr currentRivals = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);

		if (currentRivals != nullptr)
		{
			for (CharacterDataList::iterator iter = currentRivals->begin(); iter != currentRivals->end(); iter++)
			{
				CharacterData* character = (*iter);

				if ((!character->isSummon()) && (!HeroActionController::isDeadAction(character->currentAction)) && character->markCasts.size() > 0)
				{
					AIContinuousMarkCaster::checkExcuteAfterCharcterDeadReleaseTrigger(character);
				}
			}
		}

		if (currentOurCluster->size() > 0)
		{
			for (auto iter = currentOurCluster->begin(); iter != currentOurCluster->end(); ++iter)
			{
				CharacterData* character = (*iter);
				if ((!character->isSummon()) && (!HeroActionController::isDeadAction(character->currentAction)) && character->markCasts.size() > 0)
				{
					AIContinuousMarkCaster::checkExcuteAfterCharcterDeadReleaseTrigger(character);
				}
			}
		}
	}

	void AIContext::onCharacterDeadIfMarkClearFun(CharacterData *theCharacter)
	{
		// 标记施法者死亡后清除 clair add by 11.0 version

		if (theCharacter == nullptr)
		{
			return;
		}

		CharacterDataListPtr currentOurCluster = getClusterOrigin(TeamSide::kOur, current_ClusterNumOur);
		CharacterDataListPtr currentRivals = getClusterOrigin(TeamSide::kEnemy, current_ClusterNumEnemy);

		
		if (theCharacter->markCasts.size() > 0)
		{
			for (auto mkCast : theCharacter->markCasts)
			{
				if (mkCast && mkCast->origin && mkCast->origin->myID == theCharacter->myID && mkCast->innerMarkData && mkCast->innerMarkData->afterTriggerDeadIfCleared == EMarkTriggerCharDeadClearType::eMarkTriggerCharDead_Cleared)
				{
					if (!mkCast->isValid())
						continue;
					//delete mark;
					//delete mkCast;
					mkCast->invalidate();
				}
			}
		}

		if (currentRivals != nullptr)
		{
			for (CharacterDataList::iterator iter = currentRivals->begin(); iter != currentRivals->end(); iter++)
			{
				CharacterData* character = (*iter);

				if ((!HeroActionController::isDeadAction(character->currentAction)) && character->markCasts.size() > 0)
				{
					for (auto mkCast : character->markCasts)
					{
						if (mkCast && mkCast->origin->myID == theCharacter->myID && mkCast->innerMarkData && mkCast->innerMarkData->afterTriggerDeadIfCleared == EMarkTriggerCharDeadClearType::eMarkTriggerCharDead_Cleared)
						{
							if (!mkCast->isValid())
								continue;
							//delete mark;
							//delete mkCast;
							mkCast->invalidate();
						}
					}
				}
			}
		}

		if (currentOurCluster->size() > 0)
		{
			for (auto iter = currentOurCluster->begin(); iter != currentOurCluster->end(); ++iter)
			{
				CharacterData* cluster = (*iter);
				if ((!HeroActionController::isDeadAction(cluster->currentAction)) && cluster->markCasts.size() > 0)
				{
					for (auto mkCast : cluster->markCasts)
					{
						if (mkCast && mkCast->origin->myID == theCharacter->myID && mkCast->innerMarkData && mkCast->innerMarkData->afterTriggerDeadIfCleared == EMarkTriggerCharDeadClearType::eMarkTriggerCharDead_Cleared)
						{
							if (!mkCast->isValid())
								continue;
							//delete mark;
							//delete mkCast;
							mkCast->invalidate();
						}
					}
				}
			}
		}

	}

    void AIContext::onEvent(const std::string &name, CharacterData *pusher)
    {
        auto foundIter = _eventListeners.find(name);
        //auto deIter = _listenersTobeRemoved.find(name);
        if (foundIter != _eventListeners.end())
        {
            auto &listeners = foundIter->second;
            bool hasInvalidListener = false;
            for (auto &&characterMyID : listeners)
            {
                //if (deIter != _listenersTobeRemoved.end())
                //{
                //    if (deIter->second.find(characterMyID) != deIter->second.end())
                //        continue;
                //}
                if (characterMyID < 0)
                {
                    hasInvalidListener = true;
                    continue;
                }

                auto pCharacter = getCharacterByID(characterMyID);
                if (pCharacter)
                    pCharacter->onEvent(name, pusher);
            }

            //if (deIter != _listenersTobeRemoved.end())
            //{
            //    for (auto iter = deIter->second.begin(); iter != deIter->second.end(); ++iter)
            //    {
            //        auto theIter = std::find(listeners.begin(), listeners.end(), iter->first);
            //        if (theIter != listeners.end())
            //            listeners.erase(theIter);
            //    }
            //}

            if (hasInvalidListener)
            {
                for (auto iter = listeners.begin(); iter != listeners.end();)
                {
                    if (*iter < 0)
                    {
                        iter = listeners.erase(iter);
                    }
                    else
                    {
                        ++iter;
                    }
                }
            }
        }

        //_listenersTobeRemoved.clear();
    }

    void AIContext::addEventListener(const std::string &name, CharacterData *listener)
    {
        auto &listeners = _eventListeners[name];
        if (listeners.size() != 0)
        {
            auto foundIter = std::find(listeners.begin(), listeners.end(), listener->myID);
            if (foundIter != listeners.end())
                return;
        }

        listeners.push_back(listener->myID);
    }

    void AIContext::removeEventListener(const std::string &name, CharacterData *listener)
    {
        //_listenersTobeRemoved[name][listener->myID] = true;

        auto foundIter = _eventListeners.find(name);
        if (foundIter != _eventListeners.end())
        {
            auto &listeners = foundIter->second;
            for (auto iter = listeners.begin(); iter != listeners.end(); ++iter)
            {
                if (*iter == listener->myID)
                    *iter = -1;
            }
        }
    }

    void AIContext::addNoDieEffCharacter(int characterMyID)
    {
        if (_ignoreNoDieCharacters.find(characterMyID) == _ignoreNoDieCharacters.end())
            _ignoreNoDieCharacters[characterMyID] = 0;
        _ignoreNoDieCharacters[characterMyID] += 1;
    }

    void AIContext::removeNoDieEffCharacter(int characterMyID)
    {
        if (_ignoreNoDieCharacters.find(characterMyID) == _ignoreNoDieCharacters.end())
            return;

        _ignoreNoDieCharacters[characterMyID] -= 1;
        if (_ignoreNoDieCharacters[characterMyID] <= 0)
            _ignoreNoDieCharacters.erase(characterMyID);
    }

    void AIContext::onCharacterLoseHP(int hplose, CharacterData *theCharacter)
    {
        if (theCharacter->isSummon())
            return;

        if (_hploseListeners.find(theCharacter->teamSide) != _hploseListeners.end())
        {
            auto &theListeners = _hploseListeners[theCharacter->teamSide];
            for (auto iter = theListeners.begin(); iter != theListeners.end(); ++iter)
            {
                auto theListener = getCharacterByID(*iter);
                if (!theListener)
                    continue;

                AICastByTeamLoseHpEffect::checkExecute(*this, theListener, theCharacter, hplose);
            }
        }
    }

    void AIContext::addTeamHPLoseListener(int theTeamSide, int theMyId)
    {
        _hploseListeners[theTeamSide].push_back(theMyId);
    }

    void AIContext::invalidateTeamHPLoseListenerFromBegin(int theTeamSide, int theMyId)
    {
        if (_hploseListeners.find(theTeamSide) != _hploseListeners.end())
        {
            auto &theListeners = _hploseListeners[theTeamSide];
            for (auto iter = theListeners.begin(); iter != theListeners.end(); ++iter)
            {
                if (*iter == theMyId)
                {
                    *iter = -1;
                    return;
                }
            }
        }
    }

    void AIContext::stopUltraFreeze(int theCharacterMyId)
    {
        //OpCountdown, EffCountdown, myID, myCluster, startGlobalTick, freezetime
        for (std::list<int*>::iterator iter = ultraFreezeList.begin(); iter != ultraFreezeList.end(); ++iter)
        {
            if ((*iter)[2] == theCharacterMyId)
            {
                (*iter)[0] = 0;
                (*iter)[1] = 0;
            }
        }
    }

    bool AIContext::shouldStopExecWhenBeingFinalClear()
    {
        if (_isLuaReady) {
            return AIScriptBinder::getInstance()->getCommonConfigBoolean(_luaState, "is_stop_exec_when_being_final_clear");
        }
        return false;
    }

    void AIContext::setBattleMirroredForPlay(bool isBattleMirroredForPlay)
    {
        _isBattleMirroredForPlay = isBattleMirroredForPlay;
    }

    bool AIContext::isBattleMirroredForPlay()
    {
        return _isBattleMirroredForPlay;
    }

    void AIContext::setUnitHealthBarColorMirrored(bool isUnitHealthBarColorMirrored)
    {
        _isUnitHealthBarColorMirrored = isUnitHealthBarColorMirrored;
    }

    bool AIContext::isUnitHealthBarColorMirrored()
    {
        return _isUnitHealthBarColorMirrored;
    }

    void AIContext::setUnitHealthBarTypeMirrored(bool isUnitHealtBarTypeMirrored)
    {
        _isUnitHealthBarTypeMirrored = isUnitHealtBarTypeMirrored;
    }

    bool AIContext::isUnitHealtBarTypeMirrored()
    {
        return _isUnitHealthBarTypeMirrored;
    }

    CharacterDataListPtr AIContext::getTeamList(int teamlistType, int teamside, int clusternum, bool isExcludeSummon)
    {
        if (teamlistType == AITEAM_ORIGIN)
            return getClusterOrigin(TeamSide(teamside), clusternum);
        else if (teamlistType == AITEAM_FRIENDS_FOR_CLUSTER)
            return getFriendsForCluster(TeamSide(teamside), clusternum, isExcludeSummon);
        else if (teamlistType == AITEAM_RIVALS_FOR_CLUSTER)
            return getRivalsForCluster(TeamSide(teamside), clusternum, isExcludeSummon);
        
        return nullptr;
    }

    void AIContext::addCharacterToCluster(CharacterData *c, int teamlistType, int teamside, int clusternum, bool isExcludeSummon)
    {
        CharacterDataListPtr list = getTeamList(teamlistType, teamside, clusternum, isExcludeSummon);
        if (list && c) {
            list->remove(c);
            list->push_back(c);
        }
    }

    void AIContext::removeCharacterFromCluster(CharacterData *c, int teamlistType, int teamside, int clusternum, bool isExcludeSummon)
    {
        CharacterDataListPtr list = getTeamList(teamlistType, teamside, clusternum, isExcludeSummon);
        if (list && c) {
            list->remove(c);
        }
    }

    void AIContext::checkUpdateTeamAliveInfo(int side, int clusterNum)
    {
        int aliveCnt = 0;
        int deathTriggerCnt = 0;
        CheckFuncGetClusterAliveInfo(*this, TeamSide(side), clusterNum, aliveCnt, deathTriggerCnt);
        setTeamAliveInfo(side, clusterNum, aliveCnt, deathTriggerCnt);
    }

    void AIContext::setTeamAliveInfo(int side, int clusterNum, int aliveCnt, int deathTriggerCnt)
    {
        _aliveInfo[side][clusterNum] = std::make_pair(aliveCnt, deathTriggerCnt);
    }

    void AIContext::getTeamAliveInfo(int side, int clusterNum, int &outAliveCnt, int &outDeathTriggerCnt)
    {
        std::pair<int, int> &cnts = _aliveInfo[side][clusterNum];
        outAliveCnt = cnts.first;
        outDeathTriggerCnt = cnts.second;
    }

    int AIContext::getTeamAliveNumExcludeAllSummon(int side)
    {
        if (side == TeamSide::kOur)
            return friend_num[0] + friend_num[1] + friend_num[2];
        else if (side == TeamSide::kEnemy)
            return rival_num[0] + rival_num[1] + rival_num[2];

        return 0;
    }

    void AIContext::deleteCaster(int characterMyId, int casterType, int casterMyId)
    {
        CharacterData *c = getCharacterByID(characterMyId);
        if (!c) return;

        switch (casterType) {
        case eAnimationExec:
            break;
        case eHurtExec:
            for (AIContinuousHurtCaster *a_hurt_caster : c->hurtCasts) {
                if (a_hurt_caster && a_hurt_caster->myID == casterMyId) {
                    a_hurt_caster->onFree();
                    delete a_hurt_caster;
                    return;
                }
            }
            break;
        case eMoveExec:
            for (AIContinuousMoveCaster *a_move_caster : c->moveCasts) {
                if (a_move_caster && a_move_caster->myID == casterMyId){
                    delete a_move_caster;
                    return;
                }
            }
            break;
        case eProjectTileExec:
            for (AIContinuousProjectileCaster *a_projectile_caster : c->projectileCasts) {
                if (a_projectile_caster && a_projectile_caster->myID == casterMyId) {
                    delete a_projectile_caster;
                    return;
                }
            }
            break;
        case eEffectExec:
            for (AIContinuousEffectCaster *an_effect_caster : c->effectCasts) {
                if (an_effect_caster && an_effect_caster->myID == casterMyId) {
                    delete an_effect_caster;
                    return;
                }
            }
            break;
        case eSpecialUIEffectExec:
            break;
        case eSoundExec:
            break;
        case eSpecialExec:
            break;
        case eMarkExec:
            for (AIContinuousMarkCaster *a_mark_caster : c->markCasts) {
                if (a_mark_caster && a_mark_caster->myID == casterMyId) {
                    c->markCasts.remove(a_mark_caster);
                    delete a_mark_caster;
                    return;
                }
            }
            break;
        case eSummonExec:
            for (AIContinuousSummonCaster *a_summon_caster : c->summonCasts) {
                if (a_summon_caster && a_summon_caster->myID == casterMyId) {
                    delete a_summon_caster;
                    return;
                }
            }
            break;
        case eTransformExec:
            break;
        case eBodyUIEffectExec:
            break;
        default:
            break;
        }
    }

    bool AIContext::isToDeleteCasterInfoExisted(int casterMyId)
    {
        return (_toDeleteCasterMyIds.find(casterMyId) != _toDeleteCasterMyIds.end());
    }

    void AIContext::addToDeleteCasterInfo(int characterMyId, int casterType, int casterMyId)
    {
        if (_toDeleteCasterMyIds.find(casterMyId) == _toDeleteCasterMyIds.end()) {
            std::vector<int> a_record;
            a_record.push_back(characterMyId);
            a_record.push_back(casterType);
            a_record.push_back(casterMyId);
            _toDeleteCastersInfo.push_back(a_record);
            _toDeleteCasterMyIds[casterMyId] = 1;
        }
    }

    void AIContext::cleanupToDeleteCasterInfo()
    {
        _toDeleteCastersInfo.clear();
        _toDeleteCasterMyIds.clear();
    }

    void AIContext::checkDeleteToDeleteCasters()
    {
        if (_toDeleteCastersInfo.size() <= 0)
            return;

        for (std::vector<int> &a_record : _toDeleteCastersInfo) {
            if (a_record.size() >= 3)
                deleteCaster(a_record[0], a_record[1], a_record[2]);
        }

        cleanupToDeleteCasterInfo();
    }

    void AIContext::setSubjectiveTeamside(int side)
    {
        _subjectiveTeamside = side;
    }

    int AIContext::getSubjectiveTeamside()
    {
        return _subjectiveTeamside;
    }

    void AIContext::tempUpdateStopOver()
    {
#ifdef SERVER_VERIFYING
        return;
#endif
        AIActionController::executeExec(*this, 88100, 0, 1, nullptr, nullptr, nullptr, 0);
    }

    void AIContext::clearLastCombatResult()
    {
        lastCombatResult.winlose = 0;
        lastCombatResult.totaltick = 0;
        lastCombatResult.ourinfo.clear();
        lastCombatResult.enemyinfo.clear();
    }
    //cui_2023_7_3
    void AIContext::setChapterElementAtkLabel()
    {
        commonSetAtkLabelFunc(TeamSide::kOur);
        commonSetAtkLabelFunc(TeamSide::kEnemy);


    }
    void AIContext::commonSetAtkLabelFunc(TeamSide side) {
        int teamNum = 0;
        if (side == TeamSide::kOur) {
            teamNum = clustersOur.size();
        }
        else {
            teamNum = clustersEnemy.size();
        }
        int labelType[] = {501,601,602,603,604,605,606};  //cui 先写死当前7个属性
        int tempLength = sizeof(labelType) / sizeof(labelType[0]);
        int labelNum[] = { 0,0,0,0,0,0,0 };
        for (size_t i = 1; i <= teamNum; i++)
        {
            CharacterDataListPtr tempCluster = getClusterOrigin(side, i);
            for (size_t k = 0; k < tempLength; k++)
            {
                labelNum[k] = 0;
            }
            //计算标签总个数
            for (auto iter = tempCluster->begin(); iter != tempCluster->end(); ++iter)
            {
                CharacterData* character = (*iter);
                //剔除召唤物
                if (character->isSummon()) {
                    continue;
                }
                for (size_t j = 0; j < tempLength; j++)
                {
                    if (CharacterFunc::HasLabel(character, labelType[j])) {
                        labelNum[j]++;
                        break;
                    }
                }
            }
            //设置当前使用哪个标签
            for (auto iter = tempCluster->begin(); iter != tempCluster->end(); ++iter)
            {
                CharacterData* character = (*iter);
                character->atkLabelVec.clear();
                int maxTypeIndex = -1;
                for (size_t j = 0; j < tempLength; j++)
                {
                    if (labelNum[j] == 0) {
                        continue;
                    }
                    if (!CharacterFunc::HasLabel(character, labelType[j])) {
                        continue;
                    }
                    if (maxTypeIndex == -1) {
                        maxTypeIndex = j;
                        character->atkLabelVec.push_back(labelType[j]);
                        continue;
                    }
                    //比较标签个数
                    if (labelNum[j] > labelNum[maxTypeIndex]) {
                        maxTypeIndex = j;
                        character->atkLabelVec.clear();
                        character->atkLabelVec.push_back(labelType[j]);
                        continue;
                    }
                    else if (labelNum[j] == labelNum[maxTypeIndex]) {
                        //判断属性值大小
                        if (CharacterFunc::GetCharacterAttrByLabel(character, labelType[j], true) > CharacterFunc::GetCharacterAttrByLabel(character, labelType[maxTypeIndex], true)) {
                            maxTypeIndex = j;
                            character->atkLabelVec.clear();
                            character->atkLabelVec.push_back(labelType[j]);
                            continue;
                        }
                        else if (CharacterFunc::GetCharacterAttrByLabel(character, labelType[j], true) == CharacterFunc::GetCharacterAttrByLabel(character, labelType[maxTypeIndex], true)) {
                            character->atkLabelVec.push_back(labelType[j]);
                            continue;
                        }
                    }

                }

            }

        }
    }


}
