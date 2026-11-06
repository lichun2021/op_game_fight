//
//  AICharacterData.h
//  OnePiece
//
//
//

#ifndef __OnePiece__AICharacterData__
#define __OnePiece__AICharacterData__

#include <list>
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <functional>
#include <cstdint>

#include "AINode.h"

#define ATT_NUM 72
#define ACTION_ATT_NUM 16

#define STATIC_SETGET_ATT(getX,setX,i) \
    static int getX(int attributes[]){ assert(i < ATT_NUM); return attributes[i]; } \
    static void setX(int attributes[], int val){ assert(i < ATT_NUM); attributes[i] = val; }

#define STATIC_INC_ATT(incX,i) static void incX(int attributes[], int val){ assert(i < ATT_NUM); attributes[i] += val; }

#define STATIC_CLEAR_ATT(clearAtt,i) static void clearAtt(int attributes[], int val){ assert(i < ATT_NUM); memset(attributes,0,i); }

#define SETGET_ATT(getX,setX,i) \
    int getX(){ assert(i < ATT_NUM); return attributes[i]; } \
    void setX(int val){ assert(i < ATT_NUM); attributes[i] = val; }

#define INC_ATT(incX,i) void incX(int val){ assert(i < ATT_NUM); attributes[i] += val; }

#define CLEAR_ATT(clearAtt,i) void clearAtt(int val){ assert(i < ATT_NUM); memset(attributes,0,i); }

namespace OnePiece {

    class AIActionController;
    class AIInnerEffect;
    class AIContinuousEffectCaster;
    class AIContinuousSummonCaster;
    class AIContinuousHurtCaster;
    class AIContinuousMarkCaster;
    class AIContinuousMoveCaster;
    class AIContinuousProjectileCaster;
    class AIContinuousTransformCaster;
    class AccuSkillMaxTickInfo;

    typedef std::shared_ptr<AIContinuousTransformCaster> AIContinuousTransformCasterPtr;
    //typedef std::shared_ptr<AIInnerEffect> AIEffectExecResultPtr;

    enum TeamSide {
        kInvalid = 0,
        kOur = 1,
        kEnemy = 2
    };

    enum RoleType {
        kNormalCharacter = 0,
        kSummonCharacter,
        kCaptain
    };

    enum CaptainSeqType {
        kSeqNone = 0,
        kSeqTime = 1, // 时间序列-在指定总时间触发
        kSeqHurt = 2, // 友方伤害序列-友军总血量(排除幻象)低于指定百分比时触发
        kSeqDead = 3, // 友方死亡序列-友军死亡人数(排除幻象)达到指定数量时触发
        kSeqHurtEnemy = 4, // 敌方伤害序列
        kSeqDeadEnemy = 5  // 敌方死亡序列
    };

	enum ActionType {
		IDLE_ACTION				= 0,
		ATTACK_ACTION			= 1,
		MOVE_ACTION				= 2,
		RUN_ACTION				= 3,
		TURN_ACTION				= 4,
		DEFENSE_ACTION			= 5,
		IN_HURT_ACTION			= 6, //受创
		STRUCK_FLY_AWAY_ACTION	= 7, //击飞
		STRUCK_BACKWORD_ACTION	= 8, //击退
		STANDUP_ACTION			= 9,
		EFFECT_ACTION			= 10,
		SQUAT_ACTION			= 11,
		FALL_DOWN_ACTION		= 12, //击倒
		CELEBRATE_ACTION		= 13,
        DEATH_TRIGGERED_ATTACK_ACTION = 14,
		DEAD_ACTION1			= 15, // normal
		DEAD_ACTION2			= 16, // bouncing
		DEAD_ACTION3			= 17, // flying
        STRUCK_HOVER_ACTION		= 18,
		CONTROLLED_EFFECT_ACTION = 19, // 26.0 受控制状态
	};

    
    enum ECharacterAttr
    {
        ID = 0, // 角色id
        MAX_HP = 1, // 最大血量	显示用
        LV = 2, // 等级
        X = 3,
        Y = 4,
        Z = 5,
        // 一级属性
        PHY = 6, // 力量
        AGILE = 7, // 敏捷
        INT = 8, // 智力
        ACCURATE = 9, // 精准
        SPIRIT = 10, // 精神
        // 二级属性
        HP = 11, // 生命
        ENG = 12, // 能量
        SPEED = 13, // 移动速度
        PHY_AT = 14, // 物攻
        MAG_AT = 15, // 法术攻击
        PHY_CRIT = 16, // 物理暴击
        MAG_CRIT = 17, // 法术暴击
        PHY_CRIT_MUL = 18, // 物理暴击倍数
        MAG_CRIT_MUL = 19, // 法术暴击倍数
        PHY_HIT = 20, // 物理命中
        MAG_HIT = 21, // 法术命中
        PHY_DE = 22, // 物理护甲
        MAG_DE = 23, // 法术抗性
        PHY_PE = 24, // 物理穿透
        MAG_PE = 25, // 法术穿透
        DODGE_RATE = 26, // 闪避几率
        RESIST_RATE = 27, // 抵抗几率
        BLOOD_SUCK = 28, // 吸血等级
        HP_RECOVER = 29, // 生命值回复
        ENG_RECOVER = 30, // 能量回复
        CURE = 31, // 治疗效果
        ATT_INTER = 32, // 施法间隔
        AT_DIST = 33, // 攻击距离
        PHY_CRIT_RISIST = 34,// 物理抗暴
        MAG_CRIT_RISIST = 35,// 法术抗暴
        PHY_CRIT_MUL_ADD = 36,// 物理暴击倍数减免
        MAG_CRIT_MUL_ADD = 37,// 法术暴击倍数减免
		FUNC_HURT_ADD = 38, // 增伤
		FUNC_HURT_SUB = 39, // 减伤
		PHY_CRIT_ADD = 40,// 额外物理暴击
		MAG_CRIT_ADD = 41,// 额外法术暴击

        //新增加元素之力属性 cui_2023_6_29
        SLASH_ADD = 42,//斩击属性增伤
        SLASH_SUB = 43,//斩击属性减伤
        LIGHT_ADD = 44,//光属性增伤
        LIGHT_SUB = 45,//光属性减伤
        DARK_ADD = 46,//暗属性增伤
        DARK_SUB = 47,//暗属性减伤
        WATER_ADD = 48,//水属性增伤
        WATER_SUB = 49,//水属性减伤
        POISON_ADD = 50,//毒属性增伤
        POISON_SUB = 51,//毒属性减伤
        ELECTRIC_ADD = 52,//雷属性增伤
        ELECTRIC_SUB = 53,//雷属性减伤
        FIRE_ADD = 54,//火属性增伤
        FIRE_SUB = 55,//火属性减伤
        NONE_ADD = 56,//无属性增伤
        NONE_SUB = 57,//无属性减伤
     

        FINAL_ADD = 58,//最终伤害加成
        FINAL_SUB = 59,//最终伤害减免
        PHYSICS_ADD = 60, // Target Lua/native index.
        PHYSICS_CUT = 61, // Target Lua/native index.
        MAGIC_ADD = 62, // Target Lua/native index.
        MAGIC_CUT = 63, // Target Lua/native index.
        AMOR_BREAK_ADD = 64, // Target Lua/native index.
        AMOR_BREAK_CUT = 65, // Target Lua/native index.
        MAGIC_BREAK_ADD = 66, // Target Lua/native index.
        MAGIC_BREAK_CUT = 67, // Target Lua/native index.
        SWEET_ADD = 68, // Target Lua/native index.
        SWEET_CUT = 69, // Target Lua/native index.
        RUN_BREAK_ADD = 70, // Target Lua/native index.
        RUN_BREAK_CUT = 71, // Target Lua/native index.
        ATT_LEN = 72// 属性数量
    };
    
    
    enum CharacterState
    {
        Normal = 0,
    };

    enum class EAccumulateState : int {
        kNotAccumulate = 0, // 非蓄力状态
        kInAccumulate = 1,  // 蓄力中
        kAfterAccumulate = 2 // 蓄力释放中
    };



    struct SummonInfo
    {
        int characterId;
        int iLv;
        int num;
		TeamSide teamside;// 0:默认队友 1：敌方
        SummonProperty* pSummonProperty;
        std::list<std::pair<int,int>> skillList;
		int sourceMyId; // 召唤物源characterMyID


		SummonInfo()
		{
			characterId = 0;
			iLv = 0;
			num = 0;
			teamside = TeamSide::kInvalid;
			pSummonProperty = nullptr;
			skillList.clear();
			sourceMyId = -1;
		}
    };

	struct AttackActionInfo
	{
		int skillid = 0;
		int skilllv = 0;
		enum SkillType {
			SKTYPE_NORMAL = 0, // 普攻
			SKTYPE_ULTRA = 1, // 大招
			SKTYPE_OTHER = 2, // 小技能
		};
		int skilltype = SKTYPE_NORMAL;
		bool isforce = false; // 强制释放
	};

    enum ECanCastUltraSkill {
        kCanNotCastUltraSkill = 0,              // 不能释放大招
        kCanCastNormalUltraSkill = 1,           // 能释放普通大招
        kCanCastJueXingSkill = 2,               // 能释放觉醒技
        kCanCastAccumulateNormalUltraSkill = 3, // 能释放普通蓄力大招
        kCanCastAccumulateJueXingSkill = 4,     // 能释放蓄力觉醒技
        kCanCastEnergyUltraSkill = 5,
        kCanCastEnergyJueXingSkill = 6
    };

    enum CharacterTeamListType {
        CTEAM_FRIENDS = 0,
        CTEAM_TMP_FRIENDS = 1,
        CTEAM_RIVALS = 2,
        CTEAM_TMP_RIVALS = 3
    };

	class CharacterAttr
	{
	public:
		int c_id = 0; // 0 角色id
		int max_hp = 0; // 1
		int level = 0; // 2
		int x = 0; // 3
		int y = 0; // 4
		int z = 0; // 5

		// 一级属性
		int phy = 0; // 6 力量
		int agile = 0; // 7 敏捷
		int intelligence = 0; // 8 智力
		int accurate = 0; // 9 精准
		int spirit = 0; // 10 精神

		// 二级属性
		int hp = 0; // 11 生命
		int eng = 0; // 12 能量
		int speed = 0; // 13 移动速度
		int physics_attack = 0; // 14 物攻
		int magic_attack = 0; // 15 法术攻击
		int physics_crit = 0; // 16 物理暴击
		int magic_crit = 0; // 17 法术暴击
		int physics_crit_mul = 0; // 18 物理暴击倍数
		int magic_crit_mul = 0; // 19 法术暴击倍数
		int physics_hit = 0; // 20 物理命中
		int magic_hit = 0; // 21 法术命中
		int physics_de = 0; // 22 物理抵抗
		int magic_de = 0; // 23 法术抵抗
		int physics_pe = 0; // 24 物理穿透
		int magic_pe = 0; // 25 法术穿透
		int dodge_rate = 0; // 26 闪避几率
		int resist_rate = 0; // 27 抵抗几率
		int blood_suck = 0; // 28 吸血等级
		int hp_recover = 0; // 29 生命值回复
		int eng_recover = 0; // 30 能量回复
		int cure = 0; // 31 治疗效果
		int attack_interval = 0; // 32 攻击间隔
		int attack_dist = 0; // 33 攻击距离
		int physics_crit_resist = 0; // 34 物理抗暴
		int magic_crit_resist = 0; // 35 法术抗暴
		int physics_crit_mul_add = 0; // 36 物理暴击倍数减免
		int magic_crit_mul_add = 0; // 37 法术暴击倍数减免
		int func_hurt_add = 0; //38 增伤
		int func_hurt_sub = 0; //39 减伤
		int physics_crit_add = 0; // 40 额外物理暴击
		int magic_crit_add = 0; // 41 额外法术暴击

        //cui_2023_3_60 yunsushuxi
        int slash_add = 0;//斩击属性增伤
        int slash_sub = 0;//斩击属性减伤
        int light_add = 0;//光属性增伤
        int light_sub = 0;//光属性减伤
        int dark_add = 0;//暗属性增伤
        int dark_sub = 0;//暗属性减伤
        int water_add = 0;//水属性增伤
        int  water_sub = 0;//水属性减伤
        int  poison_add = 0;//毒属性增伤
        int  poison_sub = 0;//毒属性减伤
        int electric_add = 0;//雷属性增伤
        int  electric_sub = 0;//雷属性减伤
        int fire_add = 0;//火属性增伤
        int  fire_sub = 0;//火属性减伤
        int  none_add = 0;//无属性增伤
        int none_sub = 0;//无属性减伤

        int final_add = 0; //最终增伤
        int final_sub = 0; //最终减伤
        int physics_add = 0; // Target native storage index 60.
        int physics_cut = 0; // Target native storage index 61.
        int magic_add = 0; // Target native storage index 62.
        int magic_cut = 0; // Target native storage index 63.
        int amor_break_add = 0; // Target native storage index 64.
        int amor_break_cut = 0; // Target native storage index 65.
        int magic_break_add = 0; // Target native storage index 66.
        int magic_break_cut = 0; // Target native storage index 67.
        int sweet_add = 0; // Target native storage index 68.
        int sweet_cut = 0; // Target native storage index 69.
        int run_break_add = 0; // Target native storage index 70.
        int run_break_cut = 0; // Target native storage index 71.

	};

    class CharacterData;
    class SpecialSkillInfo;
    class CaptainSeqSkillInfo;
    class LoseHPAddAttrInfo;
    class AIAttributeEffectResult;
    typedef CharacterData AICharacterData;
    typedef std::list<CharacterData*> CharacterDataList;
    typedef CharacterDataList* CharacterDataListPtr;
    //typedef std::shared_ptr<SpecialSkillInfo> SpecialSkillInfoPtr;

    /////////////////////////////////////////////////////////
    // CharacterData
    /////////////////////////////////////////////////////////

	class CharacterData : public AINode
	{
		friend class AIContext;

	private:
		CharacterData(const CharacterData&);
		CharacterData(CharacterData&&);
		CharacterData& operator=(const CharacterData&);
		CharacterData(int* attList, std::list<std::pair<int, int>>& skillList, int juexing, AIContext& aiContext, int type, short k, short ifUltraLock, int clusterNum,CharacterData* pSummonSource, TeamSide side, RoleType role, int fp);

    public:
        short checkTarget(short ifJuexingSkill);
        short checkTargetNative(short ifJuexingSkill);
				
	public:
		int *attributes=NULL;//int attributes[ATT_NUM];
		bool needAdjusting;
		short juexingLevel;
		short k; // 0-后发, 1-先发, 100-船炮
		short ifUltraLock;
		int oldY;
        TeamSide teamSide;
        RoleType roleType;
        int combatPower = 0;
        CaptainSeqType captainSeqType = CaptainSeqType::kSeqNone;
		
		int adjustingX, adjustingY, adjustingZ;
        int adjustingSpeedX = 0;

		std::unique_ptr<CharacterAttr> attributesOrigin; // 开场时战报属性
		std::unique_ptr<CharacterAttr> attributesAfterBegin; // 释放完开场技能后初始属性

		AIContext* aiContext = NULL;
		AIActionController* actionController = NULL;

		ActionType currentAction = IDLE_ACTION;
		int actionAtt[ACTION_ATT_NUM];
        int actionCauserMyID = -1;//使目标进入当前状态的角色MyId,如自己使自己进入攻击,某个敌军使自己进入击倒
		std::list<AIContinuousHurtCaster*> hurtCasts;
		std::list<AIContinuousMoveCaster*> moveCasts;
		std::list<AIContinuousProjectileCaster*> projectileCasts;//only for continuous projectile caster, and it is attached to ACTION; if time expire or action change, this will be cleared
        std::list<AIContinuousMarkCaster*> markCasts;
        std::list<AIContinuousSummonCaster*> summonCasts;
        //void removeTargetSummonInSummonCaster(int characterID, int num, int excludeCasterUID);

        std::list<AIContinuousHurtCaster*> hurtCastsToDelete;

		AttackActionInfo attackActionInfo;
        
        void appendTransformCast(const AIContinuousTransformCasterPtr& caster);
        void removeTransformCast(int casterUID);
        void clearTransformCast();
        
        std::map<int, int> markDeadImmunes;//during the ticks, the object will not be affected by the marks of the specific types
		std::list<AIContinuousEffectCaster*> effectCasts;//only for continuous effect caster, and it is attached to ACTION; if time expire or action change, this will be cleared

		//by wp 2015.06.18
        std::map<int, std::vector<AIInnerEffect *>> m_aiEffectMap;//角色所拥有的AI效果的集合,同一种ID的效果可能会叠加
        std::map<int, int> innerEffectIntervals;//效果触发CD
        std::map<int, std::list<AIAttributeEffectResult>> attrEffectResults;//9.0:目前仅保存影响物理法术攻击力(14,15)的效果
        std::map<int, std::map<int, int>> innerEffectProtectCntMap; // 13.0:优先级最高的效果保护buff <int-效果执行ID,<int-内部效果ID,int-保护计数>>
        std::map<int, std::map<int, int>> innerEffectDebuffCntMap; // 13.0:debuff优先级低于保护buff <int-效果执行ID,<int-内部效果ID,int-保护计数>>

        // 不允许找到原始阵营队友/敌军
        std::map<int, int> cannotFindFriendTargets;
        std::map<int, int> cannotFindRivalTargets;

        std::map<int, int> cannotBeFoundConfig;//<相对原始阵营 0-敌方 1-我方,计数>
        //id， 伤害值，是否暴击，插入的时间 ，剩余删除时间，是否执行过 
		std::map<int, std::list<std::tuple<int, int,bool,long,long,bool>>> specialEffectLabels; // LabelEffectExec::LabelActionType - [[labelid,num,boolValue,insertTick,leftime]] 飘字传参调整 cui 2023_2_27
		std::list<std::pair<int, int>> labelEffects;//labelID, labelTitle
		std::list<std::pair<int, int>> talentList;//talentID, talentLV

		//std::list<std::pair<CharacterData*, float>> actionTargets; //pair<target, hurtPercentage>
		CharacterData* target = nullptr;
		CharacterData* m_pTmpTarget = nullptr;//嘲讽后或者 针对的对象
        CharacterData* tauntTarget = nullptr;//OP2改进:嘲讽目标
        CharacterData* attacker = nullptr;//攻击者
        CharacterData* specialFoundSkillTarget = nullptr; //特殊技能目标寻找到的目标(30,31,32,45,46,47)
        CharacterData* targetForFocus = nullptr;//集火目标
				
        CharacterDataListPtr friends = NULL;
        CharacterDataListPtr rivals = NULL;

        CharacterDataListPtr m_pTmpFriends = NULL;
        CharacterDataListPtr m_pTmpRivals = NULL;

        //cui_2023_7_4 yunsuzhili 
        std::vector<int> atkLabelVec;

        bool hasEffect(int effectID);
        int getEffectRemainCD(int effectID);
        
        bool hasMark(int markID);

        CharacterDataListPtr getOriginFriends() const { return friends; }
        CharacterDataListPtr getOriginRivals() const { return rivals; }
        CharacterDataListPtr getCurrentFriendsList(bool excludeSummon = false);
        CharacterDataListPtr getCurrentRivalsList(bool excludeSummon = false);
        CharacterDataListPtr getSummonList();

        void addSelfInToTmpRivalList(CharacterDataListPtr tmpList);
        void removeSelfFromRivalList(CharacterDataListPtr tmpList);
        void changeFriendRivalSideForCluster(TeamSide clusterSide, int clusterNum, bool isFromFriendToRival, bool excludeSummon);

		void inCharmEffect();
		void outCharmEffect();
		inline bool isInCharmEffect(){ return m_bInCharmEffect; }

        void inHalfCharmEffect();
        void outHalfCharmEffect();
        bool isInHalfCharmEffect() { return _isInHalfCharmEffect; }

        void inConfusionEffect();
        void outConfusionEffect();
        bool isInConfusionEffect() { return _isInConfusionEffect; }

		void inTauntEffect();
		void outTauntEffect();
		bool m_bInTauntEffect;
		inline bool isInTauntEffect(){ return m_bInTauntEffect; }

        void onEnterSilence(); // 被沉默

		bool m_bInCharmEffect;

        void enterAccumulateState(int skillID);
        void endAccumulateState();  // 结束蓄力，进入蓄力释放
        void exitAccumulateState(); // 退出蓄力
        bool isInAccumulateState() const;
        bool isAfterAccumulateState() const; //蓄力后释放状态

        //cui_获取元素之力增伤减伤
        int getElementAtkAdd();
        int getElementDefAdd(CharacterData* source);


        EAccumulateState getAccumulateState() const {
            return _accumulateState;
        }

        void enterTransformState();
        void exitTransformState();
        bool isInTransformState() const;

        bool isCritNoFailEffRunning = false;

        int checkIgnoreSilenceSkill(int skillID);

        std::vector<std::pair<int, int>> skillList;//skillID,skillLevel
        std::vector<std::pair<int, int>> skillsOnBegin; //开场技能
        std::map<int, int> skillInfoBackup;
		std::pair<int, int> normalSkill;
        std::map<int, bool> ignoreSilenceSkill;
		CharData* staticData = NULL;
        std::vector<CaptainSeqSkillInfo> originCaptainSkills;
        std::vector<CaptainSeqSkillInfo> remainCaptainSkills;
        std::vector<CaptainSeqSkillInfo> nextCaptainSkills;
        
		//used for later validation
		std::list<int> myActions;//IDs
        int direction;
        //int lastDirection;//HQ:remove from 10.0

        /** 0 - 不能放大; 1 - 普通大招; 2 - 觉醒技 */
		int canUltraSkill;
		bool targetStuckBackward;
        
        int _beHittedAddEnergy = 0;
        //std::unordered_map<int, int*> _loseHpAddAttrMap;// attr,   {baseAddVal,haveAddVal,minAddVal,maxAddVal,changeType}
        std::map<int, LoseHPAddAttrInfo> loseHpAddAttrMap;
        
        bool m_bEnterEffectAction;//是否进入特效效果 默认false
        bool isEnterEffectAction();
        
        int m_iSkillSeq2Index;

        int moveCasterAffectedCount = 0;

        void onEffectRestore();
        int getRestoredByEffectCount() const {
            return _restoredByEffectCount;
        }
        
		// clair_wang 记录定点范围内查找目标时，施法者Direction （有效值为-1和1， 0为无效值）
		int mStationaryDirection;
		int mStationaryX; // 记录定点范围内查找目标时，施法者X
		int mStationaryY; // 记录定点范围内查找目标时，施法者Y

        int killerUID = -1;
        int killerSkillID = -1;

        int initCharacterID = 0;

		bool mAttackConfirmedTarget = false;// 是否已确认攻击对象（适用于查找目标类型：41）
        
        //int lazyEnterWave = -1;//延迟入场时的波数,该效果整场战斗只有第一次入场才有用

		void SetFlagFirstFound26Target(bool flag);
		bool GetFlagFirstFound26Target();

		void SetFirstRoundConfirm26Target(CharacterData* theTarget = nullptr);
		CharacterData* GetFirstRoundConfirm26Target();

        void onEnterDuel(CharacterData *duelTarget);
        void onExitDuel();
        void setDuelTarget(CharacterData *duelTarget);
        inline CharacterData *getDuelTarget() { return _duelTarget; }
        bool isInDuel();
        bool isDuelWithTarget(CharacterData *duelTarget);
        void exitDuel();
        
        void resetTraversalTargetRecord();
        void setTraversalStartTick(int tick);
        int getTraversalStartTick() const { return _traversalStartTick; }
        int getTraversalLastPickTick() const { return _traversalLastPickTick; }
        void pickTraversalTarget(int targetMyID, int tick);
        bool isTargetLockedByTraversal(int targetMyID);
	
        int lastHurtExecInitHurtDelta = 0;
        int lastHurtExecRealHurtDelta = 0;
		CharacterData* pLastHurtOriginCharacter = nullptr;
		int lastHurtExecId = 0;

        int hitFlyExtendTick = 0;//击飞延长时间

        void resetFlags();
        int getSex();
        void setTempSex(int theTempSex);

        bool canFindTargetOrigin(CharacterData *theTarget);
        bool canBeFoundByRelativeSide(int theSide);//0-敌 1-我
        
        int eff249cnt = 0; //TODO:tobe optimised
        int eff250cnt = 0;

        int attackSpeed = 100;
        int currentEnergySkillId = 0;
        int currentEnergySkillTick = 0;
        int currentRepeatSkillId = 0;
        int currentRepeatSkillTick = 0;
        int currentRepeatSkillMaxRepeatTick = 0;
        
        int cannotBeAttackedBuffCnt = 0;

        int markType11Cnt = 0; // 15.0:拥有触发类型为11的标记执行数量

        //int currentAlternateAccuMaxTick = 0;
        //int nextAlternateAccuMaxTick = 0;
        //std::list< std::pair<int, int> > alternateAccuMaxTicks; // <innereff_uid,accu_tick_changevalue>
        std::map<int, std::unique_ptr<AccuSkillMaxTickInfo> > alternateAccuSkillMaxTicks;
        int getNextAlternateAccuMaxTick(int theSkillId);
        void setCurrentAlternateAccuMaxTick(int theSkillId, int theAltTick);
        int getCurrentAlternateAccuMaxTick(int theSkillId);
        void addAlternateAccuMaxTick(int theSkillId, int theInnerEffUid, int theChangevalue);
        void invalidateAlternateAccuMaxTick(int theSkillId, int theInnerEffUid);

        void addExtraUpdateFunc(int theUid, const std::function<void(AIContext&)> &theFunc);
        void invalidateExtraUpdateFunc(int theUid);

        void addHurtTransFunc(int theUid, const std::function<CharacterData *(AIContext &, CharacterData *, int, int)> &theFunc);
        void invalidateHurtTransFunc(int theUid);
        bool hasHurtTransFunc();
        CharacterData *getHurtTransTarget(int theHpDelta, int theExecId);

        bool hasSkill(int theSkillId);
        int getSkillLevel(int theSkillId);

        void addCanCastMyIdInEffect281Action(int theCasterMyId);
        void removeCanCastMyIdInEffect281Action(int theCasterMyId);

        int getAltAccuTick() const { return _altAccuTick; }
        void setAltAccuTick(int theAltAccuTick);

        void updateSkillLevel(int theSkillId, int theSkillLevel);
        void backupSkillInfo(int theSkillId, int theSkillLevel);
        bool hasSkillInfoBackup(int theSkillId);
        int getSkillInfoBackup(int theSkillId);
        
        /// 12.0
        void increaseCannotBeAttackedBuffCnt();
        void decreaseCannotBeAttackedBuffCnt();
        void increaseAllCannotBeFoundCnt();
        void decreaseAllCannotBeFoundCnt();
        ///

        /// OP2
        int penaltyHpTop = 0; // 惩罚减少的血量上限(显示用)
        float penaltyHpTopPercentage = 0; // [0,1]
        void initPenaltyHpTop(int penalty);
        ///

        /// 28.0
        void removeSpecialMoveCast(int moveType);

	protected:
		bool b_isFirstFound26Target = false;// 是否初次查找目标类型 （技能列表查找目标类型：26）
		CharacterData* pFirstRoundConfirm26Target;// 技能列表查找目标类型：26 随机一个敌人，并且若是持续伤害，每次伤害不再重新随机目标。

		
    private:
        CharacterData* summoner = nullptr;// 幻象的召唤者，与幻象所属角色的唯一id相同，初始化为-1,代表是谁的镜像 若不是镜像则应该为-1
        int _fakeSummonType = 0;//是否被认为是幻象

        std::map<ESkillCastCondition, std::vector<std::unique_ptr<SpecialSkillInfo> >> _specialSkills;

        std::vector<int>* attributesOriginalBackup; // 初始属性值备份
        int _originX;
        int _originY;
        EAccumulateState _accumulateState = EAccumulateState::kNotAccumulate;
        int _altAccuTick = 0;

        std::list<AIContinuousTransformCasterPtr> _transformCasts;
        bool _isInTransformState = false;

        std::list< std::pair<int, std::function<void(AIContext &)> > > _extraUpdateFuncs;
        std::list< std::pair<int, std::function<CharacterData *(AIContext &, CharacterData *, int, int)> > > _hurtTransFuncs; //[(eff_uid, func:ai,hero,hpdelta,execid)]

        bool _isInHalfCharmEffect = false; // 特殊魅惑,将敌人魅惑后,我方可以继续攻击该目标
        bool _isActivatingCritnofailEff = false;
        bool _isInDuel = false;
        CharacterData *_duelTarget = nullptr;

        bool _isInConfusionEffect = false; // 是否处于混乱状态 cui_2023_11_12

        int _restoredByEffectCount = 0; // 被效果执行中的复活效果复活的次数



        CharacterDataList _tmpFriends;
        CharacterDataList _tmpRivals;

        std::unordered_map<std::string, int> _tags;
        std::unordered_map<int, int> _traversalRecord; // target myid, record local tick
        int _traversalStartTick = 0;
        int _traversalLastPickTick = 0;

        int tempSex = -1; //临时性别 -1无效 0,1,2正常(与静态数据一致)

        std::unordered_map<int, bool> _canUpdateCastMyIdsInEffect281Action;

        void implUpdateCasts(AIContext& aiContext);
        void implUpdateCastsInEffect281Action(AIContext& aiContext);
        bool canCastInEffect281Action(int theCastMyId);

        /// 12.0
        int _allCannotBeFoundCnt = 0;
        ///

        /// 13.0
        int _cannotBeFoundByRealRivalCnt = 0;
        int _cannotBeFoundByRealFriendCnt = 0;
        ///

        /// 14.0
        CharacterDataList _summons;
        ///

        bool _isHpDown = false;   //当前血量是否超过21亿
        bool _isTopHpDown = false; //血量上限是否超过21亿
        //int _curHpDown = 0;    //当前血量超21亿时，此值用于存储大于21 / 1000 取余的部分 
        //int _curTopHpDown = 0; //血量上限超21亿时，此值用于存储大于21 / 1000 取余的部分 

    public:
        SummonProperty *summonProperty = nullptr;
        bool isSummonActive;
        bool hasBeenSummoned; //是否被召唤过,召唤过始终为true

        void resetSummonData();
        void onSummoned();
        inline CharacterData* getSummonSource() { return summoner; }
        inline bool isSummon(){ return summoner != NULL;}
        inline bool isCannon() { return k == 100; }
        bool canSummonCastUltra();
        int getFakeSummonType(); //0-普通召唤物 1-找目标时不被认为是召唤物/全局认为不是幻象 2- 仅257效果认为不是幻象 
        void setFakeSummonType(int theType);
        bool seemsSummon(); //seem as summon in battle
        inline bool isCaptain() const { return roleType == RoleType::kCaptain; }
        void initCaptainSeq(std::list<CaptainSeqSkillInfo> &seqList);
        AIContinuousEffectCaster *getEffectCaster(int casterMyId);
		AIContinuousMarkCaster	 *getMarkCaster(int casterMyId);
        int getMarkCountCommon(int markID);
        bool hasTag(const std::string &tag);
        void addTag(const std::string &tag);
        void eraseTag(const std::string &tag);
        int getTagValue(const std::string &tag);
        void setTagValue(const std::string &tag, int value);

        /// 13.0
        int getCannotBeFoundByTeamRelativeTypeCnt(int type);
        void setCannotBeFoundByTeamRelativeTypeCnt(int type, int cnt);
        ///

	public:
		CharacterData();
		virtual ~CharacterData();
		void update(AIContext& aiContext);
		int checkIfHit(int x, int y, int dx, int dy,int sourceDirection);
        CharacterData* lockTarget(AIContext &aiContext, bool isJudgeBy297 = false);
        CharacterData* lockTargetNative(AIContext &aiContext, bool isJudgeBy297);
		void printCharacter();
        // 是否能释放大招: 0 - 不能, 1 - 普通大招, 2 - 觉醒技, 3 - 普通蓄力大招, 4 - 蓄力觉醒技
        int canCastUltraSkill(bool isExternal = false);
        int canCastUltraSkillNative(bool isExternal);
		int checkCastUltraSkill();

        SpecialSkillInfo *getSpecialSkill(ESkillCastCondition key);
        void resetSpecialSkillConditon();
        void resetClusterSpecialSkillCondition();

        void onStartCastSkill(int skillID);
        void onHurt(bool isEffect, int id /*hurtID or effectExecID*/, int hpDelta, int hurtType, CharacterData *attacker, int casterId);
        void onEscape();
        void onHurtHit(int id /*hurtID or effectExecID*/); // 被伤害执行有效击中
        void onEvent(const std::string &name, CharacterData *pusher);
        void onFriendDead(CharacterData *theFriend);
        void onFriendSummonTimedOut(CharacterData *theFriendSummon);

		
		// clair_wang
		// 受到暴击后触发
		void onCruelHitSkillRelease();

        // 释放暴击后触发
        void onCastCritHurt();

        // 蓄力技能蓄力结束释放
        // @return 0 - 释放失败, 1 - 释放成功
        int releaseAccumulateSkill(bool ifExternal);

        /**
         * 释放大招
         * @param ifExternal 是否记录操作 @default true
         * @return 
         */
        int castUltraSkill(bool ifExternal = true);
        int castUltraSkillNative(bool ifExternal);
        void checkForceCastSkill(int skillID);
		void addTalent(int talentID, int talentLV);
		bool canBeAttacked(CharacterData *attacker, bool ignoreCanbeattackEffect = false, CharacterData *skillCaster = nullptr);
        bool isFindableToAttacker(CharacterData *attacker, bool ignoreMissingTargetEffect);
        bool isFindableToSkillCaster(CharacterData *skillCaster, bool ignoreMissingTargetEffect);
		void resetEffectExec();
        void removeMarks();
        void invalidateMarks();
        void marksOnSelfEnterDead();
        int forceCastUltraSkill(int juexing);

        void executeSpecialEffectLabel(int labelID, int num);
        void forceTurnToTarget(CharacterData *theTarget);

        SETGET_ATT(getID, setID, ECharacterAttr::ID);	//ID
        SETGET_ATT(getHPTop, setHPTop, ECharacterAttr::MAX_HP);
        SETGET_ATT(getLv, setLv, ECharacterAttr::LV);
        SETGET_ATT(getX, setX, ECharacterAttr::X);	//X
        SETGET_ATT(getY, setY, ECharacterAttr::Y);	//Y
        SETGET_ATT(getZ, setZ, ECharacterAttr::Z);	//Z
		
		/*
		int getY(){ return attributes[4]; }
		void setY(int val){ 
			attributes[4] = val;
		}
		int getZ(){ return attributes[5]; }
		void setZ(int val){
			attributes[5] = val;
		}
		*/
        //一级属性
        SETGET_ATT(getPHY, setPHY, ECharacterAttr::PHY);
        SETGET_ATT(getAGILE, setAGILE, ECharacterAttr::AGILE);
        SETGET_ATT(getINT, setINT, ECharacterAttr::INT);
        SETGET_ATT(getACCURATE, setACCURATE, ECharacterAttr::ACCURATE);
        SETGET_ATT(getSPIRIT, setSPIRIT, ECharacterAttr::SPIRIT);

        
        
        SETGET_ATT(getHP, setHP, ECharacterAttr::HP);
        SETGET_ATT(getEnergy, setEnergy, ECharacterAttr::ENG);
        SETGET_ATT(getSpeed, setSpeed, ECharacterAttr::SPEED);
        SETGET_ATT(getBAPower, setBAPower, ECharacterAttr::PHY_AT);
        SETGET_ATT(getMAPower, setMAPower, ECharacterAttr::MAG_AT);
        SETGET_ATT(getBACrit, setBACrit, ECharacterAttr::PHY_CRIT);
        SETGET_ATT(getMACrit, setMACrit, ECharacterAttr::MAG_CRIT);
        SETGET_ATT(getBACritM, setBACritM, ECharacterAttr::PHY_CRIT_MUL);
        SETGET_ATT(getMACritM, setMACritM, ECharacterAttr::MAG_CRIT_MUL);
        SETGET_ATT(getBAHit, setBAHit, ECharacterAttr::PHY_HIT);
        SETGET_ATT(getMAHit, setMAHit, ECharacterAttr::MAG_HIT);
        SETGET_ATT(getBARepel, setBARepel, ECharacterAttr::PHY_DE);
        SETGET_ATT(getMARepel, setMARepel, ECharacterAttr::MAG_DE);
        SETGET_ATT(getBABreak, setBABreak, ECharacterAttr::PHY_PE);
        SETGET_ATT(getMABreak, setMABreak, ECharacterAttr::MAG_PE);
        SETGET_ATT(getBADodge, setBADodge, ECharacterAttr::DODGE_RATE);
        SETGET_ATT(getMADodge, setMADodge, ECharacterAttr::RESIST_RATE);
        SETGET_ATT(getHPAdd1, setHPAdd1, ECharacterAttr::BLOOD_SUCK);//吸血
        SETGET_ATT(getHPAdd2, setHPAdd2, ECharacterAttr::HP_RECOVER);//过关回血
        SETGET_ATT(getENAdd1, setENAdd1, ECharacterAttr::ENG_RECOVER);//过关回能量
        SETGET_ATT(getHPAdd3, setHPAdd3, ECharacterAttr::CURE);//治疗加成
        SETGET_ATT(getAInterval, setAInterval, ECharacterAttr::ATT_INTER);
        SETGET_ATT(getATDist, setATDist, ECharacterAttr::AT_DIST);//攻击距离
        SETGET_ATT(getBACritRisist, setBACritRisist, ECharacterAttr::PHY_CRIT_RISIST);
        SETGET_ATT(getMACritRisist, setMACritRisist, ECharacterAttr::MAG_CRIT_RISIST);
        SETGET_ATT(getBACritMAdd, setBACritMAdd, ECharacterAttr::PHY_CRIT_MUL_ADD);
        SETGET_ATT(getMACritMAdd, setMACritMAdd, ECharacterAttr::MAG_CRIT_MUL_ADD);
		SETGET_ATT(getFuncHurtAdd, setFuncHurtAdd, ECharacterAttr::FUNC_HURT_ADD);
		SETGET_ATT(getFuncHurtSub, setFuncHurtSub, ECharacterAttr::FUNC_HURT_SUB);
		SETGET_ATT(getBACritAdd, setBACritAdd, ECharacterAttr::PHY_CRIT_ADD);
		SETGET_ATT(getMACritAdd, setMACritAdd, ECharacterAttr::MAG_CRIT_ADD);
        //cui_2023_6_30 yuansuzhili
        SETGET_ATT(getSlashAdd, setSlashAdd, ECharacterAttr::SLASH_ADD); //斩击属性增伤
        SETGET_ATT(getSlashSub, setSlashSub, ECharacterAttr::SLASH_SUB);//斩击属性减伤
        SETGET_ATT(getLightAdd, setLightAdd, ECharacterAttr::LIGHT_ADD);//光属性增伤
        SETGET_ATT(getLightSub, setLightSub, ECharacterAttr::LIGHT_SUB);//光属性减伤
        SETGET_ATT(getDarkAdd, setDarkAdd, ECharacterAttr::DARK_ADD);//暗属性增伤
        SETGET_ATT(getDarkSub, setDarkSub, ECharacterAttr::DARK_SUB);//暗属性减伤
        SETGET_ATT(getWaterAdd, setWaterAdd, ECharacterAttr::WATER_ADD);//水属性增伤
        SETGET_ATT(getWaterSub, setWaterSub, ECharacterAttr::WATER_SUB);//水属性减伤
        SETGET_ATT(getPosionAdd, setPosionAdd, ECharacterAttr::POISON_ADD);//毒属性增伤
        SETGET_ATT(getPosionSub, setPosionSub, ECharacterAttr::POISON_SUB);//毒属性减伤
        SETGET_ATT(getElectticAdd, setElectticAdd, ECharacterAttr::ELECTRIC_ADD);//雷属性增伤
        SETGET_ATT(getElectticSub, setElectticSub, ECharacterAttr::ELECTRIC_SUB);//雷属性减伤
        SETGET_ATT(getFireAdd, setFireAdd, ECharacterAttr::FIRE_ADD);//火属性增伤
        SETGET_ATT(getFireSub, setFireSub, ECharacterAttr::FIRE_SUB);//火属性减伤
        SETGET_ATT(getNoneAdd, setNoneAdd, ECharacterAttr::NONE_ADD);//无属性增伤
        SETGET_ATT(getNoneSub, setNoneSub, ECharacterAttr::NONE_SUB);//无属性减伤
        SETGET_ATT(getFinalAdd, setFinalAdd, ECharacterAttr::FINAL_ADD);
        SETGET_ATT(getFinalSub, setFinalSub, ECharacterAttr::FINAL_SUB);
        int getRealAttriByAttriType(int type);

        
        class AttributeData {
        public:
            int ownerMyId;
            int index;
            int originValue;
            int currentValue;
        };
        /*
        int64_t getHP() {
            if (!_isHpDown) {
                return attributes[ECharacterAttr::HP];
            }
            else
            {
                return int64_t(attributes[ECharacterAttr::HP]) * 1000 + int64_t(_curHpDown);
            }
            
        }
        void setHP(int64_t hp) { 
            if (hp >= 2100000000) {
                _isHpDown = true;
                _curHpDown = hp % 1000;
                attributes[ECharacterAttr::HP] = hp / 1000;
            }
            else
            {
                _isHpDown = false;
                attributes[ECharacterAttr::HP] = hp;
                _curHpDown = 0;
            }
        }

        int64_t getHPTop() { 
            if (!_isTopHpDown) {
                return attributes[ECharacterAttr::MAX_HP];
            }
            else
            {
                return  int64_t (attributes[ECharacterAttr::MAX_HP]) * 1000 + int64_t(_curTopHpDown);
            }
        }
        void setHPTop(int64_t hp) { 
            if (hp >= 2100000000) {
                _isTopHpDown = true;
                _curTopHpDown = hp % 1000;
                attributes[ECharacterAttr::MAX_HP] = hp / 1000;
            }
            else
            {
                _isTopHpDown = false;
                attributes[ECharacterAttr::MAX_HP] = hp;
                _curTopHpDown = 0;
            }
        }
        */
        
        
        void updateHP(int hp);
        void updateEnergy(int energy,bool isKill = false);

        void onScriptSetX(int x);
        void onScriptSetY(int y);
        int getScriptOriginX();
        int getScriptOriginY();
        void resetPos();

        void enhanceAttribute(int attribute, int enhanceRatio, bool isAdd = false/*是否累加*/);

        void onAttrChange(int attr);

        void onHPDelta(int hpdelta); // hpdelta = newhp - oldhp
        void onENDelta(int endelta); // 能量变化

        void checkOutSummonCharm253();
        void outSummonCharm253();
	};

    namespace CharacterFunc {

        extern int GetSkillIDByIndex(CharacterData& hero, int index);
        extern int GetSkillIndexByID(CharacterData *target, int skillID);

        extern bool IsOurTeam(TeamSide side);
        extern bool IsEnemyTeam(TeamSide side);

        extern bool IsOurHero(CharacterData& hero);
        extern bool IsEnemyHero(CharacterData& hero);
        
        extern float GetCharacterHPRatio(CharacterData& hero);
        extern int getSimplifiedHPRatio(CharacterData *target);
        
        extern bool IsAttackerFaceTarget(CharacterData *attacker, CharacterData *t);

        typedef int CharacterID;
        typedef std::unordered_map<CharacterID, std::map<ECharacterAttr, int>> CharacterAttrData;
        //typedef std::pair<bool, int> CharacterAttrCacheFoundResult;

        struct CharacterAttrCacheFoundResult {
            bool found = false;
            int value = 0;
            explicit CharacterAttrCacheFoundResult(bool found, int value) : found(found), value(value) {}
        };

        extern CharacterAttrData& GetAttrDataCache();

        extern void ClearAttrDataCache();
        extern void SetAttrValue(CharacterID chID, ECharacterAttr attr, int value);
        extern void SetAttrValueWhenNotExisted(CharacterID chID, ECharacterAttr attr, int value);
        extern CharacterAttrCacheFoundResult GetAttrValue(CharacterID chID, ECharacterAttr attr);
        extern bool CheckHasAttrCache(CharacterID chID, ECharacterAttr attr);
        extern void CheckInheritSummonAttr(SummonProperty *summonConf, int summonLv, int *summonAttributes, CharacterData *summoner);

        extern int GetPositionX(CharacterData *c, bool ismirrored);
        extern int GetDirection(CharacterData *c, bool ismirrored);

        extern int GetInnerEffectProtectCount(CharacterData *c, int parentEffectCasterId, int innerEffectId);
        extern void SetInnerEffectProtectCount(CharacterData *c, int parentEffectCasterId, int innerEffectId, int count);

        extern int GetInnerEffectDebuffCount(CharacterData *c, int parentEffectCasterId, int innerEffectId);
        extern void SetInnerEffectDebuffCount(CharacterData *c, int parentEffectCasterId, int innerEffectId, int count);

        extern void SetTeamListInfo(CharacterData *c, int characterTeamListType, int aiTeamListType, int aiTeamside, int aiClusternum, bool aiIsExcludeSummon);

        extern bool CheckHasCaster(CharacterData *c, int casterType, int casterId);

        extern bool IsRivalTeamAllDeadForCheckUltra(CharacterData *c);

        int64_t CalcTeamHPTopFunc(CharacterDataListPtr pCharacterList);
        extern void ApplyEnergyDelta(CharacterData *c, int endelta);
        extern bool HasLabel(CharacterData *c, int label);

		extern void SetCharacterAttr(CharacterAttr *dst, int idx, int val);
		extern int GetCharacterAttr( CharacterAttr *dst, int idx);
		extern void CopyOldFashionAttributes(int *attlist, CharacterAttr *dst);

        //cui_2023_7_3 根据元素之力标签获取属性
        extern int GetCharacterAttrByLabel(CharacterData* c, int label,bool isAtk);

    } // namespace CharacterFunc

} // namespace OnePiece

#endif /* defined(__OnePiece__AICharacterData__) */
