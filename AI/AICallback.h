//
//  AICallback.h
//  OnePiece
//
//  Created by Beileng Shan on 4/9/14.
//
//

#ifndef __OnePiece__AICallback__
#define __OnePiece__AICallback__

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

namespace OnePiece {

    typedef std::unordered_map<std::string, int> InputTrackAccumulate;
    typedef std::shared_ptr<InputTrackAccumulate> InputTrackAccumulatePtr;
    typedef std::vector<std::pair<int, int>> TrackAccumulate;
    struct TrackCannonData;
    typedef std::shared_ptr<TrackCannonData> TrackCannonDataPtr;
    typedef std::vector<TrackCannonDataPtr> InputTrackCannon;
    typedef std::shared_ptr<InputTrackCannon> InputTrackCannonPtr;

    enum class AICallbackCharacterState {
        kAccumulate = 1, // 蓄力状态
    };

	class AICallback
	{

	public:
		AICallback();
		virtual ~AICallback() {}

		//hook to interval update
		virtual void startSchedule() = 0;
		virtual void stopSchedule(int value, int x, int y, int z) = 0;
		virtual void resumeSchedule() = 0;
		//set playSpeed
		virtual void playSpeed(int multiple) = 0;
		//notify fighting result, return win=1 or lose=2
		virtual void fightingComplete(int winLose, std::vector<std::pair<int, int>>* rewardArray) = 0;
		virtual void monsterDeadReward(int monsterID, int x, int y, int z, std::vector<std::pair<int, int>>* rewardArray, bool isFatal/*致命一击*/, int waveNum) = 0;
        virtual void onHeroDead(int heroID, int x, int y, int z) = 0;
        virtual void monsterOnHit(int monsterID, int waveNum) = 0;
        virtual void spawnedEnemyCluster(int spawnedCount) = 0;
        virtual void onCreateProjectileForWarship() = 0;
		//to remove some characters
		virtual void removeCharacters(int *idlist) = 0;
		virtual void fixCamera(int x, int y, int z) = 0;

        //virtual void operationTrack(int* trackCannon, std::vector<std::pair<int, int>>& trackUltra, std::vector<int>& trackSetDistance, std::vector<std::pair<int, int>>& trackAutofight, TrackAccumulate& trackAccumulate) = 0;
        virtual void operationTrack(std::vector<TrackCannonDataPtr>& trackCannon, std::vector<std::pair<int, int>>& trackUltra, std::vector<int>& trackSetDistance, std::vector<std::pair<int, int>>& trackAutofight, TrackAccumulate& trackAccumulate, std::vector<int>& trackCenterX) = 0;

        virtual void characterEnterState(int characterUID, AICallbackCharacterState state, int skillID) = 0;
        virtual void characterExitState(int characterUID, AICallbackCharacterState state) = 0;

        virtual void characterDidCastUltraSkill(int characterUID, int staticDataID, int teamSide, int ultraType, int postertype, int altposternum) = 0;
        
        virtual void captainCastSkill(int captainUID, int staticDataID, int teamSide, int skillID) = 0;
		virtual void captainCastBeginSkill(int captainUID, int characterID, int teamSide, int skillID, bool isProtective) = 0;
        virtual void sceneAnimation(const std::vector<std::unordered_map<std::string, int>> &anims) = 0; // cuid:characteruid, cid:characterid, eid:specialexecid

        virtual void sceneBlur(int showBlue) = 0;

        virtual void playSkillAct(int skillID,int heroID,bool isBigSkill) = 0;

        virtual void tempUpdateStop(int time) = 0;

	};

}

#endif /* defined(__OnePiece__AICallback__) */
