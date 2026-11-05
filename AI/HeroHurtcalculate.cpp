//
//  HeroHurtcalculate.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/11/14.
//
//

#include "HeroHurtcalculate.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <cmath>
#include <climits>
#include "AISpecialEffectLabel.h"
#include "action_hero/HeroStruckFlyAction.h"
#include "action_hero/HeroStruckBackAction.h"
#include "action_hero/HeroDeadAction.h"
#include "action_hero/HeroInhurtAction.h"
#include "AIContext.h"
#include "action_hero/HeroActionController.h"
#include "AIContinuousSummonCaster.h"
#include "CommonType.h"
#include "AIBattleStats.h"
#include "BattleLog.h"
#include "action_hero/HeroSkillController.h"
#include "action_hero/HeroIdleAction.h"
#include "action_hero/HeroAttackAction.h"
#include "AIScriptBinder.hpp"
#include "scriptbind/AIScriptBindHurtCalculate.hpp"
#include "scriptbind/AIScriptBindActionController.hpp"
#include "scriptbind/AIScriptBindEffectCaster.hpp"


namespace OnePiece {
    
    static bool isOneTimeHurt(int casterId, AIInnerEffect *innereffect)
    {
        if (casterId <= 0)
            return false;
        if (IsHurtExec(casterId)) {
            auto hurtdata = AIDataHelper::getHurtExec(casterId);
            if (hurtdata) {
                if (hurtdata->duration > 0 && hurtdata->freq > 0 && hurtdata->duration / hurtdata->freq > 1)
                    return false;
            }
        } else if (IsEffectExec(casterId)) {
            auto effectdata = AIDataHelper::getEffectExec(casterId);
            if (effectdata && innereffect) {
                if (innereffect->innerExec->interval > 0 && effectdata->period > 0 && effectdata->period / innereffect->innerExec->interval > 1)
                    return false;
            }
        }
        return true;
    }

    float HeroHurtcalculate::HurtFuncGetBaseCHP(HurtType hurtType, CharacterData* hero)
    {
        if (HurtType::HEALING_HURT_TYPE == hurtType &&
            hero->getBAPower() > hero->getMAPower()
            )
        {
            hurtType = HurtType::PHYSICAL_HURT_TYPE;
        }

        return (HurtType::PHYSICAL_HURT_TYPE == hurtType ?
            (float)hero->getBACritM() / 100 :
            (float)hero->getMACritM() / 100);
    }

    int HeroHurtcalculate::HurtFuncGetBaseAP(HurtType hurtType, CharacterData* hero)
    {
        if (HurtType::HEALING_HURT_TYPE == hurtType &&
            hero->getBAPower() > hero->getMAPower()
            )
        {
            hurtType = HurtType::PHYSICAL_HURT_TYPE;
        }
        return (HurtType::PHYSICAL_HURT_TYPE == hurtType ?
            hero->getBAPower() :
            hero->getMAPower());
    }

    int HeroHurtcalculate::HurtFuncGetHealValue(AIContext& aiContext, CharacterData& hero, int originValue, int theMarkID)
    {
        int healValue = originValue*(1 + hero.getHPAdd3() / 100);
        healValue += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, hero, EffectType::eEFFECT_HEALING_DOWN, originValue);
        healValue += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, hero, EffectType::eEFFECT_HEAL_ADD, originValue);

        //镜像造成治疗的倍率
        if (hero.isSummon() && hero.summonProperty)
            healValue = healValue * hero.summonProperty->treatRate / 100;

        // MARK: 9.0修改
        //if (hero.hasEffect(EffectType::eEFFECT_HEAL_ADD))
        //    originValue = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, hero, EffectType::eEFFECT_HEAL_ADD, originValue);
        
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, hero, EffectType::eEFFECT_ALLOW_NEGTIVE_HEAL, 0) > 0)
        {
            if (healValue < 0 && theMarkID > 0 && hero.hasEffect(eEFFECT_HURT_DOWN_FROM_MARK))
            {
                int reduceValue = AIHurtDownFromMarkEffect::checkExecute(aiContext, &hero, theMarkID, -healValue);
                healValue += reduceValue;
            }
            return healValue;
        }

        return std::max(0, healValue);
    }

    int HeroHurtcalculate::HurtFuncGetReplEffectForProperty(int hurtProperty)
    {
        switch (hurtProperty)
        {
        case HurtPropertyType::kNoProperty:
            break;
        case HurtPropertyType::kHurtLei:
            return EffectType::eEFFECT_HURT_PROPERTY_REPL_LEI;
            break;
        case HurtPropertyType::kHurtGuang:
            return EffectType::eEFFECT_HURT_PROPERTY_REPL_GUANG;
            break;
        case HurtPropertyType::kHurtHuo:
            return EffectType::eEFFECT_HURT_PROPERTY_REPL_HUO;
            break;
        case HurtPropertyType::kHurtShui:
            return EffectType::eEFFECT_HURT_PROPERTY_REPL_SHUI;
            break;
        case HurtPropertyType::kHurtAn:
            return EffectType::eEFFECT_HURT_PROPERTY_REPL_AN;
            break;
        case HurtPropertyType::kHurtDu:
            return EffectType::eEFFECT_HURT_PROPERTY_REPL_DU;
            break;
        default:
            break;
        }

        return -1;
    }

    // 简单判断是否可转移伤害
    bool HeroHurtcalculate::HurtFuncCanTransferHurt(AIContext& aiContext, CharacterData* transfer, int HPDelta)
    {
        bool isFatalHit = (HPDelta >= transfer->getHP());
        if (!isFatalHit)
            return false;

        int transEffectValue = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *transfer, eEFFECT_FATAL_HURT_TRANS, 0);
        bool hasTransferEffect = (transEffectValue > 0);
        if (!hasTransferEffect)
            return false;

        return true;
    }

    // 伤害转移是否成功
    bool HeroHurtcalculate::HurtFuncTransferHurt(AIContext &aiContext, CharacterData *source, CharacterData *transfer, HurtCalcParam &param)
    {
        auto pFatalEffect = AIFatalHurtTransferEffect::getValidFatalTransferEffect(transfer, aiContext.currentTick, source->myID);
        if (!pFatalEffect)
            return false;

        if (param.isSimulateCalc)
            return true;

        CharacterData* receiver = pFatalEffect->origin;
        HurtFuncHurtTarget(aiContext, source, receiver, param, nullptr);

        AIFatalHurtTransferEffect::activateFatalTransferEffect(transfer, aiContext.currentTick, source->myID, pFatalEffect);

        if (receiver->getHP() <= 0 && !HeroActionController::isDeadAction(receiver->currentAction))
            characterDeadByHurt(aiContext, receiver, source, param.skillID, param.skillLevel, nullptr, nullptr);

        return true;
    }

    void HeroHurtcalculate::HurtFuncReboundAndAbsorb(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param)
    {
        int HPDelta = param.HPDelta;
        int hurtType = param.hurtType;
        int hitType = param.hitType;
        //CharacterData *source = param.pSource;

        //反弹
        int tmpHPDelta1 = 0;
        if (HPDelta > 0)
        {
            tmpHPDelta1 = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_REBOUND_HP, source->getHP()); // 113
            if (hurtType == PHYSICAL_HURT_TYPE)
                tmpHPDelta1 += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_REBOUND_BA_HURT, HPDelta); // 135
            else if (hurtType == MAGICAL_HURT_TYPE)
                tmpHPDelta1 += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_REBOUND_MA_HURT, HPDelta); // 136
        }
        
        // 12.0: sync from lua
        if (param.HPDeltaBeforeAvoidHurt > 0 && target->hasEffect(283))
            tmpHPDelta1 += luahelper_innereffect_283_check_do(aiContext.getLuaState(), target, param.hurtType, param.HPDeltaBeforeAvoidHurt);

//        if (tmpHPDelta1 != 0)
//        {
//            AISpecialEffectLabel::excute(hitType, -1 * tmpHPDelta1, aiContext, source);
//
//            int hpOffset = tmpHPDelta1 <= source->getHP() ? tmpHPDelta1 : source->getHP();
//            int statsHurtType = (hurtType >= 0) ? hurtType : PHYSICAL_HURT_TYPE;
//            StatsFunc::AppendHeroHurt(*target, HurtType(statsHurtType), hpOffset, *source);
//#if BATTLE_LOG_TEST
//            OPBattleLog("rebound", source->teamSide, source->clusterNum, source->attributes[0], target->teamSide, target->clusterNum, target->attributes[0], tmpHPDelta1, HeroActionController::getSkillID(target->actionAtt) % 10);
//#endif
//        }

        //vmp吸血
        float *PS = AIDataHelper::getFightingParameter(9);
        //吸血百分比 = A%*攻击者吸血值 / (攻击者吸血值*B + 受伤者等级*C + D)
        int theSourceHPAdd1 = source->getHPAdd1();
        theSourceHPAdd1 += AIChangeHPAdd1ByMarkEffect::getExtraHpSuck(aiContext, source, target); // 278
        float vmpPS = PS[0] * theSourceHPAdd1 / (theSourceHPAdd1*PS[1] + PS[2] * target->getLv() + PS[3]);
        // 275
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, eEFFECT_IGNORE_FIGHT_PARAM_LIMIT, 0) > 0)
        {
            if (vmpPS < 0)
                vmpPS = 0;
            if (vmpPS > 1.0f)
                vmpPS = 1.0f;
        }
        else
        {
            if (vmpPS<PS[6])
                vmpPS = PS[6];
            if (vmpPS>PS[7])
                vmpPS = PS[7];
        }

        //int tmpHPDelta2 = hurtType == HeroHurtcalculate::PHYSICAL_HURT_TYPE ? HPDelta * vmpPS : 0;
        int tmpHPDelta2 = 0;
        // 177
        if ((hurtType == PHYSICAL_HURT_TYPE) ||
            (hurtType == MAGICAL_HURT_TYPE && AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_MAG_BLOOD_SUCK, HPDelta) > 0))
        {
            tmpHPDelta2 = HPDelta * vmpPS;
        }

//        if (tmpHPDelta2 != 0)
//        {
//            AISpecialEffectLabel::excute(hitType == 100000 ? 100002 : 100003, tmpHPDelta2, aiContext, source);
//#if BATTLE_LOG_TEST
//            OPBattleLog("vmp", source->teamSide, source->clusterNum, source->attributes[0], target->teamSide, target->clusterNum, target->attributes[0], tmpHPDelta2, HeroActionController::getSkillID(target->actionAtt) % 10);
//#endif
//        }

        int resultHP = tmpHPDelta1 - tmpHPDelta2;
        if (resultHP > 0)
        {
            auto reboundParam = param;
            reboundParam.shouldSkipRebound = true;
            reboundParam.HPDelta = resultHP;
            reboundParam.statHurtSourceAlt = target;
            reboundParam.ignoreHurtLabel = false;
            HurtFuncHurtTarget(aiContext, source, source, reboundParam, nullptr);

            //自己被弹死
            if (source->getHP() <= 0)
                HeroHurtcalculate::characterDeadByHurt(aiContext, source, target, param.skillID, param.skillLevel, nullptr, nullptr);
        }
        else if (resultHP < 0)
        {
            //cui 2023_2_27 文件暴击显示修改
            int hitId = 0;
            if (hitType == 100000) {
                hitId = 100002;
            }
            else {
                hitId = 100003;
            }
            bool isCrit = AISpecialEffectLabel::checkShowCrit(hitId);
            if (param.hurtExec && param.hurtExec->realHurtDisplayId != 0) {
                hitId = param.hurtExec->realHurtDisplayId;
            }
            AISpecialEffectLabel::excute(hitId, -resultHP, aiContext, source, isCrit);
            //AISpecialEffectLabel::excute(hitType == 100000 ? 100002 : 100003, -resultHP, aiContext, source);
            source->updateHP(source->getHP() - resultHP);
        }
    }

    HitResult HeroHurtcalculate::HurtFuncHurtTargetScript(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param)
    {
        HitResult outresult(HitResult::HitResultType::HIT_HASHURT, HitResult::BlockType::HIT_UNBLOCKED);
        param.HPDelta = luahelper_hurttarget(aiContext.getLuaState(), source, target, param, outresult);
        return outresult;
    }

    bool HeroHurtcalculate::HurtOverCommonOne(int& repelEffectNum, int& HPDelta, bool& isBypassed,HitResult* outresult)
    {
        if (repelEffectNum > 0 && repelEffectNum >= HPDelta)
            isBypassed = true;

        HPDelta -= repelEffectNum;
        if (HPDelta <= 0) {
            if (outresult) {
                if (isBypassed)
                    outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                else
                    outresult->setResult(HitResult::HitResultType::HIT_NOHURT, HitResult::BlockType::HIT_UNBLOCKED);
            }
            return true;
        }
        return false;
    }
    
    void HeroHurtcalculate::HurtFuncHurtTargetNative(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param, HitResult *outresult)
    {
        // 506
        if (!param.isSimulateCalc && !param.isFromHpLost && param.HPDelta != 0) {
            param.HPDelta = AIHpLostEffect::checkExcute(aiContext, source, target, param);
        }

        if (param.HPDelta == 0) {
            if (outresult)
                outresult->setResult(HitResult::HitResultType::HIT_NOHURT, HitResult::BlockType::HIT_UNBLOCKED);
            return;
        }
        
        // param.initHpDelta = param.HPDelta -- not used now
        
        // 265
        auto shieldAbsorbHP = AIShieldTransferHurtEffect::checkExecute(aiContext, target, param.HPDelta, param.isSimulateCalc);
        if (shieldAbsorbHP != 0) {
            param.HPDelta -= shieldAbsorbHP;
            if (param.HPDelta <= 0) {
                param.HPDelta = 0;
                if (outresult)
                    outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                return;
            }
        }
        
        // 242
        if (!param.isFromSacrificeRecv) {
            param.HPDelta = AISacrificeHurtSend::checkExcute(aiContext, source, target, param);
            if (param.HPDelta == 0) {
                if (outresult)
                    outresult->setResult(HitResult::HitResultType::HIT_NOHURT, HitResult::BlockType::HIT_UNBLOCKED);
                return;
            }
        }
        
        // 172
        if (AIFatalHurtTransferEffect::getActivatedFatalTransEff(target, aiContext.currentTick, source->myID)) {
            bool transSucceed = HurtFuncTransferHurt(aiContext, source, target, param);
            if (transSucceed) {
                if (param.isSimulateCalc)
                    param.HPDelta = 0;
                if (outresult)
                    outresult->setResult(HitResult::HitResultType::HIT_HASHURT, HitResult::BlockType::HIT_UNBLOCKED);
                return;
            }
        }
        
        // 排除队友和船炮
        if (source->teamSide != target->teamSide && source->k != 100 && !param.isSimulateCalc)
            target->attacker = source;
        
        int &HPDelta = param.HPDelta;
        
        if (param.markID > 0) {
            // 259
            HPDelta -= AIHurtDownFromMarkEffect::checkExecute(aiContext, target, param.markID, HPDelta);
            if (HPDelta < 0)
                HPDelta = 0;
        }
        
        // 243
        HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, eEFFECT_ADD_HURT, HPDelta);
        
        param.HPDeltaBeforeAvoidHurt = HPDelta; // 12.0 sync from lua
        
        if (!param.isRawHurt) {
            // 174
            HPDelta += EffectHelper::checkExcuteCauseAddPropertyHurtResult(source, param.hurtProperty, HPDelta, param.isSimulateCalc);
            
            //减伤
            HPDelta = HeroHurtcalculate::getOriginalAvoidanceHurt(param.hurtType, target, source, HPDelta);
            bool isTargetFaceAttacker = false;
            if (source->direction != target->direction)
                isTargetFaceAttacker = ((target->getX() - source->getX()) * source->direction >= 0);
            
            // 属性伤害减免
            int propHurtAdd = 0;
            
            // 207
            if (!param.isFromEffectCaster)
                propHurtAdd = EffectHelper::checkActivatePropHurtAdd(target, param.hurtProperty, HPDelta);
            
            if (param.hurtType == PHYSICAL_HURT_TYPE) {
                // 117
                HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_PHY_HARM_DOWN, HPDelta);
                // 190
                if (isTargetFaceAttacker)
                    HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_PHY_HARM_FACE_DOWN, HPDelta);
            } else if (param.hurtType == MAGICAL_HURT_TYPE) {
                // 118
                HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_MAG_HARM_DOWN, HPDelta);
                // 191
                if (isTargetFaceAttacker)
                    HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_MAG_HARM_FACE_DOWN, HPDelta);
            }
            
            // 属性伤害减免效果与117,118效果用加减法叠加
            HPDelta += propHurtAdd;

            //510  28.7 cui_2023_2_20
            /*
                param[....] 判断存在的标签138 139 273.....
            */
            if (source && source->hasEffect(EffectType::eEFFECT_HURT_CHANGE_BY_EFFECT)) {
                auto iter = source->m_aiEffectMap.find(EffectType::eEFFECT_HURT_CHANGE_BY_EFFECT);
                bool isFind = false;
                for (AIInnerEffect* pEff : iter->second)
                {
                    if (pEff->innerExec->params.size() == 0) {
                        isFind = true;
                    }
                    for (size_t i = 0; i < pEff->innerExec->params.size(); i++)
                    {
                        if (target->hasEffect(pEff->innerExec->params[i])) {
                            isFind = true;
                            break;
                        }
                    }
                    if (isFind) {
                        break;
                    }
                }
                if (isFind) {
                    HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_HURT_CHANGE_BY_EFFECT, HPDelta);
                }
            }
            
            // 192
            if (isTargetFaceAttacker)
                HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_ALL_HARM_FACE_DOWN, HPDelta);
            
            if (HPDelta < 0)
                HPDelta = 0;
            
            if (!param.isFromEffectCaster && source->isSummon() && source->summonProperty) {
                HPDelta = HPDelta * source->summonProperty->attackRate / 100;
                // 173
                if (source->getFakeSummonType() == 0) // 12.0
                    HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_SUMMON_HARM_ADD, HPDelta);
            }
            
            // 149
			// 25.0起改变效果值的使用规则,效果值表示增加的伤害量
            if (target->hasEffect(EffectType::eEFFECT_HURT_ADD))
                HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_HURT_ADD, HPDelta);
            
            //免伤优先级
            // 伤害阈值 > 林肯法球 > 物理/法术伤害免疫 > 伤害吸收护盾 > 属性伤害免疫
            
            //伤害达到一定阈值才能造成伤害
            // 141
            int lowlimit = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_ONLY_HURT_ABOVE_CONDITION, target->getHPTop());
            if (lowlimit > 0 && HPDelta < lowlimit) {
                if (outresult)
                    outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                return;
            }
            
            // 伤害小于一定阈值伤害不生效，且触发执行id 271 clair_wang
            if (param.hurtType == PHYSICAL_HURT_TYPE || param.hurtType == MAGICAL_HURT_TYPE) {
                // 271
                int lowlimit2 = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_ENDURE_HURT_BY_CONDITION, target->getHPTop());
                if (lowlimit2 > 0 && HPDelta < lowlimit2)
                {
                    if (!param.isSimulateCalc)
                        AIEndureHurtByConditionEffect::checkExecute(aiContext, target);
                    
                    if (outresult)
                        outresult->setResult(HitResult::HitResultType::HIT_NOHURT, HitResult::BlockType::HIT_UNBLOCKED);
                    
                    return;
                }
            }
            
            // 178
            if (AILinkenSphereEffect::activateLinkenSphereEffect(source, target, param.isSimulateCalc)) {
                if (outresult)
                    outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                return;
            }
            
            bool isBypassed = false;
            int hurtType = param.hurtType;
            
            bool isBAMAHurt = false;

            // 9.0
            int repelEffectNum = 0;
            if (hurtType == PHYSICAL_HURT_TYPE) {
                isBAMAHurt = true;
                // 110
                repelEffectNum = AIRepelEffect::checkExecute(eEFFECT_BAREPEL_ADD, aiContext, target, HPDelta, param.isSimulateCalc);
            } else if (hurtType == MAGICAL_HURT_TYPE) {
                isBAMAHurt = true;
                // 111
                repelEffectNum = AIRepelEffect::checkExecute(eEFFECT_MAREPEL_ADD, aiContext, target, HPDelta, param.isSimulateCalc);
            }
            
            if (repelEffectNum > 0 && repelEffectNum >= HPDelta)
                isBypassed = true;
            HPDelta -= repelEffectNum;
            if (HPDelta <= 0) {
                if (outresult) {
                    if (isBypassed)
                        outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                    else
                        outresult->setResult(HitResult::HitResultType::HIT_NOHURT, HitResult::BlockType::HIT_UNBLOCKED);
                }
                return;
            }


            // 15.0
            if (isBAMAHurt && target->hasEffect(312))
            {
                isBypassed = false;
                repelEffectNum = luahelper_innereffect_312_trigger(aiContext.getLuaState(), target, hurtType, HPDelta, param.isSimulateCalc);
                if (repelEffectNum > 0 && repelEffectNum >= HPDelta)
                    isBypassed = true;

                HPDelta -= repelEffectNum;
                if (HPDelta <= 0) {
                    if (outresult) {
                        if (isBypassed)
                            outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                        else
                            outresult->setResult(HitResult::HitResultType::HIT_NOHURT, HitResult::BlockType::HIT_UNBLOCKED);
                    }
                    return;
                }
            }

            // OP2
            if (target->hasEffect(320))
            {
                isBypassed = false;
                repelEffectNum = luahelper_innereffect_320_trigger(aiContext.getLuaState(), target, hurtType, param.hurtProperty, HPDelta, param.isSimulateCalc);
                if (repelEffectNum > 0 && repelEffectNum >= HPDelta)
                    isBypassed = true;

                HPDelta -= repelEffectNum;
                if (HPDelta <= 0) {
                    if (outresult) {
                        if (isBypassed)
                            outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                        else
                            outresult->setResult(HitResult::HitResultType::HIT_NOHURT, HitResult::BlockType::HIT_UNBLOCKED);
                    }
                    return;
                }
            }
            
            isBypassed = false;
            // 112
            repelEffectNum = AIRepelEffect::checkExecute(eEFFECT_ALL_REPEL, aiContext, target, HPDelta, param.isSimulateCalc);
            if (repelEffectNum > 0 && repelEffectNum >= HPDelta)
                isBypassed = true;
            HPDelta -= repelEffectNum;
            if (HPDelta <= 0) {
                if (outresult) {
                    if (isBypassed)
                        outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                    else
                        outresult->setResult(HitResult::HitResultType::HIT_NOHURT, HitResult::BlockType::HIT_UNBLOCKED);
                }
                return;
            }
            
            //伤害吸收护盾
            int repel = 0;
            if (hurtType == PHYSICAL_HURT_TYPE)
                // 138
                repel = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_BAREDUCE_ADD, HPDelta);
            else if (hurtType == MAGICAL_HURT_TYPE)
                // 139
                repel = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_MAREDUCE_ADD, HPDelta);
            
            if (repel > 0) {
                if (repel >= HPDelta) {
                    if (outresult)
                        outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                    return;
                }
                
                HPDelta -= repel;
            }
            // 12.0 use lua
            if (target->hasEffect(273)) {
                repel = luahelper_innereffect_273_trigger(aiContext.getLuaState(), target, HPDelta, param.isSimulateCalc);

                if (repel >= HPDelta) {
                    if (outresult)
                        outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                    return;
                }

                HPDelta -= repel;
            }

			// 503
			if (source->hasEffect(503)) {
				HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_HURT_ADD_EXTRA, HPDelta);
			}

			// 504
			if (target->hasEffect(504)) {
				HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_HURT_SUB_EXTRA, HPDelta);
			}

			HPDelta += getPropertyHurtDelta(aiContext, source, target, HPDelta);

            //cui_2023_7_7 元素之力增减伤害计算
            HPDelta = getElementHurtDelta(aiContext, source, target, HPDelta);

            //28.7 治疗量溢 互盾吸收伤害 cui_2023_1_19
            if (target->hasEffect(509)) {
                isBypassed = false;
                repelEffectNum = AIEffect509::checkAbsorbHurt(aiContext, target, HPDelta, hurtType, param.isPureHurt);
                if (HeroHurtcalculate::HurtOverCommonOne(repelEffectNum, HPDelta, isBypassed, outresult)) {
                    return;
                }
            }
            
            //伤害上限阈值
            // 154
            int hurtLimit = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_HURT_LIMIT, target->getHPTop());
            auto pSkill = AIDataHelper::getSkillData(param.skillID);
            if (pSkill && pSkill->ignoreEffects.find(eEFFECT_HURT_LIMIT) != pSkill->ignoreEffects.end())
                hurtLimit = 0;
            if (hurtLimit > 0 && HPDelta > hurtLimit)
                HPDelta = hurtLimit;
            // 507
            int hurtLimit0 = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_NEW_HURT_LIMIT, target->getHPTop());
            if (pSkill && pSkill->ignoreEffects.find(eEFFECT_NEW_HURT_LIMIT) != pSkill->ignoreEffects.end())
                hurtLimit0 = 0;
            if (hurtLimit0 > 0 && HPDelta > hurtLimit0)
                HPDelta = hurtLimit0;
            
            bool isBlocked = false;
            
            //属性伤害免疫
            int blockEffect = HurtFuncGetReplEffectForProperty(param.hurtProperty);
            
            if (blockEffect > 0)
                isBlocked = (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, (EffectType)blockEffect, 0) > 0);
            
            if (!isBlocked) {
                // 226
                if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_SHIELD_BYCOUNT_ALL_HURT, 0) > 0) {
                    isBlocked = true;
                    if (!param.isSimulateCalc)
                        AIContinuousEffectCaster::change_Special_Effect(aiContext, *target, EffectType::eEFFECT_SHIELD_BYCOUNT_ALL_HURT, 1);
                }
            }
            
            if (!isBlocked && hurtType != HEALING_HURT_TYPE && hurtType >= 0) {
                //类型伤害护盾-按伤害次数
                int shieldEffect = hurtType == PHYSICAL_HURT_TYPE ? EffectType::eEFFECT_SHIELD_BYCOUNT_PHYSICS : EffectType::eEFFECT_SHIELD_BYCOUNT_MAGIC;
                if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, (EffectType)shieldEffect, 0) > 0) {
                    // 157 or 158
                    isBlocked = true;
                    if (!param.isSimulateCalc)
                        AIContinuousEffectCaster::change_Special_Effect(aiContext, *target, (EffectType)shieldEffect, 1);
                } else if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_SHIELD_BYCOUNT_BOTHPHMA, 0) > 0) {
                    // 159
                    isBlocked = true;
                    if (!param.isSimulateCalc)
                        AIContinuousEffectCaster::change_Special_Effect(aiContext, *target, EffectType::eEFFECT_SHIELD_BYCOUNT_BOTHPHMA, 1);
                }
            }
            
            if (isBlocked) {
                HPDelta = 0;
                if (outresult)
                    outresult->setResult(HitResult::HitResultType::HIT_HASHURT, HitResult::BlockType::HIT_BLOCKED);
                return;
            }
            
            if (!param.isSimulateCalc)
                HeroHurtcalculate::checkAndExecuteTalent(aiContext, source, target, HeroHurtcalculate::TalentTrigger::HIT);
            
            //击中硬值
            if (param.hurtExec && !param.isSimulateCalc) {
                HeroActionController::setYingzhi(source->actionAtt, param.hurtExec->hitStunP);
                source->totalTick = source->totalTick + param.hurtExec->hitStunP;
            }
            
            //针对被攻击对象是镜像的情况
            if (!param.isFromEffectCaster && target->isSummon() && target->summonProperty) {
                HPDelta = HPDelta * target->summonProperty->beAttackRate / 100;
            }
        } else { // END not raw hurt


            //伤害上限阈值
            // 507
            int hurtLimit0 = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_NEW_HURT_LIMIT, target->getHPTop());
            auto pSkill = AIDataHelper::getSkillData(param.skillID);
            if (pSkill && pSkill->ignoreEffects.find(eEFFECT_NEW_HURT_LIMIT) != pSkill->ignoreEffects.end())
                hurtLimit0 = 0;
            if (hurtLimit0 > 0 && HPDelta > hurtLimit0)
                HPDelta = hurtLimit0;
        }

        //511 cui 2023_3_1 溅射伤害
        AIEffect511::checkExecute(aiContext, target, source, HPDelta, param.hurtProperty, param.isSimulateCalc);
        
        // 211
        auto resistNReboundVal = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, eEFFECT_RESIST_N_REBOUND, HPDelta);
        if (resistNReboundVal > 0) {
            if (param.isSimulateCalc) {
                HPDelta = 0;
            } else {
                auto reboundParam = param;
                reboundParam.shouldSkipRebound = true;
                reboundParam.HPDelta = resistNReboundVal;
                reboundParam.statHurtSourceAlt = target;
                
                EffectHelper::activateResistNReboundEff(aiContext, target, source, param.skillID, param.skillLevel);
                // HQ:先触发移除反弹效果,防止两个人都有该效果时无限反弹
                AIContinuousEffectCaster::zero_Special_Effect(aiContext, *target, eEFFECT_RESIST_N_REBOUND);
                HurtFuncHurtTargetNative(aiContext, source, source, reboundParam, nullptr);
                // HQ:被自己的伤害反弹死,算被自己打死
                if (source->getHP() <= 0)
                    HeroHurtcalculate::characterDeadByHurt(aiContext, source, source, param.skillID, param.skillLevel, nullptr, nullptr);
            }
            
            if (outresult)
                outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_BLOCKED);
            return;
        }

        // 326
        if (target->hasEffect(326))
        {
            if (param.isSimulateCalc) 
            {
                HPDelta = 0;
            }
            else
            {
                luahelper_innereffect_326_trigger(aiContext.getLuaState(), source, target, param.skillID, param.skillLevel);
                AIContinuousEffectCaster::zero_Special_Effect(aiContext, *target, 326);
            }
            if (outresult)
                outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_BLOCKED);
            return;
        }

        // OP2 - 10001
        if (target->hasEffect(10001))
        {
            int repelnum = luahelper_innereffect_10001_trigger(aiContext.getLuaState(), target, param.hurtType, param.HPDelta, param.isSimulateCalc, param.hitType);
            if (repelnum)
            {
                param.HPDelta -= repelnum;
                if (param.HPDelta <= 0) {
                    param.HPDelta = 0;
                    if (outresult)
                        outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                    return;
                }
            }
        }
        
        int realHpHurt = 0;
        int tmpHPDelta = 0;
        if (target->hasEffect(EffectType::eEFFECT_HP_LOCK)) {
            if (HPDelta && !param.isSimulateCalc && aiContext.businessType != BusinessType::CANNON_KING && !param.ignoreHurtLabel) {
                AISpecialEffectLabel::excute(param.alternateHitLabel > 0 ? param.alternateHitLabel : param.hitType, INT_MIN, aiContext, target);
            }
            HPDelta = 0;
        }
        else {
            // 179
            HPDelta = AIFlimsyBuryingEffect::activateFilmsyBuringEffect(target, HPDelta, param.isSimulateCalc);
            
            HPDelta = HurtFuncShareHurt(aiContext, source, target, param);
            realHpHurt = HPDelta;

            //回血
            // 102
            tmpHPDelta = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_RESTORE_TMP, HPDelta);
        }
        
        if (tmpHPDelta > 0) {
            if (!param.isSimulateCalc) {
                if (target->hasEffect(316))
                {
                    int replTmpDelta = luahelper_innereffect_316_trigger(aiContext.getLuaState(), target, tmpHPDelta);
                    if (tmpHPDelta > replTmpDelta)
                    {
                        tmpHPDelta -= replTmpDelta;
                    }
                    else
                    {
                        tmpHPDelta = 0;
                    }
                }
                if (tmpHPDelta != 0) {
                    int oldHP = target->getHP();
                    target->updateHP(target->getHP() + tmpHPDelta);
                    //cui 2023_2_27 飘字显示修改
                    int hitId = 0;
                    if (param.hitType == NORMAL_HIT) {
                        hitId = 100002;
                    }
                    else {
                        hitId = 100003;
                    }
                    bool isCrit = AISpecialEffectLabel::checkShowCrit(hitId);
                    if (param.hurtExec && param.hurtExec->realHurtDisplayId != 0) {
                        hitId = param.hurtExec->realHurtDisplayId;
                    }
                    AISpecialEffectLabel::excute(hitId, tmpHPDelta, aiContext, target, isCrit);
                    //AISpecialEffectLabel::excute(param.hitType == NORMAL_HIT ? 100002 : 100003, tmpHPDelta, aiContext, target);
                    
#if BATTLE_LOG_TEST
                    OPBattleLog("heal", source->teamSide, source->clusterNum, source->attributes[0], target->teamSide, target->clusterNum, target->attributes[0], tmpHPDelta, HeroActionController::getSkillID(source->actionAtt) % 10);
#endif
                    StatsFunc::AppendHeroHurt(param.statHurtSourceAlt ? *param.statHurtSourceAlt : *source, HEALING_HURT_TYPE, target->getHP() - oldHP, *target);
                    target->onHPDelta(target->getHP() - oldHP);
                }
            }
        } else if (HPDelta != 0) {
            bool isHurtTransfered = false;
            if (HurtFuncCanTransferHurt(aiContext, target, HPDelta))
                isHurtTransfered = HurtFuncTransferHurt(aiContext, source, target, param);
            
            if (isHurtTransfered) {
                realHpHurt = 0;
                HPDelta = 0;
            } else {
                if (AIEffect272::canCast(aiContext, target, HPDelta)) {
                    if (!param.isSimulateCalc)
                    {
                        AIEffect272::checkExecute(aiContext, target);
                        if (AIEffect272::checkZeroEffect(aiContext, target))
                        {
                            AIContinuousEffectCaster::zero_Special_Effect(aiContext, *target, eEFFECT_FATAL_HURT_DO_EFFECT);
                        }
                    }
                    
                    HPDelta = target->getHP() - 1;
                }
                
                // 247
                if (AIResistFatalHurtNCastEffect::canCast(aiContext, target, HPDelta)) {
                    if (!param.isSimulateCalc) {
                        AIResistFatalHurtNCastEffect::checkExcute(aiContext, target);
                        AIContinuousEffectCaster::zero_Special_Effect(aiContext, *target, eEFFECT_RESIST_FATAL_HURT_NCAST);
                    }
                    
                    param.HPDelta = 0;
                    
                    if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, eEFFECT_FATAL_HURT_DO_EFFECT, 0) <= 0) {
                        // 272、247同时存在
                        if (outresult)
                            outresult->setResult(HitResult::HitResultType::HIT_BYPASS, HitResult::BlockType::HIT_UNBLOCKED);
                        return;
                    }
                }
                
                // 269
                int theExtraHurt = AICauseExtraHurtByHp::getExtraHurt(aiContext, source, HPDelta);
                if (param.isSimulateCalc) {
                    HPDelta += theExtraHurt;
                    if (target->hasEffect(288)) {
                        int hpreduce = luahelper_innereffect_288_check_do_simu(aiContext.getLuaState(), target, source, HPDelta);
                        if (hpreduce)
                            HPDelta -= hpreduce;
                    }
					//if (source->hasEffect(503))
					//	HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_HURT_ADD_EXTRA, HPDelta);
					//if (target->hasEffect(504))
					//	HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_HURT_SUB_EXTRA, HPDelta);
                } else {
                    if (target->hasEffect(288)) {
                        int hpreduce288 = luahelper_innereffect_288_check_do(aiContext.getLuaState(), target, source, HPDelta + theExtraHurt);
                        if (hpreduce288 - theExtraHurt >= HPDelta) { // hpdelta + extrahurt <= reduce 完全被免伤，防止数值溢出
                            if (outresult)
                                outresult->setResult(HitResult::HitResultType::HIT_NOHURT, HitResult::BlockType::HIT_UNBLOCKED);
                            return;
                        } else if (hpreduce288 > 0) {
                            if (theExtraHurt > 0) {
                                if (theExtraHurt >= hpreduce288)
                                    theExtraHurt = theExtraHurt - hpreduce288;
                                else {
                                    HPDelta += theExtraHurt - hpreduce288;
                                    theExtraHurt = 0;
                                }
                            } else {
                                HPDelta -= hpreduce288;
                            }
                        }
                    }

					// 503
					//if (source->hasEffect(503)) {
					//	HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_HURT_ADD_EXTRA, HPDelta);
					//	if (theExtraHurt)
					//		theExtraHurt += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_HURT_ADD_EXTRA, theExtraHurt);
					//}

					// 504
					//if (target->hasEffect(504)) {
					//	HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_HURT_SUB_EXTRA, HPDelta);
					//	if (theExtraHurt)
					//		theExtraHurt += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_HURT_SUB_EXTRA, theExtraHurt);
					//}
                    
                    int oldHP = target->getHP();
                    target->updateHP(target->getHP() - HPDelta - theExtraHurt);
                    if (aiContext.businessType != BusinessType::CANNON_KING && !param.ignoreHurtLabel) {
                        if (HPDelta != 0) {
                            if (HPDelta > 0 || !AIScriptBinder::getInstance()->isIgnorePositiveHurtValueHitLabelAndStat())
                            {
                                //cui 2023_2_27 飘字显示修改
                                int hitId = param.hitType;
                                if (param.alternateHitLabel > 0) {
                                    hitId = param.alternateHitLabel;

                                }
                                bool isCrit = AISpecialEffectLabel::checkShowCrit(hitId);
                                if (param.hurtExec && param.hurtExec->realHurtDisplayId != 0 && !(param.alternateHitLabel > 0)) {
                                    hitId = param.hurtExec->realHurtDisplayId;
                                }
                                
                                AISpecialEffectLabel::excute(hitId, -HPDelta, aiContext, target, isCrit);
                            }

                        }
                        
                    }
                    if (theExtraHurt) {
                        // 269
                        int theExtraHitLabel = AICauseExtraHurtByHp::getHitLabel(aiContext, source);
                        if (theExtraHitLabel) {
                            if (theExtraHurt > 0 || !AIScriptBinder::getInstance()->isIgnorePositiveHurtValueHitLabelAndStat())
                                AISpecialEffectLabel::excute(theExtraHitLabel, -theExtraHurt, aiContext, target);
                        }
                    }
                    int statsHurtType = (param.hurtType >= 0) ? param.hurtType : PHYSICAL_HURT_TYPE;
                    realHpHurt = oldHP - target->getHP();
                    if (realHpHurt > 0 || !AIScriptBinder::getInstance()->isIgnorePositiveHurtValueHitLabelAndStat())
                        StatsFunc::AppendHeroHurt(param.statHurtSourceAlt ? *param.statHurtSourceAlt : *source, HurtType(statsHurtType), realHpHurt, *target);
                    target->onHPDelta(-realHpHurt);
#if BATTLE_LOG_TEST
                    auto logOrigin = source->isSummon() ? source->getSummonSource() : source;
                    OPBattleLog("dps", logOrigin->teamSide, logOrigin->clusterNum, logOrigin->attributes[0], target->teamSide, target->clusterNum, target->attributes[0], realHpHurt, HeroActionController::getSkillID(source->actionAtt) % 10);
#endif
                    HPDelta += theExtraHurt;
                    if (!param.shouldSkipRebound)
                        HurtFuncReboundAndAbsorb(aiContext, source, target, param);
                    
                    if (realHpHurt > 0 && isOneTimeHurt(param.casterID, param.innerEffect)) {
                        target->setTagValue("lastonetimehurt", realHpHurt);
                        if (target->hasEffect(287))
                            luahelper_innereffect_287_on_onetimehurt(aiContext.getLuaState(), target);
                    }
                }
            }
        }

        if (param.isSimulateCalc) {
            if (outresult)
                outresult->setResult(HitResult::HitResultType::HIT_HASHURT, HitResult::BlockType::HIT_UNBLOCKED);
            return;
        }
        
        float tmpPrecAddEnergy = (float)realHpHurt / (float)target->getHPTop() * 100.0f;
        tmpPrecAddEnergy = std::max(0.0f, std::min(tmpPrecAddEnergy, 99.0f));
        
        if (!target->hasTag("energyskill")) {
            int energyDelta = (float)target->_beHittedAddEnergy * tmpPrecAddEnergy;
            // 208
            energyDelta += EffectHelper::checkExcuteMixChangeTypeEff(target, eEFFECT_BEHIT_GAIN_ENERGY_ADD, energyDelta);
            // 175
            energyDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_HIT_GAIN_ENERGY_ADD, energyDelta);
            
            CharacterFunc::ApplyEnergyDelta(target, energyDelta);
#if BATTLE_LOG_TEST
            OPBattleLog("targetenergy", target->teamSide, target->clusterNum, target->attributes[0], (int)((float)target->_beHittedAddEnergy * tmpPrecAddEnergy));
#endif
        }

		// TODO: 327
		if (target->hasEffect(327))
			luahelper_innereffect_327_trigger(aiContext.getLuaState(), target, source, param.isRawHurt ? RAW_HURT_TYPE : param.hurtType, param.hurtProperty);
        
        if (outresult)
            outresult->setResult(HitResult::HitResultType::HIT_HASHURT, HitResult::BlockType::HIT_UNBLOCKED);

    }
    
    void HeroHurtcalculate::HurtFuncHurtTarget(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param, HitResult *outresult)
    {
        if (AIScriptBinder::getInstance()->isHurtCalcHurtTargetByLua(aiContext.getLuaState())) {
            auto result = HurtFuncHurtTargetScript(aiContext, source, target, param);
            if (outresult)
                *outresult = result;
        } else {
            HurtFuncHurtTargetNative(aiContext, source, target, param, outresult);
        }
    }

    int HeroHurtcalculate::HurtFuncHealTarget(AIContext& aiContext, CharacterData& source, CharacterData& target, int originHealValue, int specialEffectLabel, int skillID, bool isSimulate, int theMarkID,bool isCrit)
    {
        if (source.teamSide != target.teamSide && source.k != 100 && !isSimulate)
            target.attacker = &source;

        // 加血只判断是否为0,策划会用加血类型来造成伤害
        if (AILinkenSphereEffect::activateLinkenSphereEffect(&source, &target, isSimulate))
            return 0;

        if (originHealValue == 0)
            return 0;

        int realHealValue = HurtFuncGetHealValue(aiContext, target, originHealValue, theMarkID);
        if (realHealValue == 0)
            return 0;

        if (isSimulate) {
            if (target.hasEffect(EffectType::eEFFECT_HP_LOCK)) {
                realHealValue = 0;
            }
            return realHealValue;
        }

        if (realHealValue < 0)
        {
            realHealValue += AIShieldTransferHurtEffect::checkExecute(aiContext, &target, -realHealValue, isSimulate); // 265
            
            // 12.0: sync from lua
            if (target.hasEffect(288))
                realHealValue += luahelper_innereffect_288_check_do(aiContext.getLuaState(), &target, &source, -realHealValue);
            
            if (realHealValue == 0)
            {
                return 0;
            }
        }

        // 316 吸收治疗量的护盾
        if (target.hasEffect(316))
        {
            int replnum = luahelper_innereffect_316_trigger(aiContext.getLuaState(), &target, realHealValue);
            if (std::abs(replnum) >= std::abs(realHealValue))
            {
                return 0;
            }
            realHealValue -= replnum;
        }

        HeroHurtcalculate::checkAndExecuteTalent(aiContext, &source, &target, HeroHurtcalculate::TalentTrigger::BY_HURL);

        if (target.hasEffect(EffectType::eEFFECT_HP_LOCK)) {
            realHealValue = 0;
        }
        if (specialEffectLabel > 0)
            AISpecialEffectLabel::excute(specialEffectLabel, realHealValue, aiContext, &target, isCrit);

#if BATTLE_LOG_TEST
        OPBattleLog("heal", source.teamSide, source.clusterNum, source.attributes[0], target.teamSide, target.clusterNum, target.attributes[0], realHealValue, HeroActionController::getSkillID(source.actionAtt) % 10);
#endif

		int oldHP = target.getHP();

		if (realHealValue < 0 && oldHP <= -realHealValue)
		{
			if (AIEffect272::canCast(aiContext, &target, -realHealValue)) 
			{
				if (!isSimulate)
				{
					AIEffect272::checkExecute(aiContext, &target);
					if (AIEffect272::checkZeroEffect(aiContext, &target))
					{
						AIContinuousEffectCaster::zero_Special_Effect(aiContext, target, eEFFECT_FATAL_HURT_DO_EFFECT);
					}
				}

				realHealValue = -(oldHP - 1);
			}
			else if  (AIResistFatalHurtNCastEffect::canCast(aiContext, &target, -realHealValue)) {
				if (!isSimulate) {
					AIResistFatalHurtNCastEffect::checkExcute(aiContext, &target);
					AIContinuousEffectCaster::zero_Special_Effect(aiContext, target, eEFFECT_RESIST_FATAL_HURT_NCAST);
				}

				return 0;
			}
		}

        target.updateHP(target.getHP() + realHealValue);

        //检查治疗量溢出护盾  2023_1_11
        AIContinuousEffectCaster::checkHealTriggerEffect(aiContext, &source, &target, realHealValue + oldHP - target.getHPTop());

        //if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, target, EffectType::eEFFECT_ATTR_WITH_LOSEHP_ADD, 0))
        //    AIContinuousEffectCaster::change_Special_Effect(aiContext, target, EffectType::eEFFECT_ATTR_WITH_LOSEHP_ADD, 0);

        //StatsFunc::AppendHeroHurt(source, HurtType::HEALING_HURT_TYPE, target.getHP() - oldHP, target);
        int hpDiff = target.getHP() - oldHP;
        if (target.teamSide == source.teamSide)
        {
            if (hpDiff > 0)
                StatsFunc::AppendHeroHurt(source, HurtType::HEALING_HURT_TYPE, hpDiff, target);
        }
        else
        {
            if (hpDiff < 0)
                StatsFunc::AppendHeroHurt(source, HurtType::PHYSICAL_HURT_TYPE, -hpDiff, target);
        }

        target.onHPDelta(target.getHP() - oldHP);

        AIContinuousEffectCaster::executeOnHealTriggerEffect(aiContext, &source, &target);

        return realHealValue;
    }

    int HeroHurtcalculate::HurtFuncGetRealtimeHurtProperty(CharacterData& source, int originProperty)
    {
        return source.hasEffect(eEFFECT_INVALIDATE_HURT_PROPERTY) ? HurtPropertyType::kNoProperty : originProperty;
    }

    int HeroHurtcalculate::HurtFuncShareHurt(AIContext& aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param)
    {
        if (param.HPDelta <= 0)
            return param.HPDelta;
        auto iter = target->m_aiEffectMap.find(eEFFECT_SHARE_HURT);
        if (iter == target->m_aiEffectMap.end() || iter->second.size() <= 0)
            return param.HPDelta;

        std::vector<CharacterData *> shareTargets;
        auto targetFriends = aiContext.getClusterOrigin(target->teamSide, target->clusterNum);
        for (auto iter = targetFriends->begin(); iter != targetFriends->end(); ++iter)
        {
            auto pShareTarget = *iter;
            if (!pShareTarget || pShareTarget->myID == target->myID || HeroActionController::isDeadAction(pShareTarget->currentAction))
                continue;
            auto shareEff = AIShareHurtEffect::getFirstValidShareHurt(pShareTarget);
            if (shareEff)
                shareTargets.push_back(pShareTarget);
        }
        if (shareTargets.size() <= 0)
            return param.HPDelta;

        int HPDelta = param.HPDelta;
        int retHurt = 0;

        int avgHurt = HPDelta / (shareTargets.size() + 1);
        if (avgHurt <= 0) {
            avgHurt = 1;
        }

        retHurt += avgHurt;
        HPDelta -= avgHurt; // 先扣除原始目标伤害
        for (auto &&pShareTarget : shareTargets) {
            if (HPDelta <= 0) // 不够分
                break;
            HPDelta -= avgHurt;

            if (param.isSimulateCalc)
                continue;
            
            AIShareHurtEffect::activateShareHurt(AIShareHurtEffect::getFirstValidShareHurt(pShareTarget));

            auto shareHurtParam = param;
            shareHurtParam.HPDelta = avgHurt;
            shareHurtParam.isRawHurt = true;
            shareHurtParam.isSimulateCalc = param.isSimulateCalc;
            HurtFuncHurtTarget(aiContext, source, pShareTarget, shareHurtParam, nullptr);
            //HurtFuncRawHurt(aiContext, source, pShareTarget, shareHurtParam);
            if (pShareTarget->getHP() <= 0)
                HeroHurtcalculate::characterDeadByHurt(aiContext, pShareTarget, source, shareHurtParam.skillID, shareHurtParam.skillLevel, shareHurtParam.hurtExec);
        }

        if (HPDelta < 0) {
            HPDelta = 0;
        }

        // 不能整除时，剩余伤害全部给原始目标
        return retHurt + HPDelta;
    }

    int HeroHurtcalculate::getInitHurtDelta(CharacterData *source, int skillLevel, HurtExec *hurtExec)
    {
        int baseAP = HurtFuncGetBaseAP((HurtType)hurtExec->type, source);
        //攻击力系数=技能基础攻击力系数+系数成长*技能等级+ 蓄力时间 / 蓄力时间间隔*单位蓄力间隔增加的攻击力百分比
        float skillAPFactor = (float)(hurtExec->baseAPFactor + hurtExec->APBonus * skillLevel) / 100;
        if (hurtExec->accuInterval)
        {
            int theAccuTick = HeroActionController::getAccumulateTick(source->actionAtt);
            if (source->getAltAccuTick() > 0)
                theAccuTick = source->getAltAccuTick();
            skillAPFactor += (float)(theAccuTick * hurtExec->accuAttAddPerInterval / hurtExec->accuInterval) / 100.0f;
        }
        //额外伤害值=技能基础额外伤害值+额外伤害值成长*技能等级
        float skillExtraDamage = ((float)hurtExec->baseExtraHurt + hurtExec->extraHurtBonus * skillLevel) / 100;
        //未减伤伤害=（攻击方物理攻击*攻击力系数+额外伤害值）
        int HPDelta = (baseAP * skillAPFactor + skillExtraDamage);

        return HPDelta;
    }

	int HeroHurtcalculate::getPropertyHurtDelta(AIContext &aiContext, CharacterData *source, CharacterData *target, int HPDelta)
	{
		int delta = 0;
		float ratio = 10000.f;
		if (HPDelta > 0)
		{
			delta = ((source->getFuncHurtAdd() - target->getFuncHurtSub()) / ratio) * HPDelta;
			if (delta < 0 && HPDelta > 0)
			{
				int limit = HPDelta * 8 / 10;
				if (-delta > limit)
					delta = -limit;
			}
		}
		return delta;
	}

    int HeroHurtcalculate::getElementHurtDelta(AIContext& aiContext, CharacterData* source, CharacterData* target, int HPDelta)
    {
        int delta = 0;
        float ratio = 10000.f;
        if (HPDelta > 0)
        {
            float value = 1 + (source->getElementAtkAdd() - target->getElementDefAdd(source)) / ratio;
            if (value < 0.2) {
                value = 0.2;
            }
            delta = value * HPDelta;

        }
        return delta;
    }

    

    float HeroHurtcalculate::calcMissProb(AIContext& aiContext, CharacterData* source, CharacterData* target, int hurtType)
    {
        if (hurtType == HurtType::HEALING_HURT_TYPE)
            return 0;

        //未命中概率 = A% - B%*攻击方命中值/(攻击方命中值*F+攻击方等级*C+D)+（防御者等级-攻击方等级）*E%
        int HitP1 = hurtType == HurtType::PHYSICAL_HURT_TYPE ? source->getBAHit() : source->getMAHit();
        float* PS = hurtType == HurtType::PHYSICAL_HURT_TYPE ? AIDataHelper::getFightingParameter(2) : AIDataHelper::getFightingParameter(3);
        float missProb = PS[0] - PS[1] * HitP1 / (HitP1*PS[5] + PS[2] * source->getLv() + PS[3]) + (target->getLv() - source->getLv()) *  PS[4];
        //if (hurtType == HurtType::HEALING_HURT_TYPE) missProb = 0;
        if (missProb<PS[6]) missProb = PS[6];
        if (missProb>PS[7]) missProb = PS[7];

        missProb -= hurtType == HurtType::PHYSICAL_HURT_TYPE ?
            AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_BAHIT_ADD, missProb) :
            AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_MAHIT_ADD, missProb);

        if (missProb<PS[6]) missProb = PS[6];
        if (missProb>PS[7]) missProb = PS[7];

        return missProb;
    }

    float HeroHurtcalculate::calcEscProb(AIContext& aiContext, CharacterData* source, CharacterData* target, int hurtType)
    {
        if (hurtType == HurtType::HEALING_HURT_TYPE)
            return 0;

        //受伤者基础闪避率=A%*受伤者闪避值/(受伤者闪避值*E+受伤者等级*B+C)+（防御者等级-攻击方等级）*D%
        int EscP2 = hurtType == HurtType::PHYSICAL_HURT_TYPE ? target->getBADodge() : target->getMADodge();
        float *PS = hurtType == HurtType::PHYSICAL_HURT_TYPE ? AIDataHelper::getFightingParameter(4) : AIDataHelper::getFightingParameter(5);
        float EscProb = PS[0] * EscP2 / (EscP2*PS[4] + PS[1] * target->getLv() + PS[2]) + (target->getLv() - source->getLv()) *PS[3];
        //if (hurtType == HurtType::HEALING_HURT_TYPE) EscProb = 0;

        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, eEFFECT_IGNORE_FIGHT_PARAM_LIMIT, 0) > 0)
        {
            if (EscProb < 0) EscProb = 0;
            if (EscProb > 100.0f) EscProb = 100.0f;
        }
        else
        {
            if (EscProb<PS[6]) EscProb = PS[6];
            if (EscProb>PS[7]) EscProb = PS[7];
        }

		// 12.0 108 效果 被施加此效果的人受到闪避率降低的效果。
        /*EscProb -= hurtType == HurtType::PHYSICAL_HURT_TYPE ?
            AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_BAAVOID_ADD, EscProb) :
            AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_MAAVOID_ADD, EscProb);*/
		EscProb += hurtType == HurtType::PHYSICAL_HURT_TYPE ?
			AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_BAAVOID_ADD, EscProb) :
			AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_MAAVOID_ADD, EscProb);

        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, eEFFECT_IGNORE_FIGHT_PARAM_LIMIT, 0) > 0)
        {
            if (EscProb < 0) EscProb = 0;
            if (EscProb > 100.0f) EscProb = 100.0f;
        }
        else
        {
            if (EscProb<PS[6]) EscProb = PS[6];
            if (EscProb>PS[7]) EscProb = PS[7];
        }

        //6.0:攻击者无视目标闪避，优先级比目标自身的闪避率上下限更高
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_BYPASS_AVOID, 0) > 0)
            EscProb = 0;

        return EscProb;
    }

    float HeroHurtcalculate::calcCHProb(AIContext& aiContext, CharacterData* source, CharacterData* target, int hurtType)
    {
        bool isFriendHeal = (hurtType == HEALING_HURT_TYPE && source->teamSide == target->teamSide);
		int tmpHurtType = hurtType;
		if (hurtType == HurtType::HEALING_HURT_TYPE) {
			tmpHurtType = source->getBACrit() > source->getMACrit() ? HurtType::PHYSICAL_HURT_TYPE : HurtType::MAGICAL_HURT_TYPE;
		}
        //攻击者物理暴击率=A%*攻击者实际物理暴击值/(攻击方实际物理暴击值*E+攻击方等级*B+C)+（攻击方等级-受伤者等级）*D%
		//26.0:治疗类型使用物理/法术暴击值高的属性
        int CHP1 = tmpHurtType == HurtType::PHYSICAL_HURT_TYPE ? source->getBACrit() : source->getMACrit();
        if (!isFriendHeal) {
            CHP1 += hurtType == HurtType::PHYSICAL_HURT_TYPE ?
                AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_PHYCRIT_REDUCE_ADD, CHP1) :
                AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_MAGCRIT_REDUCE_ADD, CHP1);

            //攻击者实际物理/法术暴击值=攻击者物理/法术暴击值 + 攻击者额外物理/法术暴击值 -目标物理/法术抗暴值
			CHP1 += hurtType == HurtType::PHYSICAL_HURT_TYPE ? source->getBACritAdd() : source->getMACritAdd();
            CHP1 -= hurtType == HurtType::PHYSICAL_HURT_TYPE ? target->getBACritRisist() : target->getMACritRisist();
        }
        if (CHP1 < 0)
            CHP1 = 0;
        float *PS = hurtType == HurtType::PHYSICAL_HURT_TYPE ? AIDataHelper::getFightingParameter(6) : AIDataHelper::getFightingParameter(7);
        float CHProb = PS[0] * CHP1 / (CHP1*PS[4] + PS[1] * source->getLv() + PS[2]) + (source->getLv() - target->getLv()) *  PS[3];

        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, eEFFECT_IGNORE_FIGHT_PARAM_LIMIT, 0) > 0)
        {
            if (CHProb<0) CHProb = 0;
            if (CHProb>100.0f) CHProb = 100.0f;
        }
        else
        {
            if (CHProb<PS[6]) CHProb = PS[6];
            if (CHProb>PS[7]) CHProb = PS[7];
        }

        //攻击者自身暴击率提升
		// 26.0 add:治疗类型根据攻击者暴击值高的属性选择相应效果
		CHProb += tmpHurtType == HurtType::PHYSICAL_HURT_TYPE ?
			AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_BACRIT_ADD, CHProb) :
			AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_MACRIT_ADD, CHProb);

        //目标减免暴击率
        if (!isFriendHeal) {
            CHProb += hurtType == HurtType::PHYSICAL_HURT_TYPE ?
                EffectHelper::checkExcuteFloatEffect(target, eEFFECT_PHYCRIT_PROB_REDUCE_ADD, CHProb) :
                EffectHelper::checkExcuteFloatEffect(target, eEFFECT_MAGCRIT_PROB_REDUCE_ADD, CHProb);
        }

        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, eEFFECT_IGNORE_FIGHT_PARAM_LIMIT, 0) > 0)
        {
            if (CHProb<0) CHProb = 0;
            if (CHProb>100.0f) CHProb = 100.0f;
        }
        else
        {
            if (CHProb<PS[6]) CHProb = PS[6];
            if (CHProb>PS[7]) CHProb = PS[7];
        }

        return CHProb;
    }

    bool HeroHurtcalculate::isTargetInSpecialAction(CharacterData *target)
    {
        // bool isInSpecialAction = false;
        // switch (target->currentAction)
        // {
        // case STRUCK_FLY_AWAY_ACTION:
        // case STRUCK_BACKWORD_ACTION:
        // case STRUCK_HOVER_ACTION:
        // case FALL_DOWN_ACTION:
        //     isInSpecialAction = true;
        //     break;
        // default:
        //     break;
        // }
        // if (target->hasEffect(eEFFECT_FREEZE_IN_EFFECT_ACTION))
        //     isInSpecialAction = true;
        // return isInSpecialAction;

        return luahelper_actioncontroller_is_escape_hurt_result(AIContext::getInstance()->getLuaState(), target);
    }

    void HeroHurtcalculate::handleNormalHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec)
    {
        if (isTargetInSpecialAction(target))
            return;


        //int hard1 = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_CHARACTER_HARD, hurtExec->hard);
        int hard2 = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_CHARACTER_HARD, target->staticData->hard);
        if (hurtExec->hard > target->staticData->hard + hard2)
        {
            OutPutFileLine;
            if (aiContext.localRandom->nextInt(0, 100) < hurtExec->resultParam2)
            {
                HeroInhurtAction::Enter(*target, aiContext, std::max(hurtExec->resultParam1, 1), IN_HURT_ACTION, *source);
                HeroActionController::setEffectID(target->actionAtt, 0);
            }
        }

    }

    void HeroHurtcalculate::handleKnockBackHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec, bool isRandomDirection)
    {
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_NO_HIT_DOWN_FLY_BACK, 0) > 0)
            return;

        //击退接击退：老的击退会被新的击退覆盖（持续时间会被覆盖，不并行）
        //击退接击倒：（是否能实现）在击倒状态改变X轴（躺着动），两者持续时间并行计算 //先做成覆盖
        //击退接击飞：直接覆盖动作，若击退时间较长，则击飞结束后继续改变目标X轴 //先做成直接覆盖
        if (isTargetInSpecialAction(target) && !target->hasTag("hurtresultstack"))
            return;

        source->targetStuckBackward = true;
        int speed = 0;
        if (isRandomDirection)
        {
            OutPutFileLine;
            int randomDirection = aiContext.localRandom->nextInt(0, 100) < 50 ? 1 : -1;
            randomDirection *= target->direction;
            speed = randomDirection * hurtExec->resultParam1;
        }
        else
        {
            speed = (source->getX() < target->getX() ? 1 : -1) * hurtExec->resultParam1;
        }
        HeroStruckBackAction::Enter(*target, aiContext, hurtExec->resultParam2, speed);
        HeroActionController::setEffectID(target->actionAtt, 0);
        //if (projectile == nullptr) source->target = nullptr;
        if (hurtExec->resultStack)
            target->addTag("hurtresultstack");


        AIExcuteOnHurtResultEffect::checkExcute(target);
    }

    void HeroHurtcalculate::handleKnockFlyHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec, bool isRandomDirection)
    {
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_NO_HIT_DOWN_FLY_BACK, 0) > 0)
            return;

        //击飞接击飞：若目标被击飞在半空中时受到了另一个击飞，则在当前坐标位置重新计算一次击飞，Y轴的落点一定要是目标初始被击飞的位置
        //击飞接击倒：直接在击倒的位置施放一次击飞，覆盖击倒效果，但如果击倒时间很长，则击飞结束后继续保持击倒（不会先站起来再倒，直接保持击倒的状态）//先做成直接覆盖
        //击飞接击退：若击退持续时间结束的点晚于击飞，则击飞倒地时接着改变目标的X轴（躺着动） //先做成直接覆盖
        if (isTargetInSpecialAction(target) && !target->hasTag("hurtresultstack"))
            return;


        source->targetStuckBackward = true;
        int extendTick = EffectHelper::getExtendHitdownNFlyChangeValue(aiContext, source, target, 10);
        int speedX = 0;
        if (isRandomDirection)
        {
            OutPutFileLine;
            int randomDirection = aiContext.localRandom->nextInt(0, 100) < 50 ? 1 : -1;
            randomDirection *= target->direction;
            speedX = randomDirection * hurtExec->resultParam1;
        }
        else
        {
            speedX = (source->getX() < target->getX() ? 1 : -1) * hurtExec->resultParam1;
        }
        HeroStruckFlyAction::Enter(*target, aiContext, speedX, hurtExec->resultParam2, extendTick);
        HeroActionController::setEffectID(target->actionAtt, 0);
        //if(projectile==nullptr) source->target = nullptr;
        if (hurtExec->resultStack)
            target->addTag("hurtresultstack");

    }

    void HeroHurtcalculate::handleKnockDownHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec)
    {
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_NO_HIT_DOWN_FLY_BACK, 0) > 0)
            return;

        //击倒接击倒：延长击倒时间
        //击倒接击飞：击飞动作不会被打断若击倒结束时间晚于击飞，则在击飞目标落地倒下后保持倒下状态直到击倒结束 //先做成不覆盖
        //击倒接击退：（是否能实现）在击倒状态改变X轴（躺着动），两者持续时间并行计算 //先做成直接覆盖
        if (isTargetInSpecialAction(target) && !target->hasTag("hurtresultstack"))
            return;

        if (target->currentAction == STRUCK_FLY_AWAY_ACTION)
            return;


        OutPutFileLine;
        if (aiContext.localRandom->nextInt(0, 100) < hurtExec->resultParam2)
        {
            int tickLen = hurtExec->resultParam1;
            tickLen += EffectHelper::getExtendHitdownNFlyChangeValue(aiContext, source, target, tickLen);
            if (target->currentAction == ActionType::FALL_DOWN_ACTION)
            {
                int remainTick = target->totalTick - (target->localTick - target->startTick);
                if (remainTick < tickLen)
                {
                    target->startTick = target->localTick + 1;
                    target->totalTick = tickLen;
                }
            }
            else
            {
                HeroOutofControlAction::Enter(*target, aiContext, tickLen, ActionType::FALL_DOWN_ACTION, *source);
            }
            
            HeroActionController::setEffectID(target->actionAtt, 0);
            //if (projectile == nullptr) source->target = nullptr;
            if (hurtExec->resultStack)
                target->addTag("hurtresultstack");


            AIExcuteOnHurtResultEffect::checkExcute(target);
        }
    }

    void HeroHurtcalculate::handleKnockForceDownHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec)
    {
        if (isTargetInSpecialAction(target))
            return;



        // 将目标击倒，若目标无法被击倒，则强制置于rise状态
        // 对于SSS角色做特殊处理，效果一致但是不播放rise动画只播放待机
        if (!AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_NO_HIT_FORCE_DOWN, 0))
        {
            if (aiContext.localRandom->nextInt(0, 100) < hurtExec->resultParam2)
            {
                int tickLen = hurtExec->resultParam1;
                if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_NO_HIT_DOWN_FLY_BACK, 0) == 0)
                    HeroOutofControlAction::Enter(*target, aiContext, tickLen, ActionType::FALL_DOWN_ACTION, *source);
                else if (target->staticData->comment.compare("SSS") == 0)
                    HeroOutofControlAction::Enter(*target, aiContext, tickLen, ActionType::IDLE_ACTION, *source);
                else
                    HeroOutofControlAction::Enter(*target, aiContext, tickLen, ActionType::STANDUP_ACTION, *source);
                HeroActionController::setEffectID(target->actionAtt, 0);

				target->addTag("besuppressed");// 压制 clair 2017.11.20

                AIExcuteOnHurtResultEffect::checkExcute(target);
            }
		}
		
    }

    void HeroHurtcalculate::handleHurtResult(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtExec *hurtExec)
    {
        switch (hurtExec->result) {
        case NORMAL_HURT_RESULT:
            handleNormalHurtResult(aiContext, source, target, hurtExec);
            break;
        case KNOCK_BACK_HURT_RESULT:
            handleKnockBackHurtResult(aiContext, source, target, hurtExec, false);
            break;
        case KNOCK_FLY_HURT_RESULT:
            handleKnockFlyHurtResult(aiContext, source, target, hurtExec, false);
            break;
        case KNOCK_DOWN_HURT_RESULT:
            handleKnockDownHurtResult(aiContext, source, target, hurtExec);
            break;
        case KNOCK_BACK_RANDOM_HURT_RESULT:
            handleKnockBackHurtResult(aiContext, source, target, hurtExec, true);
            break;
        case KNOCK_FLY_RANDOM_HURT_RESULT:
            handleKnockFlyHurtResult(aiContext, source, target, hurtExec, true);
            break;
        case KNOCK_FORCE_DOWN_HURT_RESULT:
            handleKnockForceDownHurtResult(aiContext, source, target, hurtExec);
            break;
        default:
            break;
        }
    }

	void HeroHurtcalculate::HurtShield(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &param, std::vector<int> &shieldIds, int &out_hurtnum)
	{
		out_hurtnum = 0;
		if (param.HPDelta == 0) {
			return;
		}

		// 265
		auto shieldAbsorbHP = AIShieldTransferHurtEffect::checkExecute(aiContext, target, param.HPDelta, param.isSimulateCalc);
		if (shieldAbsorbHP != 0) {
			param.HPDelta -= shieldAbsorbHP;
			if (param.HPDelta <= 0) {
				param.HPDelta = 0;
				return;
			}
		}

		// 242
		if (!param.isFromSacrificeRecv) {
			param.HPDelta = AISacrificeHurtSend::checkExcute(aiContext, source, target, param);
			if (param.HPDelta == 0) {
				return;
			}
		}

		//// 172
		//if (AIFatalHurtTransferEffect::getActivatedFatalTransEff(target, aiContext.currentTick, source->myID)) {
		//	bool transSucceed = HurtFuncTransferHurt(aiContext, source, target, param);
		//	if (transSucceed) {
		//		if (param.isSimulateCalc)
		//			param.HPDelta = 0;
		//		return;
		//	}
		//}

		//// 排除队友和船炮
		//if (source->teamSide != target->teamSide && source->k != 100 && !param.isSimulateCalc)
		//	target->attacker = source;

		int &HPDelta = param.HPDelta;

		if (param.markID > 0) {
			// 259
			HPDelta -= AIHurtDownFromMarkEffect::checkExecute(aiContext, target, param.markID, HPDelta);
			if (HPDelta < 0)
				HPDelta = 0;
		}

		// 243
		HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, eEFFECT_ADD_HURT, HPDelta);

		//param.HPDeltaBeforeAvoidHurt = HPDelta; // 12.0 sync from lua

		if (!param.isRawHurt) {
			// 174
			HPDelta += EffectHelper::checkExcuteCauseAddPropertyHurtResult(source, param.hurtProperty, HPDelta, param.isSimulateCalc);

			//减伤
			HPDelta = HeroHurtcalculate::getOriginalAvoidanceHurt(param.hurtType, target, source, HPDelta);
			bool isTargetFaceAttacker = false;
			if (source->direction != target->direction)
				isTargetFaceAttacker = ((target->getX() - source->getX()) * source->direction >= 0);

			// 属性伤害减免
			int propHurtAdd = 0;

			// 207
			if (!param.isFromEffectCaster)
				propHurtAdd = EffectHelper::checkActivatePropHurtAdd(target, param.hurtProperty, HPDelta);

			if (param.hurtType == PHYSICAL_HURT_TYPE) {
				// 117
				HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_PHY_HARM_DOWN, HPDelta);
				// 190
				if (isTargetFaceAttacker)
					HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_PHY_HARM_FACE_DOWN, HPDelta);
			}
			else if (param.hurtType == MAGICAL_HURT_TYPE) {
				// 118
				HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_MAG_HARM_DOWN, HPDelta);
				// 191
				if (isTargetFaceAttacker)
					HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_MAG_HARM_FACE_DOWN, HPDelta);
			}

			// 属性伤害减免效果与117,118效果用加减法叠加
			HPDelta += propHurtAdd;

			// 192
			if (isTargetFaceAttacker)
				HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_ALL_HARM_FACE_DOWN, HPDelta);

			if (HPDelta < 0)
				HPDelta = 0;

			if (!param.isFromEffectCaster && source->isSummon() && source->summonProperty) {
				HPDelta = HPDelta * source->summonProperty->attackRate / 100;
				// 173
				if (source->getFakeSummonType() == 0) // 12.0
					HPDelta += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_SUMMON_HARM_ADD, HPDelta);
			}

			// 149
			if (target->hasEffect(EffectType::eEFFECT_HURT_ADD))
				HPDelta = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_HURT_ADD, HPDelta);

			// 178
			if (AILinkenSphereEffect::activateLinkenSphereEffect(source, target, param.isSimulateCalc)) {
				return;
			}

			int hurtType = param.hurtType;

			//伤害吸收护盾
			int repel = 0;
			if (hurtType == PHYSICAL_HURT_TYPE)
				// 138
				repel = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_BAREDUCE_ADD, HPDelta);
			else if (hurtType == MAGICAL_HURT_TYPE)
				// 139
				repel = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_MAREDUCE_ADD, HPDelta);

			if (repel > 0) {
				if (repel >= HPDelta) {
					return;
				}

				HPDelta -= repel;
			}

			bool isBlocked = false;

			////属性伤害免疫
			//int blockEffect = HurtFuncGetReplEffectForProperty(param.hurtProperty);

			//if (blockEffect > 0)
			//	isBlocked = (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, (EffectType)blockEffect, 0) > 0);

			//if (!isBlocked) {
			//	// 226
			//	if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_SHIELD_BYCOUNT_ALL_HURT, 0) > 0) {
			//		isBlocked = true;
			//		if (!param.isSimulateCalc)
			//			AIContinuousEffectCaster::change_Special_Effect(aiContext, *target, EffectType::eEFFECT_SHIELD_BYCOUNT_ALL_HURT, 1);
			//	}
			//}

			//if (!isBlocked && hurtType != HEALING_HURT_TYPE && hurtType >= 0) {
			//	//类型伤害护盾-按伤害次数
			//	int shieldEffect = hurtType == PHYSICAL_HURT_TYPE ? EffectType::eEFFECT_SHIELD_BYCOUNT_PHYSICS : EffectType::eEFFECT_SHIELD_BYCOUNT_MAGIC;
			//	if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, (EffectType)shieldEffect, 0) > 0) {
			//		// 157 or 158
			//		isBlocked = true;
			//		if (!param.isSimulateCalc)
			//			AIContinuousEffectCaster::change_Special_Effect(aiContext, *target, (EffectType)shieldEffect, 1);
			//	}
			//	else if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_SHIELD_BYCOUNT_BOTHPHMA, 0) > 0) {
			//		// 159
			//		isBlocked = true;
			//		if (!param.isSimulateCalc)
			//			AIContinuousEffectCaster::change_Special_Effect(aiContext, *target, EffectType::eEFFECT_SHIELD_BYCOUNT_BOTHPHMA, 1);
			//	}
			//}

			if (isBlocked) {
				HPDelta = 0;
				return;
			}

			//if (!param.isSimulateCalc)
			//	HeroHurtcalculate::checkAndExecuteTalent(aiContext, source, target, HeroHurtcalculate::TalentTrigger::HIT);

			////击中硬值
			//if (param.hurtExec && !param.isSimulateCalc) {
			//	HeroActionController::setYingzhi(source->actionAtt, param.hurtExec->hitStunP);
			//	source->totalTick = source->totalTick + param.hurtExec->hitStunP;
			//}

			//针对被攻击对象是镜像的情况
			if (!param.isFromEffectCaster && target->isSummon() && target->summonProperty) {
				HPDelta = HPDelta * target->summonProperty->beAttackRate / 100;
			}
		} // END not raw hurt

		int hurtType = param.hurtType;
		for (auto shieldid : shieldIds)
		{
			int repelnum = 0;

			// 110,111,112,273,312,320,100001
			if (shieldid == 110)
			{
				if (param.hurtType == PHYSICAL_HURT_TYPE || param.isRawHurt)
				{
					repelnum = AIRepelEffect::checkExecute(eEFFECT_BAREPEL_ADD, aiContext, target, HPDelta, param.isSimulateCalc);
				}
			}
			else if (shieldid == 111)
			{
				if (param.hurtType == MAGICAL_HURT_TYPE || param.isRawHurt)
				{
					repelnum = AIRepelEffect::checkExecute(eEFFECT_MAREPEL_ADD, aiContext, target, HPDelta, param.isSimulateCalc);
				}
			}
			else if (shieldid == 112)
			{
				repelnum = AIRepelEffect::checkExecute(eEFFECT_ALL_REPEL, aiContext, target, HPDelta, param.isSimulateCalc);
			}
			else if (shieldid == 273)
			{
				repelnum = luahelper_innereffect_273_trigger(aiContext.getLuaState(), target, HPDelta, param.isSimulateCalc);
			}
			else if (shieldid == 312)
			{
				repelnum = luahelper_innereffect_312_trigger(aiContext.getLuaState(), target, hurtType, HPDelta, param.isSimulateCalc);
			}
			else if (shieldid == 320)
			{
				repelnum = luahelper_innereffect_320_trigger(aiContext.getLuaState(), target, hurtType, param.hurtProperty, HPDelta, param.isSimulateCalc);
			}
			else if (shieldid == 10001)
			{
				repelnum = luahelper_innereffect_10001_trigger(aiContext.getLuaState(), target, param.hurtType, param.HPDelta, param.isSimulateCalc, param.hitType);
			}

			if (repelnum)
			{
				HPDelta -= repelnum;
				out_hurtnum += repelnum;
				if (HPDelta <= 0) {
					HPDelta = 0;
					return;
				}
			}
		}
	}
    
    int HeroHurtcalculate::ExertHurt(AIContext& aiContext, CharacterData* source, AIProjectile* projectile, int hurtID, int skillID, int skillLevel, CharacterData* target, int theMarkID)
    {
        if (!AIScriptBinder::getInstance()->isHurtCalcExertHurtByLua())
            return ExertHurtNative(aiContext, source, projectile, hurtID, skillID, skillLevel, target, theMarkID);
        return luahelper_exerthurt(aiContext.getLuaState(), source, projectile, hurtID, skillID, skillLevel, target, theMarkID);
    }
    
    int HeroHurtcalculate::ExertHurtNative(AIContext& aiContext, CharacterData* source, AIProjectile* projectile, int hurtID, int skillID, int skillLevel, CharacterData* target, int theMarkID)
    {
        if (!target || target->getHP() < 1)
            return 0;
        
        HurtExec* hurtExec = AIDataHelper::getHurtExec(hurtID);
        if (!hurtExec)
            return 0;
        
        if (!AIContinuousSummonCaster::couldHurtSummon(source, hurtExec, target, skillID))
            return 0;
        
        //Round Table
        //未命中概率
        float missProb = calcMissProb(aiContext, source, target, hurtExec->type);
        
        //受伤者基础闪避率
        float EscProb = calcEscProb(aiContext, source, target, hurtExec->type);
        
        //暴击率
        float CHProb = calcCHProb(aiContext, source, target, hurtExec->type);
        
        float totalProb = 100;
        missProb = std::min(missProb, totalProb);
        totalProb -= missProb;
        EscProb = std::min(EscProb, totalProb);
        totalProb -= EscProb;
        CHProb = std::min(CHProb, totalProb);
        totalProb -= CHProb;
        //int HProb = totalProb;
        
        OutPutFileLine;
        int poll = aiContext.localRandom->nextInt(0, 100);
        int preSum = 0;
        int hitType = 0;
        
        checkAndExecuteTalent(aiContext, source, target, TalentTrigger::ALWAYS);
        
        bool isMissed = (poll < (preSum += missProb));
        bool isEscaped = (poll < (preSum += EscProb));
        bool isNormalAttack = (poll >(preSum += CHProb));
        
        // 11.0
        if (target->hasEffect(282)) {
            if (luahelper_innereffect_282_check_do(aiContext.getLuaState(), target, source) > 0)
            {
                isMissed = false;
                isEscaped = false;
                isNormalAttack = false;
            }
        }
        
        if (!isMissed && !isEscaped && isNormalAttack) {
            // 235
            if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, eEFFECT_BAMA_CRIT_NCAST, hurtExec->type) > 0) {
                isMissed = false;
                isEscaped = false;
                isNormalAttack = false;
            }
        }
        
        bool isEffCritnofailFirstRun = false;
        // 181
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, eEFFECT_CRITHIT_NOFAIL, 0) > 0) {
            if (!source->isCritNoFailEffRunning) {
                source->isCritNoFailEffRunning = true;
                isEffCritnofailFirstRun = true;
                if (isMissed || isEscaped || isNormalAttack) {
                    bool isCritNoFail = AICritHitNoFailEffect::checkActivateCritHitNoFailEffect(source);
                    if (isCritNoFail) {
                        isMissed = false;
                        isEscaped = false;
                        isNormalAttack = false;
                    }
                    else {
                        AICritHitNoFailEffect::activateCritHitNoFailEffect(source, false);
                    }
                }
            }
        }
        
        bool isMustCrit = false;
        // 221
        if (AICritOnAttrEffect::checkExcute(source, target)) {
            isMustCrit = true;
        }
        
        // 11.0
        if (!isMustCrit) {
            if (source->hasEffect(290) && luahelper_innereffect_290_check_do(aiContext.getLuaState(), source, target))
                isMustCrit = true;
        }
        
        if (isMustCrit) {
            isMissed = false;
            isEscaped = false;
            isNormalAttack = false;
        }
        
        if (isMissed) //miss
        {
            if (!HeroActionController::isDeadAction(source->currentAction))
                AISpecialEffectLabel::excute(100009, 0, aiContext, source);
            // 202
            AIEscMissExcuteEffect::checkActivateEscMissExcute(source, eEFFECT_MISS_EXCUTE);
            
            if (isEffCritnofailFirstRun)
                source->isCritNoFailEffRunning = false;
            
            return 0;
        }
        else if (isEscaped) // escape
        {
            checkAndExecuteTalent(aiContext, source, target, TalentTrigger::DODGE);
            
            AISpecialEffectLabel::excute(100007, 0, aiContext, target);
            
            if (hurtExec->type == HurtType::PHYSICAL_HURT_TYPE)
                AIEscMissExcuteEffect::checkActivateEscMissExcute(target, eEFFECT_ESCAPE_PHY_EXCUTE); // 203
            else
                AIEscMissExcuteEffect::checkActivateEscMissExcute(target, eEFFECT_ESCAPE_MAG_EXCUTE); // 204
            // clair_wang 闪避触发技能
            target->onEscape();
            
            if (isEffCritnofailFirstRun)
                source->isCritNoFailEffRunning = false;
            
            return 0;
        }
        
        //暴击or非暴击
        
        bool isReplHurt = false; //免疫单次伤害
        int oldHP = target->getHP();
        hitType = CRITICAL_HIT;
        float baseCHP = HurtFuncGetBaseCHP((HurtType)hurtExec->type, source);
        // 262
        if (target->hasEffect(eEFFECT_CRIT_IF_HIT)) {
            auto theEffectCritValue = AICritIfHitEffect::checkExecute(target, skillID);
            if (theEffectCritValue > 0) {
                isNormalAttack = false;
                baseCHP = theEffectCritValue / 100.0f;
            }
        }
        
        bool isFriendHeal = (hurtExec->type == HEALING_HURT_TYPE && source->teamSide == target->teamSide);
        if (!isFriendHeal) {
            // 166,167
            baseCHP += hurtExec->type == HurtType::PHYSICAL_HURT_TYPE ?
            EffectHelper::checkExcuteFloatEffect(target, eEFFECT_PHYCRIT_MUL_REDUCE_ADD, baseCHP) :
            EffectHelper::checkExcuteFloatEffect(target, eEFFECT_MAGCRIT_MUL_REDUCE_ADD, baseCHP);
            
            //攻击者实际暴击倍数=攻击者暴击倍数-目标暴击倍数减免
            baseCHP -= (hurtExec->type == HurtType::PHYSICAL_HURT_TYPE ? target->getBACritMAdd() : target->getMACritMAdd()) / 100.0f;
        }
        
        if (baseCHP < 1.0f)
            baseCHP = 1.0f;
        
        if (isNormalAttack) //normal attack
        {
            hitType = NORMAL_HIT;
            baseCHP = 1;
        }
        else
        {
            // 181
            if (isEffCritnofailFirstRun)
                AICritHitNoFailEffect::activateCritHitNoFailEffect(source, true);
        }
        
        if (isEffCritnofailFirstRun)
            source->isCritNoFailEffRunning = false;
        
        int HPDelta = getInitHurtDelta(source, skillLevel, hurtExec);
        
        if (target->hasEffect(291)) {
            HPDelta += luahelper_innereffect_291_check_do(aiContext.getLuaState(), source, target, HPDelta);
        }
        
        target->lastHurtExecInitHurtDelta = HPDelta;
        
        HPDelta *= baseCHP;
        
        // 217
        int distHPDelta = EffectHelper::getAddHurtOnDistChangeValue(aiContext, source, target, HPDelta);
        // 218
        int sameTargetHPDelta = AIAddHurtOnSameTargetEffect::checkExcute(source, target, HPDelta, hurtExec->type);
        HPDelta += distHPDelta + sameTargetHPDelta;
        
        const int hurtProperty = HurtFuncGetRealtimeHurtProperty(*source, hurtExec->property);
        
        if (hurtExec->type == HurtType::HEALING_HURT_TYPE) //治疗
        {
            //cui 治疗飘字修改 2023_3_7
            int labelId = 0;
            hitType == NORMAL_HIT ? labelId = 100002 : labelId = 100003;
            bool isCrit = AISpecialEffectLabel::checkShowCrit(labelId);
            if (hurtExec->realHurtDisplayId != 0) {
                labelId = hurtExec->realHurtDisplayId;
            }

            HPDelta = HurtFuncHealTarget(aiContext, *source, *target, HPDelta, labelId, skillID, false, theMarkID, isCrit);
        }
        else
        {
            HurtCalcParam param(projectile, skillID, skillLevel, HPDelta, hurtExec->type, hurtProperty, hitType, hurtExec, false, hurtID);
            param.markID = theMarkID;
            
            //auto result = HurtFuncHurtTarget(aiContext, source, target, param);
            HitResult result;
            HurtFuncHurtTargetNative(aiContext, source, target, param, &result);
            
            // 235 267效果
            target->lastHurtExecRealHurtDelta = oldHP - target->getHP();
            target->pLastHurtOriginCharacter = source;
            target->lastHurtExecId = hurtID;

            if (AIEffect267::checkExecute(aiContext, target, source) > 0)
                AIContinuousEffectCaster::zero_Special_Effect(aiContext, *target, eEFFECT_CREATE_HURT_ADD_VALUE_NEW);
            
            if (result.hitResult == HitResult::HitResultType::HIT_BYPASS)
                return 0;
            
            isReplHurt = (result.blockResult == HitResult::BlockType::HIT_BLOCKED);
            
            target->onHurt(false, hurtID, param.HPDelta, hurtExec->type, source, hurtID);
        }
        
        // 暴击后触发
        if (hitType == CRITICAL_HIT) {
            target->onCruelHitSkillRelease();
            
            source->onCastCritHurt();
            
            // 254
            if (HPDelta != 0)
                AIDurHitCritCastEffect::checkExcute(aiContext, source, target, hurtExec->type);
        }
        
        // 触发标记效果和标记死亡
        ToTriggerMarkCasterFun(aiContext, *target, skillID, skillLevel, hurtExec->type, hurtProperty, HPDelta, hitType, source, hurtID);
        
        bool shouldHitExec = false;
        switch (hurtExec->hitExecCondition) {
            case 0:
                shouldHitExec = true;
                break;
            case 1:
            case 4: // 随等级增长多选一（对应类型1）
                OutPutFileLine;
                shouldHitExec = (aiContext.localRandom->nextInt(0, 100) <= 100 + (skillLevel - target->getLv())* hurtExec->hitExecParam);
                break;
            case 2:
            case 3: // 固定几率多选一（对应类型2）
                OutPutFileLine;
                shouldHitExec = (aiContext.localRandom->nextInt(0, 100) < hurtExec->hitExecParam);
                break;
            case 5:// 几率随等级成长
                shouldHitExec = (aiContext.localRandom->nextInt(0, 100) < (hurtExec->hitExecParam + skillLevel * hurtExec->hitLevelUp / 100));
                break;
            default:
                break;
        }
        
        // 击中执行
        if (shouldHitExec) {
            std::vector<int> hitExecs = hurtExec->hitExecs;//{ hurtExec->hitExec1, hurtExec->hitExec2, hurtExec->hitExec3 };
            
            // 若某个被选中的效果执行处于触发CD，则重新选择一个
            if (hurtExec->hitExecCondition == 3 || hurtExec->hitExecCondition == 4)
            {
                /// 12.0 sync from lua
                hitExecs.clear();
                for (auto execID : hurtExec->hitExecs)
                {
                    if (execID > 0)
                        hitExecs.push_back(execID);
                }
                ///
                recursiveRandomOneValidExec(aiContext, target, hitExecs);
            }
            
            for (auto hitExecID : hitExecs) {
                if (hitExecID <= 0)
                    continue;
                AIActionController::executeExec(aiContext, hitExecID, skillID, skillLevel, source, projectile, target, 0, false, theMarkID);
            }
        }
        
        if (hurtExec->hitSpecialEffect > 0)
            AIActionController::executeDelayExec(aiContext, hurtExec->hitSpecialEffect, skillID, skillLevel, source, projectile, target, 0, false, theMarkID,false, hurtExec->delayTick);
            //AIActionController::executeExec(aiContext, hurtExec->hitSpecialEffect, skillID, skillLevel, source, projectile, target, 0, false, theMarkID);
        if (hurtExec->hitAudio > 0)
            AIActionController::executeExec(aiContext, hurtExec->hitAudio, skillID, skillLevel, source, projectile, target, 0, false, theMarkID);
        
        target->lastHurtExecInitHurtDelta = 0;
        target->lastHurtExecRealHurtDelta = 0;
        target->pLastHurtOriginCharacter = nullptr;
        target->lastHurtExecId = 0;
        
        //Action Transition
        if (target->getHP() > 0) {
            if (hurtExec->type != HurtType::HEALING_HURT_TYPE)
                target->onHurtHit(hurtID);
            
            // 266
            if ((target->currentAction != EFFECT_ACTION || target->hasEffect(eEFFECT_STATE_VERTIGO) || (AIDataHelper::getEffectExec(HeroActionController::getEffectID(target->actionAtt)) && AIDataHelper::getEffectExec(HeroActionController::getEffectID(target->actionAtt))->ifReact > 0 && !target->isEnterEffectAction())) 
				&& target->currentAction != CONTROLLED_EFFECT_ACTION
				&& !isReplHurt)
            {
                handleHurtResult(aiContext, source, target, hurtExec);
            }
            
            HeroSkillController::checkReleaseHPSkill(aiContext, target);
        } else {
            characterDeadByHurt(aiContext, target, source, skillID, skillLevel, hurtExec);
        }
        
        return hitType;
    }

    int HeroHurtcalculate::simulateCalcHurt(AIContext& aiContext, CharacterData* source, AIProjectile* projectile, int hurtID, int skillID, int skillLevel, CharacterData* target)
    {
        auto hurtExec = AIDataHelper::getHurtExec(hurtID);
        if (!hurtExec)
            return 0;

        if (!source || !target)
            return 0;

        int HPDelta = getInitHurtDelta(source, skillLevel, hurtExec);

        if (hurtExec->type == HurtType::HEALING_HURT_TYPE) //治疗
        {
            HPDelta = HurtFuncHealTarget(aiContext, *source, *target, HPDelta, 100002, skillID, true, 0);
            HPDelta = -HPDelta; // HQ:加血算负值方便直接判断
        }
        else
        {
            const int hurtProperty = HurtFuncGetRealtimeHurtProperty(*source, hurtExec->property);
            HurtCalcParam param(projectile, skillID, skillLevel, HPDelta, hurtExec->type, hurtProperty, NORMAL_HIT, hurtExec, false, hurtID);
            param.isSimulateCalc = true;

            HitResult result;
            HurtFuncHurtTarget(aiContext, source, target, param, &result);
            if (result.hitResult == HitResult::HitResultType::HIT_BYPASS)
                return 0;

            HPDelta = param.HPDelta;
        }

        return HPDelta;
    }

	//0-从来不触发, 1-默认触发，2-攻击命中触发，3-被攻击命中触发，4-闪避抵抗触发，5-被治疗时触发，7-被动作限制时
	void HeroHurtcalculate::checkAndExecuteTalent(AIContext& aiContext, CharacterData* source, CharacterData* target, TalentTrigger execTrigger)
	{
		if (source != nullptr)
		{
			std::list<std::pair<int, int>>::iterator iter = source->talentList.begin();
			for (; iter != source->talentList.end(); iter++)
			{
				BaqiSpecialExec* exec = AIDataHelper::getBaqiSpecialExec(iter->first, iter->second);
				if (exec == nullptr || exec->talentID < 1) continue;

				switch (execTrigger)
				{
				case TalentTrigger::IN_EFFECT_ACTION://source
				case TalentTrigger::ALWAYS://source
				case TalentTrigger::HIT: //source=HIT, target=BY_HIT
					if (exec->trigger == execTrigger && exec->effectExecID>0)
						AIActionController::executeExec(aiContext, exec->effectExecID, exec->talentID, exec->talentLV, source, nullptr, source, 0);
				}
			}
		}

		if (target != nullptr)
		{
			std::list<std::pair<int, int>>::iterator iter = target->talentList.begin();
			for (; iter != target->talentList.end(); iter++)
			{
				BaqiSpecialExec* exec = AIDataHelper::getBaqiSpecialExec(iter->first, iter->second);
				if (exec == nullptr || exec->talentID < 1) continue;

				switch (execTrigger)
				{
				case TalentTrigger::BY_HURL://target
				case TalentTrigger::DODGE://target
				case TalentTrigger::HIT: //target=HIT, target=BY_HIT
					if (exec->trigger == execTrigger && exec->effectExecID>0)
						AIActionController::executeExec(aiContext, exec->effectExecID, exec->talentID, exec->talentLV, target, nullptr, target, 0);
				}
			}
		}
	}



	int HeroHurtcalculate::getOriginalAvoidanceHurt(int hurtType, CharacterData *pTarget, CharacterData *pSource, int baseHurtValue)
	{
        if (hurtType < 0)
            return baseHurtValue;

		float* PS = hurtType == HurtType::PHYSICAL_HURT_TYPE ? AIDataHelper::getFightingParameter(0) : AIDataHelper::getFightingParameter(1);
		//int baseDP = hurtType == HurtType::PHYSICAL_HURT_TYPE ? pTarget->getBARepel() - pSource->getBABreak() : pTarget->getMARepel() - pSource->getMABreak();
		int baseDP = 0;
		if (hurtType == HurtType::PHYSICAL_HURT_TYPE)
		{
			int targetBARepel = pTarget->getBARepel();
			if (pSource->hasEffect(501))
				targetBARepel += AIContinuousEffectCaster::check_exert_Special_Effect(*AIContext::getInstance(), *pSource, 501, targetBARepel);
			baseDP = targetBARepel - pSource->getBABreak();
		}
		else
		{
			int targetMARepel = pTarget->getMARepel();
			if (pSource->hasEffect(502))
				targetMARepel += AIContinuousEffectCaster::check_exert_Special_Effect(*AIContext::getInstance(), *pSource, 502, targetMARepel);
			baseDP = targetMARepel - pSource->getMABreak();
		}
		if (baseDP < 0)
			baseDP = 0;
		//免伤率=受伤者实际物理防御/(受伤者实际物理防御+攻击者等级*A+B)
		float AVD = (float)baseDP / (baseDP + pSource->getLv() * PS[0] + PS[1]);
		if (AVD<PS[6]) AVD = PS[6];
		if (AVD>PS[7]) AVD = PS[7];
		if (hurtType == HurtType::HEALING_HURT_TYPE) AVD = 0;

		AVD = std::min(1.0F, AVD);
		return baseHurtValue * (1 - AVD);

	}
    
    void HeroHurtcalculate::characterDeadByHurt(AIContext& aiContext, CharacterData *pTarget, CharacterData *pSource, int skillID, int theSkillLevel, HurtExec* pHurtExec, InnerEffectExec* pEffectExec)
    {
        //luahelper_characterdeadbyhurt(aiContext.getLuaState(), pTarget, pSource, skillID, theSkillLevel, pHurtExec, pEffectExec);
        characterDeadByHurtNative(aiContext, pTarget, pSource, skillID, theSkillLevel, pHurtExec, pEffectExec);
    }

	void HeroHurtcalculate::characterDeadByHurtNative(AIContext& aiContext, CharacterData *pTarget, CharacterData *pSource, int skillID, int theSkillLevel, HurtExec* pHurtExec, InnerEffectExec* pEffectExec /*= nullptr*/)
	{
		if (!HeroActionController::isDeadAction(pTarget->currentAction))
		{
            pTarget->killerUID = pSource->myID;
            pTarget->killerSkillID = skillID;
			OutPutFileLine;
			if (pHurtExec != nullptr)
			{
                HeroDeadAction::Enter(*pTarget, aiContext, (aiContext.localRandom->nextInt(0, 100) < pHurtExec->dtrate) ? pHurtExec->deadType : 1);

                if (HeroActionController::isDeadAction(pTarget->currentAction))
                {
                    // 伤害执行里面包含的击杀执行在此进行操作 clair_wang
                    if (pHurtExec->vHitDeadExecIDs.size() > 0)
                    {
                        for (int i = 0; i<pHurtExec->vHitDeadExecIDs.size(); i++)
                        {
                            AIActionController::executeExec(aiContext, pHurtExec->vHitDeadExecIDs[i], skillID, pSource->getLv(), pSource, nullptr, pTarget, 0);
                        }
                    }
                }
			}
			else if (pEffectExec != nullptr)
			{
                bool isFatalHit = pEffectExec->ID == EffectType::eEFFECT_KNOCK_OUT;
                HeroDeadAction::Enter(*pTarget, aiContext, 1, isFatalHit);

                if (HeroActionController::isDeadAction(pTarget->currentAction))
                {
                    // 效果执行里面包含的击杀执行在此进行操作 clair_wang
                    if (pEffectExec->vExecDeadExecIDs.size() > 0)
                    {
                        for (int i = 0; i<pEffectExec->vExecDeadExecIDs.size(); i++)
                        {
                            if (pEffectExec->vExecDeadExecIDs[i] > 0)
                            {
                                AIActionController::executeExec(aiContext, pEffectExec->vExecDeadExecIDs[i], skillID, pSource->getLv(), pSource, nullptr, pTarget, 0);
                            }
                        }
                    }
                }
			}
			else
			{
				HeroDeadAction::Enter(*pTarget, aiContext, 1);
			}
			
			

            // 5.0 将死亡回能量改到角色死亡中处理, 防止有死亡触发技能的角色死后不加能量bug
            //if (HeroActionController::isDeadAction(pTarget->currentAction))
            //{
            //	CCLOG("source energy = %d", pSource->getEnergy());
            //	// clair_wang  杀死幻象不应该加能量 杀死幻象会增加能量的bug。
            //	if (!pTarget->isSummon())
            //	{
            //		int energy = pSource->getEnergy() + AIDataHelper::getEnergyRecover()[1];
            //		#if BATTLE_LOG_TEST
            //          OPBattleLog("gainenergy", pSource->teamSide, pSource->clusterNum, pSource->attributes[0], (energy - pSource->getEnergy()));
            //		#endif
            //		pSource->updateEnergy(energy);
            //	}
            //}
		}
	}

    void HeroHurtcalculate::recursiveRandomOneValidExec(AIContext &aiContext, CharacterData *target, std::vector<int> &execs) {
        if (execs.size() <= 1)
            return;
        //int avgProb = 100 / execs.size();
        //if (avgProb <= 0) {
        //    execs.clear();
        //    return;
        //}
        //OutPutFileLine;
        //int execProb = aiContext.localRandom->nextInt(0, 100);
        //int hitIdx = 0;
        //while (execProb > avgProb) {
        //    hitIdx++;
        //    avgProb += avgProb;
        //}

        /// 12.0: sync from lua
        int hitIdx = aiContext.localRandom->nextInt(0, execs.size());
        ///

        if (hitIdx >= execs.size())
            hitIdx = execs.size() - 1;
        int hitID = execs[hitIdx];
        if (hitID <= 0 || (IsEffectExec(hitID) && EffectHelper::isAnyInnerExecInCD(target, hitID))) {
            execs.erase(execs.begin() + hitIdx);
            recursiveRandomOneValidExec(aiContext, target, execs);
        }
        else {
            execs.clear();
            execs.push_back(hitID);
        }
    }

    // 触发标记效果和标记死亡
    // HPDelta:单次伤害的真实数值
    // hitType:是否暴击
    void HeroHurtcalculate::ToTriggerMarkCasterFun(AIContext& aiContext, CharacterData& target, int skillID, int skillLevel, int hurtType, int hurtProperty, int HPDelta /*= 0*/, int hitType /*=0*/, CharacterData* trigger /*= nullptr*/, int hurtID /* = 0 */)
    {
        int callType = AIContinuousMarkCaster::E_CheckExcuteEffectMarkCallType::kCheckMarkCallByHurt;

        //// clair_wang 2016/05/03 标记触发类型：队友（包括施法者）造成某一属性伤害时触发（伤害不一定打在标记所在目标身上）
        //if (HPDelta != 0)
        //{
        //    std::list<CharacterData*>::iterator iter;
        //    for (iter = trigger->getCurrentRivalsList()->begin(); iter != trigger->getCurrentRivalsList()->end(); ++iter)
        //    {
        //        CharacterData* character = (*iter);
        //        if (character)
        //        {
        //            // theCallType = AIContinuousMarkCaster::E_CheckExcuteEffectMarkCallType::kCheckMarkCallByFriendHurt; // 2016-06-17 fixed bug
        //            if ((character->canBeAttacked(trigger)) && character->target && character->target->teamSide == trigger->teamSide)
        //            {
        //                for (auto it = character->markCasts.begin(); it != character->markCasts.end(); it++)
        //                {
        //                    if (EMarkEffectTriggerType::eMarkEffectTriggerByFriendHurtType == (*it)->effectExecData->triggerType)
        //                    {
        //                        theCallType = AIContinuousMarkCaster::E_CheckExcuteEffectMarkCallType::kCheckMarkCallByFriendHurt;

        //                        // 调用mark触发
        //                        AIContinuousMarkCaster::checkExcuteEffect(aiContext, *character, skillID, skillLevel, hurtType, hurtProperty, theCallType, HPDelta, hitType, trigger/*character*/);// 11.0 modify

        //                        break;
        //                    }
        //                }
        //            }
        //        }
        //    }
        //}

        // MARK: 优化了一下上述逻辑,但实际上与定义不符;线上已经有角色使用了这种触发类型,所以保持不变
        if (HPDelta != 0 && trigger)
        {
            CharacterDataListPtr cs = trigger->getCurrentRivalsList();
            for (auto iter = cs->begin(); iter != cs->end(); ++iter)
            {
                CharacterData* c = *iter;
                if (c->canBeAttacked(trigger) && c->target && c->target->teamSide == trigger->teamSide && c->getTagValue("markt8") > 0)
                {
                    callType = AIContinuousMarkCaster::E_CheckExcuteEffectMarkCallType::kCheckMarkCallByFriendHurt;
                    AIContinuousMarkCaster::checkExcuteEffect(aiContext, *c, skillID, skillLevel, hurtType, hurtProperty, callType, HPDelta, hitType, trigger);
                }
            }
        }

        if (callType == AIContinuousMarkCaster::E_CheckExcuteEffectMarkCallType::kCheckMarkCallByHurt)
        {
            AIContinuousMarkCaster::checkExcuteEffect(aiContext, target, skillID, skillLevel, hurtType, hurtProperty, callType, HPDelta, hitType, trigger);
        }

        if (trigger && trigger->myID != target.myID && trigger->markType11Cnt > 0)
        {
            AIContinuousMarkCaster::checkExcuteEffect(aiContext, *trigger, skillID, skillLevel, hurtType, hurtProperty, AIContinuousMarkCaster::E_CheckExcuteEffectMarkCallType::kCheckMarkCallByOwnerCastHurt, HPDelta, hitType, trigger);
        }
        
        AIContinuousMarkCaster::checkExcuteDead(aiContext, target, skillID, skillLevel, hurtType, hurtProperty, trigger, hurtID);

    }
} // namespace OnePiece

