#pragma once

#include <vector>
#include <iostream>
#include <map>

namespace OnePiece {
    struct RewardCandidateItem;
}

std::vector<OnePiece::RewardCandidateItem*>* AlliancePveRewardCandidateItemFromGkid(int gkid, int cluster, int id);
std::pair<int, std::pair<std::map<int, int>*, std::vector<std::vector<std::vector<std::vector<int> > > >* > > fightAlliancePveNew(const std::string& str);
