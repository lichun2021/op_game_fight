#ifndef __AITargetHelper_H__
#define __AITargetHelper_H__

#include <vector>
#include <list>

namespace OnePiece {

    class CharacterData;
    class AIProjectile;
    struct HurtExec;
    struct EffectExec;

    class AITargetRange {
    public:
        int x1 = 0;
        int x2 = 0;
        int y1 = 0;
        int y2 = 0;
    };

    enum FilterRelationshipType {
        FR_FRIENDS = 0,
        FR_RIVALS,
        FR_ALL,
        FR_FRIENDS_ORIGIN,
        FR_RIVALS_ORIGIN,
        FR_ALL_ORIGIN
    };

    enum FilterFunctionType {
        FF_NONE = 0,
        FF_LOWEST_HPRATIO,
        FF_HIGHEST_HPRATIO,
        FF_RANDOM,
        FF_CANBEATTACK,
        FF_INRANGE,
        FF_CASTERSUMMON,
        FF_FARTHEST,
        FF_NEAREST,
        FF_SEXTYPE,
        FF_RANDOM_INRANGE,
        FF_RANDOM_ONLY_FIRSTTIME,
        FF_SUMMON,
        FF_ATTR_HIGHEST,
        FF_ATTR_LOWEST,
        FF_INABSOLUTERANGE,
        FF_CHARACTERID,
        FF_INBATTLEFIELDRANGE,
        FF_MOST_HURT,
        FF_IN_HPRATIO_RANGE,
        FF_TRANVERSAL,
        FF_HAS_MARK,
        FF_HAS_LABEL,
        FF_RANDOM_DEAD,
        FF_DEAD,
        FF_HIGHEST_COMBATPOWER,
        FF_SCORETYPE
    };

    class AITargetContext {
    public:
        CharacterData *source = nullptr;
        CharacterData *target = nullptr;
        CharacterData *skill_caster = nullptr;
        AIProjectile *projectile = nullptr;
        int search_type = 0;
        AITargetRange range;
        std::vector<int> *search_params = nullptr;
        bool is_using_skill = false;
        int skill_id = 0;
        int skill_level = 0;
        int hurt_id = 0;
        bool is_skip_random = false;
        std::list<CharacterData *> *filter_list = nullptr;
        CharacterData *exclude_character = nullptr;
        bool should_hp_ratio_higher_than_exclude_character = false;
        CharacterData *lowerhplimit_character = nullptr;
        
    public:
        void initWithHurtExec(HurtExec *a_hurt_exec, CharacterData *s, CharacterData *t, CharacterData *sc, AIProjectile *p, int a_sk_id, int a_sk_lv);
        void initWithFilterHurtExec(HurtExec *a_hurt_exec, CharacterData *s, CharacterData *t, CharacterData *sc, AIProjectile *p, int a_sk_id, int a_sk_lv, std::list<CharacterData *> *a_filter_list);
        void initWithEffectExec(EffectExec *an_effect_exec, CharacterData *s, CharacterData *t, CharacterData *sc, AIProjectile *p, int a_sk_id, int a_sk_lv);

        /// from here, is set by internal
        int ft_real_search_type = 0;
        bool ft_is_ignorecanbeattackeffect = false;
        bool ft_is_searchagainifexcludesummon = false;
        bool ft_is_excludesummon = false;
        bool ft_is_onlysummon = false;

        bool ft_ispresearch = false;
        int ft_relationship = FR_FRIENDS;
        int ft_func = FF_NONE;
        CharacterData *ft_range_origin = nullptr;
        int ft_sex_type = 0;
        CharacterData *ft_random_exclude_character = nullptr;
        CharacterData *ft_random_first_character = nullptr;
        std::vector<int> *ft_attr_comp_idxs = 0;
        CharacterData *ft_absolute_range_exclude_character = nullptr;
        int ft_mark_id_to_have = 0;
    };

    class AITargetHelper {
    public:
        static bool isUseNewHelper();
        static void init();
        static void searchTarget(AITargetContext *ctx, std::list<CharacterData*> *outlist);
        static int parseTargetType(int atype, bool *out_isignorecanbeattackeffect, bool *out_issearchagainifexcludesummon, bool *out_isexcludesummon, bool* out_isonlysummon);
    };

}

#endif
