#pragma once

#include <vector>
#include <memory>
#include <string>
#include "../jsonx/json.h"

namespace OnePiece {
    struct RewardCandidateItem;
}

extern bool hasFlagForFixVerifyReward(Json::Value& root);
extern void addCannon();
extern void addCaptains(std::map<int, Json::Value > &captains, int k, int bkey);
extern void addInput(Json::Value& root);
extern std::vector<OnePiece::RewardCandidateItem*>* RewardCandidateItemFromGkid(int gkid, int cluster, int id);
extern int *createAttr(const std::string &str);
extern int getFirstClusterCenterX(Json::Value& root);

extern void printResult(int winlose);
