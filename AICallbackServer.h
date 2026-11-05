#pragma once

#include <map>
#include "AI/AICallback.h"
#include "AI/AICharacterData.h"

class  AICallbackServer : public OnePiece::AICallback
{
public:
    int ongoing = 1;
    int win = 0;
    std::vector<std::pair<int, int> >* reward = nullptr;
    std::map<int, int>* rewardVec = new std::map<int, int>();
    std::vector<std::vector<std::vector<std::vector<int> > > >* hpinfoVec = new std::vector<std::vector<std::vector<std::vector<int> > > >();

public:
    AICallbackServer();
    virtual ~AICallbackServer();

    //hook to interval update
    void startSchedule() override;
    void stopSchedule(int value, int x, int y, int z) override;
    void resumeSchedule() override;
    //set playSpeed
    void playSpeed(int multiple) override;
    //notify fighting result, return win=1 or lose=2
    void fightingComplete(int winLose, std::vector<std::pair<int, int>>* rewardArray) override;
    void monsterDeadReward(int monsterID, int x, int y, int z, std::vector<std::pair<int, int>>* rewardArray, bool isFatal, int waveNum) override;
    //to remove some characters
    void removeCharacters(int *idlist) override;
    void fixCamera(int x, int y, int z) override;

    void operationTrack(std::vector<OnePiece::TrackCannonDataPtr>& trackCannon, std::vector<std::pair<int, int>>& trackUltra, std::vector<int>& trackSetDistance, std::vector<std::pair<int, int>>& trackAutofight, OnePiece::TrackAccumulate& trackAccumulate, std::vector<int>& trackCenterX) override;
    
    void startSchedule(int value, int x, int y, int z);
    void stopSchedule();

    virtual void characterEnterState(int characterUID, OnePiece::AICallbackCharacterState state, int skillID) override {}
    virtual void characterExitState(int characterUID, OnePiece::AICallbackCharacterState state) override {}

    virtual void monsterOnHit(int monsterID, int waveNum) override;
    virtual void spawnedEnemyCluster(int spawnedCount) override;
    virtual void onCreateProjectileForWarship() override;
    virtual void characterDidCastUltraSkill(int characterUID, int staticDataID, int teamSide, int ultraType, int postertype, int altposternum) override {}
    void captainCastSkill(int captainUID, int staticDataID, int teamSide, int skillID) override;
    virtual void captainCastBeginSkill(int captainUID, int characterID, int teamSide, int skillID, bool isProtective) override {}
    virtual void sceneAnimation(const std::vector<std::unordered_map<std::string, int>> &anims) override {}
    
    virtual void sceneBlur(int showBlue) {};

    virtual void playSkillAct(int skillID,int heroID,bool isBigSkill) {};

    virtual void tempUpdateStop(int time) {};

    virtual void onHeroDead(int heroID, int x, int y, int z) {};

private:
    void fixTeamHP(std::list<OnePiece::CharacterData *> *characters);
};
