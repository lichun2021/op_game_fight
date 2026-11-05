#include "AICallbackServer.h"
#include <stdio.h>
#include "AI/AICharacterData.h"
#include "AI/AIContext.h"
#include "AI/action_hero/HeroActionController.h"
#include "verify/fight_shipking.h"

AICallbackServer::AICallbackServer() {
}

AICallbackServer::~AICallbackServer() {
    delete rewardVec;
    delete hpinfoVec;
    printf("%s\n", "delete callback");
}

void AICallbackServer::fixTeamHP(std::list<OnePiece::CharacterData *> *characters) {
    if (!characters)
        return;

    for (auto iter = characters->begin(); iter != characters->end(); ++iter) {
        auto pcharacter = *iter;
        if (pcharacter->roleType == OnePiece::RoleType::kNormalCharacter && pcharacter->k != 100) {
            std::cout << "myID:" << pcharacter->myID << ",id:" << pcharacter->getID() 
                << ",hp:" << pcharacter->getHP() << ",mp:" << pcharacter->getEnergy() << std::endl;
            if (OnePiece::HeroActionController::isDeadAction(pcharacter->currentAction) && pcharacter->getHP()) {
                // MARK: tmp fix hp bug caused by eff 198
                pcharacter->setHP(0);
                std::cout << "  " << pcharacter->myID << " fix hp to 0" << std::endl;
            }
        }
    }
}

void AICallbackServer::operationTrack(std::vector<OnePiece::TrackCannonDataPtr>& trackCannon, std::vector<std::pair<int, int>>& trackUltra, std::vector<int>& trackSetDistance, std::vector<std::pair<int, int>>& trackAutofight, OnePiece::TrackAccumulate& trackAccumulate, std::vector<int>& trackCenterX){
    ongoing = 0;
    printf("%s\n", "complete!");
    //auto friends = OnePiece::AIContext::getInstance()->getClusterOrigin(OnePiece::TeamSide::kOur, 1);
    //for (std::list<OnePiece::CharacterData*>::iterator iter = friends->begin(); iter != friends->end(); iter++) {
    //    if ((*iter)->myID > 0) {
    //        int myID = (*iter)->myID;
    //        int id = (*iter)->getID();
    //        int hp = (*iter)->getHP();
    //        int mp = (*iter)->getEnergy();
    //        std::cout << "myID:" << myID << "id:" << id << " hp:" << hp << " mp:" << mp << std::endl;
    //    }
    //}
    
    auto aicontext = OnePiece::AIContext::getInstance();
    for (int i = 1; i <= aicontext->current_ClusterNumOur; i++) {
        auto our = aicontext->getClusterOrigin(OnePiece::TeamSide::kOur, i);
        fixTeamHP(our);
    }

    for (int i = 1; i <= aicontext->current_ClusterNumEnemy; i++) {
        auto enemy = aicontext->getClusterOrigin(OnePiece::TeamSide::kEnemy, i);
        fixTeamHP(enemy);
    }
}

void AICallbackServer::startSchedule(int value, int x, int y, int z) {
    printf("%s\n", "startSchedule");
    OnePiece::AIContext::getInstance()->printAIContext();
}

void AICallbackServer::startSchedule() {
    printf("%s\n", "startSchedule");
}

void AICallbackServer::resumeSchedule() {

}

void AICallbackServer::stopSchedule(int value, int x, int y, int z) {
    printf("%s\n", "stopSchedule");
    OnePiece::AIContext::getInstance()->printAIContext();
}

void AICallbackServer::stopSchedule(){
    printf("%s\n", "stopSchedule");
}

void AICallbackServer::playSpeed(int multiple) {

}

void AICallbackServer::fightingComplete(int fightRes, std::vector<std::pair<int, int> >* rewardArray) {
    //if (fightRes != 1) {
    win = fightRes;
    //printf("fightRes%dwin%d\n", fightRes,win); 
    //} 
    if (rewardArray != nullptr) {
        for (std::vector<std::pair<int, int>>::iterator iter = rewardArray->begin(); iter != rewardArray->end(); iter++) {
            std::map<int, int>::iterator it = rewardVec->find(iter->first);
            if (it == rewardVec->end()) {
                rewardVec->insert(*iter);
            }
            else {
                it->second += iter->second;
            }
        }
    }

    if (hpinfoVec != nullptr) {
        std::vector<std::vector<std::vector<int> > > vsinglefightrecord;

        auto aicontext = OnePiece::AIContext::getInstance();
        std::vector<std::vector<int> > veclipsetimeinfo;
        std::vector<int> veclipsetime_unit_s;
        //veclipsetime_unit_s.push_back(aicontext->currentTick / OnePiece::AIDataHelper::s_aiUpdateTimePerSecond);
        veclipsetime_unit_s.push_back(aicontext->currentTick); // 22.0 modify: return tick
        veclipsetimeinfo.push_back(veclipsetime_unit_s);

        std::vector<std::vector<int> > vheroinfo;
        auto our = aicontext->getClusterOrigin(OnePiece::TeamSide::kOur, OnePiece::AIContext::getInstance()->current_ClusterNumOur);
        fixTeamHP(our);
        for (auto iter = our->begin(); iter != our->end(); ++iter)
        {
            auto pcharacter = *iter;
            if (pcharacter->isSummon())
                continue;
            std::vector<int> thp;
            thp.push_back(pcharacter->getID());
            thp.push_back(pcharacter->getHP());
            thp.push_back(pcharacter->getEnergy());
            printf("alliace pve verify card: %d,%d,%d\n", pcharacter->getID(), pcharacter->getHP(), pcharacter->getEnergy());

            vheroinfo.push_back(thp);
        }

        std::vector<std::vector<int> > vmonsterinfo;
        auto enemy = aicontext->getClusterOrigin(OnePiece::TeamSide::kEnemy, aicontext->current_ClusterNumEnemy);
        fixTeamHP(enemy);
        if (enemy != nullptr)
        {
            for (auto iter = enemy->begin(); iter != enemy->end(); ++iter)
            {
                auto pcharacter = *iter;
                if (pcharacter->isSummon())
                    continue;
                std::vector<int> thp;
                thp.push_back(pcharacter->getID());
                thp.push_back(pcharacter->getHP());
                thp.push_back(pcharacter->getEnergy());
                printf("alliace pve verify monster: %d,%d,%d\n", pcharacter->getID(), pcharacter->getHP(), pcharacter->getEnergy());

                vmonsterinfo.push_back(thp);
            }
        }
        vsinglefightrecord.push_back(vheroinfo);
        vsinglefightrecord.push_back(vmonsterinfo);
        vsinglefightrecord.push_back(veclipsetimeinfo);
        (*hpinfoVec).push_back(vsinglefightrecord);
    }
}

void AICallbackServer::monsterDeadReward(int monsterID, int x, int y, int z, std::vector<std::pair<int, int>>* rewardArray, bool isFatal, int waveNum) {
    if (rewardArray != nullptr) {
        for (std::vector<std::pair<int, int>>::iterator iter = rewardArray->begin(); iter != rewardArray->end(); iter++) {
            std::map<int, int>::iterator it = rewardVec->find(iter->first);
            if (it == rewardVec->end()) {
                rewardVec->insert(*iter);
            }
            else {
                it->second += iter->second;
            }
        }
    }

    SKMonsterDead(monsterID, isFatal, waveNum);
}

void AICallbackServer::removeCharacters(int *idlist) {

}

void AICallbackServer::fixCamera(int x, int y, int z) {

}

void AICallbackServer::monsterOnHit(int monsterID, int waveNum) {
    SKMonsterOnHit(monsterID, waveNum);
}

void AICallbackServer::spawnedEnemyCluster(int spawnedCount) {
    SKOnSpawnedEnemyCluster(spawnedCount);
}

void AICallbackServer::onCreateProjectileForWarship() {
    SKOnCannonExploded();
}

void AICallbackServer::captainCastSkill(int captainUID, int staticDataID, int teamSide, int skillID) {
    ////复活技能id配置见 燃烧海域 配置表
    //if (skillID != 80200 && skillID != 80201)
    //    return;
    //
    //auto theCaptain = OnePiece::AIContext::getInstance()->getCharacterByID(captainUID);
    //if (!theCaptain)
    //{
    //    std::cout << "9.0 error:captain not found!" << captainUID << std::endl;
    //    return;
    //}
    //
    //int reviveRate = 0;
    //if (skillID == 80200)
    //{
    //    reviveRate = 30;
    //}
    //else if(skillID == 80201)
    //{
    //    reviveRate = 50;
    //}
    //
    //OnePiece::CharacterDataList theLives;
    //OnePiece::CharacterDataList theDeads;
    //std::map<int, int> theOriginCurrentActions; // <myID, currentAction>
    //auto theCluster = OnePiece::AIContext::getInstance()->getClusterOrigin(theCaptain->teamSide, theCaptain->clusterNum);
    //for (auto iter = theCluster->begin(); iter != theCluster->end(); ++iter)
    //{
    //    auto theCharacter = *iter;
    //    if (theCharacter->isSummon())
    //        continue;
    //    if (OnePiece::HeroActionController::isDeadAction(theCharacter->currentAction))
    //    {
    //        //theCharacter->currentAction = OnePiece::ActionType::IDLE_ACTION;
    //        
    //        //int reviveHP = theCharacter->getHPTop() * reviveRate / 100;
    //        //theCharacter->setHP(reviveHP);
    //        
    //        //std::cout << "9.0 tmp fix buring sea revive bug:" << captainUID << "-" << teamSide << "-" << skillID << "-" << theCharacter->myID << "-" << reviveHP;
    //        //break;

    //        theDeads.push_back(theCharacter);
    //    }
    //    else
    //    {
    //        theLives.push_back(theCharacter);
    //    }

    //    theOriginCurrentActions[theCharacter->myID] = theCharacter->currentAction;
    //}

    //if (theDeads.size() <= 0)
    //{
    //    return;
    //}

    //// 1. set lives to dead
    //for (auto aHero : theLives)
    //{
    //    aHero->currentAction = OnePiece::ActionType::DEAD_ACTION1;
    //}

    //// 2. set deads to live
    //for (auto aHero : theDeads)
    //{
    //    aHero->currentAction = OnePiece::ActionType::IDLE_ACTION;
    //}

    //// 3. force release captain skill
    //theCaptain->releaseAccumulateSkill(false);

    //// 4. live set back
    //for (auto aHero : theLives)
    //{
    //    aHero->currentAction = OnePiece::ActionType(theOriginCurrentActions[aHero->myID]);
    //}

    //// 5. dead set back
    //for (auto aHero : theDeads)
    //{
    //    aHero->currentAction = OnePiece::ActionType(theOriginCurrentActions[aHero->myID]);
    //}
}
