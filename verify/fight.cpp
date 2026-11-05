#include "fight.h"
#include <chrono>
#include "../json/document.h"
#include "../jsonx/json.h"
#include "../AI/AIContext.h"
#include "../AICallbackServer.h"
#include "../rankserialization.h"
#include "verifyhelper.h"
#include "../AI/action_hero/HeroSkillController.h"

//void addCaptains(std::map<int, Json::Value > &captains, int k, int bkey) {
//    for (auto iter = captains.begin(); iter != captains.end(); ++iter) {
//        int cluster = iter->first;
//        auto &&captainValue = iter->second;
//        int *attrList = createAttr(captainValue["attr"].toStyledString());
//        //if (attrList[11] <= 0)
//        //    continue;
//        std::list<OnePiece::CaptainSeqSkillInfo> seqList;
//        for (unsigned int j = 0; j < captainValue["seq"].size(); j++) {
//            auto &&seq = captainValue["seq"][j];
//            OnePiece::CaptainSeqSkillInfo info(j, seq["t"].asInt(), seq["n"].asInt(), seq["i"].asInt(), seq["lv"].asInt());
//            auto &&es = seq["e"];
//            if (es.isArray()) {
//                for (int idx = 0; idx < es.size(); idx++) {
//                    info.castIDs.push_back(es[idx].asInt());
//                }
//            }
//            seqList.push_back(info);
//        }
//            
//        int juexing = captainValue["x"].asInt();
//        int type = 1;
//        int theK = (k == cluster) ? 1 : 0;
//        short Ultralock = (bkey>10) ? 0 : 1;
//        Ultralock = (cluster == 0) ? 1 : Ultralock;
//        OnePiece::TeamSide side = OnePiece::TeamSide::kInvalid;
//        int id = OnePiece::AIContext::getInstance()->addCaptain(attrList, seqList, juexing, type, theK, Ultralock, cluster, side);
//
//        OnePiece::CharacterData* chara = OnePiece::AIContext::getInstance()->getCharacterByID(id);
//        if (captainValue["bqsl"].isArray()) {
//            for (int j = 0; j < captainValue["bqsl"].size(); j++){
//                for (int l = 0; l< captainValue["bqsl"][j].size(); l++){
//                    auto &&bqslValue = captainValue["bqsl"][j][l];
//                    int bqi = bqslValue["i"].asInt();
//                    int bqlv = bqslValue["lv"].asInt();
//                    chara->addTalent(bqi, bqlv);
//                }
//            }
//        }
//    }
//}

std::pair<int, std::map<int, int>* > fight(const std::string& str, bool useTestSeed, int testSeed) {
    try
    {
        auto time_start = std::chrono::high_resolution_clock::now();
        char buff[20];
        Json::Reader reader;
        Json::Value root;
        reader.parse(str, root);
        int seed = root["seed"].asInt();
        if (useTestSeed)
            seed = testSeed;
        //printf(str);
        printf("seed: %d\n", seed);
        int k = root["k"].asInt();
        int bkey = root["bkey"].asInt();
        int fightingtime;
        if (&root["ftime"] == nullptr){
            fightingtime = 90;
            printf("aaaaaaaaa: %d\n", fightingtime);
        }
        else{
            fightingtime = root["ftime"].asInt();
            if (!fightingtime) fightingtime = 90;
            printf("bbbbbbbbb: %d\n", fightingtime);
        }
        printf("fightingtime: %d\n", fightingtime);
        printf("k: %d\n", k);
        printf("team size %d\n", root["teams"].size());
        OnePiece::AIContext::getInstance()->clear();
        addCannon();
        OnePiece::AIContext::getInstance()->setRewardSeed(seed);
        int clusterNum = root["teams"].size() - 1;
        int gkid = root["gkid"].asInt();
        bool fallback20151010 = hasFlagForFixVerifyReward(root);
        std::map<int, Json::Value > captains; // <cluster, captain>
        for (int cluster = 0; cluster <= clusterNum; cluster++){
            for (int i = 0; i < root["teams"][cluster].size(); i++) {
                auto &&characterValue = root["teams"][cluster][i];
                if (characterValue["captain"].asInt() == 1) {
                    captains[cluster] = characterValue;
                    continue;
                }
                int *attrList = createAttr(characterValue["attr"].toStyledString());
                if (attrList[11] <= 0) continue;
                std::list<std::pair<int, int> > skillList;
                for (int j = 0; j < characterValue["j"].size(); j++){
                    skillList.push_back(std::pair<int, int>());
                    auto &&skillValue = characterValue["j"][j];
                    skillList.back().first = skillValue["i"].asInt();
                    skillList.back().second = skillValue["lv"].asInt();
                }
                int juexing = characterValue["x"].asInt();
                int type = 1;
                int theK = (k == cluster) ? 1 : 0;
                short Ultralock = (bkey>10) ? 0 : 1;
                Ultralock = (cluster == 0) ? 1 : Ultralock;
                int fp = characterValue["fp"].asInt();
                int id = OnePiece::AIContext::getInstance()->addHeroCharacters(attrList, skillList, juexing, type, theK, Ultralock, cluster, OnePiece::TeamSide::kInvalid, fp);

                OnePiece::CharacterData* chara = OnePiece::AIContext::getInstance()->getCharacterByID(id);
                if (characterValue["bqsl"].isArray()) {
                    for (int j = 0; j < characterValue["bqsl"].size(); j++){
                        for (int l = 0; l< characterValue["bqsl"][j].size(); l++){
                            auto &&bqslValue = characterValue["bqsl"][j][l];
                            int bqi = bqslValue["i"].asInt();
                            int bqlv = bqslValue["lv"].asInt();
                            chara->addTalent(bqi, bqlv);
                        }
                    }
                }

                int monsterId = characterValue["i"].asInt();
                if (cluster > 0 && gkid > 0){
                    std::vector<OnePiece::RewardCandidateItem*>* monster_reward = RewardCandidateItemFromGkid(gkid, cluster, monsterId);

                    if (monster_reward != nullptr) {
                        printf("%d,%d,%d\n", cluster, id, monster_reward->size());
                        if (fallback20151010) // fix verify for different 3.0 client
                            sprintf(buff, "%d-%d-0", cluster, id);
                        else
                            sprintf(buff, "%d-%d", cluster, id);
                        OnePiece::AIContext::getInstance()->setRewardCandidate(monster_reward, buff, cluster);
                    }
                }

            }
        }
        printf("addCaptains");
        // ��������ͨ��ɫ֮�����,��ͻ��˱���һ��
        addCaptains(captains, k, bkey);

        if (gkid > 0){
            // MARK: commented by h7ing
            // TODO: !!! NEED TEST !!!
            //std::vector<OnePiece::RewardCandidateItem*>* gkreward = RewardCandidateItemFromGkid(gkid, 0,0);
            //if (gkreward != nullptr) {
            //	sprintf(buff, "%d", 1);
            //	OnePiece::AIContext::getInstance()->setRewardCandidate(gkreward, buff, 1);
            //}
        }
        else {
            OnePiece::AIContext::getInstance()->setAutoFight(true);
        }

        OnePiece::AIContext::getInstance()->setBusinessType(bkey);
        OnePiece::AIContext::getInstance()->setGuanqiaID(gkid>0 ? gkid : -1);
        OnePiece::AIContext::getInstance()->setSeed(seed, 1);
        AICallbackServer *callback = new AICallbackServer;
        OnePiece::AIContext::getInstance()->setCallback(*callback);
        float * t1 = new float[1];
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

        std::cout << "minY" << delta_min_y << "maxY" << delta_max_y << std::endl;

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

        auto time_finish = std::chrono::high_resolution_clock::now();
        std::cout << "\nfight delta:" << std::chrono::duration_cast<std::chrono::milliseconds>(time_finish - time_start).count() << " ms" << std::endl;

        return std::pair<int, std::map<int, int>* >(callback->win, callback->rewardVec);
    }
    catch (readException& e)
    {
        return std::pair<int, std::map<int, int>* >(0, nullptr);
    }
};

std::pair<int, std::map<int, int>* > fightDouji(const string& str, const int seq, std::map<int, int>* HPMAP) {
    try
    {
        char buff[20];
        Json::Reader reader;
        Json::Value root;
        reader.parse(str, root);
        int seed = root[seq]["seed"].asInt();
        //printf(str);
        printf("seed: %d\n", seed);
        int k = root[seq]["k"].asInt();
        int bkey = root[seq]["bkey"].asInt();
        printf("k: %d\n", k);
        printf("%d\n", root[seq]["teams"].size());
        OnePiece::AIContext::getInstance()->clear();
        addCannon();
        OnePiece::AIContext::getInstance()->setRewardSeed(seed);
        int clusterNum = root[seq]["teams"].size() - 1;
        int gkid = root[seq]["gkid"].asInt();
        bool fallback20151010 = hasFlagForFixVerifyReward(root);
        for (int cluster = 0; cluster <= clusterNum; cluster++){
            for (int i = 0; i < root[seq]["teams"][cluster].size(); i++) {
                auto &&characterValue = root[seq]["teams"][cluster][i];
                int *attrList = createAttr(characterValue["attr"].toStyledString());

                if (HPMAP != nullptr) {
                    std::map<int, int>::iterator it = HPMAP->find(attrList[0]);
                    if (it != HPMAP->end()) {
                        attrList[11] = it->second;
                    }
                }
                if (attrList[11] <= 0) continue;
                std::list<std::pair<int, int> > skillList;
                for (int j = 0; j < characterValue["j"].size(); j++){
                    skillList.push_back(std::pair<int, int>());
                    auto &&skillValue = characterValue["j"][j];
                    skillList.back().first = skillValue["i"].asInt();
                    skillList.back().second = skillValue["lv"].asInt();
                }
                int juexing = characterValue["x"].asInt();
                int type = 1;
                int theK = (k == cluster) ? 1 : 0;
                short Ultralock = (bkey>10) ? 0 : 1;
                Ultralock = (cluster == 0) ? 1 : Ultralock;
                int fp = characterValue["fp"].asInt();
                int id = OnePiece::AIContext::getInstance()->addHeroCharacters(attrList, skillList, juexing, type, theK, Ultralock, cluster, OnePiece::TeamSide::kInvalid, fp);

                OnePiece::CharacterData* chara = OnePiece::AIContext::getInstance()->getCharacterByID(id);
                if (characterValue["bqsl"].isArray()) {
                    for (int j = 0; j < characterValue["bqsl"].size(); j++){
                        for (int l = 0; l< characterValue["bqsl"][j].size(); l++){
                            auto &&bqslValue = characterValue["bqsl"][j][l];
                            int bqi = bqslValue["i"].asInt();
                            int bqlv = bqslValue["lv"].asInt();
                            chara->addTalent(bqi, bqlv);
                        }
                    }
                }

                int monsterId = characterValue["i"].asInt();
                if (cluster > 0 && gkid > 0){
                    std::vector<OnePiece::RewardCandidateItem*>* monster_reward = RewardCandidateItemFromGkid(gkid, cluster, monsterId);
                    printf("%d,%d,%d\n", cluster, id, monster_reward->size());
                    if (monster_reward != nullptr) {
                        if (fallback20151010)
                            sprintf(buff, "%d-%d-0", cluster, id);
                        else
                            sprintf(buff, "%d-%d", cluster, id);
                        OnePiece::AIContext::getInstance()->setRewardCandidate(monster_reward, buff, cluster);
                    }
                }

            }
        }
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
        float * t1 = new float[1];
        float *t2 = new float[1];
        float *t3 = new float[1];
        t1[0] = 90;
        t2[0] = 90;
        t3[0] = 90;
        OnePiece::AIContext::getInstance()->setWinLoseCheck(t1, 1);
        OnePiece::AIContext::getInstance()->setWinLoseCheck(t2, 2);
        OnePiece::AIContext::getInstance()->setWinLoseCheck(t3, 3);

        int delta_min_y = root[seq]["battleRect"]["y"].asDouble();
        int delta_max_y = root[seq]["battleRect"]["y"].asDouble() + root[seq]["battleRect"]["height"].asDouble();

        std::cout << "minY" << delta_min_y << "maxY" << delta_max_y << endl;

        OnePiece::AIContext::getInstance()->setDeltaY(delta_min_y, delta_max_y);

        addInput(root);	
        OnePiece::AIContext::getInstance()->toNextCluster((int)root[seq]["operation"]["distance"][0u].asInt(), getFirstClusterCenterX(root));
        const auto waveCount = root[seq]["operation"]["distance"].size();
        do{
            OnePiece::AIContext::getInstance()->updateAIContext(0.05);
            if (OnePiece::AIContext::getInstance()->currentWaveCount > waveCount) {
                callback->win = 2;
                callback->ongoing = 0;
                std::cout << std::endl << "verify error!" << std::endl;
            }
        } while (callback->ongoing == 1);

        return std::pair<int, std::map<int, int>* >(callback->win, callback->rewardVec);
    }
    catch (readException& e)
    {
        return std::pair<int, std::map<int, int>* >(0, nullptr);
    }
}
