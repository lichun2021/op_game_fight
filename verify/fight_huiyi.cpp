#include "fight_huiyi.h"
#include "verifyhelper.h"
#include "../json/document.h"
#include "../jsonx/json.h"
#include "../AI/AIContext.h"
#include "../AI/AIDataManager.h"
#include "../AICallbackServer.h"
#include "../rankserialization.h"

std::shared_ptr<std::vector<int> > GetTowerDefenceWinloseCheckCondition(int hyid) {
    if (hyid <= 0)
        return nullptr;

    rapidjson::Document* pDoc = OnePiece::AIDataManager::getJsonFromFile("_battle_mode_tower_defence");
    if (!pDoc)
        return nullptr;

    auto& doc = *pDoc;
    std::string idStr = OnePiece::stringWithFormat("%d", hyid);

    auto& config = doc[idStr.c_str()];
    if (!config.IsNull() && config["conf"].Size() > 2) {
        auto ret = std::shared_ptr<std::vector<int> >(new std::vector<int>);
        auto& conf = config["conf"];

        ret->push_back(conf[1].GetInt());
        ret->push_back(conf[2].GetInt());
        return ret;
    }

    return nullptr;
}

std::shared_ptr<std::vector< std::vector<int> > > GetTowerDefenceEnemyWaveInfo(int hyid) {
    if (hyid <= 0)
        return nullptr;

    rapidjson::Document* pDoc = OnePiece::AIDataManager::getJsonFromFile("_battle_mode_tower_defence");
    if (!pDoc)
        return nullptr;

    auto& doc = *pDoc;
    std::string idStr = OnePiece::stringWithFormat("%d", hyid);

    auto& config = doc[idStr.c_str()];
    if (!config.IsNull() && config["plus"].Size() > 0) {
        auto ret = std::shared_ptr<std::vector< std::vector<int> > >(new std::vector< std::vector<int> >);
        auto& waveValue = config["plus"];

        for (int i = 0; i < waveValue.Size(); i++) {
            std::vector<int> info;

            for (auto it = waveValue[i].MemberonBegin(); it != waveValue[i].MemberonEnd(); ++it) {
                ret->push_back({ std::stoi(it->name.GetString()), it->value.GetInt() });
            }

            //for (int j = 0; j < waveValue[i].Size(); j++) {
            //    strBuff.str("");
            //    strBuff << j;
            //    info.push_back(waveValue[i][strBuff.str().c_str()].GetInt());
            //}
            //ret->push_back(info);
        }
        return ret;
    }

    return nullptr;
}

std::shared_ptr<std::vector<int> > GetSoloWheelwarFailInheritInfo(int hyid) {
    if (hyid <= 0)
        return nullptr;

    rapidjson::Document* pDoc = OnePiece::AIDataManager::getJsonFromFile("_battle_mode_solo_wheelwar");
    if (!pDoc)
        return nullptr;

    auto& doc = *pDoc;
    std::string idStr = OnePiece::stringWithFormat("%d", hyid);

    auto& config = doc[idStr.c_str()];
    if (!config.IsNull() && config["conf"].Size() > 3) {
        auto& conf = config["conf"];
        auto ret = std::shared_ptr<std::vector<int> >(new std::vector<int>);
        ret->push_back(conf[1].GetInt());
        ret->push_back(conf[2].GetInt());
        ret->push_back(conf[3].GetInt());
        return ret;
    }

    return nullptr;
}

WWFightResult fightWWTD(const std::string& str) {
    try
    {
        char buff[20];
        Json::Reader reader;
        Json::Value root;
        reader.parse(str, root);
        int seed = root["seed"].asInt();
        //printf(str);
        printf("seed: %d\n", seed);
        int k = root["k"].asInt();
        int bkey = root["bkey"].asInt();
        printf("k: %d\n", k);
        printf("%d\n", root["teams"].size());
        int team1ultralock = root["ultralock"].asInt();
        //for (std::map<unsigned short, OnePiece::CharacterData*>::iterator iter = OnePiece::AIContext::getInstance()->allCharacters.begin(); iter != OnePiece::AIContext::getInstance()->allCharacters.end(); iter++)
        //{
        //	delete (iter)->second;
        //}
        OnePiece::AIContext::getInstance()->clear();
        addCannon();
        OnePiece::AIContext::getInstance()->setRewardSeed(seed);
        //root["teams"][0] <- our
        //root["teams"][1] <- their
        int gkid = root["gkid"].asInt();
        bool fallback20151010 = hasFlagForFixVerifyReward(root);
        for (int team = 0; team < 2; team++) {
            int teamSide = team + 1;
            Json::Value& teamValue = root["teams"][team];
            for (int cluster = 0; cluster < teamValue.size(); cluster++)
            {
                int clusterNum = cluster + 1;

                ///
                for (int i = 0; i < teamValue[cluster].size(); i++) {
                    Json::Value& heroValue = teamValue[cluster][i];
                    //x,y,z
                    //int *attrList = new int[heroValue["attr"].size()];
                    //for (int j = 0; j < heroValue["attr"].size(); j++){
                    //    attrList[j] = heroValue["attr"][j].asInt();
                    //}
                    int *attrList = createAttr(heroValue["attr"].toStyledString());
                    if (attrList[11] <= 0) continue;
                    std::list<std::pair<int, int> > skillList;
                    for (int j = 0; j < heroValue["j"].size(); j++){
                        skillList.push_back(std::pair<int, int>());
                        int x = heroValue["j"][j]["i"].asInt();
                        int y = heroValue["j"][j]["lv"].asInt();
                        skillList.back().first = heroValue["j"][j]["i"].asInt();
                        skillList.back().second = heroValue["j"][j]["lv"].asInt();
                    }
                    int juexing = heroValue["x"].asInt();
                    int type = 1;
                    short Ultralock = (bkey>10) ? 0 : 1;
                    if (1 == team && 1 == team1ultralock)
                        Ultralock = 1;
                    int fp = heroValue["fp"].asInt();
                    int id = OnePiece::AIContext::getInstance()->addHeroCharacters(attrList, skillList, juexing, type, (k == team) ? 1 : 0, (team == 0) ? 1 : Ultralock, clusterNum, OnePiece::TeamSide(teamSide), fp);

                    OnePiece::CharacterData* chara = OnePiece::AIContext::getInstance()->getCharacterByID(id);
                    chara->setX(attrList[3]);
                    chara->setY(attrList[4]);
                    chara->onScriptSetX(attrList[3]);
                    chara->onScriptSetY(attrList[4]);
                    if (heroValue["bqsl"].isArray()) {
                        for (int j = 0; j < heroValue["bqsl"].size(); j++){
                            for (int l = 0; l< heroValue["bqsl"][j].size(); l++){
                                int bqi = heroValue["bqsl"][j][l]["i"].asInt();
                                int bqlv = heroValue["bqsl"][j][l]["lv"].asInt();
                                chara->addTalent(bqi, bqlv);
                            }
                        }
                    }

                    int monsterId = heroValue["i"].asInt();
                    if (teamSide == OnePiece::TeamSide::kEnemy && clusterNum > 0 && gkid > 0){
                        std::vector<OnePiece::RewardCandidateItem*>* monster_reward = RewardCandidateItemFromGkid(gkid, clusterNum, monsterId);

                        if (monster_reward != nullptr) {
                            printf("%d,%d,%d\n", clusterNum, id, monster_reward->size());
                            if (fallback20151010)
                                sprintf(buff, "%d-%d-0", clusterNum, id);
                            else
                                sprintf(buff, "%d-%d", clusterNum, id);
                            OnePiece::AIContext::getInstance()->setRewardCandidate(monster_reward, buff, clusterNum);
                        }
                    }

                }
                ///
            } // END for cluster
        } // END for team side

        if (gkid > 0){
            std::vector<OnePiece::RewardCandidateItem*>* gkreward = RewardCandidateItemFromGkid(gkid, 0, 0);
            if (gkreward != nullptr) {
                if (fallback20151010)
                    sprintf(buff, "%d-0", 1);
                else
                    sprintf(buff, "%d", 1);
                OnePiece::AIContext::getInstance()->setRewardCandidate(gkreward, buff, 1);
            }
        }
        else {
            OnePiece::AIContext::getInstance()->setAutoFight(true);
        }

        OnePiece::AIContext::getInstance()->setBusinessType(bkey);
        OnePiece::AIContext::getInstance()->setGuanqiaID(gkid>0 ? gkid : -1);
        OnePiece::AIContext::getInstance()->setSeed(seed, 1);
        AICallbackServer *callback = new AICallbackServer;
        OnePiece::AIContext::getInstance()->setCallback(*callback);

        int hyid = 0;
        if (root["hyid"].isInt())
            hyid = root["hyid"].asInt();

        auto pCondition = GetTowerDefenceWinloseCheckCondition(hyid);
        if (pCondition && pCondition->size() > 1) {
            int condition = pCondition->at(0);
            int param = pCondition->at(1);
            int theirTeamClusterCount = root["teams"][1].size();
            for (int i = 1; i <= theirTeamClusterCount; i++) {
                float *t = new float[2];
                if (1 == condition) { // tower defence wave
                    t[0] = 600;
                    t[1] = param;
                }
                else if (2 == condition) { // tower defence time
                    t[0] = param;
                    t[1] = 0;
                }
                OnePiece::AIContext::getInstance()->setWinLoseCheck(t, i);
            }
        }
        else {
            for (int i = 1; i <= 5; i++) {
                float *t = new float[1];
                t[0] = 90;
                OnePiece::AIContext::getInstance()->setWinLoseCheck(t, i);
            }
        }

        int delta_min_y = root["battleRect"]["y"].asDouble();
        int delta_max_y = root["battleRect"]["y"].asDouble() + root["battleRect"]["height"].asDouble();

        std::cout << "minY" << delta_min_y << "maxY" << delta_max_y << std::endl;

        OnePiece::AIContext::getInstance()->setDeltaY(delta_min_y, delta_max_y);

        addInput(root);

        auto pInherit = GetSoloWheelwarFailInheritInfo(hyid);
        if (pInherit && pInherit->size() > 2) {
            OnePiece::AttrInheritInfosPtr inherits = OnePiece::AttrInheritInfosPtr(new OnePiece::AttrInheritInfos);

            OnePiece::AttrInheritInfoPtr info = OnePiece::AttrInheritInfoPtr(new OnePiece::AttributeInheritanceInfo);
            info->attribute = pInherit->at(0);
            info->ratio = pInherit->at(1);
            info->limit = pInherit->at(2);

            inherits->push_back(info);

            OnePiece::AIContext::getInstance()->setServerVerifyClusterAttrInheritInfos(inherits);
        }

        auto pWaveInfo = GetTowerDefenceEnemyWaveInfo(hyid);
        if (pWaveInfo) {
            OnePiece::EnemyWaveInfosPtr infos = OnePiece::EnemyWaveInfosPtr(new OnePiece::EnemyWaveInfos);
            for (int i = 0; i < pWaveInfo->size(); i++) {
                auto& waveInfo = pWaveInfo->at(i);
                auto pInfo = OnePiece::EnemyWaveInfoPtr(new OnePiece::EnemyWaveInfo);
                pInfo->first = waveInfo[0];

                auto enhanceInfos = OnePiece::AttrEnhanceInfosPtr(new OnePiece::AttrEnhanceInfos);
                auto enhanceInfo = OnePiece::AttrEnhanceInfoPtr(new OnePiece::AttrbuteEnhanceInfo);
                enhanceInfo->attribute = -1;
                enhanceInfo->scaleRatio = waveInfo[1];
                enhanceInfos->push_back(enhanceInfo);

                pInfo->second = enhanceInfos;

                infos->push_back(pInfo);

                if (0 == i) {
                    OnePiece::AIContext::getInstance()->setNextEnemyClusterNumber(pInfo->first);
                    OnePiece::AIContext::getInstance()->setClusterAttrEnhanceInfos(pInfo->second);
                }
            }
            OnePiece::AIContext::getInstance()->setServerVerifyEnemyWaveInfos(infos);
        }

        WWFightRecords records;
        OnePiece::AIContext::getInstance()->toNextCluster((int)root["operation"]["distance"][0u].asInt(), getFirstClusterCenterX(root));
        const auto waveCount = root["operation"]["distance"].size();
        do{
            OnePiece::AIContext::getInstance()->updateAIContext(0.05);

            if (OnePiece::AIContext::getInstance()->currentWaveCount > waveCount) {
                callback->win = 2;
                callback->ongoing = 0;
                std::cout << std::endl << "verify error!" << std::endl;
            }

        } while (callback->ongoing == 1);

        if ((bkey == OnePiece::WHEELWAR_SOLO || bkey == OnePiece::WHEELWAR_GROUP) && callback->win == 0) {
            bool isOurLast = OnePiece::AIContext::getInstance()->isCurrentLastClusterForTeam(OnePiece::TeamSide::kOur);
            bool isTheirLast = OnePiece::AIContext::getInstance()->isCurrentLastClusterForTeam(OnePiece::TeamSide::kEnemy);
            if (isTheirLast && !isOurLast) {
                callback->win = 1;
            }
        }

        std::vector< std::pair<OnePiece::TeamSide, int/*clusterNum*/> > sides;
        sides.push_back(std::make_pair(OnePiece::TeamSide::kOur, OnePiece::AIContext::getInstance()->current_ClusterNumOur));
        sides.push_back(std::make_pair(OnePiece::TeamSide::kEnemy, OnePiece::AIContext::getInstance()->current_ClusterNumEnemy));

        WWFightRecord record;
        for (int i = 0; i < sides.size(); ++i) {
            auto team = OnePiece::AIContext::getInstance()->getClusterOrigin(sides[i].first, sides[i].second);
            WWTeamInfo teamInfo;
            for (auto iter = team->begin(); iter != team->end(); ++iter) {
                if ((*iter)->isSummon())
                    continue;
                auto hero = *iter;
                WWHeroInfo info;
                info.push_back(hero->getID());
                info.push_back(hero->getHP());
                info.push_back(hero->getEnergy());

                teamInfo.push_back(info);
            }
            record.push_back(teamInfo);
        }

        return WWFightResult(callback->win, record);
    }
    catch (readException& e)
    {
        return WWFightResult(0, WWFightRecord());
    }
}

std::pair<int, std::vector<std::vector<std::vector<std::vector<int> > > > >  fightCarWheel(const string& str) {
    //	try
    //	{
    int win = 0;
    char buff[20];
    Json::Reader reader;
    Json::Value root;
    reader.parse(str, root);
    int seed = root["seed"].asInt();
    printf("seed: %d\n", seed);
    int k = root["k"].asInt();
    int bkey = root["bkey"].asInt();
    printf("k: %d\n", k);
    printf("%d\n", root["teams"].size());
    //for (std::map<unsigned short, OnePiece::CharacterData*>::iterator iter = OnePiece::AIContext::getInstance()->allCharacters.begin(); iter != OnePiece::AIContext::getInstance()->allCharacters.end(); iter++)
    //{
    //	delete (iter)->second;
    //}
    OnePiece::AIContext::getInstance()->clear();
    addCannon();
    int clusterNum = root["teams"].size();
    int gkid = root["gkid"].asInt();
    int heroindex = 0;
    int monsterindex = 1;
    std::vector<std::vector<std::vector<std::vector<int> > > > vtotalfightrecord;
    std::vector<std::vector<int> > vheroinfo;
    std::vector<int> vdeadheroid;

	std::map<int, Json::Value > captains; // <cluster, captain>

    for (int cluster = 1; cluster<clusterNum; cluster++){
        
        OnePiece::AIContext::getInstance()->clear();
        for (int i = 0; i < root["teams"][heroindex].size(); i++) {
            //int *attrList = new int[root["teams"][heroindex][i]["attr"].size()];
            //for (int j = 0; j < root["teams"][heroindex][i]["attr"].size(); j++){
            //    attrList[j] = root["teams"][heroindex][i]["attr"][j].asInt();
            //}
			auto &&characterValue = root["teams"][heroindex][i];
			if (characterValue["captain"].asInt() == 1) {
				captains[heroindex] = characterValue;
				continue;
			}

            int *attrList = createAttr(root["teams"][heroindex][i]["attr"].toStyledString());
            attrList[29] = 0;
            attrList[30] = 0;

            for (int j = 0; j < vheroinfo.size(); ++j){
                if (attrList[0] == vheroinfo[j][0]){
                    attrList[11] = vheroinfo[j][1];
                }
            }
            bool deadflag = false;
            for (int j = 0; j < vdeadheroid.size(); ++j){
                if (attrList[0] == vdeadheroid[j]){
                    deadflag = true;
                    break;
                }
            }
            if (attrList[11] <= 0 || deadflag) continue;
            std::list<std::pair<int, int> > skillList;
            for (int j = 0; j < root["teams"][heroindex][i]["j"].size(); j++){
                skillList.push_back(std::pair<int, int>());
                int x = root["teams"][heroindex][i]["j"][j]["i"].asInt();
                int y = root["teams"][heroindex][i]["j"][j]["lv"].asInt();
                skillList.back().first = root["teams"][heroindex][i]["j"][j]["i"].asInt();
                skillList.back().second = root["teams"][heroindex][i]["j"][j]["lv"].asInt();
            }
            int juexing = root["teams"][heroindex][i]["x"].asInt();
            int type = 1;
            short Ultralock = (bkey>10) ? 0 : 1;
            int fp = root["teams"][heroindex][i]["fp"].asInt();
            int id = OnePiece::AIContext::getInstance()->addHeroCharacters(attrList, skillList, juexing, type, (k == heroindex) ? 1 : 0, (heroindex == 0) ? 1 : Ultralock, heroindex, OnePiece::TeamSide::kInvalid, fp);

            OnePiece::CharacterData* chara = OnePiece::AIContext::getInstance()->getCharacterByID(id);
            if (root["teams"][heroindex][i]["bqsl"].isArray()) {
                for (int j = 0; j < root["teams"][heroindex][i]["bqsl"].size(); j++){
                    for (int l = 0; l< root["teams"][heroindex][i]["bqsl"][j].size(); l++){
                        int bqi = root["teams"][heroindex][i]["bqsl"][j][l]["i"].asInt();
                        int bqlv = root["teams"][heroindex][i]["bqsl"][j][l]["lv"].asInt();
                        chara->addTalent(bqi, bqlv);
                    }
                }
            }
        }

        for (int i = 0; i < root["teams"][monsterindex].size(); i++) {

			auto &&characterValue = root["teams"][monsterindex][i];
			if (characterValue["captain"].asInt() == 1) {
				captains[monsterindex] = characterValue;
				continue;
			}

            //x,y,z
            //int *attrList = new int[root["teams"][monsterindex][i]["attr"].size()];
            //for (int j = 0; j < root["teams"][monsterindex][i]["attr"].size(); j++){
            //    attrList[j] = root["teams"][monsterindex][i]["attr"][j].asInt();
            //}
            int *attrList = createAttr(root["teams"][monsterindex][i]["attr"].toStyledString());
            attrList[29] = 0;
            attrList[30] = 0;

            if (attrList[11] <= 0) continue;
            std::list<std::pair<int, int> > skillList;
            for (int j = 0; j < root["teams"][monsterindex][i]["j"].size(); j++){
                skillList.push_back(std::pair<int, int>());
                int x = root["teams"][monsterindex][i]["j"][j]["i"].asInt();
                int y = root["teams"][monsterindex][i]["j"][j]["lv"].asInt();
                skillList.back().first = root["teams"][monsterindex][i]["j"][j]["i"].asInt();
                skillList.back().second = root["teams"][monsterindex][i]["j"][j]["lv"].asInt();
            }
            int juexing = root["teams"][monsterindex][i]["x"].asInt();
            int type = 1;
            short Ultralock = (bkey>10) ? 0 : 1;
            int fp = root["teams"][monsterindex][i]["fp"].asInt();
            int id = OnePiece::AIContext::getInstance()->addHeroCharacters(attrList, skillList, juexing, type, (k == monsterindex) ? 1 : 0, (monsterindex == 0) ? 1 : Ultralock, 1, OnePiece::TeamSide::kInvalid, fp);

            OnePiece::CharacterData* chara = OnePiece::AIContext::getInstance()->getCharacterByID(id);
            if (root["teams"][monsterindex][i]["bqsl"].isArray()) {
                for (int j = 0; j < root["teams"][monsterindex][i]["bqsl"].size(); j++){
                    for (int l = 0; l< root["teams"][monsterindex][i]["bqsl"][j].size(); l++){
                        int bqi = root["teams"][monsterindex][i]["bqsl"][j][l]["i"].asInt();
                        int bqlv = root["teams"][monsterindex][i]["bqsl"][j][l]["lv"].asInt();
                        chara->addTalent(bqi, bqlv);
                    }
                }
            }
        }

		addCaptains(captains, k, bkey);

        OnePiece::AIContext::getInstance()->setAutoFight(true);

        OnePiece::AIContext::getInstance()->setBusinessType(bkey);
        OnePiece::AIContext::getInstance()->setGuanqiaID(gkid>0 ? gkid : -1);
        OnePiece::AIContext::getInstance()->setSeed(seed, 1);
        AICallbackServer *callback = new AICallbackServer;
        OnePiece::AIContext::getInstance()->setCallback(*callback);
        float * t1 = new float[1];
        float *t2 = new float[1];
        float *t3 = new float[1];
        t1[0] = 90;
        t2[0] = 90;
        t3[0] = 90;
        OnePiece::AIContext::getInstance()->setWinLoseCheck(t1, 1);
        OnePiece::AIContext::getInstance()->setWinLoseCheck(t2, 2);
        OnePiece::AIContext::getInstance()->setWinLoseCheck(t3, 3);

        int delta_min_y = root["battleRect"]["y"].asDouble();
        int delta_max_y = root["battleRect"]["y"].asDouble() + root["battleRect"]["height"].asDouble();

        std::cout << "minY" << delta_min_y << "maxY" << delta_max_y << endl;

        OnePiece::AIContext::getInstance()->setDeltaY(delta_min_y, delta_max_y);

        addInput(root);
        OnePiece::AIContext::getInstance()->toNextCluster((int)root["operation"]["distance"][0u].asInt(), getFirstClusterCenterX(root));
        const auto waveCount = root["operation"]["distance"].size();
        do{
            OnePiece::AIContext::getInstance()->updateAIContext(0.05);
            if (OnePiece::AIContext::getInstance()->currentWaveCount > waveCount) {
                callback->win = 2;
                callback->ongoing = 0;
                std::cout << std::endl << "verify error!" << std::endl;
            }
        } while (callback->ongoing == 1);

        // std::vector<std::vector<int> > vheroinfo;
        vheroinfo.clear();
        auto friends = OnePiece::AIContext::getInstance()->getClusterOrigin(OnePiece::TeamSide::kOur, 1);
        if (friends->size() > 0) {
            for (std::list<OnePiece::CharacterData*>::iterator iter = friends->begin(); iter != friends->end(); iter++) {
                if ((*iter)->isSummon())
                    continue;
                std::vector<int> vmonster_unit;
                vmonster_unit.push_back((*iter)->getID());
                vmonster_unit.push_back((*iter)->getHP());
                vmonster_unit.push_back((*iter)->getEnergy());
                vheroinfo.push_back(vmonster_unit);
                if ((*iter)->getHP() <= 0) vdeadheroid.push_back((*iter)->getID());

                std::cout << "heroId" << (*iter)->myID << "id" << (*iter)->getID() << "hp" << (*iter)->getHP() << "mp" << (*iter)->getEnergy() << std::endl;
            }
        }

        std::vector<std::vector<int> > vmonsterinfo;
        auto enemies = OnePiece::AIContext::getInstance()->getClusterOrigin(OnePiece::TeamSide::kEnemy, 1);
        if (enemies->size() > 0) {
            for (std::list<OnePiece::CharacterData*>::iterator iter = enemies->begin(); iter != enemies->end(); ++iter) {
                OnePiece::CharacterData *c = *iter;
                if (!c->isCannon()) {
                    if (c->isSummon())
                        continue;
                    std::vector<int> vmonster_unit;
                    vmonster_unit.push_back(c->getID());
                    vmonster_unit.push_back(c->getHP());
                    vmonster_unit.push_back(c->getEnergy());
                    vmonsterinfo.push_back(vmonster_unit);
                    std::cout << "monsterId" << c->myID << "id" << c->getID() << "hp" << c->getHP() << "mp" << c->getEnergy() << std::endl;
                }
            }
        }
        std::vector<std::vector<int> > veclipsetimeinfo;
        std::vector<int> veclipsetime_unit_s;
        veclipsetime_unit_s.push_back(90);
        veclipsetimeinfo.push_back(veclipsetime_unit_s);
        std::vector<std::vector<std::vector<int> > > vsinglefightrecord;
        vsinglefightrecord.push_back(vheroinfo);
        vsinglefightrecord.push_back(vmonsterinfo);
        vsinglefightrecord.push_back(veclipsetimeinfo);

        vtotalfightrecord.push_back(vsinglefightrecord);
        if (1 != callback->win) {
            return std::pair<int, std::vector<std::vector<std::vector<std::vector<int> > > > >(callback->win, vtotalfightrecord);
        }
        monsterindex += 1;
        win = callback->win;
    }

    return std::pair<int, std::vector<std::vector<std::vector<std::vector<int> > > >>(win, vtotalfightrecord);
    //	}
    //	catch (readException& e)
    //	{
    //		return std::pair<int, std::vector<std::vector<std::vector<int> > > >(0, nullptr);
    //	}
}
