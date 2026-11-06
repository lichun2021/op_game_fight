//
//  AIActionController.h
//  OnePiece
//
//  Created by Beileng Shan on 3/11/14.
//
//

#ifndef __OnePiece__AIActionController__
#define __OnePiece__AIActionController__

#include <list>
#include <memory>
#include <vector>

namespace OnePiece {

	class CharacterData;
	class AIContext;
	class AIProjectile;
	struct HurtExec;
    struct AISkill;
    typedef std::list<CharacterData*> CharacterDataList;

    class ExecExcuteInfo {
    public:
        ExecExcuteInfo(
        int theExecID, 
        int theSkillID,
        int theSkillLevel, 
        CharacterData* theSource, 
        AIProjectile* theProjectile, 
        CharacterData* theTarget, 
        int theParentTicklen, 
        bool theUseLogicPos = false,
        int theWaitTickLen = 0);

        int waitTickLen = 0;
        int execID = 0;
        int skillID = 0;
        int skillLv = 0;
        CharacterData *pSource = nullptr;
        CharacterData *pTarget = nullptr;
        AIProjectile *pProjectile = nullptr;
        int parentTickLen = 0;
        bool useLogicPos = false;
    };

	class AIActionController
	{
	public:
		virtual void update(CharacterData& aiCharacter, AIContext& aiContext) = 0;
		~AIActionController();
		
	public:
		AIActionController();
// 		AIActionController(const AIActionController&);
// 		AIActionController(AIActionController&);
// 		AIActionController& operator=(AIActionController&);


		//执行总入口
		// isproj12reachexec:飞行道具12到达指定坐标后触发的执行
        static void executeExec(AIContext& aiContext, int execID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos = false, int markID = 0, bool isproj12reachexec = false);

        //新增延时执行，主要针对特效 声音 文字 等表现入口 cui 2020_2_23  op3
        static void executeDelayExec(AIContext& aiContext, int execID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos = false, int markID = 0, bool isproj12reachexec = false,int delayTick =0);

		//音效暂时走UI
		static void executeAudio(AIContext& aiContext, int audioID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen);
		//特效暂时走UI
		static void executeSpecialEffect(AIContext& aiContext, int parentExecID, int seffID, int skillLevel, int skillID, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos = false);
        static void executeSpecialEffect(AIContext& aiContext, int parentExecID, int seffID, int skillLevel, int skillID, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos, int altLinkEffLinkTick);
		//特殊执行暂时走UI
		static void executeSpecialExec(AIContext& aiContext, int seffID, int skillID, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen);
		//动画暂时走UI
		static void executeAnimation(AIContext& aiContext, int animationID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen);

		//伤害执行
		static void executeHurt(AIContext& aiContext, int skillID, int hurtID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, int markID, bool isproj12reachexec);
		//飞行道具执行
		static void executeProjectile(AIContext& aiContext, int projectileID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen);
		//效果执行
		static void executeEffect(AIContext& aiContext, int effectID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, int markID, bool isproj12reachexec);
		//移动执行
		static void executeMovement(AIContext& aiContext, int movementID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen);
        //标记执行
        static void executeMark(AIContext& aiContext, int markID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool isproj12reachexec);
        //幻象执行
        static void executeSummon(AIContext& aiContext, int markID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen);
        //身体执行
        static void executeBodyEffect(AIContext& aiContext, int parentExecID, int bodyEffectID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen);
        //变身执行
        static void executeTransformExec(AIContext& aiContext, int execID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos);

        //typedef std::unique_ptr<std::list<CharacterData*> > CharacterDataListUniPtr;

        static bool isTargetTypeRandom(int searchType);
		static std::unique_ptr<std::list<CharacterData*> > searchTarget(CharacterData& source, CharacterData* target, AIProjectile* projectile, AIContext& aiContext, int searchType, int x1, int x2, int y1, int y2, int z1, int z2, std::vector<int> *searchParams, CharacterData& skillCaster, bool isUsingSkill = false, int theSkillId = 0, int hurtID = 0, int skillLevel = 0, CharacterDataList *filterList = nullptr, CharacterData *excludeCharacter = nullptr);
        static std::unique_ptr<std::list<CharacterData*> > searchTarget(CharacterData& source, CharacterData* target, AIProjectile* projectile, AIContext& aiContext, int searchType, int x1, int x2, int y1, int y2, int z1, int z2, std::vector<int> *searchParams, CharacterData& skillCaster, bool isUsingSkill, int theSkillId, int hurtID, int skillLevel, CharacterDataList *filterList, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter);
        static std::unique_ptr<std::list<CharacterData*> > searchTargetNative(CharacterData& source, CharacterData* target, AIProjectile* projectile, AIContext& aiContext, int searchType, int x1, int x2, int y1, int y2, int z1, int z2, std::vector<int> *searchParams, CharacterData& skillCaster, bool isUsingSkill, int theSkillId, int hurtID, int skillLevel, CharacterDataList *filterList, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter);
        static CharacterData* getLowestHpCharacter(CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect);
        static CharacterData* getHighestHpCharacter(CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect);
        static CharacterData* getFarawayCharacter(CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect);
        static CharacterData* getRandomCharacter(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect);
        static CharacterData* getRandomCharacter(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect, bool excludeSummon);
        static CharacterData* getAllRandomCharacter(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *list1, std::list<CharacterData*> *list2, bool ignoreCanbeattackEffect);
        static void addCharactInRange(CharacterData& source, std::list<CharacterData*> *characterList, CharacterDataList *outPutList, CharacterData* target, AIProjectile* projectile, short x1, short x2, short y1, short y2, short z1, short z2, bool ignoreCanbeattackEffect);
        static void getRandomCharacterInRange(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *characterList, CharacterDataList *outPutList, CharacterData* target, AIProjectile* projectile, short x1, short x2, short y1, short y2, short z1, short z2, bool ignoreCanbeattackEffect);
        static CharacterData* getRandomCharacterAboveHPLowerLimit(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *list, float lowerHPRatioLimit, bool ignoreCanbeattackEffect);
        static CharacterData* getNearestCharacter(CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect);
        static CharacterData* getRandomCharacterWithoutTarget(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *list, CharacterData* target, bool ignoreCanbeattackEffect);
        static void addSummonCharacterInList(CharacterData& source, std::list<CharacterData*> *characterList, CharacterDataList *outPutList, bool ignoreCanbeattackEffect, int searchType);
        static CharacterData* getHighestCombatPowerCharacter(CharacterData& source, std::list<CharacterData* > *list1, std::list<CharacterData* > *list2, bool ignoreCanbeattackEffect);

		/** clair_wang 某项属性最高的目标 */
        //static void addAllAttrHighestCharacterInList(CharacterData& source, std::list<CharacterData*>* theInFriendList, std::list<CharacterData*>* theInRivalList, int theAttrIndex, CharacterDataListUniPtr &theOutList, bool ignoreCanbeattackEffect);

        //static void addAllAttrHighestCharacterInList(CharacterData& source, std::list<CharacterData*>* theInFriendList, std::list<CharacterData*>* theInRivalList, std::vector<int> &theAttrs, CharacterDataListUniPtr &theOutList, bool ignoreCanbeattackEffect);

		/** 定点范围，不随角色转向而转向 **/
        static void addCharactInAbsoluteRange(CharacterData& source, std::list<CharacterData*> *characterList, CharacterDataList *outPutList, CharacterData* target, AIProjectile* projectile, short x1, short x2, short y1, short y2, short z1, short z2, bool bIncludeOwn, bool ignoreCanbeattackEffect);

		/** clair_wang 获取屏幕绝对坐标范围内的目标 **/
        static void getScreenAbsolutePointTargetLists(std::list<CharacterData*> *characterList, CharacterDataList *outPutList, CharacterData& source, AIContext& aiContext, short x1, short x2, short y1, short y2, short z1, short z2, bool ignoreCanbeattackEffect);

        enum CompareType {
            COMP_HIGHEST,
            COMP_LOWEST
        };
        //static CharacterData *getAttrCompareCharacter(CharacterData& source, std::list<CharacterData*> *list, int attrIndex, CompareType compType, bool ignoreCanbeattackEffect);
        //static CharacterData *getAttrCompareCharacter(CharacterData& source, std::list<CharacterData*> *list1, std::list<CharacterData*> *list2, int attrIndex, CompareType compType, bool ignoreCanbeattackEffect);
        static CharacterData *getAttrEstCharacter(CharacterData& source, std::list<CharacterData*> *list1, std::list<CharacterData*> *list2, std::vector<int> &theAttrs, CompareType compType, bool ignoreCanbeattackEffect);

        static std::unique_ptr<std::list<CharacterData*>> getFilteredFriendList(CharacterData &skillCaster, CharacterData &source, bool excludeSummon, bool onlySummon, bool ignoreCanbeattackEffect, CharacterData *lowerHpLimitCharacter, int realSearchType);
        static std::unique_ptr<std::list<CharacterData*>> getFilteredRivalList(CharacterData &skillCaster, CharacterData &source, bool excludeSummon, bool onlySummon, bool ignoreCanbeattackEffect, CharacterData *lowerHpLimitCharacter, int realSearchType);

        static void invalidateCaster(CharacterData *c, int casterType, int casterID);
        // 将指定执行ID添加到待删除队列
        static void addToRemoveCaster(CharacterData *c, int casterType, int casterID);
        static void removeInvalidCasters(AIContext &aiContext);

    private:
        class SearchParam;

    private:
        static CharacterData* getAllRandomDeadCharacter(AIContext& aiContext, std::list<CharacterData* > *list1, std::list<CharacterData* > *list2);
        static CharacterData* getRandomDeadCharacter(AIContext& aiContext, std::list<CharacterData*> *list);
        static void addAllDeadCharacter(CharacterDataList *characterList, CharacterDataList *outPutList);
        static void addCharacterBySex(CharacterData& source, CharacterDataList *filterList, CharacterDataList *targetList, int sex, bool ignoreCanbeattackEffect);
        static void addMaxHurtTarget(AIContext &aiContext, CharacterData *source, AIProjectile *projectile, CharacterDataList *filterList, CharacterDataList *targetList, int hurtID, int skillLevel, bool ignoreCanbeattackEffect);
        static void addSomeoneCampAndRoleWithIdOld(CharacterDataList *filterList, CharacterData *source, AISkill *skillData, CharacterDataList *targetList, CharacterDataList *friendList, CharacterDataList *rivalList, bool ignoreCanbeattackEffect);
        static void addSomeoneCampAndRoleWithId(CharacterDataList *filterList, CharacterData *source, std::vector<int> &targetParams, CharacterDataList *targetList, CharacterDataList *friendList, CharacterDataList *rivalList, bool ignoreCanbeattackEffect);
        static void addAllCharacterInHPRange(CharacterData& source, CharacterDataList *characterList, CharacterDataList *targetList, int rangeLow, int rangeHigh, bool ignoreCanbeattackEffect);
        static CharacterData *getTargetOneEnemyRandomContinueHurtNotRandomTarget(AIContext &aiContext, CharacterData *source, CharacterDataList *characterList, bool ignoreCanbeattackEffect);
        static void addCharacterTraversal(CharacterData& source, CharacterDataList *characterList, CharacterDataList *targetList, int intervalTick, int totalTick, bool ignoreCanbeattackEffect);
        static void addAllCharacterHasMark(CharacterData &source, CharacterDataList *characterList, CharacterDataList *targetList, int markID, bool ignoreCanbeattackEffect);
        static void addSelfSummonInRange(AIContext& aiContext, CharacterData& source, CharacterDataList *outPutList, short x1, short x2, short y1, short y2, short z1, short z2, bool ignoreCanbeattackEffect);
        static void addCharacterHasLabels(CharacterData& source, CharacterDataList *characterList, CharacterDataList *outPutList, std::vector<int> &labels, bool ignoreCanbeattackEffect);
        static CharacterDataList *pickFilterList(CharacterDataList *filterList, CharacterDataList *anotherList);
        static std::unique_ptr<std::list<CharacterData*>> getFilteredOriginList(std::list<CharacterData*> *theList, CharacterData &skillCaster, CharacterData &source, bool excludeSummon, bool onlySummon, bool ignoreCanbeattackEffect, CharacterData *lowerHpLimitCharacter, int realSearchType);
        static void implSearchTargets(const SearchParam &param);
	};

}

#endif /* defined(__OnePiece__AIActionController__) */
