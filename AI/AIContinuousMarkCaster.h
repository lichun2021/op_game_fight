//
//  AIContinuousMarkCaster.h
//  OnePiece
//
//  Created by h7ing on Mar/16/2015.
//
//

#ifndef __op_game__AIContinuousMarkCaster__
#define __op_game__AIContinuousMarkCaster__

#include "AINode.h"
//#include "AIContext.h"
#include <set>
#include <map>
namespace OnePiece {
    
    class CharacterData;
    class AIContext;
    class AIProjectile;
    struct MarkExec;
    class AIContinuousEffectCaster;

    enum MarkExcuteResult {
        EXCUTE_RESULT_NONE = 0,
        EXCUTE_RESULT_REMOVE_MARK = 1, // 需要移除
        EXCUTE_RESULT_HAS_REMOVED_MARK = 2 // 已经移除对象
    };
    
    class AIContinuousMarkCaster : public AINode {
    public:
        enum TriggerType {
            kEffect = 0,
            kDeath,
        };
		enum E_TriggerCharacterType
		{// 角色类型（第一位：0-友军，1-敌军，2-敌我；第二位：0-不包含幻象，1-包含幻象）
			kTriggerFirendType = 0, // 友军
			kTriggerEnemyType  = 1, // 敌军
			kTriggerAllType    = 2  // 敌我
		};

        enum ParamType {
            kNormalParam = 0,
            kHurtType, // 受到的伤害类型(-1所有 0-物理，1-法术 3-纯粹 等)
            kHurtProperty, // 受到的伤害属性(火，雷等)
        };
        typedef int ParamID;
        typedef std::map<ParamType, std::vector<ParamID> > Params;
        static const ParamID kInvalidParamID = -1;

		// clair_wang
		enum E_CheckExcuteEffectMarkCallType
		{
			kCheckMarkCallBySkill       = 0, // source释放技能调用
			kCheckMarkCallByHurt        = 1, // target受到伤害调用 
			kCheckMarkCallByFriendHurt  = 2, // 队友（包括施法者）造成某一属性伤害时触发
            kCheckMarkCallByOwnerCastHurt = 3 // 标记所在的目标对其他人造成伤害时触发
		};

		enum E_MathLogicSymbolType
		{
			eMathGatherThan      = 0, // 大于
			eMathGatherOrEqualTo = 1, // 大于等于
			eMathEqualTo         = 2, // 等于
			eMathLessOrEqualTo   = 3, // 小于等于
			eMathLessThan        = 4  // 小于
		};



    public:
        AIContext& aiContext;
        const int markID;
        //bool isRemoved;
        int skillID;
        int skillLevel; // 触发标记的当前技能等级
        int initSkillLevel; // 创建标记时的技能等级

		int hitContinualNum; // 非暴击持续次数

        MarkExec* staticData;
        MarkEffectExec *innerMarkData;

        CharacterData* origin;
        AIProjectile* projectile;
        CharacterData* target;
        CharacterData* initOrigin; // 创建标记的初始施法者

    private:
        int _markLevel;
        Params _triggerEffectParams;
        Params _triggerDeathParams;
        std::set<int> _triggeredEffects;
        std::set<int> _triggeredHurts;
        bool _isValid = true;
        int _maxLevel;
        int _initLevel;

		int _effectTriggedMarkLevel = 0;// 触发时的层数 clair_wang

        int _nextTriggerTick = 0; // 有触发间隔时下一次可触发的tick
        int _triggeredNum = 0; // 已触发效果次数

    public:
        virtual ~AIContinuousMarkCaster();
        AIContinuousMarkCaster(AIContext& aiContext, int skillID, int skillLevel, int markID, int markLevel, CharacterData* origin, AIProjectile* projectile, CharacterData* target);

    public:
        static void checkAddMark(AIContext& aiContext, int skillID, int skillLevel, CharacterData* origin, AIProjectile* projectile, CharacterData* target, MarkExec* markToBeAdd);

		static void checkExcuteEffect(AIContext& aiContext, CharacterData& target, int skillID, int skillLevel, int hurtType, int hurtProperty, int theCallType, int HPDelta = 0, int hitType = 0, CharacterData* trigger = nullptr);
		static void checkExcuteDead(AIContext& aiContext, CharacterData& target, int skillID, int skillLevel, int hurtType, int hurtProperty, CharacterData* trigger = nullptr, int hurtID = 0);
        static void reshowUIEffects(AIContext &aiContext, CharacterData &target); // 12.0
        int Excute(AIContext& aiContext);
        void onCharacterDead();
		static void checkExcuteAfterCharcterDeadReleaseTrigger(CharacterData* theCharacterData); // OP2 deprecated
        static void checkDoCharacterDead(AIContext &aiContext, CharacterData *c); // OP2 场上某个角色死亡后响应
        static void checkTriggerOnCharacterDead(AIContext &aiContext, CharacterData *c); // OP2 场上某个角色死亡后触发标记
        int getMarkLevel() const { return _markLevel; }
        bool isValid() const { return _isValid; };
        void invalidate();

        void reshowUIEffect(); // 12.0

        void updateUIEffect();
        void removeUIEffect();

        int updateMarkCountForce(int markNum,CharacterData* origin,int staticMarkLv);

		bool isTriggerParam(TriggerType triggerType, int paramA, int paramB, int paramC = kInvalidParamID);

        void removeTriggeredEffectCasts();

        void removeTriggeredHurtCasts();

        int updateMarkLevelOffset(int offset, int skillLevel, CharacterData* origin);
        int excuteEffect(int skillLevel, CharacterData* trigger);

        void excuteDeath(int skillLevel, CharacterData* trigger);
		

    private:
        void setEffectExecData(MarkEffectExec *data);
        int checkExcuteForTriggeredByMark();
        static bool isParamExistInParams(const Params& theParams, ParamType theType, ParamID theKey);
        static void checkExcuteAllEffectTriggeredByMark(CharacterData* target, int markID, AIContinuousMarkCaster* markInst = nullptr);
        static void checkExcuteEffectTriggeredByReachLevel(CharacterData* target, AIContinuousMarkCaster* mark);
		static void checkExcuteEffectTriggerdByReachLevelMark(CharacterData* target, int markID, AIContinuousMarkCaster* markInst = nullptr);
	//	static void checkExcuteEffectTriggerdByNoneCruelHitParams(AIContinuousMarkCaster* mark, CharacterData* target, int skillLevel, int HitType);

		bool isDeadSomeOneParamExist(const Params& theParams, ParamType theType);
		bool isSingleHurtParamTrigger(const Params& theParams, ParamType theType, ParamID theKeyA, ParamID theKeyB);
		bool isTypeLevelMarkParamExistInParams(const Params& theParams, ParamType theType, ParamID theKeyA, ParamID theKeyB);

		static bool isHasCoverMarkFlag(CharacterData* target, int theType);

		void setEffectTriggedMarkLevel(int theLevel) { _effectTriggedMarkLevel = theLevel; }
		int getEffectTriggedMarkLevel(){ return _effectTriggedMarkLevel; }

    }; // class AIContinuousMarkCaster
    
} // namespace OnePiece

#endif /* defined(__op_game__AIContinuousMarkCaster__) */
