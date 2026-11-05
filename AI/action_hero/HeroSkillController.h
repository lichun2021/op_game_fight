//
//  HeroSkillAction.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__HeroSkillController__
#define __OnePiece__HeroSkillController__

#include "../AIContext.h"
#include <list>
#include <memory>
#include <cstdint>

namespace OnePiece {

	enum ESkillTargetType{
	};

    struct SkillSeqInfo {
        int skillIDIndex = -1; // 角色列表静态数据技能列表的索引(sk)
        int attackSeqIndex = -1; // 角色列表静态数据攻击序列的索引(seq)
        int specialSkillID = -1; // HQ:默认-1,只在选取被动技时有效
        int specialSkillCastContiond = -1; //默认-1,只在选取被动技时有效

        SkillSeqInfo(int idIndex, int seqIndex, int specialSkillID, int specialSkillCastCondition);
        
        void setInfo(int idIndex, int seqIndex, int specialSkillID, int specialSkillCastCondition);

    private:
        SkillSeqInfo();
    };

    class SkillTargetInfo {
    public:
        int realSearchType = 0;
        bool excludeSummon = false;
        bool ignoreCanbeattackEffect = false;
        bool searchAgainIfExcludeSummon = false; //排除召唤物时，如果没有范围目标，则从召唤物中找
        bool isonlySummon = false;
    };

    class CaptainSeqSkillInfo {
    public:
        int seqIdx = 0;
        int seqType = 0;
        int seqValue = 0; // 节点值
        int skillID = 0;
        int skillLevel = 0;
        std::vector<int> castIDs;

        CaptainSeqSkillInfo(int theseqidx, int theseqtype, int theseqvalue, int theskillid, int theskilllevel);

        CaptainSeqSkillInfo() = delete;
    };

    class AccuSkillMaxTickInfo {
    public:
        int currentAlternateAccuMaxTick = -1;
        std::list< std::pair<int, int> > altAccuMaxTickChangeValues;
    };

	class HeroSkillController
	{	
		friend class HeroActionController;
	public:
        static void pickSkill(CharacterData& aiCharacter, AIContext& aiContext, SkillSeqInfo *outresult);
		static bool isTargetInSkilllRange(CharacterData* owner, int skillId, CharacterData* target);
		static int calculateSkillID(AIContext& aiContext,CharacterData& owner, int skillSeq);
		static int calculateSkillID(AIContext& aiContext,CharacterData& owner);
		static int calculateSkillLength(int skillId);
        static int calculateAccumulateSkillReleaseLength(int skillId); // 蓄力大招释放需要的时间
        static int calculateAccuSkillLength(int skillId, int theAlternateMaxAccuTick);
        static int calculateRepeatSkillFirstRoundTickLength(int skillId);
        static int calculateEnergySkillFirstRoundTickLength(int skillId);
		static int calculateExecLength(int execID);
		static std::pair<int, int> getUltraSkill(CharacterData& aiCharacter);
		static void executeExecIDInTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& caster, CharacterData& target);
        static void endAccumulateSkill(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& caster, CharacterData& target);
        static void forceReleaseAccumulateSkill(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& caster, CharacterData& target);
        static void checkReleaseHPSkill(AIContext& aiContext, CharacterData* target);
        static void executeExecOnExitEnergySkill(AIContext& aiContext, int skillId, int skillLv, CharacterData& caster, CharacterData& target);
        static void executeBeginSkillExecIDInTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData *source, CharacterData *target, AISkill *skillData, bool isProtective, int lazyTick); // 12.0

		static std::list<CharacterData*>* getSkillTarget(AIContext& aiContext, CharacterData* caster, ESkillTargetType targetType);

        static bool isSkillSwapWithNormal(int skillID);
        static int getAttackSeqFirstMatchIndex(CharacterData* character, int skillIndex, bool useSeq2);
        static uint32_t calculateSkillRangeSquare(int skillID);
        static bool isSkillIgnoreCD(int skillID);
        static void updateSpecialSkillCD(CharacterData *character);
        static SkillTargetInfo getRealSearchInfo(int originSearchType);
        static bool isTargetSameNoSummonOption(int type, int comp);
        static bool isSkillIgnoreSilence(int skillID);
        static bool isSkillDataIgnoreSilence(AISkill *skillData);
        static bool isTargetIgnoreSilenceBySkill(AIContext &aiContext, CharacterData *target, int skillID);
        static bool isTargetIgnoreSilenceBySkillIndex(AIContext &aiContext, CharacterData *target, int index);

        static int getRepeatSkillMaxRepeatTick(CharacterData *character, int skillId);
        static int getRepeatSkillRepeatType(int skillId);
        static int getRepeatSkillExtendRepeatTickForCrit(int skillId);
        static int getRepeatSkillMaxRepeatTick(int skillId);
        static int getRepeatSkillReadyTick(int skillId);
        static int getRepeatSkillBasicTick(int skillId);

        static int getAccuSkillReadyTick(int skillId); //蓄力技能起手时间
        static int getAccuSkillMaxAccuTick(int skillId);
        
        static bool isSkillTargetForSingle(AISkill *skillData);

	private:
        static void executeExecID(int eid, AISkill* skillData, AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target);
		static std::unique_ptr<std::list<CharacterData*> > GetSkillTargetListsByType(AISkill* skillData, AIContext& aiContext, int skillId, CharacterData& source, CharacterData& target); // HQ:7.0起废弃
        static void implExecuteExecIDInNormalSkillTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target, AISkill *skillData);
        static void implExecuteExecIDInAccuSkillTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target, AISkill *skillData);
        static void implExecuteExecIDInEnergySkillTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target, AISkill *skillData);
        static void implExecuteExecIDInRepeatSkillTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target, AISkill *skillData);
	};

    /////////////////////////////////////////////////////////
    // SpecialSkillInfo
    /////////////////////////////////////////////////////////

    class SpecialSkillInfo {
    public:
        ESkillCastCondition castCondition;
        int skillID = 0;
        int skillIDIndex = -1;
        int skillLevel = 0;
        int releaseCount = 0;

    public:
        static std::unique_ptr<SpecialSkillInfo> create(CharData *charData, int skillID, int skillLevel, AISkill *skillData);
        SpecialSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel);
        virtual ~SpecialSkillInfo() {}
        virtual bool canCast(CharacterData *caster) = 0;
        virtual void onConditon() = 0;
        virtual void onCast() = 0;
        virtual void resetCondition() = 0;
        virtual void resetClusterCondition() { resetCondition(); };
    protected:
        SpecialSkillInfo();
        SpecialSkillInfo(SpecialSkillInfo&&);
    };

    // 死亡触发
    class OnDeadSkillInfo : public SpecialSkillInfo {
    public:
        OnDeadSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel, int limitValue);

        virtual bool canCast(CharacterData *caster) override;
        virtual void onConditon() override;
        virtual void onCast() override;
        virtual void resetCondition() override;
        void resetClusterCondition() override;

    private:
        OnDeadSkillInfo() = delete;
        OnDeadSkillInfo(OnDeadSkillInfo &&) = delete;

    private:
        int _limit = 0;
    };

    // 低于血量触发
    class OnHPLimitSkillInfo : public SpecialSkillInfo {
    public:
        static bool GreaterOrEqual(int lhs, int rhs) { return lhs >= rhs; }
        static bool LessOrEqual(int lhs, int rhs) { return lhs <= rhs; }
        static bool InvalidComp(int, int) { return false; }
        typedef bool(*CompareFuncPtr)(int lhs, int rhs);

        enum CompareType {
            kGreaterOrEqual = 0, // >=
            kLessOrEqual = 1     // <=
        };
        enum LimitParamType {
            kLiteral = 0, // 固定值
            kPercent = 1  // 百分比
        };

        OnHPLimitSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel, int limitValue, CompareType compType, LimitParamType limitType);

        virtual bool canCast(CharacterData *caster) override;
        virtual void onConditon() override;
        virtual void onCast() override;
        virtual void resetCondition() override;

        bool isLimitHigher(CharacterData *selfCaster, OnHPLimitSkillInfo *another, CharacterData *anotherCaster);

    private:
        OnHPLimitSkillInfo() = delete;
        OnHPLimitSkillInfo(const OnHPLimitSkillInfo&) = delete;
        OnHPLimitSkillInfo(OnHPLimitSkillInfo&&) = delete;

    private:
        int _limit = 0;
        LimitParamType _limitType = LimitParamType::kLiteral;
        CompareFuncPtr _compFunc = &InvalidComp;
    };

    // 闪避触发
    class OnEscapeSkillInfo : public SpecialSkillInfo {
    public:
        OnEscapeSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel);
        virtual bool canCast(CharacterData *caster) override;
        virtual void onConditon() override;
        virtual void onCast() override;
        virtual void resetCondition() override;
    private:
        OnEscapeSkillInfo() = delete;
        OnEscapeSkillInfo(const OnEscapeSkillInfo&) = delete;
        OnEscapeSkillInfo(OnEscapeSkillInfo&&) = delete;

    private:
        int _escapeCount = 0;
    };


    // 造成暴击后触发
    class OnAfterCruelHitSkillInfo : public SpecialSkillInfo {
    public:
        OnAfterCruelHitSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel);
        virtual bool canCast(CharacterData *caster) override;
        virtual void onConditon() override;
        virtual void onCast() override;
        virtual void resetCondition() override;
    private:
        OnAfterCruelHitSkillInfo() = delete;
        OnAfterCruelHitSkillInfo(const OnAfterCruelHitSkillInfo&) = delete;
        OnAfterCruelHitSkillInfo(OnAfterCruelHitSkillInfo&&) = delete;

    private:
        int _nAfterCruelHitCount = 0;
    };


    // 受到x次伤害触发技能
    class OnHurtSkillInfo : public SpecialSkillInfo {
    public:
        OnHurtSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel, int limitValue);

        virtual bool canCast(CharacterData *caster) override;
        virtual void onConditon() override;
        virtual void onCast() override;
        virtual void resetCondition() override;
    private:
        OnHurtSkillInfo() = delete;
        OnHurtSkillInfo(const OnHurtSkillInfo&) = delete;
        OnHurtSkillInfo(OnHurtSkillInfo&&) = delete;

    private:
        int _limit = 0;
        int _onHurtCount = 0;
    };

    // 任意敌方血量触发
    class OnEnemyHPSkillInfo : public SpecialSkillInfo {
    public:
        OnEnemyHPSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel, int limitValue, int compType, int limitType, int isIncludeSummon);
        virtual bool canCast(CharacterData *caster) override;
        virtual void onConditon() override;
        virtual void onCast() override;
        virtual void resetCondition() override;
        bool isInCD();
		bool isTargetMeetCondition(CharacterData *target, CharacterData *source);
        void addCheckTarget(CharacterData *target);
        void tick();
    private:
        OnEnemyHPSkillInfo() = delete;

        int _onConditionCount = 0;
        int _interval = 0;
        int _boundsLow = 0;
        int _boundsHigh = 0;
        int _localInterval = 0;
		int _isIncludeSummon = 0;

        std::map<int/*characterMyID*/, bool> _checkTargets;
    };
}
#endif /* defined(__OnePiece__HeroSkillAction__) */
