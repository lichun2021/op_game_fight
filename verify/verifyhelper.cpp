#include "verifyhelper.h"
#include "../json/document.h"
#include "../AI/AIContext.h"
#include "../AI/AIDataManager.h"
#include "../AI/BattleLog.h"
#include "../AI/action_hero/HeroSkillController.h"

bool hasFlagForFixVerifyReward(Json::Value& root)
{
    Json::Value& fallback = root["fallback20151010"];
    return (!fallback.isNull());
}

void addCannon() {
	// MARK: op2 hotfix
	// 紧急修复: op2开始,客户端不再添加船炮角色,船炮之王模式也不再开启
	return;
    int type = 1;
    int clusterNum = 0;
    int * attList = new int[34] { 10001, 0, 31, 0, 0, 0, 19181, 9804, 5371, 9401, 5402, 0, 0, 15, 456, 254, 99, 58, 200, 200, 184, 144, 348, 177, 0, 0, 3, 3, 0, 0, 0, 0, 30, 200 };
    std::list<std::pair<int, int> > skilllist;
    int cannonUID = OnePiece::AIContext::getInstance()->addHeroCharacters(attList, skilllist, 0, type, 100, 1, clusterNum, OnePiece::TeamSide::kInvalid, 0);
    OnePiece::CharacterData * cannon = OnePiece::AIContext::getInstance()->getCharacterByID(cannonUID);
    cannon->currentAction = OnePiece::DEAD_ACTION1;
};

OnePiece::InputTrackCannonPtr getTrackCannon(Json::Value& root)
{
    OnePiece::InputTrackCannonPtr trackCannon = nullptr;
    Json::Value& cannon = root["operation"]["cannon"];
    if (cannon.size() > 0){
        trackCannon = OnePiece::InputTrackCannonPtr(new OnePiece::InputTrackCannon);
        for (int i = 0; i + 9 < cannon.size();){
            OnePiece::TrackCannonDataPtr pData = OnePiece::TrackCannonDataPtr(new OnePiece::TrackCannonData);
            pData->trackTick = cannon[i].asInt();
            pData->projectileID = cannon[i + 1].asInt();
            pData->characterid = cannon[i + 2].asInt();
            pData->sourceX = cannon[i + 3].asInt();
            pData->sourceY = cannon[i + 4].asInt();
            pData->sourceZ = cannon[i + 5].asInt();
            pData->targetX = cannon[i + 6].asInt();
            pData->targetY = cannon[i + 7].asInt();
            pData->targetZ = cannon[i + 8].asInt();
            pData->skillLevel = cannon[i + 9].asInt();
            trackCannon->push_back(pData);

            i += 10;
        }
    }
    return trackCannon;
}

std::unordered_map<std::string, int> *getTrackUltra(Json::Value& root)
{
    char buff[20];
    std::unordered_map<std::string, int> *input_trackUltra = nullptr;
    auto& trackUltraValue = root["operation"]["trackUltra"];
    if (!trackUltraValue.isNull() && trackUltraValue.size() > 0){
        input_trackUltra = new std::unordered_map<std::string, int>();
        for (int i = 0; i < root["operation"]["trackUltra"].size(); i += 2) {
            int tick = root["operation"]["trackUltra"][i].asInt();
            int id = root["operation"]["trackUltra"][i + 1].asInt();
            sprintf(buff, "%d-%d", tick, id);
            input_trackUltra->insert(std::pair<std::string, int>(buff, id));
        }
    }
    return input_trackUltra;
}

std::unordered_map<int, int> *getTrackAutoFight(Json::Value& root)
{
    std::unordered_map<int, int> *input_trackAutofigh = nullptr;
    auto& autoFightValue = root["operation"]["autofight"];
    if (!autoFightValue.isNull() && autoFightValue.size() > 0) {
        input_trackAutofigh = new std::unordered_map<int, int>();
        for (int i = 0; i < root["operation"]["autofight"].size(); i += 2) {
            int tick = root["operation"]["autofight"][i].asInt();
            int id = root["operation"]["autofight"][i + 1].asInt();
            input_trackAutofigh->insert(std::pair<int, int>(tick, id));
        }
    }
    return input_trackAutofigh;
}

std::vector<int> *getTrackSetDistance(Json::Value& root)
{
    std::vector<int> * input_trackSetDistance = nullptr;
    auto& trackSetDistanceValue = root["operation"]["distance"];
    if (!trackSetDistanceValue.isNull() && trackSetDistanceValue.size() > 0){
        input_trackSetDistance = new std::vector<int>;
        for (int i = 0; i < root["operation"]["distance"].size(); i++){
            input_trackSetDistance->push_back(root["operation"]["distance"][i].asInt());
        }
    }
    return input_trackSetDistance;
}

OnePiece::InputTrackAccumulatePtr getTrackAccumulate(Json::Value& root)
{
    char buff[20];
    OnePiece::InputTrackAccumulatePtr input_trackAccumulate = nullptr;
    input_trackAccumulate = nullptr;
    auto& trackAccumulate = root["operation"]["trackAccumulate"];
    if (!trackAccumulate.isNull() && trackAccumulate.size() > 0){
        input_trackAccumulate = OnePiece::InputTrackAccumulatePtr(new OnePiece::InputTrackAccumulate);
        for (int i = 0; i < trackAccumulate.size(); i += 2) {
            int tick = trackAccumulate[i].asInt();
            int id = trackAccumulate[i + 1].asInt();
            sprintf(buff, "%d-%d", tick, id);
            input_trackAccumulate->insert(std::pair<std::string, int>(buff, id));
        }
    }
    return input_trackAccumulate;
}

std::shared_ptr< std::vector<int> > getTrackCenterX(Json::Value& root)
{
    char buff[20];
    std::shared_ptr< std::vector<int> > ret_input_track_centerx = nullptr;
    auto& json_value = root["operation"]["centerx"];
    if (json_value.isArray() && json_value.size() > 0)
    {
        ret_input_track_centerx = decltype(ret_input_track_centerx)(new std::vector<int>);
        for (int i = 0; i < json_value.size(); i++)
        {
            int a_centerx = json_value[i].asInt();
            ret_input_track_centerx->push_back(a_centerx);
        }
    }

    return ret_input_track_centerx;
}

void addInput(Json::Value& root)
{
    auto trackCannon = getTrackCannon(root);
    auto input_trackUltra = getTrackUltra(root);
    auto input_trackSetDistance = getTrackSetDistance(root);
    auto input_trackAutofigh = getTrackAutoFight(root);
    auto input_trackAccumulate = getTrackAccumulate(root);
    auto input_centerX = getTrackCenterX(root);
    OnePiece::AIContext::getInstance()->setOPInput(trackCannon, input_trackUltra, input_trackSetDistance, input_trackAutofigh, input_trackAccumulate, input_centerX);
}

std::vector< OnePiece::RewardCandidateItem*>* RewardCandidateItemFromGkid(int gkid, int cluster, int id)
{
    if (cluster == 0){
        if (gkid <= 0) {
            std::vector< OnePiece::RewardCandidateItem*>* rewardVec = new std::vector< OnePiece::RewardCandidateItem*>();
            return rewardVec;
        }
        rapidjson::Document* doc = OnePiece::AIDataManager::getJsonFromFile("_pve_guanqia_hash");
        if (!doc)
            return nullptr;
        std::string gk = OnePiece::stringWithFormat("%d", gkid);
        rapidjson::Value& json_object = doc->operator[](gk.c_str())["reward"];
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
        return rewardVec;
    }
    else {
        if (gkid <= 0) {
            std::vector< OnePiece::RewardCandidateItem*>* rewardVec = new std::vector< OnePiece::RewardCandidateItem*>();
            return rewardVec;
        }
        rapidjson::Document* doc = OnePiece::AIDataManager::getJsonFromFile("_pve_guanqia_character_reward");
        if (!doc)
            return nullptr;
        //, "10512":{"1":{"1":{"reward":[{"i":[{"i":90003, "a" : 1, "r" : 100}], "a" : 50, "r" : 100, "icon" : [70001], "q" : 4, "n" : "½ð±Ò", "d" : "½ð±Ò", "t" : 100}, { "i":[{"i":109008, "a" : 1, "r" : 100}], "a" : 1, "r" : 10, "icon" : [59003], "q" : 4, "n" : "Ä¾Ö®Æ¤¸ïËéÆ¬", "d" : "Ä¾Ö®Æ¤¸ïËéÆ¬", "t" : 100 }, { "i":[{"i":108409, "a" : 1, "r" : 100}], "a" : 1, "r" : 10, "icon" : [58003], "q" : 4, "n" : "Ä¾Ö®²ÊÖéËéÆ¬", "d" : "Ä¾Ö®²ÊÖéËéÆ¬", "t" : 100 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }], "hit_reward" : [{"i":0, "a" : 0, "r" : 0}, { "i":0, "a" : 0, "r" : 0 }, { "i":0, "a" : 0, "r" : 0 }]}, "2" : {"reward":[{"i":[{"i":90003, "a" : 1, "r" : 100}], "a" : 50, "r" : 100, "icon" : [70001], "q" : 4, "n" : "½ð±Ò", "d" : "½ð±Ò", "t" : 100}, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }], "hit_reward" : [{"i":0, "a" : 0, "r" : 0}, { "i":0, "a" : 0, "r" : 0 }, { "i":0, "a" : 0, "r" : 0 }]}, "3" : {"reward":[{"i":[{"i":90003, "a" : 1, "r" : 100}], "a" : 50, "r" : 100, "icon" : [70001], "q" : 4, "n" : "½ð±Ò", "d" : "½ð±Ò", "t" : 100}, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, { "i":[], "a" : 0, "r" : 0, "t" : 0 }, {
        std::string gk = OnePiece::stringWithFormat("%d", gkid);
        std::string cl = OnePiece::stringWithFormat("%d", cluster);
        std::string i = OnePiece::stringWithFormat("%d", id + 1);

        std::cout << " ========== gkid: " << gkid << ":" << cl << ":" << i << std::endl;
        rapidjson::Value& j1 = doc->operator[](gk.c_str());
        if (j1.IsNull()) return nullptr;
        if (j1[cl.c_str()].IsNull()) return nullptr;
        if (j1[cl.c_str()][i.c_str()].IsNull()) return nullptr;

        rapidjson::Value& json_object = doc->operator[](gk.c_str())[cl.c_str()][i.c_str()]["reward"];
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
        return rewardVec;
    }
}

int *createAttr(const std::string &str) {
    int * array = nullptr;
    Json::Reader reader;
    Json::Value json_object;
    reader.parse(str, json_object);
    if (json_object.isArray()){
        int arraySize = json_object.size();
        if (arraySize < ATT_NUM)
            arraySize = ATT_NUM;
        array = new int[arraySize];
        memset(array, 0, arraySize * sizeof(int));
        for (int i = 0; i < json_object.size(); ++i){
            array[i] = (int)json_object[i].asInt();
        }
    }

    return array;
}

int getFirstClusterCenterX(Json::Value& root) {
    auto &json_value = root["operation"]["centerx"];
    if (json_value.isArray() && json_value.size() > 0) {
        return json_value[0u].asInt();
    }
    return 0;
}

void printResult(int winlose) {

    std::cout << "records:" << std::endl;
    std::cout << "winlose:" << winlose << std::endl;

    OPBattleLog("records:");
    OPBattleLog("winlose", winlose);

    std::cout << "our:" << std::endl;
    OPBattleLog("our:");
    auto context = OnePiece::AIContext::getInstance();
    for (int i = 1; i <= context->current_ClusterNumOur; i++) {
        std::cout << "cluster:" << i << std::endl;
        OPBattleLog("cluster", i);
        auto ours = context->getClusterOrigin(OnePiece::TeamSide::kOur, i);
        for (auto it = ours->begin(); it != ours->end(); ++it) {
            auto hero = *it;
            std::cout << hero->getID() << ", " << hero->getHP() << ", " << hero->getEnergy() << std::endl;
            OPBattleLog(hero->getID(), hero->getHP(), hero->getEnergy());
        }
    }

    std::cout << "their:" << std::endl;
    OPBattleLog("their:");
    for (int i = 1; i <= context->current_ClusterNumEnemy; i++) {
        std::cout << "cluster:" << i << std::endl;
        OPBattleLog("cluster", i);
        auto their = context->getClusterOrigin(OnePiece::TeamSide::kEnemy, i);
        for (auto it = their->begin(); it != their->end(); ++it) {
            auto hero = *it;
            std::cout << hero->getID() << ", " << hero->getHP() << ", " << hero->getEnergy() << std::endl;
            OPBattleLog(hero->getID(), hero->getHP(), hero->getEnergy());
        }
    }
}

void addCaptains(std::map<int, Json::Value > &captains, int k, int bkey) {
	for (auto iter = captains.begin(); iter != captains.end(); ++iter) {
		int cluster = iter->first;
		auto &&captainValue = iter->second;
		int *attrList = createAttr(captainValue["attr"].toStyledString());
		//if (attrList[11] <= 0)
		//    continue;
		std::list<OnePiece::CaptainSeqSkillInfo> seqList;
		for (unsigned int j = 0; j < captainValue["seq"].size(); j++) {
			auto &&seq = captainValue["seq"][j];
			OnePiece::CaptainSeqSkillInfo info(j, seq["t"].asInt(), seq["n"].asInt(), seq["i"].asInt(), seq["lv"].asInt());
			auto &&es = seq["e"];
			if (es.isArray()) {
				for (int idx = 0; idx < es.size(); idx++) {
					info.castIDs.push_back(es[idx].asInt());
				}
			}
			seqList.push_back(info);
		}

		int juexing = captainValue["x"].asInt();
		int type = 1;
		int theK = (k == cluster) ? 1 : 0;
		//short Ultralock = (bkey>10) ? 0 : 1;
		//Ultralock = (cluster == 0) ? 1 : Ultralock;
		short Ultralock = 1;
		OnePiece::TeamSide side = OnePiece::TeamSide::kInvalid;
		int id = OnePiece::AIContext::getInstance()->addCaptain(attrList, seqList, juexing, type, theK, Ultralock, cluster, side);

		OnePiece::CharacterData* chara = OnePiece::AIContext::getInstance()->getCharacterByID(id);
		if (captainValue["bqsl"].isArray()) {
			for (int j = 0; j < captainValue["bqsl"].size(); j++) {
				for (int l = 0; l< captainValue["bqsl"][j].size(); l++) {
					auto &&bqslValue = captainValue["bqsl"][j][l];
					int bqi = bqslValue["i"].asInt();
					int bqlv = bqslValue["lv"].asInt();
					chara->addTalent(bqi, bqlv);
				}
			}
		}
	}
}
