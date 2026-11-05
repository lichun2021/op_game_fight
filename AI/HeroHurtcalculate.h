//
//  HeroHurtcalculate.h
//  OnePiece
//
//  Created by Beileng Shan on 3/11/14.
//
//
#include "AIContext.h"
#include <vector>

#ifndef __OnePiece__HeroHurtcalculate__
#define __OnePiece__HeroHurtcalculate__

namespace OnePiece {

    class CharacterData;
    class AIInnerEffect;

    enum HitLabel {
        NORMAL_HIT = 100000,
        CRITICAL_HIT = 100001
    };

    enum HurtType {
        PHYSICAL_HURT_TYPE = 0,
        MAGICAL_HURT_TYPE  = 1,
        HEALING_HURT_TYPE  = 2,

		// 25.0
		RAW_HURT_TYPE = 3 // 纯粹/真实伤害(为表示原来效果执行中的真实伤害类型)
    };

    enum HurtPropertyType {
        kNoProperty = 0, //无属性
        kHurtLei = 1,//雷系伤害
        kHurtGuang = 2,//光系伤害
        kHurtHuo = 3,//火系伤害
        kHurtShui = 4,//水系伤害
        kHurtAn = 5,//暗系伤害
        kHurtDu = 6,//毒系伤害
    };

    class HurtCalcParam {
    public:
        AIProjectile *projectile = nullptr;
        int skillID = 0;
        unsigned short skillLevel = 0;
        int hitType = NORMAL_HIT;
        int HPDelta = 0;
        int hurtProperty = HurtPropertyType::kNoProperty;
        int hurtType = PHYSICAL_HURT_TYPE;
        HurtExec *hurtExec = nullptr;
        bool shouldSkipRebound = false;
        bool ignoreHurtLabel = false;
        bool isFromEffectCaster = false;
        bool isRawHurt = false;
        bool isSimulateCalc = false;
        bool isFromSacrificeRecv = false;
        bool isFromHpLost = false;
        bool isPureHurt = false;  //是否是真正的纯粹伤害 cui_2023_02_08
        CharacterData *statHurtSourceAlt = nullptr; // 指定伤害统计角色
        int markID = 0;//伤害来源是标记的则有标记id
        int casterID = 0;//伤害的直接来源执行id
        int alternateHitLabel = 0;
        AIInnerEffect *innerEffect = nullptr;
        
        int HPDeltaBeforeAvoidHurt = 0; // 12.0 sync from lua

        HurtCalcParam(AIProjectile *proj, int skId, unsigned short skLv, int hpdel, int hurtTp, int theCasterId)
            : projectile(proj), skillID(skId), skillLevel(skLv), HPDelta(hpdel), hurtType(hurtTp), casterID(theCasterId) {}

        HurtCalcParam(AIProjectile *proj, int skId, unsigned short skLv, int hpdel, int hurtTp, int hurtProp, int hitTp, HurtExec *hurtExec, bool skipRebound, int theCasterId)
            : projectile(proj), skillID(skId), skillLevel(skLv), HPDelta(hpdel), hurtType(hurtTp), hurtProperty(hurtProp), hitType(hitTp), hurtExec(hurtExec), shouldSkipRebound(skipRebound), casterID(theCasterId) {}
    };

    class HitResult {
    public:
        enum class BlockType {
            HIT_UNBLOCKED = 0,
            HIT_BLOCKED        // 被直接免疫(不考虑伤害数值的免疫，如按类型免疫效果和按次数免疫护盾)
        };
        enum class HitResultType {
            HIT_HASHURT = 0,
            HIT_NOHURT,       // 伤害值被减为0(抵消伤害数值的效果)
            HIT_BYPASS        // 被盾吸收且不走其他的标记触发和击中执行
        };

        BlockType blockResult = BlockType::HIT_UNBLOCKED;
        HitResultType hitResult = HitResultType::HIT_HASHURT;

        HitResult() {}
        
        HitResult(HitResultType hit, BlockType blocked) : hitResult(hit), blockResult(blocked) {}
        void setResult(HitResultType hit, BlockType blocked) {
            hitResult = hit;
            blockResult = blocked;
        }
    };

	class HeroHurtcalculate {
	public:
        enum HurtResult {
			NONE_HURT_RESULT = 0,
			NORMAL_HURT_RESULT = 1,
			KNOCK_BACK_HURT_RESULT = 2,
			KNOCK_FLY_HURT_RESULT = 3,
			KNOCK_DOWN_HURT_RESULT = 4,
            KNOCK_BACK_RANDOM_HURT_RESULT = 5, //随机方向击退
            KNOCK_FLY_RANDOM_HURT_RESULT = 6, //随机方向击飞
            KNOCK_FORCE_DOWN_HURT_RESULT = 7, //将目标击倒，若目标无法被击倒，则强制置于rise状态
		};

		enum TalentTrigger {
			//0-从来不触发, 1-默认触发，2-攻击命中触发，3-被攻击命中触发，4-闪避抵抗触发，5-被治疗时触发，7-被动作限制时
			NEVER = 0,
			ALWAYS,
			HIT,
			BY_HIT,
			DODGE,
			BY_HURL,
			IN_EFFECT_ACTION
		};

	public:
        static float calcMissProb(AIContext& aiContext, CharacterData* source, CharacterData* target, int hurtType);
        static float calcEscProb(AIContext& aiContext, CharacterData* source, CharacterData* target, int hurtType);
        static float calcCHProb(AIContext& aiContext, CharacterData* source, CharacterData* target, int hurtType);
		static int ExertHurt(AIContext& aiContext, CharacterData* aiCharacter, AIProjectile* projectile, int hurtID, int skillID, int skillLevel, CharacterData* target, int theMarkID);
        static int ExertHurtNative(AIContext& aiContext, CharacterData* source, AIProjectile* projectile, int hurtID, int skillID, int skillLevel, CharacterData* target, int theMarkID);
        static int simulateCalcHurt(AIContext& aiContext, CharacterData* source, AIProjectile* projectile, int hurtID, int skillID, int skillLevel, CharacterData* target);
		static void checkAndExecuteTalent(AIContext& aiContext, CharacterData* source, CharacterData* target, TalentTrigger execTrigger);
		//免伤率=受伤者实际物理防御/(受伤者实际物理防御+攻击者等级*A+B)
		static int getOriginalAvoidanceHurt(int hurtType, CharacterData *pTarget, CharacterData *pSource, int baseHurtValue);
		static void characterDeadByHurt(AIContext& aiContext, CharacterData *pTarget, CharacterData *pSource, int skillID, int theSkillLevel, HurtExec* pHurtExec, InnerEffectExec* pEffectExec = nullptr);
		static void characterDeadByHurtNative(AIContext& aiContext, CharacterData *pTarget, CharacterData *pSource, int skillID, int theSkillLevel, HurtExec* pHurtExec, InnerEffectExec* pEffectExec = nullptr);
		
		static void ToTriggerMarkCasterFun(AIContext& aiContext, CharacterData& target, int skillID, int skillLevel, int hurtType, int hurtProperty, int HPDelta = 0, int hitType = 0, CharacterData* trigger = nullptr, int hurtID = 0);

        // 返回实际加血量
        static int HurtFuncHealTarget(AIContext& aiContext, CharacterData& source, CharacterData& target, int originHealValue, int specialEffectLabel, int skillID, bool isSimulate, int theMarkID, bool isCrit = false);
        static int HurtFuncGetRealtimeHurtProperty(CharacterData& source, int originProperty);

        static HitResult HurtFuncHurtTargetScript(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param);
        static void HurtFuncHurtTargetNative(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param, HitResult *outresult);
        static void HurtFuncHurtTarget(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param, HitResult *outresult);
        static int HurtFuncShareHurt(AIContext& aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param);

        static bool HurtFuncCanTransferHurt(AIContext& aiContext, CharacterData* transfer, int HPDelta);
        static bool HurtFuncTransferHurt(AIContext &aiContext, CharacterData *source, CharacterData *transfer, HurtCalcParam &param);
        
        static void handleHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec);

		// 只对护盾造成伤害
		static void HurtShield(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param, std::vector<int> &shieldIds, int &out_hurtnum);


        //28.7 
        //部分通用方法处理
        static bool HurtOverCommonOne(int& repelEffectNum, int& HPDelta, bool& isBypassed, HitResult* outresult);

    private:
        static float HurtFuncGetBaseCHP(HurtType hurtType, CharacterData* hero);
        static int HurtFuncGetBaseAP(HurtType hurtType, CharacterData* hero);
        static int HurtFuncGetHealValue(AIContext& aiContext, CharacterData& hero, int originValue, int theMarkID);
        static int HurtFuncGetReplEffectForProperty(int hurtProperty);

        static void HurtFuncReboundAndAbsorb(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param);
        
        static void recursiveRandomOneValidExec(AIContext &aiContext, CharacterData *target, std::vector<int> &execs);
        static int getInitHurtDelta(CharacterData *source, int skillLevel, HurtExec *hurtExec);
		static int getPropertyHurtDelta(AIContext &aiContext, CharacterData *source, CharacterData *target, int HPDelta = 0);

        static int getElementHurtDelta(AIContext& aiContext, CharacterData* source, CharacterData* target, int HPDelta = 0);

        static bool isTargetInSpecialAction(CharacterData *target);
        static void handleNormalHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec);
        static void handleKnockBackHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec, bool isRandomDirection);
        static void handleKnockFlyHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec, bool isRandomDirection);
        static void handleKnockDownHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec);
        static void handleKnockForceDownHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec);
	};

}

#endif /* defined(__OnePiece__HeroHurtcalculate__) */
