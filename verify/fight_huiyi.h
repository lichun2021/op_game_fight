// 回忆之战
#pragma once

#include <memory>
#include <vector>
#include <iostream>

typedef std::vector<int> WWHeroInfo;
typedef std::vector<WWHeroInfo> WWTeamInfo;
typedef std::vector<WWTeamInfo> WWFightRecord;
typedef std::vector<WWFightRecord> WWFightRecords;
typedef std::pair<int, WWFightRecord> WWFightResult;

std::shared_ptr<std::vector<int> > GetTowerDefenceWinloseCheckCondition(int hyid);
std::shared_ptr<std::vector< std::vector<int> > > GetTowerDefenceEnemyWaveInfo(int hyid);
std::shared_ptr<std::vector<int> > GetSoloWheelwarFailInheritInfo(int hyid);

WWFightResult fightWWTD(const std::string& str); // 车轮战&塔防
std::pair<int, std::vector<std::vector<std::vector<std::vector<int> > > > >  fightCarWheel(const std::string& str); // 同盟战
