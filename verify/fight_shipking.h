#pragma once

#include <iostream>

extern int shipKingScore;
extern int shipKingCurrentWaveCount;

// ·µ»Ø·ÖÊý
int fightShipKingPveVerify(const std::string& str);

void SKMonsterDead(int monsterID, bool isFatal, int waveNum);
void SKMonsterOnHit(int monsterID, int waveNum);
void SKOnSpawnedEnemyCluster(int spawnedCount);
void SKOnCannonExploded();
