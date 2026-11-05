#include "AITargetHelper.h"
#include <unordered_map>
#include <algorithm>
#include "cocos2d.h"
#include "action_hero/HeroActionController.h"
#include "AICharacterData.h"
#include "AIContext.h"
#include "CommonType.h"
#include "AIScriptBinder.hpp"
#include "scriptbind/AIScriptBindActionController.hpp"
#include "scriptbind/AIScriptBindEffectCaster.hpp"

namespace OnePiece {

    static std::unordered_map<int, std::function<void(AITargetContext *, std::list<CharacterData*> *)> > s_targethelpers;
    
    void AITargetContext::initWithHurtExec(HurtExec *a_hurt_exec, CharacterData *s, CharacterData *t, CharacterData *sc, AIProjectile *p, int a_sk_id, int a_sk_lv)
    {
        source = s;
        target = t;
        skill_caster = sc;
        projectile = p;
        skill_id = a_sk_id;
        skill_level = a_sk_lv;
        hurt_id = a_hurt_exec->ID;
        search_type = a_hurt_exec->target;
        range.x1 = a_hurt_exec->x1;
        range.x2 = a_hurt_exec->x2;
        range.y1 = a_hurt_exec->z1;
        range.y2 = a_hurt_exec->z2;
        search_params = &a_hurt_exec->targetParams;
        filter_list = nullptr;
    }
    
    void AITargetContext::initWithFilterHurtExec(HurtExec *a_hurt_exec, CharacterData *s, CharacterData *t, CharacterData *sc, AIProjectile *p, int a_sk_id, int a_sk_lv, std::list<CharacterData *> *a_filter_list)
    {
        source = s;
        target = t;
        skill_caster = sc;
        projectile = p;
        skill_id = a_sk_id;
        skill_level = a_sk_lv;
        hurt_id = a_hurt_exec->ID;
        search_type = a_hurt_exec->filterTarget;
        range.x1 = a_hurt_exec->filterX1;
        range.x2 = a_hurt_exec->filterX2;
        range.y1 = a_hurt_exec->filterZ1;
        range.y2 = a_hurt_exec->filterZ2;
        search_params = &a_hurt_exec->filterParams;
        filter_list = a_filter_list;
    }
    
    void AITargetContext::initWithEffectExec(EffectExec *an_effect_exec, CharacterData *s, CharacterData *t, CharacterData *sc, AIProjectile *p, int a_sk_id, int a_sk_lv)
    {
        source = s;
        target = t;
        skill_caster = sc;
        projectile = p;
        skill_id = a_sk_id;
        skill_level = a_sk_lv;
        search_type = an_effect_exec->target;
        range.x1 = an_effect_exec->range[0];
        range.x2 = an_effect_exec->range[1];
        range.y1 = an_effect_exec->range[2];
        range.y2 = an_effect_exec->range[3];
        search_params = &an_effect_exec->targetParams;
    }

    int parseRelationParam(int p)
    {
        // 与目标69的定义一致
        // 阵营 0-原始全体 1-原始我方 2-原始敌方 3-当前全体 4-当前我方 5-当前敌方
        switch (p) {
        case 0: return FR_ALL_ORIGIN;
        case 1: return FR_FRIENDS_ORIGIN;
        case 2: return FR_RIVALS_ORIGIN;
        case 3: return FR_ALL;
        case 4: return FR_FRIENDS;
        case 5: return FR_RIVALS;
        }

        return FR_ALL_ORIGIN;
    }

    /// @bool true:pre search has target
    static bool presearch(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        if (ctx->source->hasEffect(248) && ctx->skill_id == ctx->source->normalSkill.first) {
            outlist->push_back(ctx->source);
            return true;
        }

        if (ctx->skill_caster->hasEffect(248) && ctx->skill_id == ctx->skill_caster->normalSkill.first) {
            outlist->push_back(ctx->skill_caster);
            return true;
        }
        return false;
    }

    static void filterOriginList(std::list<CharacterData*> *originlist, AITargetContext *ctx, std::list<CharacterData*> *out_list)
    {
        for (CharacterData *c : *originlist) {
            /*
            if (ctx->ft_is_excludesummon && c->isSummon() && c->getFakeSummonType() == 0)
                continue;
            */
            if (ctx->ft_is_excludesummon && c->seemsSummon()) {
                continue;
            }

            if (ctx->ft_is_onlysummon && !c->seemsSummon()) {
                continue;
            }
            
            if (!c->isFindableToAttacker(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;

            if (c->hasTag("misseff263withorigin") && !c->isFindableToAttacker(ctx->skill_caster, ctx->ft_is_ignorecanbeattackeffect))
                continue;

            if (!c->isFindableToSkillCaster(ctx->skill_caster, ctx->ft_is_ignorecanbeattackeffect))
                continue;

            if (ctx->lowerhplimit_character && CharacterFunc::GetCharacterHPRatio(*c) <= CharacterFunc::GetCharacterHPRatio(*ctx->lowerhplimit_character))
                continue;

            if (c->hasEffect(297) && !luahelper_targethelper_is_target297findabletosearcher(AIContext::getInstance()->getLuaState(), c, ctx->source, ctx->ft_real_search_type))
                continue;

            if (ctx->exclude_character && ctx->exclude_character->myID == c->myID)
                continue;
            
            // 12.0
            if (ctx->source->isInDuel()) {
                if (ctx->source->myID != c->myID && ctx->source->getDuelTarget()->myID != c->myID)
                    continue;
            }

            // 14.0
            if (c->isSummon() && c->getFakeSummonType() == 3)
                continue;

            out_list->push_back(c);
        }
    }

    static void filterFriendList(AITargetContext *ctx, CharacterDataListPtr outlist)
    {
        if (ctx->source->myID == ctx->skill_caster->myID && (ctx->source->hasEffect(253) || ctx->source->hasTag("summonincharm253"))) {
            filterOriginList(ctx->source->getCurrentRivalsList(), ctx, outlist);
            outlist->remove(ctx->source);
            outlist->push_back(ctx->source);
        } else {
            filterOriginList(ctx->source->getCurrentFriendsList(), ctx, outlist);
        }
    }

    static void filterRivalList(AITargetContext *ctx, CharacterDataListPtr outlist)
    {
        if (ctx->source->myID == ctx->skill_caster->myID && (ctx->source->hasEffect(253) || ctx->source->hasTag("summonincharm253"))) {
            filterOriginList(ctx->source->getCurrentFriendsList(), ctx, outlist);
            outlist->remove(ctx->source);
        } else {
            filterOriginList(ctx->source->getCurrentRivalsList(), ctx, outlist);
        }
    }

    static void filterFriendOriginList(AITargetContext *ctx, CharacterDataListPtr outlist)
    {
        filterOriginList(ctx->source->getOriginFriends(), ctx, outlist);
    }

    static void filterRivalOriginList(AITargetContext *ctx, CharacterDataListPtr outlist)
    {
        filterOriginList(ctx->source->getOriginRivals(), ctx, outlist);
    }

    static CharacterDataListPtr autoPickFilterList(AITargetContext *ctx, CharacterDataListPtr inlist)
    {
        if (ctx->source->isInDuel())
            return inlist;
        
        if (ctx->filter_list)
            return ctx->filter_list;

        return inlist;
    }

    static void targetLowestHPRatio(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        float lowest = 100000.0;
        CharacterData *t = nullptr;
        for (auto c : *inlist) {
            if (c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect)) {
                float c_ratio = CharacterFunc::GetCharacterHPRatio(*c);
                if (c_ratio < lowest) {
                    lowest = c_ratio;
                    t = c;
                }
            }
        }

        if (t)
            outlist->push_back(t);
    }

    static void targetHighestHPRatio(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        float highest = 0;
        CharacterData *t = nullptr;
        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            float c_ratio = CharacterFunc::GetCharacterHPRatio(*c);
            if (!t || c_ratio > highest) {
                t = c;
                highest = c_ratio;
            }
        }
        
        if (t)
            outlist->push_back(t);
    }

    static void targetRandom(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        std::vector<CharacterData*> tmpcs;
        for (auto c : *inlist) {
            if (ctx->ft_random_exclude_character && ctx->ft_random_exclude_character->myID == c->myID)
                continue;
            if (c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                tmpcs.push_back(c);
        }
        if (tmpcs.size() > 0) {
            int idx = AIContext::getInstance()->localRandom->nextInt(0, tmpcs.size());
            auto t = tmpcs.at(idx);
            outlist->push_back(t);
        }
    }

    static void targetCanBeAttack(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        for (auto c : *inlist) {
            if (c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                outlist->push_back(c);
        }
    }

    static void targetInRange(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        if (!ctx->ft_range_origin && !ctx->projectile)
            return;

        auto projectile = ctx->projectile;
        auto origin = ctx->ft_range_origin;
        auto x1 = ctx->range.x1;
        auto x2 = ctx->range.x2;
        auto y1 = ctx->range.y1;
        auto y2 = ctx->range.y2;

        int x11 = projectile == nullptr ? origin->getX() + x1 : projectile->currentX + x1;
        int x22 = projectile == nullptr ? origin->getX() + x2 : projectile->currentX + x2;
        if (origin->direction == -1)
        {
            x11 = projectile == nullptr ? origin->getX() - x2 : projectile->currentX - x2;
            x22 = projectile == nullptr ? origin->getX() - x1 : projectile->currentX - x1;
        }
        int y11 = projectile == nullptr ? origin->getY() + y1 : projectile->currentY + y1;
        int y22 = projectile == nullptr ? origin->getY() + y2 : projectile->currentY + y2;

        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            auto cx = c->getX();
            auto cy = c->getY();
            if (cx >= x11 && cx <= x22 && cy >= y11 && cy <= y22)
                outlist->push_back(c);
        }
    }

    static void targetCasterSummon(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        if (!ctx->source || ctx->source->isSummon())
            return;
        for (auto c : *inlist) {
            if (c->isSummon() && c->getSummonSource() && c->getSummonSource()->myID == ctx->source->myID && c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                outlist->push_back(c);
        }
    }

    static void targetFarthest(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        int distance = -1;
        CharacterData *t = nullptr;
        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;

            int dist = HeroActionController::distanceToCharacter(ctx->source, c);
            if (dist > distance) {
                distance = dist;
                t = c;
            }
        }
        if (t)
            outlist->push_back(t);
    }

    static void targetNearest(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        CharacterData *t = nullptr;
        int distance = 100000000;
        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            
            int dist = HeroActionController::distanceToCharacter(ctx->source, c);
            if (dist < distance) {
                distance = dist;
                t = c;
            }
        }
        
        if (t)
            outlist->push_back(t);
    }

    static void targetSexType(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        for (auto c : *inlist) {
            if (c->getSex() == ctx->ft_sex_type && c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                outlist->push_back(c);
        }
    }

    static void targetRandomInRange(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        if (!ctx->ft_range_origin && !ctx->projectile)
            return;

        auto projectile = ctx->projectile;
        auto origin = ctx->ft_range_origin;
        auto x1 = ctx->range.x1;
        auto x2 = ctx->range.x2;
        auto y1 = ctx->range.y1;
        auto y2 = ctx->range.y2;

        int x11 = projectile == nullptr ? origin->getX() + x1 : projectile->currentX + x1;
        int x22 = projectile == nullptr ? origin->getX() + x2 : projectile->currentX + x2;
        if (origin->direction == -1)
        {
            x11 = projectile == nullptr ? origin->getX() - x2 : projectile->currentX - x2;
            x22 = projectile == nullptr ? origin->getX() - x1 : projectile->currentX - x1;
        }
        int y11 = projectile == nullptr ? origin->getY() + y1 : projectile->currentY + y1;
        int y22 = projectile == nullptr ? origin->getY() + y2 : projectile->currentY + y2;

        std::vector<CharacterData*> tmpcs;

        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            auto cx = c->getX();
            auto cy = c->getY();
            if (cx >= x11 && cx <= x22 && cy >= y11 && cy <= y22)
                tmpcs.push_back(c);
        }

        if (tmpcs.size() > 0) {
            int idx = AIContext::getInstance()->localRandom->nextInt(0, tmpcs.size());
            auto t = tmpcs.at(idx);
            outlist->push_back(t);
        }
    }

    static void targetRandomOnlyFirst(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        if (ctx->ft_random_first_character) {
            outlist->push_back(ctx->ft_random_first_character);
            return;
        }

        targetRandom(ctx, inlist, outlist);
    }

    static void targetSummon(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        for (auto c : *inlist) {
            if (ctx->ft_relationship == FR_FRIENDS) {
                if (c->isSummon() && c->getFakeSummonType() == 0 /*12.0*/ && c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                    outlist->push_back(c);
            } else {
                if (c->hasTag("fakesummoneff_summon"))
                    outlist->push_back(c);
                else if (c->isSummon()) {
                    if (c->hasTag("fakesummoneff_notsummon"))
                        continue;
                    if (c->getFakeSummonType() == 0 /*12.0*/ && c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                        outlist->push_back(c);
                }
            }
        }
    }

    static CharacterData *getTargetAttrHighest(AITargetContext *ctx, int idx, CharacterDataListPtr inlist)
    {
        if (idx <= 0 || idx >= ATT_LEN)
            return nullptr;
        CharacterData *t = nullptr;
        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            if (!t || t->attributes[idx] < c->attributes[idx]) {
                t = c;
            }
        }

        return t;
    }

    static void targetAttrHighest(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        if (!ctx->ft_attr_comp_idxs)
            return;
        
        CharacterData *t = nullptr;
        int *hv = nullptr;

        for (auto idx : *ctx->ft_attr_comp_idxs)
        {
            auto c = getTargetAttrHighest(ctx, idx, inlist);
            if (!c)
                continue;

            if (!t || (t && hv && *hv < c->attributes[idx])) {
                t = c;
                hv = &(c->attributes[idx]);
            }
        }

        if (t)
            outlist->push_back(t);
    }

    static CharacterData *getTargetAttrLowest(AITargetContext *ctx, int idx, CharacterDataListPtr inlist)
    {
        if (idx <= 0 || idx >= ATT_LEN)
            return nullptr;
        CharacterData *t = nullptr;
        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            if (!t || t->attributes[idx] > c->attributes[idx]) {
                t = c;
            }
        }

        return t;
    }

    static void targetAttrLowest(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        if (!ctx->ft_attr_comp_idxs)
            return;
        
        CharacterData *t = nullptr;
        int *lv = nullptr;

        for (auto idx : *ctx->ft_attr_comp_idxs)
        {
            auto c = getTargetAttrLowest(ctx, idx, inlist);
            if (!c)
                continue;

            if (!t || (t && lv && *lv > c->attributes[idx])) {
                t = c;
                lv = &(c->attributes[idx]);
            }
        }

        if (t)
            outlist->push_back(t);
    }

    static void targetInAbsoluteRange(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        int x11 = 0;
        int x22 = 0;
        int y11 = 0;
        int y22 = 0;

        auto range_origin = ctx->ft_range_origin;
        auto projectile = ctx->projectile;
        if (range_origin) {
            if (range_origin->mStationaryDirection && range_origin->mStationaryX && range_origin->mStationaryY) {
                if (range_origin->mStationaryDirection == -1) {
                    x11 = projectile == nullptr ? range_origin->mStationaryX - ctx->range.x2 : projectile->currentX - ctx->range.x2;
                    x22 = projectile == nullptr ? range_origin->mStationaryX - ctx->range.x1 : projectile->currentX - ctx->range.x1;
                } else if (range_origin->mStationaryDirection == 1) {
                    x11 = projectile == nullptr ? range_origin->mStationaryX + ctx->range.x1 : projectile->currentX + ctx->range.x1;
                    x22 = projectile == nullptr ? range_origin->mStationaryX + ctx->range.x2 : projectile->currentX + ctx->range.x2;
                }
                
                y11 = projectile == nullptr ? range_origin->mStationaryY + ctx->range.y1 : projectile->currentY + ctx->range.y1;
                y22 = projectile == nullptr ? range_origin->mStationaryY + ctx->range.y2 : projectile->currentY + ctx->range.y2;
            }
        }

        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;

            if (c->getX() >= x11 && c->getX() <= x22 && c->getY() >= y11 && c->getY() <= y22) {
                if (!ctx->ft_absolute_range_exclude_character || ctx->ft_absolute_range_exclude_character->myID != c->myID) {
                    outlist->push_back(c);
                }
            }
        }
    }

    static void targetCharacterId(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        if (!ctx->search_params || !ctx->source)
            return;

        // [[side,id], ...]
        std::map<int, bool> ids_our;
        std::map<int, bool> ids_enemy;
        for (int i = 0; i < ctx->search_params->size() - 1;)
        {
            int side = (*ctx->search_params)[i];
            int an_id = (*ctx->search_params)[i+1];
			// 1-友军阵营 2-敌军阵营
            if (side == 1)
                ids_our[an_id] = true;
            else if (side == 2)
                ids_enemy[an_id] = true;
            
            i += 2;
        }

        for (auto c : *inlist) {
            if ((c->teamSide == ctx->source->teamSide && ids_our.find(c->getID()) != ids_our.end()) ||
                (c->teamSide != ctx->source->teamSide && ids_enemy.find(c->getID()) != ids_enemy.end())) {
                if (c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                    outlist->push_back(c);
            }
        }
    }

    static void targetInBattleFieldRange(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        int c_direction = 0;
        if (ctx->source->teamSide == TeamSide::kOur)
            c_direction = 1;
        else if (ctx->source->teamSide == TeamSide::kEnemy)
            c_direction = -1;

        auto cx = AIContext::getInstance()->getBattleFieldCenterX();
        auto cy = (AIContext::getInstance()->getDeltaMaxY() - AIContext::getInstance()->getDeltaMinY()) / 2;

        int x11 = cx + (c_direction * ctx->range.x1);
        int x22 = cx + (c_direction * ctx->range.x2);

        int y11 = cy + ctx->range.y1;
        int y22 = cy + ctx->range.y2;

        if (x11 > x22) {
            int tmpx = x11;
            x11 = x22;
            x22 = tmpx;
        }
        if (y11 > y22) {
            int tmpy = y11;
            y11 = y22;
            y22 = tmpy;
        }

        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            if (c->getX() >= x11 && c->getX() <= x22 && c->getY() >= y11 && c->getY() <= y22)
                outlist->push_back(c);
        }
    }

    static void targetMostHurt(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        if (ctx->hurt_id <= 0)
            return;

        int max_hpdelta = 0;
        CharacterData *t = nullptr;
        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            int a_hpdelta = HeroHurtcalculate::simulateCalcHurt(*AIContext::getInstance(), ctx->source, ctx->projectile, ctx->hurt_id, 0, ctx->skill_level, c);
            if (!t || a_hpdelta > max_hpdelta) {
                max_hpdelta = a_hpdelta;
                t = c;
            }
        }

        if (t)
            outlist->push_back(t);
    }

    static void targetInHpRatioRange(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        int low = ctx->range.x1;
        int high = ctx->range.x2;
        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            int a_hpratio = CharacterFunc::GetCharacterHPRatio(*c) * 100.0f;
            if (a_hpratio >= low && a_hpratio <= high)
                outlist->push_back(c);
        }
    }

    static void targetTranversal(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        auto s = ctx->source;
        // param.x1 - 换目标的时间间隔, param.x2 - 重置遍历的时间
        int intervalTick = ctx->range.x1;
        int totalTick = ctx->range.x2;
        if (s->getTraversalStartTick() <= 0 || s->localTick > s->getTraversalStartTick() + totalTick) {
            s->resetTraversalTargetRecord();
            s->setTraversalStartTick(s->localTick);
        }

        if (s->getTraversalLastPickTick() > 0 && s->localTick < s->getTraversalLastPickTick() + intervalTick)
            return;

        for (auto c : *inlist) {
            if (c->canBeAttacked(s, ctx->ft_is_ignorecanbeattackeffect) && !s->isTargetLockedByTraversal(c->myID)) {
                outlist->push_back(c);
                s->pickTraversalTarget(c->myID, s->localTick);
                return;
            }
        }
    }

    static void targetHasMark(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            for (auto a_mark : c->markCasts) {
                if (a_mark->markID == ctx->ft_mark_id_to_have) {
                    outlist->push_back(c);
                    break;
                }
            }
        }
    }

    static void targetHasLabel(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            bool has_label = true;
            auto &&labels = c->staticData->vLabels;
            for (int i = 0; i < ctx->search_params->size(); i++) {
                auto the_label = (*ctx->search_params)[i];
                if (std::find(labels.begin(), labels.end(), the_label) == labels.end()) {
                    has_label = false;
                    break;
                }
            }

            if (has_label)
                outlist->push_back(c);
        }
    }

    static void targetRandomDead(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        std::vector<CharacterData*> tmpcs;
        for (auto c : *inlist) {
            if (HeroActionController::isDeadAction(c->currentAction) && !c->isSummon())
                tmpcs.push_back(c);
        }
        if (tmpcs.size() > 0) {
            int idx = AIContext::getInstance()->localRandom->nextInt(0, tmpcs.size());
            auto t = tmpcs.at(idx);
            outlist->push_back(t);
        }
    }

    static void targetDead(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        for (auto c : *inlist) {
            if (HeroActionController::isDeadAction(c->currentAction) && !c->isSummon())
                outlist->push_back(c);
        }
    }
    
    static void targetHighestCombatPower(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        CharacterData *t = nullptr;
        for (auto c : *inlist) {
            if (!HeroActionController::isDeadAction(c->currentAction) && c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect)) {
                if (!t || c->combatPower > t->combatPower)
                    t = c;
            }
        }
        
        if (t)
            outlist->push_back(t);
    }

    static void targetScoreType(AITargetContext *ctx, CharacterDataListPtr inlist, CharacterDataListPtr outlist)
    {
        CharacterData *t = nullptr;
        for (auto c : *inlist) {
            if (!HeroActionController::isDeadAction(c->currentAction) && c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect)) {
                // 参数第二列起,配置星级:
                for (int i = 1; i < ctx->search_params->size(); i++) {
                    bool hasMatch = false;
                    int theScore = ctx->search_params->at(i);
                    switch (theScore) {
                    case 1:
                        hasMatch = c->staticData->comment.compare("SSS") == 0;
                        break;
                    case 2:
                        hasMatch = c->staticData->comment.compare("SS+") == 0;
                        break;
                    case 3:
                        hasMatch = c->staticData->comment.compare("SS") == 0;
                        break;
                    case 4:
                        hasMatch = c->staticData->comment.compare("S") == 0;
                        break;
                    case 5:
                        hasMatch = c->staticData->comment.compare("A") == 0;
                        break;
                    case 6:
                        hasMatch = c->staticData->comment.compare("B") == 0;
                        break;
                    }
                    if (hasMatch) {
                        outlist->push_back(c);
                        break;
                    }
                }
            }
        }
    }

    ///
    static void targetCommon(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        // 1.
        if (ctx->ft_ispresearch && presearch(ctx, outlist))
            return;
        
        // 2.
        std::list<CharacterData *> cs;
        if (FR_FRIENDS == ctx->ft_relationship || FR_ALL == ctx->ft_relationship) {
            filterFriendList(ctx, &cs);
        }
        if (FR_RIVALS == ctx->ft_relationship || FR_ALL == ctx->ft_relationship) {
            filterRivalList(ctx, &cs);
        }
        if (FR_FRIENDS_ORIGIN == ctx->ft_relationship || FR_ALL_ORIGIN == ctx->ft_relationship) {
            filterFriendOriginList(ctx, &cs);
        }
        if (FR_RIVALS_ORIGIN == ctx->ft_relationship || FR_ALL_ORIGIN == ctx->ft_relationship) {
            filterRivalOriginList(ctx, &cs);
        }
        auto inlist = autoPickFilterList(ctx, &cs);

        // 3.
        switch (ctx->ft_func) {
            case FF_LOWEST_HPRATIO:
                targetLowestHPRatio(ctx, inlist, outlist);
                break;
            case FF_HIGHEST_HPRATIO:
                targetHighestHPRatio(ctx, inlist, outlist);
                break;
            case FF_RANDOM:
                targetRandom(ctx, inlist, outlist);
                break;
            case FF_CANBEATTACK:
                targetCanBeAttack(ctx, inlist, outlist);
                break;
            case FF_INRANGE:
                targetInRange(ctx, inlist, outlist);
                break;
            case FF_CASTERSUMMON:
                targetCasterSummon(ctx, inlist, outlist);
                break;
            case FF_FARTHEST:
                targetFarthest(ctx, inlist, outlist);
                break;
            case FF_NEAREST:
                targetNearest(ctx, inlist, outlist);
                break;
            case FF_SEXTYPE:
                targetSexType(ctx, inlist, outlist);
                break;
            case FF_RANDOM_INRANGE:
                targetRandomInRange(ctx, inlist, outlist);
                break;
            case FF_RANDOM_ONLY_FIRSTTIME:
                targetRandomOnlyFirst(ctx, inlist, outlist);
                break;
            case FF_SUMMON:
                targetSummon(ctx, inlist, outlist);
                break;
            case FF_ATTR_HIGHEST:
                targetAttrHighest(ctx, inlist, outlist);
                break;
            case FF_ATTR_LOWEST:
                targetAttrLowest(ctx, inlist, outlist);
                break;
            case FF_INABSOLUTERANGE:
                targetInAbsoluteRange(ctx, inlist, outlist);
                break;
            case FF_CHARACTERID:
                targetCharacterId(ctx, inlist, outlist);
                break;
            case FF_INBATTLEFIELDRANGE:
                targetInBattleFieldRange(ctx, inlist, outlist);
                break;
            case FF_MOST_HURT:
                targetMostHurt(ctx, inlist, outlist);
                break;
            case FF_IN_HPRATIO_RANGE:
                targetInHpRatioRange(ctx, inlist, outlist);
                break;
            case FF_TRANVERSAL:
                targetTranversal(ctx, inlist, outlist);
                break;
            case FF_HAS_MARK:
                targetHasMark(ctx, inlist, outlist);
                break;
            case FF_HAS_LABEL:
                targetHasLabel(ctx, inlist, outlist);
                break;
            case FF_RANDOM_DEAD:
                targetRandomDead(ctx, inlist, outlist);
                break;
            case FF_DEAD:
                targetDead(ctx, inlist, outlist);
                break;
            case FF_HIGHEST_COMBATPOWER:
                targetHighestCombatPower(ctx, inlist, outlist);
                break;
            case FF_SCORETYPE:
                targetScoreType(ctx, inlist, outlist);
                break;
            default:
                CCLOG("ai:target func type not found:%d", ctx->ft_func);
                break;
        }
    }

    static void seqFindTargetByLabel(AITargetContext *ctx, std::list<CharacterData*> *inlist, std::list<CharacterData*> *outlist, std::vector<int> *findlabels)
    {
        for (auto c : *inlist) {
            if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                continue;
            bool has_label = true;
            auto &&labels = c->staticData->vLabels;
            for (int the_label : *findlabels)
            {
                if (std::find(labels.begin(), labels.end(), the_label) == labels.end())
                {
                    has_label = false;
                    break;
                }
            }

            if (has_label)
                outlist->push_back(c);
        }
    }

    static void seqFindTargetByAttr(AITargetContext *ctx, std::list<CharacterData*> *inlist, std::list<CharacterData*> *outlist, int idx, int hl)
    {
        CharacterData *t = nullptr;
        if (hl == 0)
            t = getTargetAttrHighest(ctx, idx, inlist);
        else if (hl == 1)
            t = getTargetAttrLowest(ctx, idx, inlist);
        if (t)
            outlist->push_back(t);
    }

    static void seqFindTargetByCount(AITargetContext *ctx, std::list<CharacterData*> *inlist, std::list<CharacterData*> *outlist, int rule, int cnt)
    {
        size_t sz = inlist->size();
        bool ismatch = false;

        // 条件 0-小于 1-小于等于 2-等于 3-大于 4-大于等于
        switch (rule)
        {
        case 0: if (sz < cnt) ismatch = true; break;
        case 1: if (sz <= cnt) ismatch = true; break;
        case 2: if (sz == cnt) ismatch = true; break;
        case 3: if (sz > cnt) ismatch = true; break;
        case 4: if (sz >= cnt) ismatch = true; break;
        }

        if (ismatch)
        {
            outlist->assign(inlist->begin(), inlist->end());
        }
    }

	static void seqFindTargetByGender(AITargetContext *ctx, std::list<CharacterData*> *inlist, std::list<CharacterData*> *outlist, int gender)
	{
		for (CharacterData *c : *inlist) {
			if (!c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
				continue;

			if (c->getSex() == gender)
				outlist->push_back(c);
		}
	}

    static void seqFindTargetByRandom(AITargetContext* ctx, std::list<CharacterData*>* inlist, std::list<CharacterData*>* outlist, int count, int canSame)
    {   
        std::vector<CharacterData*> tmpcs;
        for (auto c : *inlist) {
            if (ctx->ft_random_exclude_character && ctx->ft_random_exclude_character->myID == c->myID)
                continue;
            if (c->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                tmpcs.push_back(c);
        }
        size_t sz = tmpcs.size();
        if (sz == 0) {
            return;
        }

        if (canSame == 0) {
            if (sz <= count) {
                outlist->assign(tmpcs.begin(), tmpcs.end());
            } else {
                if (ctx->is_skip_random) {
                    for (int i = 0; i < count; i++) {
                        auto t = tmpcs.at(i);
                        outlist->push_back(t);
                    }
                }
                else 
                {
                    for (int i = 0; i < count; i++) {
                        int idx = AIContext::getInstance()->localRandom->nextInt(0, tmpcs.size());
                        auto t = tmpcs.at(idx);
                        outlist->push_back(t);
                        tmpcs.erase(tmpcs.begin() + idx);
                    }
                }
            }
        } else {
            if (ctx->is_skip_random) {
                for (int i = 0; i < count; i++) {
                    auto t = tmpcs.at(i % sz);
                    outlist->push_back(t);
                }
            }
            else
            {
                for (int i = 0; i < count; i++) {
                    targetRandom(ctx, inlist, outlist);
                }
            }
        }
    }

    void targetSequence(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        // 1.
        if (ctx->ft_ispresearch && presearch(ctx, outlist))
            return;

        // 2.
        std::list<CharacterData *> cs;
        if (FR_FRIENDS == ctx->ft_relationship || FR_ALL == ctx->ft_relationship) {
            filterFriendList(ctx, &cs);
        }
        if (FR_RIVALS == ctx->ft_relationship || FR_ALL == ctx->ft_relationship) {
            filterRivalList(ctx, &cs);
        }
        if (FR_FRIENDS_ORIGIN == ctx->ft_relationship || FR_ALL_ORIGIN == ctx->ft_relationship) {
            filterFriendOriginList(ctx, &cs);
        }
        if (FR_RIVALS_ORIGIN == ctx->ft_relationship || FR_ALL_ORIGIN == ctx->ft_relationship) {
            filterRivalOriginList(ctx, &cs);
        }
        std::list<CharacterData *> *filterlist = autoPickFilterList(ctx, &cs);

		// 25.0 参数第二列：排除规则 0-无效 1-排除自己
		if (ctx->search_params->at(1) == 1)
			filterlist->remove(ctx->source);

        // inlist, outlist
        std::list<CharacterData *> inlist = *filterlist;
        std::list<CharacterData *> tmplist;

        // 25.0 参数第三列开始筛选规则
        for (int i = 2; i < ctx->search_params->size();)
        {
            int t = ctx->search_params->at(i);
            if (t == 1) // label
            {
                int p1 = ctx->search_params->at(i + 1); // p1指定标签数量
                std::vector<int> labels;
                for (int ii = 0; ii < p1; ii++)
                {
                    labels.push_back(ctx->search_params->at(i + 2 + ii)); // p1后续字段为标签参数
                }
                i += p1 + 2;

                seqFindTargetByLabel(ctx, &inlist, &tmplist, &labels);
            }
            else if (t == 2) // attr
            {
                int p1 = ctx->search_params->at(i + 1); // p1属性类型
                int p2 = ctx->search_params->at(i + 2); // p2排序类型 最高/最低
                i += 3;

                seqFindTargetByAttr(ctx, &inlist, &tmplist, p1, p2);
            }
            else if (t == 3) // cnt
            {
                int p1 = ctx->search_params->at(i + 1);
                int p2 = ctx->search_params->at(i + 2);
                i += 3;

                seqFindTargetByCount(ctx, &inlist, &tmplist, p1, p2);
            }
			else if (t == 4) // gender 25.0 性别
			{
				int p1 = ctx->search_params->at(i + 1);
				i += 2;

				seqFindTargetByGender(ctx, &inlist, &tmplist, p1);
			}
            else if (t == 5) // random 28.0
            {
                int p1 = ctx->search_params->at(i + 1); //随机数量
                int p2 = ctx->search_params->at(i + 2); //目标是否可重复
                i += 3;

                seqFindTargetByRandom(ctx, &inlist, &tmplist, p1, p2);
            }
            else
            {
                break;
            }

            if (tmplist.size() <= 0)
                return;

            // 最后一次的筛选结果作为最终结果
            if (i < ctx->search_params->size())
            {
                inlist = tmplist;
                tmplist.clear();
            }
        }

        for (CharacterData *c : tmplist)
        {
            outlist->push_back(c);
        }
    }
    
    template<int N>
    extern void targetFunc(AITargetContext *ctx, std::list<CharacterData*> *outlist);

    /// 0 - 继承目标
    template<>
    void targetFunc<0>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target0(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        outlist->push_back(ctx->source);
    }

    /// 1 - 当前目标
    template<>
    void targetFunc<1>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target1(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        if (presearch(ctx, outlist))
            return;
        if (ctx->target && ctx->target->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect, ctx->skill_caster))
            outlist->push_back(ctx->target);
    }

    /// 2 - 所有友军
    template<>
    void targetFunc<2>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target2new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_CANBEATTACK;

        targetCommon(ctx, outlist);
    }

    /// 3 - 血量百分比最低友军
    template<>
    void targetFunc<3>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_LOWEST_HPRATIO;
        targetCommon(ctx, outlist);
    }

    /// 4 - 随机友军
    template<>
    void targetFunc<4>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target4new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_RANDOM;
        targetCommon(ctx, outlist);
    }

    /// 5 - 对方所有
    template<>
    void targetFunc<5>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target5new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_CANBEATTACK;
        targetCommon(ctx, outlist);
    }

    /// 6 - 对方血量百分比最低单位
    template<>
    void targetFunc<6>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target6new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_LOWEST_HPRATIO;
        targetCommon(ctx, outlist);
    }

    /// 7 - 对方随机单位
    template<>
    void targetFunc<7>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target7new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_RANDOM;
        targetCommon(ctx, outlist);
    }

    /// 8 - 自己范围内所有敌对单位
    template<>
    void targetFunc<8>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target8new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_INRANGE;
        ctx->ft_range_origin = ctx->source;
        targetCommon(ctx, outlist);
    }

    /// 9 - 自己范围内所有友方单位
    template<>
    void targetFunc<9>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target9new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_INRANGE;
        ctx->ft_range_origin = ctx->source;
        targetCommon(ctx, outlist);
    }

    /// 10 - 当前目标范围内所有敌对单位
    template<>
    void targetFunc<10>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target10new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_INRANGE;
        ctx->ft_range_origin = ctx->target;
        targetCommon(ctx, outlist);
    }

    /// 11 - 当前目标范围内所有友方单位
    template<>
    void targetFunc<11>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target11new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_INRANGE;
        ctx->ft_range_origin = ctx->target;
        targetCommon(ctx, outlist);
    }

    /// 12 - 自己的所有召唤单位
    template<>
    void targetFunc<12>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target12new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_CASTERSUMMON;
        targetCommon(ctx, outlist);
    }

    /// 13 - 自己范围内所有人
    template<>
    void targetFunc<13>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target13new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_INRANGE;
        ctx->ft_range_origin = ctx->source;
        targetCommon(ctx, outlist);
    }

    /// 14 - 距离最远的敌人
    template<>
    void targetFunc<14>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target14new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_FARTHEST;
        targetCommon(ctx, outlist);
    }

    /// 15 - 技能施法者
    template<>
    void targetFunc<15>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target15(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        if (!HeroActionController::isDeadAction(ctx->skill_caster->currentAction))
            outlist->push_back(ctx->skill_caster);
    }

    /// 16 - 所有人中一个随机单位
    template<>
    void targetFunc<16>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target16new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_RANDOM;
        targetCommon(ctx, outlist);
    }

    /// 17 - 最远的友方
    template<>
    void targetFunc<17>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target17new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_FARTHEST;
        targetCommon(ctx, outlist);
    }

    /// 18 - 最近的友方
    template<>
    void targetFunc<18>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target18new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_NEAREST;
        targetCommon(ctx, outlist);
    }

    /// 19 - 最近的敌方
    template<>
    void targetFunc<19>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target19new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_NEAREST;
        targetCommon(ctx, outlist);
    }

    /// 20 - 友方的男性
    template<>
    void targetFunc<20>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target20new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_SEXTYPE;
        ctx->ft_sex_type = CharData::Sex_Male;
        targetCommon(ctx, outlist);
    }

    /// 21 - 敌方的男性
    template<>
    void targetFunc<21>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target21new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_SEXTYPE;
        ctx->ft_sex_type = CharData::Sex_Male;
        targetCommon(ctx, outlist);
    }

    /// 22 - 范围内随机的一个友军
    template<>
    void targetFunc<22>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target22new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_RANDOM_INRANGE;
        ctx->ft_range_origin = ctx->source;
        targetCommon(ctx, outlist);
    }

    /// 23 - 范围内随机的一个敌军
    template<>
    void targetFunc<23>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target23new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_RANDOM_INRANGE;
        ctx->ft_range_origin = ctx->source;
        targetCommon(ctx, outlist);
    }

    /// 24 - 不含搜索者自己的随机友军(不会随到自己)
    template<>
    void targetFunc<24>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target24new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_RANDOM;
        ctx->ft_random_exclude_character = ctx->source;
        targetCommon(ctx, outlist);
    }

    /// 25 - 不含搜索者自己的随机敌军(不会随到自己)
    template<>
    void targetFunc<25>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target25new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_RANDOM;
        ctx->ft_random_exclude_character = ctx->source;
        targetCommon(ctx, outlist);
    }
    
    /// 26 - 随机一个敌人，并且若是持续伤害，每次伤害不再重新随机目标
    template<>
    void targetFunc<26>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target26new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_RANDOM_ONLY_FIRSTTIME;
        if (ctx->source->GetFlagFirstFound26Target() && ctx->source->GetFirstRoundConfirm26Target())
            ctx->ft_random_first_character = ctx->source->GetFirstRoundConfirm26Target();
        targetCommon(ctx, outlist);
    }

    /// 27 - 所有友军召唤物
    template<>
    void targetFunc<27>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target27new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_SUMMON;
        targetCommon(ctx, outlist);
    }

    /// 28 - 所有敌军召唤物
    template<>
    void targetFunc<28>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target28new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_SUMMON;
        targetCommon(ctx, outlist);
    }

    /// 29 - 所有召唤物
    template<>
    void targetFunc<29>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target29new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_SUMMON;
        targetCommon(ctx, outlist);
    }

    /// 30 - 所有人中某项属性最高的目标
    template<>
    void targetFunc<30>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target30new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_ATTR_HIGHEST;
        ctx->ft_attr_comp_idxs = ctx->search_params;
        targetCommon(ctx, outlist);
    }

    /// 31 - 友方某项属性最高的目标
    template<>
    void targetFunc<31>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target31new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_ATTR_HIGHEST;
        ctx->ft_attr_comp_idxs = ctx->search_params;
        targetCommon(ctx, outlist);
    }

    /// 32 - 敌方某项属性最高的目标
    template<>
    void targetFunc<32>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target32new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_ATTR_HIGHEST;
        ctx->ft_attr_comp_idxs = ctx->search_params;
        targetCommon(ctx, outlist);
    }

    /// 33 - 友方的女性
    template<>
    void targetFunc<33>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target33new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_SEXTYPE;
        ctx->ft_sex_type = CharData::Sex_Female;
        targetCommon(ctx, outlist);
    }

    /// 34 - 敌方的女性
    template<>
    void targetFunc<34>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target34new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_SEXTYPE;
        ctx->ft_sex_type = CharData::Sex_Female;
        targetCommon(ctx, outlist);
    }

    /// 35 - 敌我双方的女性
    template<>
    void targetFunc<35>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target35new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_SEXTYPE;
        ctx->ft_sex_type = CharData::Sex_Female;
        targetCommon(ctx, outlist);
    }

    /// 36 - 定点范围内的所有敌军，不随角色转向而转向
    template<>
    void targetFunc<36>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target36new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_INABSOLUTERANGE;
        ctx->ft_range_origin = ctx->source;
        targetCommon(ctx, outlist);
    }

    /// 37 -  定点范围内的所有友军，不随角色转向而转向
    template<>
    void targetFunc<37>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target37new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_INABSOLUTERANGE;
        ctx->ft_range_origin = ctx->source;
        targetCommon(ctx, outlist);
    }

    /// 38 - 定点范围内的所有人，不随角色转向而转向
    template<>
    void targetFunc<38>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target38new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_INABSOLUTERANGE;
        ctx->ft_range_origin = ctx->source;
        targetCommon(ctx, outlist);
    }

    /// 39 - 定点范围内的所有敌/友方(不包含自己），不随角色转向而转向
    template<>
    void targetFunc<39>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target39new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_INABSOLUTERANGE;
        ctx->ft_range_origin = ctx->source;
        ctx->ft_absolute_range_exclude_character = ctx->source;
        targetCommon(ctx, outlist);
    }

    /// 40 - 指定阵营的某个或某些角色id
    template<>
    void targetFunc<40>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target40new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        //FF_CHARACTERID
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_CHARACTERID;
        targetCommon(ctx, outlist);
    }

    /// 41 - 考虑自身攻击范围的随机敌人
    template<>
    void targetFunc<41>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target41new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_RANDOM;
        auto s = ctx->source;
        if (s->mAttackConfirmedTarget && s->target && s->target->canBeAttacked(s, ctx->ft_is_ignorecanbeattackeffect)) {
            ctx->ft_random_first_character = s->target;
        }
        targetCommon(ctx, outlist);
    }

    /// 42 - 攻击者
    template<>
    void targetFunc<42>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target42new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        if (presearch(ctx, outlist))
            return;
        auto s = ctx->source;
        if (s->attacker && s->attacker->canBeAttacked(s, ctx->ft_is_ignorecanbeattackeffect, ctx->skill_caster))
            outlist->push_back(s->attacker);
        else if (s->target && s->target->canBeAttacked(s, ctx->ft_is_ignorecanbeattackeffect, ctx->skill_caster))
            outlist->push_back(s->target);
    }

    /// 43 - 屏幕绝对坐标范围内的目标
    template<>
    void targetFunc<43>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target43new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_INBATTLEFIELDRANGE;
        targetCommon(ctx, outlist);
    }

    /// 44 - 绝对坐标内的所有友军
    template<>
    void targetFunc<44>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target44new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_INBATTLEFIELDRANGE;
        targetCommon(ctx, outlist);
    }

    /// 45 - 所有人中某项属性最低的目标
    template<>
    void targetFunc<45>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target45new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_ATTR_LOWEST;
        ctx->ft_attr_comp_idxs = ctx->search_params;
        targetCommon(ctx, outlist);
    }

    /// 46 - 友方某项属性最低的目标
    template<>
    void targetFunc<46>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target46new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_ATTR_LOWEST;
        ctx->ft_attr_comp_idxs = ctx->search_params;
        targetCommon(ctx, outlist);
    }

    /// 47 - 敌方某项属性最低的目标
    template<>
    void targetFunc<47>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target47new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_ATTR_LOWEST;
        ctx->ft_attr_comp_idxs = ctx->search_params;
        targetCommon(ctx, outlist);
    }

    /// 48 - 能造成最多伤害的敌方
    template<>
    void targetFunc<48>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target48new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_MOST_HURT;
        targetCommon(ctx, outlist);
    }

    /// 49 - 生命值处于某个百分比区间的所有敌人
    template<>
    void targetFunc<49>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target49new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_IN_HPRATIO_RANGE;
        targetCommon(ctx, outlist);
    }

    /// 50 - 对所有敌人依次遍历，一次只选一个，时间间隔内只能选一次，选过后找不到目标
    template<>
    void targetFunc<50>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target50new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_TRANVERSAL;
        targetCommon(ctx, outlist);
    }

    /// 51 - 幻象的召唤者
    template<>
    void targetFunc<51>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target51new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        if (presearch(ctx, outlist))
            return;
        auto s = ctx->source;
        if (s->isSummon() && s->getSummonSource() && s->getSummonSource()->canBeAttacked(s, ctx->ft_is_ignorecanbeattackeffect, ctx->skill_caster))
            outlist->push_back(s->getSummonSource());
    }

    /// 52 - 拥有指定标记的目标
    template<>
    void targetFunc<52>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target52new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        if (presearch(ctx, outlist))
            return;
        if (ctx->search_params && ctx->search_params->size() >= 3) {
            //参数[a, b, c]
            //    a--标记ID
            //    b--我方还是敌方 0-所有 1-我方 2-敌方
            //    c--没有目标时是否使用当前目标（0-否，1 - 是，目标选为当前目标）
            ctx->ft_ispresearch = false;
            
            int side = (*ctx->search_params)[1];
            if (side == 0)
                ctx->ft_relationship = FR_ALL;
            else if (side == 1)
                ctx->ft_relationship = FR_FRIENDS;
            else if (side == 2)
                ctx->ft_relationship = FR_RIVALS;
            ctx->ft_func = FF_HAS_MARK;
            ctx->ft_mark_id_to_have = (*ctx->search_params)[0];
            targetCommon(ctx, outlist);

            if (outlist->size() < 1 && (*ctx->search_params)[2] == 1) {
                if (ctx->source->target && ctx->source->target->canBeAttacked(ctx->source, ctx->ft_is_ignorecanbeattackeffect))
                    outlist->push_back(ctx->source->target);
            }
        }
    }

    /// 53 - 范围内的施法者召唤单位
    template<>
    void targetFunc<53>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target53new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        if (presearch(ctx, outlist))
            return;

        auto s = ctx->source;
        int x11 = s->getX() + ctx->range.x1;
        int x22 = s->getX() + ctx->range.x2;
        if (s->direction == -1) {
            x11 = s->getX() - ctx->range.x2;
            x22 = s->getX() - ctx->range.x1;
        }
        int y11 = s->getY() + ctx->range.y1;
        int y22 = s->getY() + ctx->range.y2;

        for (auto a_summoncaster : s->summonCasts) {
            for (auto a_summonuid : a_summoncaster->summonUIDList) {
                auto c = AIContext::getInstance()->getCharacterByID(a_summonuid);
                if (c && c->isSummon() && c->getFakeSummonType() == 0 /*12.0*/ && c->getSummonSource()->myID == s->myID && c->canBeAttacked(s, ctx->ft_is_ignorecanbeattackeffect, ctx->skill_caster)) {
                    if (c->getX() >= x11 && c->getX() <= x22 && c->getY() >= y11 && c->getY() <= y22)
                        outlist->push_back(c);
                }
            }
        }
    }

    /// 54 - 拥有所有指定标签的敌方单位
    template<>
    void targetFunc<54>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target54new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_HAS_LABEL;
        targetCommon(ctx, outlist);
    }

    /// 55 - 拥有所有指定标签的所有单位
    template<>
    void targetFunc<55>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target55new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_HAS_LABEL;
        targetCommon(ctx, outlist);
    }

    /// 56 - 随机一个已死亡的友军
    template<>
    void targetFunc<56>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target56new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_RANDOM_DEAD;
        targetCommon(ctx, outlist);
    }

    /// 57 - 随机一个已死亡的敌军
    template<>
    void targetFunc<57>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target57new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_RANDOM_DEAD;
        targetCommon(ctx, outlist);
    }

    /// 58 - 随机一个已死亡的友军/敌军
    template<>
    void targetFunc<58>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target58new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_RANDOM_DEAD;
        targetCommon(ctx, outlist);
    }

    /// 59 - 所有已死亡的友军
    template<>
    void targetFunc<59>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target59new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_DEAD;
        targetCommon(ctx, outlist);
    }

    /// 60 - 所有已死亡的敌军
    template<>
    void targetFunc<60>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target60new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_DEAD;
        targetCommon(ctx, outlist);
    }

    /// 61 - 所有已死亡的友军/敌军
    template<>
    void targetFunc<61>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target61new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_DEAD;
        targetCommon(ctx, outlist);
    }

    /// 62 - 血量百分比最高的友军
    template<>
    void targetFunc<62>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target62new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_HIGHEST_HPRATIO;
        targetCommon(ctx, outlist);
    }

    /// 63 - 血量百分比最低的敌军
    template<>
    void targetFunc<63>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target63new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_HIGHEST_HPRATIO;
        targetCommon(ctx, outlist);
    }

    /// 64 - 敌军之中战力最高的目标
    template<>
    void targetFunc<64>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_RIVALS;
        ctx->ft_func = FF_HIGHEST_COMBATPOWER;
        targetCommon(ctx, outlist);
    }
    
    /// 65 - 友军之中战力最高的目标
    template<>
    void targetFunc<65>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_HIGHEST_COMBATPOWER;
        targetCommon(ctx, outlist);
    }
    
    /// 66 - 所有人中战力最高的目标
    template<>
    void targetFunc<66>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_ALL;
        ctx->ft_func = FF_HIGHEST_COMBATPOWER;
        targetCommon(ctx, outlist);
    }
    
    /// 67 - 效果286中记录的对自身造成伤害最高的敌方单位
    template<>
    void targetFunc<67>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        if (presearch(ctx, outlist))
            return;
        auto t = luahelper_innereffect_288_get_first_mosthpdelta_attacker(AIContext::getInstance()->getLuaState(), ctx->source);
        if (t)
            outlist->push_back(t);
        else if (ctx->source->target)
            outlist->push_back(ctx->source->target);
    }

    /// 68 - 自己的召唤物中所有类型为3的召唤物
    template<>
    void targetFunc<68>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        CharacterDataListPtr summons = ctx->source->getSummonList();
        if (!summons)
            return;
        for (CharacterData *c : *summons) {
            if (!HeroActionController::isDeadAction(c->currentAction) && c->isSummon() && c->getFakeSummonType() == 3)
                outlist->push_back(c);
        }
    }

    /// 69 - 指定品质的角色
    template<>
    void targetFunc<69>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        if (!ctx->search_params || ctx->search_params->size() < 2)
            return;
        ctx->ft_ispresearch = true;
        // 参数第一列:阵营 0 - 原始全体 1 - 原始我方 2 - 原始敌方 3-相对全体 4-相对我方 5-相对敌方
        switch ((*ctx->search_params)[0]) {
        case 0:
            ctx->ft_relationship = FR_ALL_ORIGIN;
            break;
        case 1:
            ctx->ft_relationship = FR_FRIENDS_ORIGIN;
            break;
        case 2:
            ctx->ft_relationship = FR_RIVALS_ORIGIN;
            break;
        case 3:
            ctx->ft_relationship = FR_ALL;
            break;
        case 4:
            ctx->ft_relationship = FR_FRIENDS;
            break;
        case 5:
            ctx->ft_relationship = FR_RIVALS;
            break;
        }

        ctx->ft_func = FF_SCORETYPE;
        targetCommon(ctx, outlist);
    }

    /// 70 - 根据多个通用条件逐层筛选目标
    template<>
    void targetFunc<70>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        if (!ctx->search_params)
            return;
        ctx->ft_relationship = parseRelationParam(ctx->search_params->at(0));
        targetSequence(ctx, outlist);
    }

    
    /// 99 - 拥有所有指定标签的友军
    template<>
    void targetFunc<99>(AITargetContext *ctx, std::list<CharacterData*> *outlist)
//    static void target99new(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        ctx->ft_ispresearch = true;
        ctx->ft_relationship = FR_FRIENDS;
        ctx->ft_func = FF_HAS_LABEL;
        targetCommon(ctx, outlist);
    }
    
    template<int N>
    void preInitTargets()
    {
        s_targethelpers[N] = targetFunc<N>;
        preInitTargets<N - 1>();
    }
    
    template<>
    void preInitTargets<0>()
    {
        s_targethelpers[0] = targetFunc<0>;
    }
    
    bool AITargetHelper::isUseNewHelper()
    {
#ifdef SERVER_VERIFYING
        return true;
#else
        return AIScriptBinder::getInstance()->isUseNewTargetHelper(AIContext::getInstance()->getLuaState());
#endif
    }

    void AITargetHelper::init()
    {
        preInitTargets<70>();
        s_targethelpers[99] = targetFunc<99>;
    }

    int AITargetHelper::parseTargetType(int atype, bool *out_isignorecanbeattackeffect, bool *out_issearchagainifexcludesummon, bool *out_isexcludesummon, bool * out_isonlysummon)
    {
        int ret = atype;

        // +10000:无视不能成为目标的效果
        if (out_isignorecanbeattackeffect)
            *out_isignorecanbeattackeffect = (ret >= 10000);
        if (ret >= 10000)
            ret -= 10000;

        // +3000:仅从召唤物中找
        if (out_isonlysummon)
            *out_isonlysummon = (ret >= 3000);
        if (ret >= 3000)
            ret -= 3000;

        // +2000:先排除召唤物，如果没有找到，再从召唤物中找
        if (out_issearchagainifexcludesummon)
            *out_issearchagainifexcludesummon = (ret >= 2000);
        if (ret >= 2000)
            ret -= 1000;

        // +1000:目标类型为不含召唤物的原目标类型
        if (out_isexcludesummon)
            *out_isexcludesummon = (ret >= 1000);
        if (ret >= 1000)
            ret -= 1000;

        return ret;
    }

    void AITargetHelper::searchTarget(AITargetContext *ctx, std::list<CharacterData*> *outlist)
    {
        if (AIScriptBinder::getInstance()->isSearchTargetByLua(AIContext::getInstance()->getLuaState(), ctx->search_type)) {
            luahelper_searchtargetwith(AIContext::getInstance()->getLuaState(), ctx, outlist);
            return;
        }
        
        int therealsearchtype = parseTargetType(ctx->search_type, &ctx->ft_is_ignorecanbeattackeffect, &ctx->ft_is_searchagainifexcludesummon, &ctx->ft_is_excludesummon, &ctx->ft_is_onlysummon);
        auto iter_helper = s_targethelpers.find(therealsearchtype);
        if (iter_helper == s_targethelpers.end()) {
            // target type not found!
            CCLOG("ai:target type not found:%d(%d)", therealsearchtype, ctx->search_type);
#ifdef SERVER_VERIFYING
            assert(0);
#endif
            return;
        }
        auto &the_helper = iter_helper->second;
        the_helper(ctx, outlist);

        if (ctx->ft_is_excludesummon && outlist->size() < 1 && ctx->ft_is_searchagainifexcludesummon) {
            ctx->ft_is_excludesummon = false;
            ctx->ft_is_searchagainifexcludesummon = false;
            the_helper(ctx, outlist);
        }
    }

}
