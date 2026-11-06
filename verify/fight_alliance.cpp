#include "fight_alliance.h"
#include "verifyhelper.h"
#include "../AI/BattleAttributeSchema.h"
#include "../json/document.h"
#include "../jsonx/json.h"
#include "../AI/AIContext.h"
#include "../AI/AIDataManager.h"
#include "../AICallbackServer.h"
#include "../rankserialization.h"

std::vector< OnePiece::RewardCandidateItem*>* AlliancePveRewardCandidateItemFromGkid(int gkid, int cluster, int id){
    rapidjson::Document* doc = OnePiece::AIDataManager::getJsonFromFile("_alliancebattle_guanqia_reward");
    if (!doc)
        return nullptr;
    //, "10512":{"1":{"1":{"reward":[{"i":[{"i":90003, "a" : 1, "r" : 100}], "a" : 50, "r" : 100, "icon" : [70001], "q" : 4, "n" : "���", "d" : "���", "t" : 100}, { "i":[{"i":109008, "a" : 1, "r" : 100}], "a" : 1, "r" : 10, "icon" : [59003], "q" : 4, "n" : "ľ֮Ƥ����Ƭ", "d" : "ľ֮Ƥ����Ƭ", "t" : 100 }, { "i":[{"i":108409, "a" : 1, "r" : 100}], "a" : 1, "r" : 10, "icon" : [58003], "q" : 4, "n" : "ľ֮������Ƭ", "d" : "ľ֮������Ƭ", "t" : 100 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }], "hit_reward" : [{"i":0, "a" : 0, "r" : 0}, { "i":0, "a" : 0, "r" : 0 }, { "i":0, "a" : 0, "r" : 0 }]}, "2" : {"reward":[{"i":[{"i":90003, "a" : 1, "r" : 100}], "a" : 50, "r" : 100, "icon" : [70001], "q" : 4, "n" : "���", "d" : "���", "t" : 100}, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }], "hit_reward" : [{"i":0, "a" : 0, "r" : 0}, { "i":0, "a" : 0, "r" : 0 }, { "i":0, "a" : 0, "r" : 0 }]}, "3" : {"reward":[{"i":[{"i":90003, "a" : 1, "r" : 100}], "a" : 50, "r" : 100, "icon" : [70001], "q" : 4, "n" : "���", "d" : "���", "t" : 100}, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, {
    std::string gk = OnePiece::stringWithFormat("%d", gkid);
    std::string cl = OnePiece::stringWithFormat("%d", cluster);
    std::string ind = OnePiece::stringWithFormat("%d", id);

    std::cout << " ========== gkid: " << gkid << ", cl:" << cl << ", ind:" << ind << std::endl;
    rapidjson::Value& j1 = doc->operator[](gk.c_str());
    if (j1.IsNull()) return nullptr;
    if (j1[cl.c_str()].IsNull()) return nullptr;
    if (j1[cl.c_str()][ind.c_str()].IsNull()) return nullptr;

    rapidjson::Value& json_object = doc->operator[](gk.c_str())[cl.c_str()][ind.c_str()]["reward"];
    std::vector< OnePiece::RewardCandidateItem*>* rewardVec = new std::vector< OnePiece::RewardCandidateItem*>();

    if (json_object.IsArray()){
        int a = json_object.Size();
        for (int i = 0; i < json_object.Size(); ++i){

            OnePiece::RewardCandidateItem* rewardItem = nullptr;
            rapidjson::Value& value = json_object[i];

            rapidjson::Value& i_value = value["i"];
            if (i_value.IsArray()){
                rewardItem = new OnePiece::RewardCandidateItem();
                for (int j = 0; j < i_value.Size(); ++j){
                    rapidjson::Value& j_value = i_value[j];
                    OnePiece::IARitem *iaritem = new OnePiece::IARitem(j_value["i"].GetInt(), j_value["a"].GetInt(), j_value["r"].GetInt());
                    rewardItem->iarlist.push_back(iaritem);
                }
            }

            ///icon 
            rapidjson::Value& icon_value = value["icon"];
            if (icon_value.IsArray()){
                for (int ic = 0; ic < icon_value.Size(); ++ic){
                    rewardItem->icon.push_back(icon_value[ic].GetInt());
                }
            }

            rewardItem->a = value["a"].GetInt(); ///a 
            rewardItem->r = value["r"].GetInt(); ///r
            rewardItem->t = value["t"].GetInt();///t

            rewardVec->push_back(rewardItem);
        }
    }

    rapidjson::Value& json_object2 = doc->operator[](gk.c_str())[cl.c_str()][ind.c_str()]["alliance_reward"];

    if (json_object2.IsArray()){
        int a = json_object2.Size();
        for (int i = 0; i < json_object2.Size(); ++i){

            OnePiece::RewardCandidateItem* rewardItem = nullptr;
            rapidjson::Value& value = json_object2[i];

            rapidjson::Value& i_value = value["i"];
            if (i_value.IsArray()){
                rewardItem = new OnePiece::RewardCandidateItem();
                for (int j = 0; j < i_value.Size(); ++j){
                    rapidjson::Value& j_value = i_value[j];
                    OnePiece::IARitem *iaritem = new OnePiece::IARitem(j_value["i"].GetInt(), j_value["a"].GetInt(), j_value["r"].GetInt());
                    rewardItem->iarlist.push_back(iaritem);
                }
            }

            ///icon 
            rapidjson::Value& icon_value = value["icon"];
            if (icon_value.IsArray()){
                for (int ic = 0; ic < icon_value.Size(); ++ic){
                    rewardItem->icon.push_back(icon_value[ic].GetInt());
                }
            }

            rewardItem->a = value["a"].GetInt(); ///a 
            rewardItem->r = value["r"].GetInt(); ///r
            rewardItem->t = value["t"].GetInt();///t

            rewardVec->push_back(rewardItem);
        }
    }

    return rewardVec;
    // }
}

std::pair<int, std::pair<std::map<int, int>*, std::vector<std::vector<std::vector<std::vector<int> > > >* > > fightAlliancePveNew(const std::string& str) {
    char buff[20];
    Json::Reader reader;
    Json::Value root;
    if(!reader.parse(str, root)) throw OnePiece::BattleAttributeSchema::Error("attribute_input_invalid_JSON");
    validateBattleAttributeSchema(root);
    int seed = root["seed"].asInt();

    printf("alliance pve seed: %d\n", seed);
    int k = root["k"].asInt();
    int bkey = root["bkey"].asInt();
    int jdid = root["jdid"].asInt();
    printf("alliance pve k: %d\n", k);
    printf("alliance pve jdid: %d\n", jdid);
    printf("alliance pve bkey: %d\n", bkey);
    printf("size: %d\n", root["teams"].size());
    int fightingtime = 90;
    if (&root["ftime"] == nullptr){
        fightingtime = 90;
        printf("aaaaaaaaa: %d\n", fightingtime);
    } else {
        fightingtime = root["ftime"].asInt();
        if (!fightingtime) fightingtime = 90;
        printf("bbbbbbbbb: %d\n", fightingtime);
    }
    printf("fightingtime: %d\n", fightingtime);

    std::vector<std::vector<std::vector<std::vector<int> > > > vtotalfightrecord;
    std::vector<std::vector<int> > vheroinfo;
    std::vector<int> vdeadheroid;
    std::pair<std::map<int, int>*, std::vector<std::vector<std::vector<std::vector<int> > > >> retinof;

    OnePiece::AIContext::getInstance()->clear();
    addCannon();
    int clusterNum = root["teams"].size();
    int gkid = root["gkid"].asInt();
	std::map<int, Json::Value > captains; // <cluster, captain>
    for (int cluster = 0; cluster < clusterNum; cluster++){
        for (int i = 0; i < root["teams"][cluster].size(); i++) {
            //x,y,z
            //int *attrList = new int[root["teams"][cluster][i]["attr"].size()];
            //for (int j = 0; j < root["teams"][cluster][i]["attr"].size(); j++){
            //    attrList[j] = root["teams"][cluster][i]["attr"][j].asInt();
            //}
			auto &&characterValue = root["teams"][cluster][i];
			if (characterValue["captain"].asInt() == 1) {
				captains[cluster] = characterValue;
				continue;
			}
            int *attrList = createBattleAttr(characterValue);
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
            int id = OnePiece::AIContext::getInstance()->addHeroCharacters(attrList, skillList, juexing, type, (k == cluster) ? 1 : 0, (cluster == 0) ? 1 : Ultralock, cluster, OnePiece::TeamSide::kInvalid, fp);

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

	addCaptains(captains, k, bkey);

    OnePiece::AIContext::getInstance()->setBusinessType(bkey);
    OnePiece::AIContext::getInstance()->setGuanqiaID(gkid>0 ? gkid : -1);
    OnePiece::AIContext::getInstance()->setSeed(seed, 1);
    AICallbackServer *callback = new AICallbackServer;
    OnePiece::AIContext::getInstance()->setCallback(*callback);
    float *t1 = new float[1];
    float *t2 = new float[1];
    float *t3 = new float[1];
    t1[0] = fightingtime;
    t2[0] = fightingtime;
    t3[0] = fightingtime;
    OnePiece::AIContext::getInstance()->setWinLoseCheck(t1, 1);
    OnePiece::AIContext::getInstance()->setWinLoseCheck(t2, 2);
    OnePiece::AIContext::getInstance()->setWinLoseCheck(t3, 3);

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
            std::cout << std::endl << "alliance pve verify error!" << std::endl;
        }
    } while (callback->ongoing == 1);

    std::pair<std::map<int, int>*, std::vector<std::vector<std::vector<std::vector<int> > > >* > tmp;
    tmp.first = callback->rewardVec;
    tmp.second = callback->hpinfoVec;

    // return std::pair<int, std::map<int, int>* >(win, callback->rewardVec);
    return std::pair<int, std::pair<std::map<int, int>*, std::vector<std::vector<std::vector<std::vector<int> > > >* > >(callback->win, tmp);
};
