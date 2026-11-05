//
//  AIContext.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__AIContext__
#define __OnePiece__AIContext__

#include <list>
#include <vector>
#include <iostream>
#include <cmath>
#include <memory>
#include <tuple>
#include <cstdint>

#include "RandomGen.h"
#include "AIProjectile.h"
#include "AIActionController.h"
#include "CommonType.h"
#include "AIContinuousProjectileCaster.h"
#include "AIContinuousEffectCaster.h"
#include "AIContinuousHurtCaster.h"
#include "AIContinuousMoveCaster.h"
#include "AIContinuousMarkCaster.h"
#include "AIContinuousSummonCaster.h"
#include "ExternalDelegateAnimation.h"
#include "ExternalDelegateAudio.h"
#include "ExternalDelegateMarkEffect.h"
#include "ExternalDelegateSpecialEffect.h"
#include "ExternalDelegateSpecialExec.h"
#include "ExternalDelegateCombo.h"
#include "ExternalDelegateSummon.h"
#include "ExternalDelegateTransform.h"
#include "ExternalDelegateBodyEffect.h"
#include "AICallback.h"
#include "AINode.h"
#include "AICharacterData.h"

struct lua_State;

namespace OnePiece {

    // MARK:
    // 增加新的战斗类型，需要更新战斗类型判断和过关回血回能量逻辑
	enum BusinessType {
		PVP_WANTED=1,
		PVP_FRIEND=2,

        PVE_GONGHUI=3,
		PVE_GUANQIA = 11,
		PVE_ISLAND_CRUISE = 12,
		PVE_CALENDAR = 13,

        WHEELWAR_SOLO = 21,
        WHEELWAR_GROUP = 22,

        TOWER_DEFENCE_WAVE = 31,
        TOWER_DEFENCE_TIME = 32,

        CANNON_KING = 41
	};

    extern bool isBusinessPVP(int business);
    extern bool isBusinessPVE(int business);
    extern bool isBusinessTowerDefence(int business);

    enum AIContextTeamListType {
        AITEAM_ORIGIN = 0,
        AITEAM_FRIENDS_FOR_CLUSTER = 1,
        AITEAM_RIVALS_FOR_CLUSTER = 2
    };

    // 过节点属性继承
    struct AttributeInheritanceInfo {
        int attribute = -1;
        int ratio = 0;
        int limit = 0;
    };
    typedef std::shared_ptr<AttributeInheritanceInfo> AttrInheritInfoPtr;
    typedef std::vector<AttrInheritInfoPtr> AttrInheritInfos;
    typedef std::shared_ptr<AttrInheritInfos> AttrInheritInfosPtr;

    // 过节点属性加强
    struct AttrbuteEnhanceInfo {
        int attribute = -1; // 约定: >=0 对应单一属性类型; -1 对应所有战斗属性(除id，坐标，能量)
        int scaleRatio = 0;
    };
    typedef std::shared_ptr<AttrbuteEnhanceInfo> AttrEnhanceInfoPtr;
    typedef std::vector<AttrEnhanceInfoPtr> AttrEnhanceInfos;
    typedef std::shared_ptr<AttrEnhanceInfos> AttrEnhanceInfosPtr;

    typedef std::pair<int, AttrEnhanceInfosPtr> EnemyWaveInfo; // 队伍编号, 属性加强
    typedef std::shared_ptr<EnemyWaveInfo> EnemyWaveInfoPtr;
    typedef std::vector<EnemyWaveInfoPtr> EnemyWaveInfos;
    typedef std::shared_ptr<EnemyWaveInfos> EnemyWaveInfosPtr;

    struct AIBattleStats;
    typedef std::shared_ptr<AIBattleStats> AIBattleStatsPtr;
    typedef std::unordered_map<int, AIBattleStatsPtr> StatsContainer;

    class AICannonKing;
    typedef std::shared_ptr<AICannonKing> AICannonKingPtr;

    struct TrackCannonData {
        int trackTick = -1;
        int projectileID = 0;
        int characterid = 0;
        int sourceX = 0;
        int sourceY = 0;
        int sourceZ = 0;
        int targetX = 0;
        int targetY = 0;
        int targetZ = 0;
        int skillLevel = 0;
    };
    typedef std::shared_ptr<TrackCannonData> TrackCannonDataPtr;

    //cui 2023_2_23 延时执行数据结构
    struct DelayAIActionData {
        int deleteTick = 0;
        int execID = 0;
        int skillID = 0;
        int skillLevel = 0;
        CharacterData* source = NULL;
        AIProjectile* projectile = NULL;
        CharacterData* target = NULL;
        int parentTicklen = 0;
        bool useLogicPos = false;
        int markID = 0;
        bool isproj12reachexec = false;
        int delayTick = 0;
        int hurtType1 = 0; //伤害类型 0 物理 1 魔法
        int hurtType2 = 0;
    };

	class AIContext : public cocos2d::Ref
	{
	public:
		AICallback *aiCallback;

		int local_incremential;
		int current_ClusterNumEnemy;//0=Invalid, 1=wave#1,2=wave#2,3=wave#3
        int current_ClusterNumOur;//0=Invalid,1, 2, 3
        short currentFriendK = 0;
        short currentRivalK = 0;
		int friend_num[3], rival_num[3];
		int battleFieldCenterX;
		int adjustTick;
        int clusterRangeToCenterX;
        int currentWaveCount;
        bool pauseAIFalg = false;
        int64_t currentClusterHPTopOur = 0;
        int64_t currentClusterHPTopEnemy = 0;
		
		//int ultraFreeze[] = { 0, 0, 0, 0};//Countdown, exception_characterID, ifFreezeScreen, ifFreezeUltra
		std::list<int*> ultraFreezeList; //OpCountdown, EffCountdown, myID, myCluster, startGlobalTick, freezeTime, teamSide
		//大招锁屏,每个人的动作都静止,但是考虑到伤害执行,动作可以进入,但不执行。

        //cui 2023_2_23 延时执行 行为列表 delayAIAction
        std::list<DelayAIActionData*> dalayAIActionList;

		//int trackCannon[30];//tick, int projectileID, int characterid, int sourceX, int sourceY, int sourceZ, int targetX, int targetY, int targetZ, int skillLevel
        std::vector<TrackCannonDataPtr> trackCannons;
		//int trackCannon_index;
		//std::vector<int>* input_trackCannon; 
        InputTrackCannonPtr input_trackCannons;
		int input_trackCannon_index;
		std::vector<std::pair<int, int>> trackUltra;//tick,characterID
        std::unordered_map<std::string, int>* input_trackUltra;//<tick-characterID>
        TrackAccumulate trackAccumulate;
        InputTrackAccumulatePtr input_trackAccumulate; //蓄力技能手动释放
		//int trackSetDistance[6];//distance for max 3 wave
        std::vector<int> trackSetDistance;
		std::vector<int>* input_trackSetDistance;//distance for each wave

		std::vector<std::pair<int, int>> trackAutofight;//tick,0/1
        std::unordered_map<int, int>* input_trackAutofight;//tick,0/1
        std::unordered_map<int, int> comboConstraints;//<skillID,1> 组合技

        std::vector<int> trackCenterX;
        std::shared_ptr< std::vector<int> > input_trackCenterX = nullptr;

		int businessType;

		RandomGen* localRandom;
        RandomGen* rewardRandom;

		//CharacterDataListPtr currentOurCluster;
		std::list<AIProjectile*> projectileList;

        struct cmp_str
        {
            bool const operator()(char const *a, char const *b) const
            {
                return strcmp(a, b) < 0;
            }
        };
        std::map<char*, std::vector<RewardCandidateItem*>*, cmp_str> reward_candidates; //key: enemy_waveID(-MonsterID)

        typedef int ClusterNumber;
        typedef std::unordered_map<ClusterNumber, CharacterDataListPtr> Clusters;
        Clusters clustersEnemy;
        Clusters clustersOur;
        CharacterData *captainOur;
        CharacterData *captainEnemy;

		////针对魅惑 和 等等可能会改变攻击对象阵营的状态 使得战斗时候真正确定目标的list是可以变化的
		////同时需要管理内容成员
        //Clusters clustersRealEnemy;
        //Clusters clustersRealOur;

        std::unordered_map<unsigned short, float* > winlosecheck_clusters;

        std::unordered_map<unsigned short, CharacterData*> allCharacters;

		long currentTick;
		long length;
		long deltaMaxY;
        long deltaMinY;
		int myPlaySpeed;

		short autoFight;
		bool monsterCanMove;
		bool isFighting;
		int adjustingDistance;

        /// OP2
        int asyncMeleeCombatCnt = 0;
        bool stopAsyncMeleeCombatFlag = false;

        MeleeCombatResult lastCombatResult;
        std::string lastClientOperationTrack;

        bool isEnterIdleInUIActionAttack = false;

        std::map<int, std::map<int, std::map<int, int>>> castersToRemove; // <CharacterData::myID, <casterType,<casterID, int>>>
        std::map<int, std::map<int, int>> hurtCastersToRemove; // <CharacterData::myID,<casterID, int>>

        std::vector<std::unordered_map<std::string, int>> sceneAnims;
        ///
        //cui_2023_3_14  暂停所有动画，包括逻辑帧
        //int realStopConut;

		ExternalDelegateAnimation* animationProxy;
		ExternalDelegateAudio* audioProxy;
		ExternalDelegateSpecialEffect* seffProxy;
		ExternalDelegateSpecialExec* sexecProxy;
		ExternalDelegateCombo* comboProxy;
        ExternalDelegateMarkEffect* markProxy;
        ExternalDelegateSummon* summonProxy;
        ExternalDelegateTransform* transformProxy;
        ExternalDelegateBodyEffect* bodyEffectProxy;
		inline void setProxy(
            ExternalDelegateAnimation* animationProxy1, 
            ExternalDelegateAudio* audioProxy1, 
            ExternalDelegateSpecialEffect* seffProxy1, 
            ExternalDelegateSpecialExec* sexecProxy1, 
            ExternalDelegateCombo* comboProxy1, 
            ExternalDelegateMarkEffect* markProxy1, 
            ExternalDelegateSummon* summonPro,
            ExternalDelegateTransform* transformProxy1,
            ExternalDelegateBodyEffect* bodyEffectProxy1)
		{
			animationProxy = animationProxy1;
			audioProxy = audioProxy1;
			seffProxy = seffProxy1;
			sexecProxy = sexecProxy1;
			comboProxy = comboProxy1;
            markProxy = markProxy1;
			summonProxy = summonPro;
            transformProxy = transformProxy1;
            bodyEffectProxy = bodyEffectProxy1;
		}

	public:
		static AIContext* getInstance();

        /// OP2
        void fightAsyncUntilWinlose();
        static void fightAsyncHelper();
        void startFightAsync(const std::string &battlelog);
        static void verifyMeleeCombatByClient();
        std::unique_ptr<MeleeCombatBattleInfo> meleeCombatBattleInfo;
        ///

		void clear();
		void clearWave(bool bCelebrate); int clearWave_Countdown; int winlose;
        void toNextCluster(int adjust, int clusterCenterX);
        CharacterDataListPtr getClusterOrigin(TeamSide teamSide, int clusterNum);
        //CharacterDataListPtr getClusterReal(TeamSide teamSide, int clusterNum);

        CharacterDataListPtr getFriendsForCluster(TeamSide side, int clusterNum, bool excludeSummon = false);
        CharacterDataListPtr getRivalsForCluster(TeamSide side, int clusterNum, bool excludeSummon = false);

        bool isCurrentLastClusterForTeam(TeamSide side);
		
		std::vector<std::pair<int, int>>* getPveGuanqiaReward(std::vector<RewardCandidateItem*>* rewards);


		CharacterDataList m_summonCharacterList;

		int calculateBattleFieldCenterX();
        int getAdjustedPosXWithinRange(int originPosX);
        int getAdjustedPosYWithinRange(int originPosY);
        int getBattleFieldXMostLeft();
        int getBattleFieldXMostRight();
        int getBattleFieldCenterX() const { return battleFieldCenterX; }

        void setServerVerifyClusterAttrInheritInfos(const AttrInheritInfosPtr& pInfos);
        void setServerVerifyEnemyWaveInfos(const EnemyWaveInfosPtr& pInfos);
        void setOPInput(
            //std::vector<int>* trackCannon, 
            InputTrackCannonPtr input_trackCannon,
            std::unordered_map<std::string, int>* input_trackUltra, 
            std::vector<int>* input_trackSetDistance, 
            std::unordered_map<int, int>* input_trackAutofight,
            InputTrackAccumulatePtr input_trackAccumulate,
            std::shared_ptr< std::vector<int> > input_trackCenterX);
		void setAutoFight(short autoF)
		{ 
			autoFight = autoF;
			trackAutofight.push_back(std::pair<int, int>(currentTick, autoF));
		}
		void setSeed(int seed, int clusterNum);
        void setRewardSeed(int seed);
		void setRewardCandidate(std::vector<RewardCandidateItem*>* rclist, char* thekey, int clusterNum);
		//conditionList[0]=timeElapse, if conditionList[0]=0, no time constraints
		//conditionList[1..n] monsterID to be killed to meet the win condition
		void setWinLoseCheck(float *conditionList, int clusterNum);
		void setCallback(AICallback &aiCallback);
		//void init(AICallback &aiCallback, int *conditionList, int clusterNum)
		void updateAIContext(float dt);
		void setGuanqiaID(int guanqiaID);
		void setLength(long length);
		void setDeltaY(long minY = 0,long maxY = 0);
        long getDeltaMinY();
        long getDeltaMaxY();
		void playSpeed(int multiple);
		int playSpeed();
		inline void setBusinessType(int bt) { businessType = bt; }

        int addHeroCharacters(int* attList, std::list<std::pair<int, int>>& skillList, int juexing, int type, short k, short ifUltraLock, int clusterNum, TeamSide teamSide, int fp);
        int addCaptain(int* attList, std::list<CaptainSeqSkillInfo> &seqList, int juexing, int type, short k, short ifUltraLock, int clusterNum, TeamSide teamSide);

        // TODO: optimise
        bool checkHeroCharacterHaveSummonSkill(CharacterData *pCharacterData,std::vector<SummonInfo*>& vSummonCharacterIds);
        //bool checkTargetSummonIsDead(CharacterData *pCharacterData,int iTargetSummonId);
        
		CharacterData* getCharacterByID(int id);
		AIProjectile* getProjectileByID(int id);

        CharacterData* getCharacterByTagValue(const std::string &tag, int tagval); // OP2

		const char *jsonString();
		void printAIContext();

		void createProjectileForWarship(int projectileID, int characterid, int sourceX, int sourceY, int sourceZ, int targetX, int targetY, int targetZ, int skillLevel);
        
        void getSummonSkillList(SummonProperty * pSummonProperty,CharacterData *pSource,std::list<std::pair<int,int>>& skillList,int summonSkillLv);
        void getSkillAttr(std::list<std::pair<int, int>>& skillList,int* tskill_attr);
        int* initSummonCharacterAttrFromConfig(int characterid,int iLv,int juexingLv,int jinjie,std::list<std::pair<int, int>>& skillList);

        StatsContainer& getBattleStats();

        // 失败后属性继承
        void setClusterAttrInheritInfos(const AttrInheritInfosPtr& pInfos);

        // 过节点属性加强
        void setClusterAttrEnhanceInfos(const AttrEnhanceInfosPtr& pInfos);

        void setNextEnemyClusterNumber(int nextEnemyClusterNum);

		// 判断一下角色是否在屏障有效范围内
		bool getOwnerIsInVaildBarrier(CharacterData* theOwner);
		// 某一角色死亡，借此判断整个角色列表中是否存在标记（标记执行释放条件：达到层数，有人死亡触发。）
		void onMarkReleaseFun(); // OP2 deprecated
		// 某一角色死亡，借此判断整个角色列表中 标记施法者死亡后清除 clair add by 11.0 version
		void onCharacterDeadIfMarkClearFun(CharacterData *theCharacter); // OP2 deprecated

        AICannonKingPtr getCannonKing() const { return _pCannonKing; }
        void setCannonCount(int count);
        void setCannonKingConfig(int normalWaveTick, int bossWaveTick, int bossWaveClusterNumInterval);
        int getCharacterWaveNum(int uid);

        int getFriendOrRivalAliveCount(TeamSide side, int clusterNum, bool isFriend, bool excludeSummon = false);

        void scheduleNotifyUIEff(int scheduleTick, int parentExecMyID);
        void scheduleExcuteExec(const ExecExcuteInfo &execInfo);
        void scheduleExcute(
            int theExecID, 
            int theSkillID, 
            int theSkillLevel,
            CharacterData* theSource,
            AIProjectile* theProjectile,
            CharacterData* theTarget,
            int theParentTicklen,
            bool theUseLogicPos = false,
            int theWaitTickLen = 0);

        void onEvent(const std::string &name, CharacterData *pusher);
        void addEventListener(const std::string &name, CharacterData *listener);
        void removeEventListener(const std::string &name, CharacterData *listener);

        void addNoDieEffCharacter(int characterMyID);
        void removeNoDieEffCharacter(int characterMyID);
        const std::map<int/*myID*/, int/*count*/> &getIgnoreNoDieEffCharacters() const { return _ignoreNoDieCharacters; }

        void onCharacterLoseHP(int hplose, CharacterData *theCharacter);
        void addTeamHPLoseListener(int theTeamSide, int theMyId);
        void invalidateTeamHPLoseListenerFromBegin(int theTeamSide, int theMyId);

        bool hasWinlose() { return _hasWinlose; }
        
        void stopUltraFreeze(int theCharacterMyId);
        
        lua_State *getLuaState() { return _luaState; }

        /// 12.0
        bool isBeingFinalClear() { return _isBeingFinalClear; }
        bool shouldStopExecWhenBeingFinalClear();
        ///

        /// 13.0
        void setBattleMirroredForPlay(bool isBattleMirroredForPlay);
        bool isBattleMirroredForPlay();
        void setUnitHealthBarColorMirrored(bool isUnitHealthBarColorMirrored);
        bool isUnitHealthBarColorMirrored();
        void setUnitHealthBarTypeMirrored(bool isUnitHealtBarTypeMirrored);
        bool isUnitHealtBarTypeMirrored();
        ///

        /// 14.0
        CharacterDataListPtr getTeamList(int teamlistType, int teamside, int clusternum, bool isExcludeSummon);
        void addCharacterToCluster(CharacterData *c, int teamlistType, int teamside, int clusternum, bool isExcludeSummon);
        void removeCharacterFromCluster(CharacterData *c, int teamlistType, int teamside, int clusternum, bool isExcludeSummon);

        void checkUpdateTeamAliveInfo(int side, int clusterNum);
        void setTeamAliveInfo(int side, int clusterNum, int aliveCnt, int deathTriggerCnt);
        void getTeamAliveInfo(int side, int clusterNum, int &outAliveCnt, int &outDeathTriggerCnt);
        int getTeamAliveNumExcludeAllSummon(int side);

        void deleteCaster(int characterMyId, int casterType, int casterMyId);
        bool isToDeleteCasterInfoExisted(int casterMyId);
        void addToDeleteCasterInfo(int characterMyId, int casterType, int casterMyId);
        void cleanupToDeleteCasterInfo();
        void checkDeleteToDeleteCasters();

        void setSubjectiveTeamside(int side);
        int getSubjectiveTeamside();
        ///

        /// 15.0
        void clearLastCombatResult();
        ///

        //cui_2023_3_14  暂停所有动画，包括逻辑帧
//        void setRealStopConut(int num) { realStopConut = num; };
        void tempUpdateStopOver();

        void setChapterElementAtkLabel();
        void commonSetAtkLabelFunc(TeamSide side);

	private:
		AIContext();
		~AIContext();
        void implUpdate();
        void checkAddHeroSummonCharacter(CharacterData* hero, int type, TeamSide teamSide);
        void preAddTransformCharacter(CharacterData* hero);
		//void removeRivals(int clusterNum);
		void restoreEnergyAndHP(std::list<CharacterData*>* heros);
        void inheritAttributes(TeamSide side, int lastClusterNum, int currentClusterNum);
        void enhanceHero(CharacterData* hero);
        bool isBattleFinished(int wl); // 整场战斗结束
        void spawnCannonKingEnemy();
        void initRivalsRealForCluster(TeamSide side, int clusterNum, CharacterDataListPtr pOriginRivals);
        void updateCluster(CharacterDataListPtr pCharacterList);
        void releaseSkillOnBegin(CharacterDataListPtr pCharacterList);
        void releaseSkillOnBeginByProtectiveProperty(CharacterDataListPtr pCharacterList, bool isProtective, int lazyTick);
		static AIContext* isme;

        CharacterData* cannon = nullptr;
        AICannonKingPtr _pCannonKing = nullptr;
        
        lua_State *_luaState = nullptr;
        bool _isLuaReady = false;
        int initLua();

		int fixCamera;
		int guanqiaID;
		int afterwarCountdown;
		int serverToNext = -1;
        int nextClusterNumEnemy;
        int passedClusterCount;

        bool _hasWinlose = false;
        
        AttrInheritInfosPtr clusterAttrInheritInfos;
        AttrEnhanceInfosPtr clusterAttrEnhanceInfos;

        AttrInheritInfosPtr _serverVerifyAttrInheritInfos = nullptr;
        EnemyWaveInfosPtr _serverVerifyEnemyWaveInfos = nullptr;

        enum {
            kTupleIndexFriends = 0,
            kTupleIndexFriendsNoSummon,
            kTupleIndexRivals,
            kTupleIndexRivalsNoSummon
        };
        typedef std::tuple<CharacterDataList, CharacterDataList, CharacterDataList, CharacterDataList> FriendsRivals;
        std::map<TeamSide, std::map<int, FriendsRivals>> _allFriendsRivals; //每个队伍各自的友军和敌军

        std::unordered_map<int, std::vector<int> > _scheduleNotifyUIEffs;

        std::vector<ExecExcuteInfo> _scheduleExcuteExecs;
        std::vector<ExecExcuteInfo> _appendedExcuteExecs;

        std::unordered_map<std::string, std::vector<int> > _eventListeners; // < eventname, [heromyid...] >
        //std::unordered_map<std::string, std::unordered_map<int,bool> > _listenersTobeRemoved;

        std::map<int/*myID*/, int/*count*/> _ignoreNoDieCharacters;
        std::map<int, std::list<int> > _hploseListeners; //<teamside, <myid>>

        int _inputBattleFieldCenterX = 0;
        int _toNextClusterCnt = 0;
        /// 12.0
        bool _isBeingFinalClear = false;
        int _beginSkillLazyTick = 0;
        int _beginSkillToReleaseTick = 0;
        ///

        /// 13.0
        bool _isBattleMirroredForPlay = false;
        bool _isUnitHealthBarColorMirrored = false;
        bool _isUnitHealthBarTypeMirrored = false;
        ///

        /// 14.0
        std::map<int, std::map<int, std::pair<int, int>>> _aliveInfo;
        std::vector< std::vector<int> > _toDeleteCastersInfo; // <[character_myid,caster_type,caster_myid]>
        std::map<int, int> _toDeleteCasterMyIds; // <caster_myid, cnt>

        int _subjectiveTeamside = TeamSide::kOur; // 主视角阵营
        ///
	};

}

#endif /* defined(__OnePiece__AIContext__) */
