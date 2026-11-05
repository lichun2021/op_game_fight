#pragma once

#include <iostream>
#include <map>

extern std::pair<int, std::map<int, int>* > fight(const std::string& str, bool useTestSeed = false, int testSeed = 0);
extern std::pair<int, std::map<int, int>* > fightDouji(const std::string& str, const int seq, std::map<int, int>* HPMAP);
