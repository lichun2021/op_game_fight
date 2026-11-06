#include "fight_shipking.h"
#include "verifyhelper.h"
#include "../AI/BattleAttributeSchema.h"
#include "../json/document.h"
#include "../jsonx/json.h"
#include "../AI/AIContext.h"
#include "../AI/AIDataManager.h"
#include "../AICallbackServer.h"
#include "../rankserialization.h"

int shipKingScore = 0;
int shipKingCurrentWaveCount = 0;
int shipKingActivityID = 0;
int shipKingAmmoMax = 0;
int shipKingAmmoCurrent = 0;
int shipKingKillBonus = 0;

rapidjson::Value* getShipKingActivity(int activityID) {
    rapidjson::Document* pDoc = OnePiece::AIDataManager::getJsonFromFile("_event_koc_list");
    std::string idstr = OnePiece::stringWithFormat("%d", activityID);
    if (pDoc)
        return &((*pDoc)[idstr.c_str()]);
    return nullptr;
}

rapidjson::Value* getMonsterRewardRecord(int monsterID) {
    rapidjson::Document* pDoc = OnePiece::AIDataManager::getJsonFromFile("_event_koc_enemy");
    std::string idstr = OnePiece::stringWithFormat("%d", monsterID);
    if (pDoc)
        return &((*pDoc)[idstr.c_str()]);
    return nullptr;
}

int getShipKingKillBonus()
{
    auto pActivity = getShipKingActivity(shipKingActivityID);
    if (pActivity && !pActivity->IsNull())
        return (*pActivity)["killBonus"].GetInt();
    return 1;
}

int getShipKingMaxAmmo()
{
    auto pActivity = getShipKingActivity(shipKingActivityID);
    if (pActivity && !pActivity->IsNull())
        return (*pActivity)["maxAmmo"].GetInt();
    return 1;
}

int fightShipKingPveVerify(const std::string& str) {
    char buff[20];
    Json::Reader reader;
    Json::Value root;
    if(!reader.parse(str, root)) throw OnePiece::BattleAttributeSchema::Error("attribute_input_invalid_JSON");
    validateBattleAttributeSchema(root);
    shipKingScore = 0;
    shipKingCurrentWaveCount = 1;
    int seed = root["seed"].asInt();
    shipKingActivityID = root["eid"].asInt();
    shipKingAmmoMax = getShipKingMaxAmmo();
    shipKingAmmoCurrent = shipKingAmmoMax;
    shipKingKillBonus = getShipKingKillBonus();

    printf("alliance pve seed: %d\n", seed);
    int k = root["k"].asInt();
    int bkey = root["bkey"].asInt();
    int jdid = root["jdid"].asInt();
    printf("alliance pve k: %d\n", k);
    printf("alliance pve jdid: %d\n", jdid);
    printf("alliance pve bkey: %d\n", bkey);
    printf("size: %d\n", root["teams"].size());

    std::vector<std::vector<std::vector<std::vector<int> > > > vtotalfightrecord;
    std::vector<std::vector<int> > vheroinfo;
    std::vector<int> vdeadheroid;
    std::pair<std::map<int, int>*, std::vector<std::vector<std::vector<std::vector<int> > > >> retinof;

    OnePiece::AIContext::getInstance()->clear();
    OnePiece::AIContext::getInstance()->setBusinessType(bkey);
    //addCannon();
    int clusterNum = root["teams"].size();
    int gkid = root["gkid"].asInt();
    OnePiece::AIContext::getInstance()->setGuanqiaID(gkid>0 ? gkid : -1);
    OnePiece::AIContext::getInstance()->setSeed(seed, 1);
    for (int cluster = 0; cluster < clusterNum; cluster++){
        for (int i = 0; i < root["teams"][cluster].size(); i++) {
            //x,y,z
            //int *attrList = new int[root["teams"][cluster][i]["attr"].size()];
            //for (int j = 0; j < root["teams"][cluster][i]["attr"].size(); j++){
            //    attrList[j] = root["teams"][cluster][i]["attr"][j].asInt();
            //}
            int *attrList = createBattleAttr(root["teams"][cluster][i]);
            if (attrList[11] <= 0) continue;
            std::list<std::pair<int, int> > skillList;
            for (int j = 0; j < root["teams"][cluster][i]["j"].size(); j++){
                skillList.push_back(std::pair<int, int>());
                int x = root["teams"][cluster][i]["j"][j]["i"].asInt();
                int y = root["teams"][cluster][i]["j"][j]["lv"].asInt();
                skillList.back().first = root["teams"][cluster][i]["j"][j]["i"].asInt();
                skillList.back().second = root["teams"][cluster][i]["j"][j]["lv"].asInt();
            }
            int juexing = root["teams"][cluster][i]["x"].asInt();
            int type = 1;
            short Ultralock = (bkey>2) ? 0 : 1;
            int fp = root["teams"][cluster][i]["fp"].asInt();
            int id = OnePiece::AIContext::getInstance()->addHeroCharacters(attrList, skillList, juexing, type, /*(k == cluster) ? 1 : 0*/k, (cluster == 0) ? 1 : Ultralock, cluster, OnePiece::TeamSide::kInvalid, fp);

            OnePiece::CharacterData* chara = OnePiece::AIContext::getInstance()->getCharacterByID(id);
            if (root["teams"][cluster][i]["bqsl"].isArray()) {
                for (int j = 0; j < root["teams"][cluster][i]["bqsl"].size(); j++){
                    for (int l = 0; l< root["teams"][cluster][i]["bqsl"][j].size(); l++){
                        int bqi = root["teams"][cluster][i]["bqsl"][j][l]["i"].asInt();
                        int bqlv = root["teams"][cluster][i]["bqsl"][j][l]["lv"].asInt();
                        chara->addTalent(bqi, bqlv);
                    }
                }
            }

            chara->onScriptSetX(chara->getX());
            chara->onScriptSetY(chara->getY());


            // int monsterId = root["teams"][cluster][i]["pos"].asInt();
            // if (cluster > 0 && gkid > 0){
            //     printf("AlliancePveRewardCandidateItemFromGkid gkid: %d, jdid: %d, monsterId: %d\n", gkid, jdid, monsterId);
            //     std::vector<OnePiece::RewardCandidateItem*>* monster_reward = AlliancePveRewardCandidateItemFromGkid(gkid, jdid, monsterId);

            //     if (monster_reward != nullptr) {
            //         printf("cluster%d, id%d, monster_reward->size%d\n", cluster, id, monster_reward->size());
            //         sprintf(buff, "%d-%d", cluster, id);
            //         OnePiece::AIContext::getInstance()->setRewardCandidate(monster_reward, buff, cluster);
            //     }
            // }
        }
        jdid += 1;
    }

    addCannon();

    AICallbackServer *callback = new AICallbackServer;
    OnePiece::AIContext::getInstance()->setCallback(*callback);
    auto pActivity = getShipKingActivity(shipKingActivityID);
    int battleTime = (*pActivity)["battleTime"].GetInt();
    float *t1 = new float[1];
    float *t2 = new float[1];
    float *t3 = new float[1];
    t1[0] = battleTime;
    t2[0] = battleTime;
    t3[0] = battleTime;
    OnePiece::AIContext::getInstance()->setWinLoseCheck(t1, 0);
    OnePiece::AIContext::getInstance()->setWinLoseCheck(t2, 1);
    OnePiece::AIContext::getInstance()->setWinLoseCheck(t3, 2);

    int waveInterval = (*pActivity)["waveInterval"].GetInt();
    int turnInterval = (*pActivity)["turnInterval"].GetInt();
    auto enhanceInfos = OnePiece::AttrEnhanceInfosPtr(new OnePiece::AttrEnhanceInfos);
    auto enhanceInfo = OnePiece::AttrEnhanceInfoPtr(new OnePiece::AttrbuteEnhanceInfo);
    enhanceInfo->attribute = OnePiece::ECharacterAttr::MAX_HP;
    enhanceInfo->scaleRatio = (*pActivity)["extraHP"].GetInt();
    enhanceInfos->push_back(enhanceInfo);

    OnePiece::AIContext::getInstance()->setCannonCount(shipKingAmmoCurrent);
    OnePiece::AIContext::getInstance()->setCannonKingConfig(waveInterval, turnInterval, 5);
    OnePiece::AIContext::getInstance()->setClusterAttrEnhanceInfos(enhanceInfos);

    int delta_min_y = root["battleRect"]["y"].asDouble();
    int delta_max_y = root["battleRect"]["y"].asDouble() + root["battleRect"]["height"].asDouble();

    std::cout << "alliance pve minY: " << delta_min_y << ", alliance pve maxY: " << delta_max_y << endl;

    OnePiece::AIContext::getInstance()->setDeltaY(delta_min_y, delta_max_y);

    addInput(root);

    OnePiece::AIContext::getInstance()->toNextCluster((int)root["operation"]["distance"][0u].asInt(), getFirstClusterCenterX(root));
    const auto waveCount = root["operation"]["distance"].size();
    do{
        OnePiece::AIContext::getInstance()->updateAIContext(0.05);
        if (OnePiece::AIContext::getInstance()->currentWaveCount > waveCount) {
            callback->win = 2;
            callback->ongoing = 0;
            std::cout << std::endl << "alliance skip verify error!" << std::endl;
        }
    } while (callback->ongoing == 1);

    return shipKingScore;
}

void SKMonsterDead(int monsterID, bool isFatal, int waveNum)
{
    if (OnePiece::AIContext::getInstance()->businessType != OnePiece::BusinessType::CANNON_KING)
        return;

    auto monster = OnePiece::AIContext::getInstance()->getCharacterByID(monsterID);
    auto pValue = getMonsterRewardRecord(monster->getID());
    if (!pValue || pValue->IsNull())
        return;
    
    // 击杀分数 = (当前波数+10) * 目标分数倍率  * 击杀奖励倍数
    int bonus = (*pValue)["bonus"].GetInt();
    int addScore = (waveNum + 10) * bonus * shipKingKillBonus;
    shipKingScore += addScore;
    if (shipKingScore < 0)
        shipKingScore = 0;

    int ammo = (*pValue)["ammo"].GetInt();
    if (ammo > 0) {
        shipKingAmmoCurrent += ammo;
        if (shipKingAmmoCurrent > shipKingAmmoMax)
            shipKingAmmoCurrent = shipKingAmmoMax;
        OnePiece::AIContext::getInstance()->setCannonCount(shipKingAmmoCurrent);
    }

    //std::ofstream outfream_ai;
    //outfream_ai.open("d:\\AiLog2.txt", std::ios::app | std::ios::ate);
    //outfream_ai << "!!!ck score add:" << addScore << std::endl;
    //outfream_ai << "!!!ck score after:" << shipKingScore << std::endl;
    //outfream_ai.close();
}

void SKMonsterOnHit(int monsterID, int waveNum)
{
    if (OnePiece::AIContext::getInstance()->businessType != OnePiece::BusinessType::CANNON_KING)
        return;

    auto monster = OnePiece::AIContext::getInstance()->getCharacterByID(monsterID);
    auto pValue = getMonsterRewardRecord(monster->getID());
    if (!pValue || pValue->IsNull())
        return;

    // 击中分数 = (当前波数+10) * 目标分数倍率
    int bonus = (*pValue)["bonus"].GetInt();
    int addScore = (waveNum + 10) * bonus;
    shipKingScore += addScore;

    if (shipKingScore < 0)
        shipKingScore = 0;

    //std::ofstream outfream_ai;
    //outfream_ai.open("d:\\AiLog2.txt", std::ios::app | std::ios::ate);
    //outfream_ai << "!!!ck score add:" << addScore << std::endl;
    //outfream_ai << "!!!ck score after:" << shipKingScore << std::endl;
    //outfream_ai.close();
}

void SKOnSpawnedEnemyCluster(int spawnedCount) {
    shipKingCurrentWaveCount = spawnedCount;
}

void SKOnCannonExploded() {
    shipKingAmmoCurrent--;
    OnePiece::AIContext::getInstance()->setCannonCount(shipKingAmmoCurrent);
}
