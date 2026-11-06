//
//  CommonType.h
//  OnePiece
//
//  Created by Jianguo Yang on 14-4-15.
//
//

#ifndef __OnePiece__CommonType__
#define __OnePiece__CommonType__

//#include "cocos2d.h"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "cocos2d.h"


namespace OnePiece
{
	std::string stringWithFormat(const char *pFormat, ...);

	enum AIScriptConfigDataType {
		CONFIGDATA_CHARACTERDATA = 1,
		CONFIGDATA_SKILLDATA = 2,
		CONFIGDATA_EFFECTEXEC = 3,
		CONFIGDATA_HURTEXEC = 4,
		CONFIGDATA_PROJECTILEEXEC = 5,
		CONFIGDATA_BAQISPECIALEXEC = 6,
		CONFIGDATA_SUMMONPROPERTY = 7,
        CONFIGDATA_SUMMONEXEC = 8
	};

    // MARK: 增加新的随机目标类型时，需要在AIActionController::isTargetTypeRandom中增加相应的判断
    // 因为CharacterData::canCastUltraSkill方法（JS绑定接口）会调用查找目标接口,使用了随机数会影响
    // 战斗结果
	enum TargetType	//for seach target to cast skill
	{
		TargetCaster = 0, //目标不变化（与上一层目标一致）
		TargetCurrent = 1, //当前敌对目标
		TargetFriendAll = 2, //本方所有
		TargetFriendHPLowest = 3, //本方血量百分比最低单位
		TargetFriendRandom = 4, //本方随机单位
		TargetEnemyAll = 5, //对方所有
		TargetEnemyHPLowest = 6, //对方血量百分比最低单位
		TargetEnemyRandom = 7, //对方随机单位
		TargetCasterEnemyRange = 8, //自己范围内所有敌对单位
		TargetCasterFriendRange = 9, //自己范围内所有有方单位
		TargetCurrentEnemyRange = 10, //当前目标范围内所有敌对单位
		TargetCurrentFriendRange = 11, //当前目标范围内所有有方单位
		TargetCasterSummon = 12, //自己的所有召唤单位
		TargetAll = 13, //所有人
		TargetFurthest = 14, //距离最远的敌人
        TargetSkillCaster = 15, //技能的释放者
        TargetRandomAll = 16, //所有人中一个随机单位
        TargetFurthestFriend = 17,//最远的友方
        TargetNearestFriend = 18,//最近的友方
        TargetNearestEnemy = 19,//最近的敌方
		TargetFriendMale = 20,//友方的男性
		TargetEnemyMale = 21,//敌方的男性
		TargetRangeRandomFriendOne = 22,//范围内随机的一个友军
		TargetRangeRandomEnemyOne = 23,//范围内随机的一个敌军
        TargetFriendRandomWithoutSearcher = 24, //不含搜索者自己的随机友军(不会随到自己)
        TargetEnemyRandomWithoutSearcher = 25, // 不含搜索者自己的随机敌军(不会随到自己)
		TargetOneEnemyRandomContinueHurtNotRandomTarget = 26, // 随机一个敌人，并且若是持续伤害，每次伤害不再重新随机目标。
        TargetAllFriendSummon = 27,//所有友军召唤物
        TargetAllEnemySummon = 28,//所有敌军召唤物
        TargetAllSummon = 29,//所有召唤物
		TargetAllAttrHighest = 30,// 所有敌/友方，某项属性最高的目标
		TargetFriendAttrHighest = 31,// 友方，某项属性最高的目标
		TargetEnemyAttrHighest = 32,// 敌方，某项属性最高的目标
		TargetFriendFemale = 33,//友方的女性
		TargetEnemyFemale = 34,//敌方的女性
		TargetAllFemale = 35,//敌我双方的女性
		TargetAbsoluteRangeEnemy = 36,// 定点范围内的所有敌军，不随角色转向而转向
		TargetAbsoluteRangeFriend = 37,// 定点范围内的所有友军，不随角色转向而转向
		TargetAbsoluteRangeAll = 38,// 定点范围内的所有敌/友方，不随角色转向而转向
		TargetAbsoluteRangeAllWithoutSearcher = 39,// 定点范围内的所有敌/友方(不包含自己），不随角色转向而转向
		TargetSomeoneCampAndRoleWithId = 40,// 指定阵营的某个或某些角色id（作用对象类型填新类型，作用范围考虑支持数组填阵营和角色id）
		TargetSelfAttackRangeRandomEnemy = 41,// 考虑自身攻击范围的随机敌人，比如近战A随机到远程B，那么A必须走到B附近才能攻击B。
		TargetByAttacker = 42, // 攻击者
		TargetScreenAbsolutePointEnemy = 43, // 绝对坐标内的所有敌军 绝对坐标是指，根据本阵营的出场方向，面朝敌人出场方向为x轴正方向，屏幕上方为y轴正方向，以屏幕正中心为坐标原点。
		TargetScreenAbsolutePointFriend = 44, // 绝对坐标内的所有友军 绝对坐标是指，根据本阵营的出场方向，面朝敌人出场方向为x轴正方向，屏幕上方为y轴正方向，以屏幕正中心为坐标原点。
        TargetAllAttrLowest = 45,// 所有敌/友方，某项属性最低的目标
        TargetFriendAttrLowest = 46,// 友方，某项属性最低的目标
        TargetEnemyAttrLowest = 47,// 敌方，某项属性最低的目标
        TargetEnemyMaxHurt = 48,// 能造成最多伤害的敌方
        TargetAllEnemyInHPRange = 49,// 生命值处于某个百分比区间的所有敌人
        TargetTraversal = 50,// 对所有敌人依次遍历，一次只选一个，时间间隔内只能选一次，选过后找不到目标
		TargetSummonBySummoner = 51, // = 51 幻象的召唤者（一般用于幻象的技能）
        TargetHasMark = 52,// 拥有指定标记的目标
        TargetSelfSummonInRange = 53,// 范围内的施法者召唤单位
        TargetHasLabelEnemy = 54,// 拥有所有指定标签的敌方单位
        TargetHasLabelAll = 55,// 拥有所有指定标签的所有单位
        TargetRandomDeadFriend = 56,// 随机一个已死亡的友军
        TargetRandomDeadEnemy = 57,// 随机一个已死亡的敌军
        TargetRandomDeadAll = 58,// 随机一个已死亡的友军/敌军
        TargetDeadAllFriend = 59,// 所有已死亡的友军
        TargetDeadAllEnemy = 60,// 所有已死亡的敌军
        TargetDeadAll = 61,// 所有已死亡的友军/敌军
        TargetFriendHPHighest = 62,//血量百分比最高的友军
        TargetEnemyHPHighest = 63, //血量百分比最低的敌军
        // 64 - 敌军之中战力最高的目标
        // 65 - 友军之中战力最高的目标
        // 66 - 所有人中战力最高的目标
        // 67 - 效果286中记录的对自身造成伤害最高的敌方单位
        // 68 - 自己的召唤物中所有类型为3的召唤物
		TargetAccordRingofLightFriend = 99,// 查找友方符合（技能目标参数[为标签表中标签ID列表]）的友方
	};

	struct IAitem
	{
		int i;
		int a;
	};
	struct IARitem
	{
		int i;		//id
		int a;	//amount
		int r;	//rate

		bool isInReward = false; //12.0:开场前预先随机掉落结果

		IARitem(int i, int a, int r) :i(i), a(a), r(r) {}
	};
	struct RewardCandidateItem
	{
		std::vector<IARitem*> iarlist;//多选一
		int a;
		int r;
		std::vector<int> icon;
		int q;
		std::string n;
		std::string d;
		int t;

		bool isInReward = false; //12.0:开场前预先随机掉落结果
	};
	
	//pve_guanqia_characters
	struct pve_guanqia_character
	{
		int guqnqiaID;
		int seq;
		std::vector<int> pos;
	};

	enum SkillExecType
	{
		eSkillExecStart = 0,
		eSkillExecWithParentLoop,
		eSkillExecWithParentNoLoop,
		eSkillExecFollow,
		eSkillExecTypeEnd
	};

	enum SkillExecIDType
	{
		eAnimationExec = 0,
		eHurtExec,
		eMoveExec,
		eProjectTileExec,
		eEffectExec,
		eSpecialUIEffectExec,
		eSoundExec,
		eSpecialExec,
        eMarkExec,
        eSummonExec,
        eTransformExec,
        eBodyUIEffectExec,
		eSkillExecIDTypeEnd
	};

	struct SkillExec
	{
		int execId = 0;
		int execType = 0;
		int param0 = 0;
		int param1 = 0;
	};

	inline bool IsAnimationExec(int exeId)
	{
		return ((exeId >= 10000 && exeId < 19999) || (exeId >= 1000000 && exeId < 1999999));
	}

	inline bool IsLogicAnimationExec(int exeId)
	{
		return exeId >= 15000 && exeId <= 19999;
	}

	inline bool IsHurtExec(int exeId)
	{
		return (exeId >= 20000 && exeId <= 29999) || (exeId >= 200000 && exeId <= 299999);
	}

	inline bool IsMoveExec(int exeId)
	{
		return (exeId >= 30000 && exeId <= 39999) || (exeId >= 300000 && exeId <= 399999);
	}

	inline bool IsProjectileExec(int exeId)
	{
		return (exeId >= 40000 && exeId <= 49999) || (exeId >= 400000 && exeId <= 499999);
	}

	inline bool IsEffectExec(int exeId)
	{
		return (exeId >= 50000 && exeId <= 59999) || (exeId >= 500000 && exeId <= 599999);
	}

	inline bool IsSpecialUIEffectExec(int exeId)
	{
		return (exeId >= 60000 && exeId < 69999) || (exeId >= 600000 && exeId <= 699999);
	}

	inline bool IsLogicSpecialUIEffectExec(int exeId)
	{
		return exeId >= 65000 && exeId <= 69999;
	}

	inline bool IsSoundExec(int exeId)
	{
		return (exeId >= 70000 && exeId <= 79999) || (exeId >= 700000 && exeId <= 799999);
	}

	inline bool IsSpecialExec(int exeId)
	{
		return (exeId >= 80000 && exeId <= 89999) || (exeId >= 800000 && exeId <= 899999);
	}

    inline bool IsMarkExec(int exeId)
    {
        return (exeId >= 90000 && exeId <= 99999) || (exeId >= 900000 && exeId <= 999999);
    }
    
    inline bool IsSummonExec(int exeId)
    {
        return exeId >= 110000 && exeId <= 119999;
    }

    inline bool IsTransformExec(int exeId)
    {
        return exeId >= 120000 && exeId <= 129999;
    }

    inline bool IsBodyUIEffectExec(int exeId)
    {
        return exeId >= 130000 && exeId <= 139999;
    }

#ifdef OPTOOLS
	inline bool IsSummonChildExec(int exeId)
	{
		return exeId >= 1100000 && exeId <= 1199990;
	}
#endif

    struct BodyUIEffectExec
    {
        int execId = 0;
        std::string name = "";
        int duration = 0;
        int affectTarget = 0;
        int affectRangeX1 = 0;
        int affectRangeX2 = 0;
        int affectRangeY1 = 0;
        int affectRangeY2 = 0;
        int actionType = 0;
        int param0 = 0;
        int param1 = 0;
        int param2 = 0;
        int param3 = 0;
        int param4 = 0;
        int param5 = 0;
    };

	struct SpecialExec
	{
		int execId;
		std::string name;
		std::string textureName;
		int duration;
		int affectTarget;
		int affectRangeX1;
		int affectRangeX2;
		int affectRangeY1;
		int affectRangeY2;
		int actionType;
		int param0;
		int param1;
		int param2;
		int param3;
		int param4;

		SpecialExec()
		{
			execId = 0;
			actionType = 0;
		}
	};

	//AI静态数据

	struct AnimationExec
	{
		int execId;                    //执行ID
		std::string name;              //名称
		std::string characterName;     //角色名
		std::string aniName;           //角色动作
		int playSpeed;                //播放速度
		int playTime;                 //播放次数
		int duration;                 //持续时间
		int hardFrameTime;            //硬值帧数
		int hardTime;                 //硬值时间

		AnimationExec()
		{
			execId = 0;
			playSpeed = 0;
			playTime = 0;
			duration = 0;
			hardFrameTime = 0;
			hardTime = 0;
		}

		int getFrame(int frameIdx)
		{
			if (frameIdx <= hardFrameTime)
				return frameIdx;
			else if (frameIdx > hardFrameTime && frameIdx <= hardFrameTime + hardTime)
				return hardFrameTime;
			else
				return frameIdx - hardTime;
		}

		int getDuration()
		{
			return duration + hardTime;
		}
	};

	struct LogicFrameData
	{
		std::string aniName;
		char frameIdx;
		char frameDuration;

		LogicFrameData()
		{
			frameIdx = 0;
			frameDuration = 0;
		}
	};

	struct LogicAnimationExec
	{
		int execId;
		std::string aniName;
		std::string charName;
		int playTime;
		int duration;                 //持续时间
		std::vector<LogicFrameData> frameVec;

		LogicAnimationExec()
		{
			execId = 0;
			playTime = 1;
		}

		LogicFrameData* getLogicFrameData(int frameIdx, int &aniIdx); 
		int             getTotalFrameCount();
		int             getDuration() { return duration; }
	};

	struct LogicEffectData
	{
		int effectId;
		int frameIdx;
		int frameDuration;
        int playTime;

		LogicEffectData()
		{
			effectId = 0;
			frameIdx = 0;
			frameDuration = 0;
            playTime = 1;
		}
	};

	struct LogicSpecialUIEffectExec
	{
		int execId;
		std::string name;
		int bFollow;
        int followUnitTurnType = 0; // 12.0 bFollow原始数据为3时，不跟随角色转向而转向
		int duration;
		std::vector<LogicEffectData> frameVec;
        int totalFrameCount = 0;
		
		LogicSpecialUIEffectExec()
		{
			execId = 0;
		}

		LogicEffectData* getLogicFrameData(int frameIdx, int &aniIdx);
		int             getTotalFrameCount();
		int             getDuration() { return duration;  }
	};

	enum SpecialEffectType
	{
		eEffectSequence = 0,
		eEffectParticle = 1,
		eEffectAnimation = 2,
		eEffectSequenceTick = 3,
		eEffectSeqAnimation = 4,
		eEffectCharAnimation = 5,
		eEffectResizedSequence = 6,
		eEffectResizedTickSequence = 7,
		eEffectLink = 8,
        eEffectSpineAnimation = 9,
		eSpecialEffectTypeEnd
	};

	enum EBlendType
	{
		eBlendNormal = 0,
		eBlendAdditive = 1,
		eBlendTypeEnd
	};

	struct SpecialUIEffectData
	{
		std::string name;                   //特效ID
		SpecialEffectType type;                    //特效类型
		std::string spriteName;             //特效贴图名
		std::string plistName;              //贴图所在的Plist文件名
		EBlendType blendType;

		SpecialUIEffectData()
		{
			type = eSpecialEffectTypeEnd;
			blendType = eBlendNormal;
		}
	};

	struct SpecialUIEffectExec
	{
		enum
		{
			Follow_None = 0,// 0 - 不跟随
			Follow_FollowFight, // 1 - 战斗跟随
			Follow_FollowNonFight, // 2 - 非战斗跟随
		};

		enum
		{
			Stationary_None = 0,// 0 - 跟随所在目标(不固定）
			Stationary_Fasten, // 1 - 相对位置固定
			Stationary_AbsoluteFasten,// 2 - 绝对位置固定
		};
		int execId;                    //执行ID
		std::string name;              //名称
		std::string specID;            //特效ID
        std::string spAnimName;        //OP2:特效名(针对spine动画)
		float deltaX;                  //x修正
		float deltaY;                  //y修正
		float deltaZ;                  //z修正
		int  bFollow;                 //是否跟随 0 - 不跟随， 1 - 战斗跟随， 2 - 非战斗跟随
        int followUnitTurnType = 0;    // 12.0 bFollow原始数据为3时，不跟随角色转向而转向  0-跟随 1-不跟随
		int playTime;                  //播放次数
		int duration;                  //持续时间-单位tick
		bool  bSync;                   //是否同步
		unsigned short target;					//0-own,1=skilltarget
		float scale;                    //缩放比例
		unsigned char opacity;                //透明度
		int rotation;                   //每tick的旋转角度
		int r;
		int g;
		int b;
		int duration2;                 //目前用于link effect的生成时间
        int flipX;                     //是否翻转 0 - 跟随所在目标, 1 - 翻转, 2 - 不翻转
		int mStationaryMode;			// 是否固定  0-跟随所在目标   1-固定   2-绝对位置固定。只有该类型会直接读取后面的绝对XYZ坐标而不读取前面的坐标偏移字段；
		float absoluteX;               //x修正
		float absoluteY;               //y修正
		float absoluteZ;               //z修正
        int canSpeedup;                //是否支持加减速 0-不支持  1-支持
        int keepWhenDead;             //目标死后是否保留特效 0-不保留 1-保留
		int unShow = 0;                  // 不显示
		int allUnShow = 0;               // 所有不显示

		std::vector<int> delaySound;// 延时执行播放声音


		SpecialUIEffectExec()
		{
			execId = 0;
			deltaX = 0;
			deltaY = 0;
			deltaZ = 0;
			bFollow = 0;
			playTime = 1;
			duration = 0;
			bSync = 0;
			target = 0;
			scale = 1.0f;
			opacity = 255;
			rotation = 0;
			r = -1;
			g = -1;
			b = -1;
            flipX = 0;
			mStationaryMode = 0;
			absoluteX = 0;
			absoluteY = 0;
			absoluteZ = 0;
            canSpeedup = 0;
            keepWhenDead = 0;
		}
	};

	struct SoundExec
	{
		int execId;                   //执行ID
		std::string name;             //名称
		std::string soundId;          //音效ID
		int duration;                 //持续时间（0代表不限时间，按次数播放）
		bool bSync;                   //是否同步
		int playtime;                 //播放次数
        int type = 0;                 //类型0-wav 1-criware
        int cueId = 0;                //criware的音效索引
	};

	struct SkillBuffExec
	{
		int ID;                  
		int buffid;                //
		int numerical;             //正负值处理
		std::string desc;          //效果描述
		int maintype;                 //主类型
		int subtype;                   //副类型
		int dispelgroup;                 //驱散组
		int immunegroup = 0;                 //免疫组
		int ismark = 0;                //标记类型判断
	};

	struct LabelEffectExec
	{
        enum LabelActionType {
            MOVEUP_OUT = 0, // 向上移动后消失
            SCALE_OUT = 1 // 缩放后消失
        };

		struct LabelColor
		{
			int r;
			int g;
			int b;
			LabelColor()
			{
				Reset();
			}
			void Reset()
			{
				r = 0; g = 0; b = 0;
			}
		};
		struct LabelScale
		{
			float duration;
			float scaleNum;
			LabelScale()
			{
				Reset();
			}
			void Reset()
			{
				duration = 0;
				scaleNum = 0;
			}
		};
		int execId;                    //执行ID
		std::string name;              //名称
		std::string res;
		std::string res2;
		std::string res3;         //对应res2 右边资源 cui 2023_4_17
		int type;
		int pos;
		int onlyOneSameType;
		int backgroundParticle;
		int period;
		int deadType;
		int deltaX;
		int deltaY;
		int OurColor;
		int EnemyColor;
		float upSpeed; // 上升速度 0：缩放后渐隐  >0:上升速度 需要除以10000
		float initScale; // 初使缩放大小 需要除以10000
		LabelScale actionScale; // 动作时缩放大小 需要除以10000

		bool canMerge;    //是否可以合并 cui
		long mergeTick;	  //最小合并帧数差 cui

		std::vector<int> actType;  //飘字运动方式 cui
		
		
		LabelEffectExec()
		{
			//OurColor.Reset();
			//EnemyColor.Reset();
			upSpeed = 0;
			initScale = 0;
			actionScale.Reset();
			canMerge = false;
			mergeTick = 0;
		}
	};

	struct MoveExec
	{
		int execId;
		std::string name;
        int target;
        std::vector<int> range;
		int targetTracing;// 0-普通有目标模式 1-普通无目标模式 2-特殊穿墙模式 3-黑洞模式
		int duration;
		int speedX;
		int accX;
		int durationX;
		int targetDistanceX;
		int speedY;
		int accY;
		int durationY;
		int targetDistanceY;
		int needReturn;
		int returnPause;
        int keepWhenChangeAction; // 角色切换状态时是否保留移动执行
		int bIsUseactionFreq;			// 作用间隔字段是否使用
		int actionFreq;			// 作用间隔，(配合持续时间字段计算出移动次数)	
		std::vector<int> vAfterMoveExecs; // 移动结束后执行ID次序
	//	int movingExecuteID;  // 移动中持续执行ID
		std::vector<int> vMovingExecuteIds;// 移动中持续执行ID列表
		int syncTargetMech = 0; // 是否同步目标继承（0：否 1：是）
		int getRepeatCasterCnt();
	};

	struct HurtExec
	{
		int ID = 0;							//执行ID
		std::string name;				//伤害名称
		int target = 0;						//作用对象
		int x1 = 0;							//作用范围
        int x2 = 0;							//作用范围
        int z1 = 0;							//作用范围
        int z2 = 0;							//作用范围
        std::vector<int> targetParams;      //目标参数
        int filterTarget = -1;				// 筛选目标对象类型	// clair_wang begin
        int filterX1 = 0;					// 筛选目标对象作用范围
        int filterX2 = 0;					// 筛选目标对象作用范围
        int filterZ1 = 0;					// 筛选目标对象作用范围
        int filterZ2 = 0;					// 筛选目标对象作用范围
        std::vector<int> filterParams;      // 筛选目标参数
        int delayFrameTick = 0;					// 延迟执行帧数// clair_wang end
		int duration = 0;					//持续时间
        int freq = 0;						//伤害频率
        int type = 0;						//伤害类型
        int property = 0;					//伤害属性
        int baseAPFactor = 0;				//攻击力系数
        int APBonus = 0;					//攻击力系数
        int baseExtraHurt = 0;				//额外伤害值
        int extraHurtBonus = 0;				//额外伤害值
        int hitExecCondition = 0;			//击中伴随执行条件
        int hitExecParam = 0;				//击中伴随执行参数
        int hitLevelUp = 0;					// 击中伴随执行成长值
		//int hitExec1;					//击中执行1
		//int hitExec2;					//击中执行2
		//int hitExec3;					//击中执行3
        std::vector<int> hitExecs;      //击中执行
		std::vector<int> vHitDeadExecIDs;	// 击杀执行ID列表 // clair_wang
        int hitStunP = 0;					//击中硬值
        int hitSpecialEffect = 0;			//击中特效
        int hitAudio = 0;					//击中音效
        int stunP = 0;						//受创硬值
        int result = 0;						//受创结果
        int resultParam1 = 0;				//受创结果参数1
        int resultParam2 = 0;				//受创结果参数2
        int resultStack = 0;                //受创结果可叠加
        int attackType = 0;					//释放类型
        int sourceEnergyAdd = 0;			//释放者增加能量
        int targetEnergyAdd = 0;			//目标增加能量,现在更改为按照掉血百分比加成
        int hard = 0;						//攻击硬度
        int deadType = 0;					//死亡效果
        int dtrate = 0;						//死亡效果概率
        int accuInterval = 0;               //蓄力时间间隔
        int accuAttAddPerInterval = 0;      //单位蓄力间隔增加的攻击力百分比
        bool isDeadClear = false;           //施法者死亡后消失
		int delayTick = 0;					//延时执行19的时间 cui 2023_2_24
		int realHurtDisplayId = 0;          //播放飘子的实际ID cui 2023_2_27
        
        /// 12.0
        int targetExclude = -1; // 排除目标
        int excludeX1 = 0;
        int excludeX2 = 0;
        int excludeZ1 = 0;
        int excludeZ2 = 0;
        std::vector<int> targetExcludeParams;
        ///
	};

	//释放类型（0-当前目标，1-无目标释放，2-随机敌方，3-随机己方，4-敌方全体，5-己方全体，6-敌方弹射释放（可相同目标），7-己方弹射释放（可相同目标），8-敌方弹射释放（不可相同目标），9-己方弹射释放（不可相同目标））
	enum EProjectileTargetType
	{
		eProjectileTargetCurrent = 0,       //当前目标
		eProjectileTargetNone = 1,				//无目标释放
		eProjectileTargetRandomEnemy = 2,       //随机敌方
		eProjectileTargetRandomFriend = 3,      //随机己方
		eProjectileTargetAllEnemy = 4,			//敌方全体
		eProjectileTargetAllFriend = 5,			//己方全体
		eProjectileTarget_6 = 6,				//6
		eProjectileTarget_7 = 7,				//7
		eProjectileTarget_8 = 8,				//8
		eProjectileTarget_9 = 9,				//9
		eProjectileTargetDistance = 10,		    //10-固定飞行距离
        eProjectileTargetFollowCaster = 11,      //11-跟随释放者
		eProjectileTargetPosition = 12,			//固定坐标

		eProjectileTargetAllAttrHighest = 30,	//所有敌/友方，某项属性最高的目标
		eProjectileTargetFriendAttrHighest = 31,//友方，某项属性最高的目标
		eProjectileTargetEnemyAttrHighest = 32,//敌方，某项属性最高的目标

		eProjectileTargetAllAttrLowest = 45, //所有敌/友方，某项属性最低的目标
		eProjectileTargetFriendAttrLowest = 46, //友方，某项属性最低的目标
		eProjectileTargetEnemyAttrLowest = 47, //敌方，某项属性最低的目标

        eProjectileTargetOnSearch = 99, //根据配置的搜索目标寻找

		EProjectileTargetTypeEnd
	};

	//伤害触发（0-碰到即触发，1-碰到友方触发，2-遇到敌方触发，3-只有遇到目标才触发，4-碰到最近友方触发，5-碰到最近敌方触发）
	enum EProjectileHurtTriggerType
	{
		eProjectileHurtTrigger_HitAny = 0,		//碰到即触发
		eProjectileHurtTrigger_HitFriend,		//碰到友方触发
		eProjectileHurtTrigger_HitEnemy,		//遇到敌方触发
		eProjectileHurtTrigger_HitTarget,		//只有遇到目标才触发
        eProjectileHurtTrigger_HitFriendNearest,
        eProjectileHurtTrigger_HitEnemyNearest,
		eProjectileHurtTriggerEnd
	};
	
	enum EProjectileDeadType
	{
		eProjectileTimeDead     = 0,         //时间死亡
		eProjectileDistanceDead = 1,         //距离死亡
		eProjectileDropDead     = 2,         //落地死亡
		eProjectileCollideDead  = 3,         //碰撞死亡
		eProjectilePositionDead = 4,         //到达坐标死亡
		eProjectileDeadTypeEnd
	};

	enum EProjectileOrbitType
	{
		eProjectileOrbitLinear = 0,       //直线
		eProjectileOrbitLowParabola,      //低抛物线
		eProjectileOrbitMediumParabola,   //中抛物线
		eProjectileOrbitHighParabola,     //高抛物线
		eProjectileOrbitTypeEnd  
	};

	struct ProjectileExec
	{
		int ID = 0;				            //执行ID
		std::string name;               //名称
		std::vector<std::string> resVec;//飞行道具资源
		std::vector<int> hitVec;        //特殊击中执行参数
		int throwDuration = 0;              //持续时间
        EProjectileTargetType throwType = eProjectileTargetCurrent;//释放类型
		std::vector<int> throwParams; //释放类型的条件参数
        EProjectileHurtTriggerType hurtTrigger = eProjectileHurtTrigger_HitAny;	//伤害触发
		int throwNum = 0;					//释放个数
		int throwInterval = 0;				//释放间隔	
        EProjectileOrbitType orbitType = eProjectileOrbitLinear; //飞行道具轨迹
		int projectileType = 0;             //飞行道具种类
        int throwDirectionType = 0;         //出手方向
		int x = 0;							//出手位置x
		int y = 0;							//出手位置y
		int z = 0;							//出手位置z
		int dx = 0;							//x速度
		int dy = 0;							//y速度
		//int dz;
		int followingExec1 = 0;				//伴随执行1
		int followingExec2 = 0;				//伴随执行2
	//	int hitExec = 0;					//击中执行
		std::vector<int> hitExecVec;		// 击中执行列表
        EProjectileDeadType deadType = eProjectileTimeDead;	//死亡类型
		int deadParam = 0;					//死亡参数
		float levelup = 0;					// 死亡参数成长值

		//增加五列 效果下限 效果上限
		int limitOnOff = 0; // 上下限开关 0-无 1-启用开关
		int limitValLowerBase = 0; // 数值下限基础值
		int limitValLowerGrowth = 0; // 下限成长
		int limitValUpperBase = 0; // 数值上限基础值
		int limitValUpperGrowth = 0; // 上限成长

		int deadExec = 0;					//死亡触发执行
		int deadSpecialEffect1 = 0;			//死亡特效
		int deadSpecialEffect2 = 0;			//死亡特效
		int deadSpecialEffect3 = 0;			//死亡特效
		int deadSpecialEffect4 = 0;			//死亡特效
		int deadSpecialEffect5 = 0;			//死亡特效
		int deadAudio = 0;					//死亡音效
		int rotation = 0;                   //每tick的旋转角度
		int scale = 0;                      //缩放
		int shadowScale = 0;                //阴影缩放系数
        int opacity = 0;                    //透明度

        int redius = 0;						//半径
        unsigned short target = 0;			//目标判断类型

        int searchTarget = 0;						//作用对象
        int searchX1 = 0;							//作用范围
        int searchX2 = 0;							//作用范围
        int searchZ1 = 0;							//作用范围
        int searchZ2 = 0;							//作用范围
        std::vector<int> searchParams;

        int filterTarget = -1;
        int filterX1 = 0;							//作用范围
        int filterX2 = 0;							//作用范围
        int filterZ1 = 0;							//作用范围
        int filterZ2 = 0;							//作用范围
        std::vector<int> filterParams;
        
        /// 12.0
        int targetExclude = -1; // 排除目标
        int excludeX1 = 0;
        int excludeX2 = 0;
        int excludeZ1 = 0;
        int excludeZ2 = 0;
        std::vector<int> targetExcludeParams;
        ///

        int lazyTick = -1;              //延迟帧数 <0-不延迟也不转向 0-不延迟会转向 >0延迟开始和结束转向

        std::vector<int> bouncingExecs; //弹射碰到目标时触发执行

		// 28.0 add
		int r = -1;
		int g = -1;
		int b = -1;

		//28.7 cui add
		int offsetX = 0;
		int offsetY = 0;

	};

	struct InnerEffectExec
	{
        int idx;
		int ID;				            //执行ID
		int type;
		int label;
		int rate;
		int changeType;
		int targetAttribute;
		int valueBase;
		int changeValue;
		int levelup;
        
        //增加两列 效果下限 效果上限
        int limitOnOff; // 上下限开关
        int limitValLowerBase; // 数值下限基础值
        int limitValLowerGrowth; // 下限成长
        int limitValUpperBase; // 数值上限基础值
        int limitValUpperGrowth; // 上限成长
        //int lowerLimitVal;
        //int upperLimitVal;
        
		int onlySameType;
		int exitClear;
		int interval;
		int canBeAttack;
		//int wulifashuAdd;
        int hurtType;//0真实 1物理 2法术

		std::vector<int> vExecDeadExecIDs;	// 击杀执行ID列表 // clair_wang

        int triggerCD;//每两次触发该效果的最小间隔时间
        std::vector<int> uiSpecialEffects;
        std::vector<int> bodyEffects;
        std::vector<int> params;
        std::vector<int> exParams; // Target exparam per execution; semantic support is separate.
        /// 12.0
        int uiStartEffect = 0; // 同类效果开始特效
        int uiEndEffect = 0; // 同类效果结束特效
        ///
	};
	
	enum EEffectOptionLvUpType
	{
		eOptionLvUpNormal = 0,
		eOptionLvUpBySkillID,
	};

	struct EffectExec
	{
		int ID;				            //效果ID
		std::string name;               //名称
		std::vector<int> range;
		std::vector<InnerEffectExec> execs;
		std::vector<int> delExecs;
		int target;
        std::vector<int> targetParams; // 目标参数
		unsigned short period;
		unsigned short recurrenceGrowTimeValue; // 时间随等级成长的成长值
		unsigned short isBuff;           //0-不是buff，1-恶性buff，2-良性buff
		unsigned short deadClear;
		unsigned short stateAnimation; // +1000时不能被免疫
		unsigned short resumeToOrig;
		unsigned short ifReact;
        bool canStateAnimationBeDebuffed;
        int coverBySameType; // 是否可被同类覆盖 0-可 1-不可，且忽略onlySameType 2-可，且忽略onlySameType
        int stateAnimationParam;

		unsigned char red;
		unsigned char green;
		unsigned char blue;

		
		int optionLevelUp;// 等级成长选项  0-根据效果执行所在技能等级（目前  默认） 1-指定的技能ID的等级
		int optionLevelUpSkillID;//// 等级成长选项 -指定的技能ID


        int followSkillTick; // 是否跟随技能时间 0-不跟随 1-跟随

        /// 12.0
        //int targetExclude = -1; // 排除目标
        //int excludeX1 = 0;
        //int excludeX2 = 0;
        //int excludeZ1 = 0;
        //int excludeZ2 = 0;
        //std::vector<int> targetExcludeParams;
        ///

        /// 14.0
        // 同组效果执行覆盖
        int groupID; //0-不覆盖
        int groupCoverRule; //权重相同时覆盖规则 0-不覆盖，先上的生效 1-覆盖，后上的生效
        int groupWeight;
        ///

		/// OP20.1
		int autoClear = 0; // 首次触发是否自动清除效果执行本身 0-否 1-当所有内部效果ID没有触发成功时自动清除
		///

		EffectExec()
		{
			ID = 0;
			name = "";
			execs.clear();
			delExecs.clear();
			target = -1;
			period = 0;
			recurrenceGrowTimeValue = 0;
			isBuff = -1;
			deadClear = -1;
			stateAnimation = -1;
			resumeToOrig = -1;
			ifReact = -1;
            canStateAnimationBeDebuffed = true;
            coverBySameType = 0;
            stateAnimationParam = 0;

			optionLevelUp = EEffectOptionLvUpType::eOptionLvUpNormal;
			optionLevelUpSkillID = 0;

            followSkillTick = 0;
            groupID = 0;
            groupCoverRule = 0;
            groupWeight = 0;
		}
	};

	struct BaqiSpecialExec
	{
		unsigned short talentID;
		unsigned short talentLV;

		unsigned short trigger;
		unsigned short target;
		unsigned short effectExecID;
	};

    // 标记叠加、消耗时是否重置持续时间
    enum EMarkResetDurationType
    {
        eMarkResetDurationNegative = 0, // 不重置
        eMarkResetDurationRefresh  = 1, // 重置
        eMarkResetDurationAddon    = 2, // 累加
    };

    // 标记效果触发种类
    enum EMarkEffectTriggerType
    {
        eMarkEffectTriggerReachLevel    = 0, // 达到层数就触发
        eMarkEffectTriggerAttackBySkill = 1, // 达到层数被某个技能触发
        eMarkEffectTriggerAttackByHurt  = 2, // 达到层数被打某种类型伤害触发
        eMarkEffectTriggerUseSkill      = 3, // 使用某个技能触发
        eMarkEffectTriggerByMark        = 4, // 达到层数被某种标记触发
		eMarkEffectTriggerByReachLevelAndDead   = 5, // 达到层数，有人死亡触发。
		eMarkEffectTriggerBySomeoneMark         = 6, // 对某种标记生效
		eMarkEffectTriggerBySingleHurtNum       = 7, // 被施加此标记的目标受到伤害达到固定值X / 达到其生命值上限的X% 时会触发某效果ID。触发种类参数：[0,500]或[1,6000]  0:固定值 1:百分比  百分比数值需要除100
		eMarkEffectTriggerByFriendHurtType      = 8, // 队友（包括施法者）造成某一属性伤害时触发（伤害不一定打在标记所在目标身上）
		eMarkEffectTriggerByReachLevelMark      = 9, // 被某种达到层数的标记触发 

		eMarkEffectTriggerByMarkedSomeoneHurted = 10, // 被拥有某个标记的角色造成某种伤害触发（伤害类型和种类可以填参数）,指定施法者参数填0时为默认，填1时施法者为拥有某个标记的角色。
        eMarkEffectTriggerByOwnerCastHurt       = 11 // 带有这个标记的角色释放某个属性的伤害时触发(自己打自己的时候无法触发此效果)
    };

    // 标记死亡类型
    enum EMarkEffectDeadType
    {
        eMarkEffectDeadByTime      = 0, // 时间死亡
        eMarkEffectDeadByTriggered = 1, // 触发效果生效后死亡
        eMarkEffectDeadBySkill     = 2, // 被技能消耗掉死亡
        eMarkEffectDeadByHurt      = 3, // 被某类型伤害消耗掉死亡
        eMarkEffectDeadWhenHeroDead = 4, // 持续时间结束前死亡
        eMarkEffectDeadByAllReplaceDeadTime = 5, // 被替换、角色死、参数时间到，都触发死亡执行(0和4合体）
        eMarkEffectDeadByTriggerCnt = 6, // OP2 触发指定次数后死亡
    };

	// 标记触发效果时，施法者选择字段
	enum EMarkTriggerEffectSelectType
	{
		eMarkTriggerEffectByMarkCaster          = 0, // 此标记的施法者
		eMarkTriggerEffectByTrigerMarkCaster    = 1, // 触发此标记的施法者
		eMarkTriggerEffectByCreatedMarkCaster   = 2, // 标记生成时候的施法者

		/// 0~2实际实现逻辑有bug
		eMarkTriggerEffectByMarkCurrentOrigin  = 3, // 标记的当前施法者
		eMarkTriggerEffectByMarkCurrentTarget  = 4, // 标记的拥有者
		eMarkTriggerEffectByMarkCurrentTrigger = 5, // 触发标记时的触发者
		eMarkTriggerEffectByMarkInitOrigin     = 6  // 创建标记时的原始施法者
	};

	enum EMarkTriggerCharDeadClearType
	{
		eMarkTriggerCharDead_NoneClear = 0, // 标记施法者死亡后 不清除
		eMarkTriggerCharDead_Cleared   = 1  // 标记施法者死亡后 清除
	};

    struct MarkExec
    {
		struct MaxLevelUpParam
		{
			int camp = 0; //  阵营  0：无效 1：我方 2：敌方
			int characterId = 0;// 指定角色ID
			int upLevel = 0;// 提升上限的层数
		};

		enum
		{
			// 上限提升条件 0-无条件 1 - 当某个角色在场上时，提升上限
			Enum_NoneCondition = 0,
			Enum_HaveCondition = 1
		};
		
		enum
		{
			Enum_Counteact_Direct = 0,  // 直接抵消
			Enum_Counteact_OneByOne = 1 // 一层层抵消
		};

        int ID = 0;                  // id
        std::string name;        // 标记名称
        std::vector<int> range;  // 作用范围
        int type = 0;          // 标记类型（相同类型的标记，按第一附加的标记计算。）
        int target = 0;              // 作用对象
        int duration = 0;            // 持续时间
        int durationTimeAddUp = 0; // 持续时间随技能等级提升 
        int durationPromoteType = 0; // 持续时间随技能等级提升 是否关联施法者的某个技能 0-无 1-关联 
        int durationPromoteRelateSkillID = 0; // 持续时间随技能等级提升关联技能id
        int initLevel = 0;           // 初始层数
        int maxLevelBase = 0;            // 最大叠加层数 基础值
		int maxLevelUp = 0; // 最大叠加层数随技能等级提升 成长值
		int maxLevelPromoteCond = 0;  // 最大叠加层数上限提升条件 0-无条件 1 - 当某个角色在场上时，提升上限
        MaxLevelUpParam maxLevelUpParam; // 参数：[a,b] ; a -指定角色ID，b -提升上限的层数
		int maxLevelValue = 0;        // 最大叠加层数
        int increaseLevel = 0;       // 叠加时增加层数
        int resetDurationType = 0; // 叠加、消耗时是否重置持续时间 EMarkResetDurationType
		int counteractSwitch = 0;    //  抵消类型开关（0：直接抵消  1：一层层抵消）
        std::vector< int > counteractTypes; // 抵消哪些类型的标记
        std::vector< int > coverTypes;      // 覆盖哪些类型标记
    };

    struct MarkEffectExec
    {
        int type = 0;                           // 父类型
        int level = 0;                          // 叠加层数
        int triggerType = 0;                    // 触发种类 EMarkEffectTriggerType
        int triggerCD = 0;                      // 触发CD from OP2
        std::vector< int > triggers;        // 触发种类参数
    //    int triggerEffectID;                // 触发效果id
		std::vector<int> triggerEffectIDs;  // 触发效果id列表
        int iconID = 0;                         // 标记icon
        int specialEffectID = 0;                // 标记特效
        int soundID = 0;                        // 标记音效
        int deadType = 0;       // 死亡类型 EMarkEffectDeadType
        std::vector< int > deadParams; // 死亡参数（死亡类型3的参数类型）0-物理，1-法术，2-治疗，3-雷系伤害，4-光系伤害，5-火系伤害，6-水系伤害，7-暗系伤害，8-毒系伤害
        int decreaseLevel = 0;                  // 死亡消耗层数
        int deadExecID = 0;                     // 死亡执行
        int deadEffectID = 0;                   // 死亡特效
        int deadSoundID = 0;                    // 死亡音效
        int notAffactCDTick = 0;                // 死亡后多少时间内不会再次受到效果
	//	int ifShowOnTrigger; // 触发时是否显示标记（默认0：否 1：是）
        int triggerEftSelectType = 0; // 触发效果时，施法者选择字段（0-此标记的施法者 1-触发此标记的施法者 2-标记生成时候的施法者）EMarkTriggerEffectSelectType
		int afterTriggerDeadIfCleared = 0;  // 标记施法者死亡后清除 EMarkTriggerCharDeadClearType
		int ifClearHurtAfterMarkDead = 0;  // 标记消失时是否清除伤害执行
    };
    

    
    struct SummonInnerExec
    {
        int type;//幻象类型
        int rate;//幻象触发几率
        int interval;//触发最小间隔
        int summonNum;//幻象数量
        int maxSummonNum;//同类幻象最大存在数量
        int oneSample;//是否替换已有同类幻象
        int summonId;//幻象ID
		int showSummonInterval;//幻象出现延后时间
		//int specialExec[3];//幻象特效执行
		//int specialExecInterval[3];//幻象特效执行延后时间
        //std::vector<std::pair<int, int> > extraExecs; // <execID, interval> 幻象额外特效执行
        std::vector<std::pair<int, int> > execs; //<execID, interval>
    };
    
    struct SummonDeadRun
    {
        /**
        0-无，1-关联id也已经被杀死，2-持续时间结束死亡，3-持续时间未结束被杀死亡，4-任何死亡条件且只执行一次
        */
        int deadRunCondition;
        int deadRunParam;
        int run1;
        int run2;
    };
    
    struct SummonSkillDetail
    {
        int skillId;
        int param0;//解锁条件 解锁条件0-填0，解锁条件1-填角色等级，解锁条件2-填技能id
        int param1;
    };
    
    enum SummonTeamsideType {
        SUMMON_TEAMSIDE_FRIEND_TO_SUMMONER = 0,
        SUMMON_TEAMSIDE_RIVAL_TO_SUMMONER = 1
    };
    
    struct SummonColoration
    {
        int shouldColor; // 幻象是否需要染色 0-不需要 1-需要
        int opacity;
        int r;
        int g;
        int b;

        SummonColoration()
        {
            clear();
        }
        void clear()
        {
            shouldColor = 0;
            opacity = 0;
            r = 0;
            g = 0;
            b = 0;
        }
    };

    struct SummonProperty
    {
        int summonId;
        std::vector<int> associationIdVec;
        int target;//（0-复制作用对象的幻象，1-指定角色id的幻象）
        int targetId;
        int lx;
        int rx;
        int ly;
        int ry;
        std::vector<SummonSkillDetail> summonSkillInfos;

        int beAttackRate;//受到伤害倍率
        int attackRate;//造成伤害倍率
        int treatRate;//治疗倍率
        int durationTick;//0代表一直存在除非被杀死，填帧数
        unsigned short deadType;//0-正常被打死亡，1-被特定技能打死，2-被某类型伤害打死
        std::vector<int> deadParams; // 死亡参数 1为技能数组 （死亡类型2的参数类型）0-物理，1-法术，2-治疗，3-雷系伤害，4-光系伤害，5-火系伤害，6-水系伤害，7-暗系伤害，8-毒系伤害
        std::vector<SummonDeadRun> deadRunVec;
        //int shouldColor; // 幻象是否需要对友军染色 0-不需要 1-需要
        //      int opacity;
        //      int r;
        //      int g;
        //      int b;
        SummonColoration shouldColorFriend;
        SummonColoration shouldColorEnemy;
        int attrInheritBase;//基础继承百分比（填万分比）
        int attrInheritGrow;//百分比成长值（填万分比）
        int deadClear;//本体死亡是否清除幻象
        struct AttrInheritConf
        {
            int attr = 0;
            int inheritBase = 0;
            int inheritGrow = 0;
        };
        std::vector<AttrInheritConf> specialAttrInherit;//特殊属性继承配置

        int posType = 0;
        int teamsideType = 0; //幻象初始化阵营类型 0-与召唤者相同 1-与召唤者敌对
        int affectWinlose = 1; //影响胜负判断 0-不影响 1-影响
    };
    
    
    struct SummonExec
    {
        int ID;
        std::string name;
        int target;              // 作用对象
        std::vector<int> range;
        int fakeSummon = 0; // 是否可被认为是幻象 0-始终认为是幻象 1-全局认为不是幻象 2-仅257效果认为不是幻象
        int groupID = 0;
        int groupWeight = 0;
        int groupMaxNum = 0; //同类幻象组最大存在数量
        std::vector<SummonInnerExec> execs;
        
    };

    struct TransformProperty
    {
        int ID = 0;                         // 变身ID
        int animation = 0;                  // 变身动画文件
        int keepEffects = 0;                // 变身后效果是否保留（效果执行）
        int keepMarks = 0;                  // 变身后标记是否保留（标记执行）
        int keepHP = 0;                     // 变身后血量是否保留
        int duration = 0;                   // 变身持续时间
        std::vector<int> execIDVecWhenTrigger;  // 变身后触发（执行id，多为被动效果，比如把敌人变羊后使其沉默）
        std::vector<int> execIDVecWhenEnd;      // 结束后执行
    };

    struct TransformExec
    {
        int ID;                         // 执行ID
        std::string name;               // 变身名称
        int target;                     // 作用对象
        std::vector<int> range;         // 作用范围
        TransformProperty transform;    // 变身属性
        int transformType;              // 变身类型
        int rate;                       // 变身触发几率
        int interval;                   // 执行间隔
		int newTransform;               // 使用新变身 ：更换id 更换静态数据 更换普通攻击id
    };

	struct ComboTrigger
	{
		int ID;
		std::string file; 
		std::vector<int> team;
		int triggerSkill;
		unsigned short rate;
		unsigned short totalPeriod;
		unsigned short hurtPeriod;
		unsigned short hurtID;
		unsigned short juexing;
		unsigned short levlup;
	};

	struct ComboAnimation
	{
		int startTick;
		int duration;
		std::string targetState;
		int targetDirection; //target direction
		int type; //2 - target; 3 - self
		int roleid;
		int aniExecId;
		int posX;
		int posY;
		int selfDirection; //self direction
	};

	struct ComboAnimationExec
	{
		int execId;
		std::vector<ComboAnimation> comboAniVec;
	};

	struct ComboAudio
	{
		int startTick;
		int targetSpecialEffectId;
		int targetAudioEffectId;
		int duration;
		int frequency;
		int roleid;
	};

	struct ComboAudioExec
	{
		int execId;
		std::vector<ComboAudio> comboAudioVec;
	};

	struct ComboHit
	{
		int startTick;
		int duration;
		int increaseFrequency;
		int posX;
		int posY;
	};
	struct ComboHitExec
	{
		int execId;
		std::vector<ComboHit> comboHitVec;
	};

    enum ESkillCastCondition {
        eSkillCastConditionNoLimit               = 0, // 无限制
        eSkillCastConditionOnFightBegin          = 1, // 战斗开始时
        eSkillCastConditionOnDeath               = 2, // 死亡时释放
        eSkillCastConditionOnSelfHP              = 3, // 自身HP判定
        eSkillCastConditionOnTargetHP            = 4, // 目标HP判定 --没有实现
        eSkillCastConditionOnHit                 = 5, // 被击中时释放
        eSkillCastConditionRandomSwapNormal      = 6, // 触发时有几率变成普攻
        eSkillCastConditionAccumulate            = 7, // 蓄力
        eSkillCastConditionAfterDeath            = 8, // 死亡后触发
        eSkillCastConditionOnMultipleHurt        = 9, // 每受到x次伤害触发
        eSkillCastConditionOnAvoidAndBreakAction = 10, // 闪避 / 抵抗后触发（触发时打断当前施法）
        eSkillCastConditionOnAvoidNoBreakAction  = 11, // 闪避 / 抵抗后触发（触发时不打断当前施法）
		eSkillCastConditionAfterCruelHit         = 12, // 暴击后触发
        eSkillCastConditionOnSelfHPBreakAction   = 13, // 自身HP判定(打断当前施法)
        eSkillCastConditionOnTargetHPBreakAction = 14, // 目标HP判定(打断当前施法) --没有实现
        eSkillCastConditionAccumulatePlus        = 15, // 加强版蓄力(角色死亡或被沉默时也释放)
        eSkillCastConditionOnHurtPlus            = 16, // 每受到x次直接伤害触发
        eSkillCastConditionAnyEnemyHP            = 17, // 敌方任意一名角色HP判定
        eSkillCastConditionAccuRepeatAttack      = 18, // 蓄力攻击次数
        eSkillCastConditionEnergySkill           = 19, // 持续消耗能量型技能
        eSkillCastConditionRepeatSkill           = 20, // 循环释放型技能
        eSkillCastConditionTotalCount
    };

    inline bool IsAccumulateSkill(int castCondition) {
        return eSkillCastConditionAccumulate == castCondition || eSkillCastConditionAccumulatePlus == castCondition || eSkillCastConditionAccuRepeatAttack == castCondition;
    }
    
    inline bool IsEnergySkill(int castCondition) {
        return eSkillCastConditionEnergySkill == castCondition;
    }

    inline bool IsRepeatSkill(int castCondition) {
        return eSkillCastConditionRepeatSkill == castCondition;
    }
    
    inline bool IsFightBeginSkill(int castCondition) {
        return eSkillCastConditionOnFightBegin == castCondition;
    }

	struct AISkill
	{
		enum
		{// clair_wang
			SkillType_ultraSkill = 0, // 大招
			SkillType_littleSkill,// 小招
			SkillType_unactiveSkill,// 被动技能
		}SkillType_Enum;

		std::vector<int> aiSkillSeq;
		int upperBound;	//等级上限
		int startLevel;	//开始学习的等级要求
		int skillType;	//技能类型	0－手动释放 1－主动技能 2－被动技能
		int requireEn;	//需要能量
		int consumeEn;	//消耗能量
		int interruptRate;//受创中断类型 0-受创中断 1-受创不中断
		int freezeScreen;	//条件参数
		int freezeEnemy;	//条件参数 // 是否静止敌方释放大技能
		int freezeTime;	//条件参数  // 暂停时间
        ESkillCastCondition castCondition;//释放条件 0-无限制 1-战斗开始时释放 2-死亡时释放 3-自身HP判定
		//int castParam;	//条件参数
        std::vector<int> castParams; //条件参数
		int targetType;	//技能目标
		std::vector<int> vTargetParms;	// 目标参数列表：标签表中标签ID 
		int range;		//范围
		int icon;			
        int interruptByUltra; //是否能被自己的大招打断 0-不能 1-能
        std::map<int, bool> ignoreEffects; //释放时无视指定效果 <效果ID,true>
        bool canBrokenWhenEnter; //技能准备阶段是否能被打断 0-不能 1-能
        int rangeType; //技能性质 0-不判断 1-单体 2-范围

		int  darkTime;		//黑屏时间 cui 2023_7_27
		int  virtualTime;   //虚化时间
		int skillEftId;     //释放大招时播放的节能id

		std::vector<SkillExec> skillExec;

		AISkill()
		{
			upperBound = 100;
			skillType = SkillType_littleSkill;
			requireEn = 0;
			consumeEn = 0;
            castCondition = ESkillCastCondition::eSkillCastConditionNoLimit;
			//castParam = 0;
			targetType = 0;
			vTargetParms.clear();
			range = 0;
			icon = 0;
            interruptByUltra = 1;
            canBrokenWhenEnter = false;
            rangeType = 0;

			darkTime = 0;
			virtualTime = 0;
			skillEftId = 0;
		}
	};

	//角色数据
	struct CharData 
	{
		enum
		{
			Sex_Male = 0,// 0-男
			Sex_Female, // 1-女
			Sex_TwoInOne,// 2-人妖
		};
		int id;                         //ID
		std::string name;               //名称
		std::vector<int> vLabels;		// 标签列表 // 目前有5个标签 对应标签表中标签ID
		int sex;						//性别 0-男，1-女，2-人妖
		std::string comment;            //评价
		std::string aniName;            //动画文件
		std::string resuseAniName;      //服用骨骼文件
		std::string icon;               //icon
        std::vector<std::pair<std::string, std::string> > alternateAniNameNIcon; //换装动画文件和icon
		std::vector<int> attackSeq;     //攻击次序
        std::vector<int> attackSeq2;    //攻击次序2
		int fragmentID;                 //碎片ID
		int bMirror;                   //是否镜像 0-否，1-镜像，2-只有镜像
		float damping;                 //移动阻尼系数
		int hard;						//硬度

		int hpPosX;                    //调整血条和Boos光圈的X坐标
		int hpPosY;		               //调整血条的高度
		
		int jxj[18];

		struct{
			int sellPrice;                  //出售价格
			int height;                     //高度
			int quality;                    //品质
			int line;                       //排
			int positionPriority;           //位置优先级
		} confg;
		
		/*
		struct {
			int normalAtttack;              //普通攻击
			int ultraSkill;                 //技能1
			int skill2;                     //技能2
			int skill3;                     //技能3
			int skill4;                     //技能4
		};*/
		//int sk[5];
        std::vector<int> sk;
		
		struct {
			int strenght;            //初始力量
			int agile;               //初始敏捷
			int intelligence;        //初始智力
			int precision;           //初始精准
			int sprit;               //初始精神
			int life;                //生命值
			int energy;              //能量值
			int moveSpeed;           //移动速度
			int melee;               //物攻攻击力
			int spell;               //法术攻击力
			int meleeCrit;           //物理暴击
			int spellCrit;           //法术暴击
			int meleeCritTime;       //物理暴击倍数
			int spellCritTime;       //法术暴击倍数
			int meleeHit;            //物理命中
			int spellHit;            //法术命中
			int meleeArmor;          //物理护甲
			int spellAntiBody;       //法术抗体
			int meleePenetrate;      //物理穿透
			int spellPenetrate;      //法术穿透
			int dodgePercentage;     //躲避几率
			int defencePercentage;   //抵抗几率
			int vampireLevel;        //吸血等级
			int lifeRecover;         //生命值恢复
			int energyRecover;       //能量值恢复
			int healEffect;          //治疗效果
			int spellInterval;       //施法间隔
			int attackDistance;      //攻击距离
			int funcHurtAdd;		 //增伤
			int funcHurtSub;		 //减伤
			int meleeCritAdd;		 //额外物理暴击
			int spellCritAdd;		 //额外法术暴击
		} att;
		
//		struct {
//			int zeroJueStrenghtGrowth;             //力量成长
//			int zeroJueAgileGrowth;                //敏捷成长
//			int zeroJueIntelligenceGrowth;         //智力成长
//			int zeroJuePrecisionGrowth;            //精准成长
//			int zeroJueSpiritGrowth;               //精神成长
//			int oneJueStrenghtGrowth;             //力量成长
//			int oneJueAgileGrowth;                //敏捷成长
//			int oneJueIntelligenceGrowth;         //智力成长
//			int oneJuePrecisionGrowth;            //精准成长
//			int oneJueSpiritGrowth;               //精神成长
//			int twoJueStrenghtGrowth;             //力量成长
//			int twoJueAgileGrowth;                //敏捷成长
//			int twoJueIntelligenceGrowth;         //智力成长
//			int twoJuePrecisionGrowth;            //精准成长
//			int twoJueSpiritGrowth;               //精神成长
//			int threeJueStrenghtGrowth;             //力量成长
//			int threeJueAgileGrowth;                //敏捷成长
//			int threeJueIntelligenceGrowth;         //智力成长
//			int threeJuePrecisionGrowth;            //精准成长
//			int threeJueSpiritGrowth;               //精神成长
//			int fourJueStrenghtGrowth;             //力量成长
//			int fourJueAgileGrowth;                //敏捷成长
//			int fourJueIntelligenceGrowth;         //智力成长
//			int fourJuePrecisionGrowth;            //精准成长
//			int fourJueSpiritGrowth;               //精神成长
//			int fiveJueStrenghtGrowth;             //力量成长
//			int fiveJueAgileGrowth;                //敏捷成长
//			int fiveJueIntelligenceGrowth;         //智力成长
//			int fiveJuePrecisionGrowth;            //精准成长
//			int fiveJueSpiritGrowth;               //精神成长
//		} juexing;
        
        int juexing[30];
         //总共19阶 每阶21个属性成长
        int levelup[399];
		
//		struct {
//			int oneLevelLife;             //进阶生命
//			int oneLevelMelee;            //进阶物理攻击
//			int oneLevelSpell;            //进阶法术攻击
//			int oneLevelMellDefense;      //进阶物理防御
//			int oneLevelSpellDefense;     //进阶法术防御
//			int twoLevelLife;             //进阶生命
//			int twoLevelMelee;            //进阶物理攻击
//			int twoLevelSpell;            //进阶法术攻击
//			int twoLevelMellDefense;      //进阶物理防御
//			int twoLevelSpellDefense;     //进阶法术防御
//			int threeLevelLife;             //进阶生命
//			int threeLevelMelee;            //进阶物理攻击
//			int threeLevelSpell;            //进阶法术攻击
//			int threeLevelMellDefense;      //进阶物理防御
//			int threeLevelSpellDefense;     //进阶法术防御
//			int fourLevelLife;             //进阶生命
//			int fourLevelMelee;            //进阶物理攻击
//			int fourLevelSpell;            //进阶法术攻击
//			int fourLevelMellDefense;      //进阶物理防御
//			int fourLevelSpellDefense;     //进阶法术防御
//			int fiveLevelLife;             //进阶生命
//			int fiveLevelMelee;            //进阶物理攻击
//			int fiveLevelSpell;            //进阶法术攻击
//			int fiveLevelMellDefense;      //进阶物理防御
//			int fiveLevelSpellDefense;     //进阶法术防御
//			int sixLevelLife;             //进阶生命
//			int sixLevelMelee;            //进阶物理攻击
//			int sixLevelSpell;            //进阶法术攻击
//			int sixLevelMellDefense;      //进阶物理防御
//			int sixLevelSpellDefense;     //进阶法术防御
//			int sevenLevelLife;             //进阶生命
//			int sevenLevelMelee;            //进阶物理攻击
//			int sevenLevelSpell;            //进阶法术攻击
//			int sevenLevelMellDefense;      //进阶物理防御
//			int sevenLevelSpellDefense;     //进阶法术防御
//			int eightLevelLife;             //进阶生命
//			int eightLevelMelee;            //进阶物理攻击
//			int eightLevelSpell;            //进阶法术攻击
//			int eightLevelMellDefense;      //进阶物理防御
//			int eightLevelSpellDefense;     //进阶法术防御
//			int nineLevelLife;             //进阶生命
//			int nineLevelMelee;            //进阶物理攻击
//			int nineLevelSpell;            //进阶法术攻击
//			int nineLevelMellDefense;      //进阶物理防御
//			int nineLevelSpellDefense;     //进阶法术防御
//		} levelup;

		CharData()
		{
			id = 0;
		}
	};
    
    struct ConvertAttrParams
    {
        int param0;
        int param1;
        float param2;
    };
    
	enum MathLogicOperatorType
	{
		e_OperateT_None = 0,// 
		e_OperateT_LessThan, // 小于 ＜ : 1
		e_OperateT_LessEqual, // 小于等于 ≤ : 2
		e_OperateT_Equal, // 等于 = : 3
		e_OperateT_GreaterEqual, // 大于等于 ≥ : 4
		e_OperateT_GreaterThan, // 大于 ＞ : 5
		e_OperateT_Threshold, // 大于等于且小于等于 [>= && <= ]：6
	};

    enum AiUpdateModeType
    {
        AIUPDATEMODE_CONTINUE_FIGHT_AFTER_WINLOSE = 0,
        AIUPDATEMODE_STOP_FIGHT_AFTER_WINLOSE = 1
    };

    enum AiCenterXModeType
    {
        AICENTERX_SELF_CALCULATE = 0, // ai中自行计算
        AICENTERX_USE_INPUT = 1 // 使用外部设置
    };

    // 大招被打断后的处理
    enum AiUltraBrokenModeType
    {
        AIULTRABROKEN_DO_NOTHING = 0,
        AIULTRABROKEN_STOP_ALL = 1, // 停止暂停时间和屏幕特效
        AIULTRABROKEN_STOP_FREEZE = 2, // 只停止暂停时间
        AIULTRABROKEN_STOP_SCREEN_EFFECT = 3 // 只停止屏幕特效
    };

    enum AiRepeatSkillRepeatTickType
    {
        AIREPEATSKILL_DEFAULT_MAX_TICK = 0, // 使用默认的最大重复执行时间
        AIREPEATSKILL_EXTEND_ON_CRIT = 1, // 使用基础重复执行时间，当施法者暴击时延长
        AIREPEATSKILL_EXTEND_ON_HAS_SKILL = 2 // 使用基础重复执行时间，当施法者拥有某技能时延长
    };

    // 效果执行绑定的ui特效使用的持续时间模式
    enum AiUiEffectModeType
    {
        AIUIEFFECTMODE_DEFAULT = 0, // 10.0前逻辑
        AIUIEFFECTMODE_VERY_LONG = 1 // 使用极大值3600
    };

    class MeleeCombatCharacterInfo
    {
    public:
        //(attList, skilllist, juexing, type, k, ifUltraLock, clusterNum, OnePiece::TeamSide(teamSide), fp);
        std::vector<int> attrs;
        std::vector< std::pair<int, int> > skills;
        std::vector< std::pair<int, int> > bqsls;
        int juexing = 0;
        int type = 1;
        int k = 0;
        int ifUltraLock = 0;
        int clusterNum = 0;
        int teamSide = 0;
        int fp = 0;

        int posX = 0;
        int posY = 0;
    };

    class MeleeCombatBattleInfo
    {
    public:
        std::vector<MeleeCombatCharacterInfo> allcharacters;
        std::unordered_map< int, float > conditions; // <clusternum, t>

        int battleRectX = 0;
        int battleRectY = 0;
        int battleRectWidth = 0;
        int battleRectHeight = 0;

        int battleMinY = 0;
        int battleMaxY = 0;

        int seed = 1;
        int businesstype = 0;

        int maxticklimit = 3600;
    };

    class MeleeCombatCharacterSimpleInfo
    {
    public:
        int character_myid = 0;
        int character_dataid = 0;
        int hp = 0;
        int mp = 0;
        int init_x = 0;
        int init_y = 0;
        int action = 0;
    };

    class MeleeCombatResult
    {
    public:
        int winlose = 0;
        int totaltick = 0;
        std::vector<MeleeCombatCharacterSimpleInfo> ourinfo;
        std::vector<MeleeCombatCharacterSimpleInfo> enemyinfo;
    };

	class AIDataHelper
	{
	public:
#ifdef DENA_OP3
		const static std::string system_config;
		const static std::string skill_list;
		const static std::string skill_animation;
		const static std::string skill_ui_effect;
		const static std::string skill_ui_effect_res;
		const static std::string skill_move;
		const static std::string skill_hurt;
		const static std::string skill_label_effect;
		const static std::string skill_audio;
		const static std::string character_position;
		const static std::string fighting_parameter;
		const static std::string skill_composite_animation;
		const static std::string skill_composite_ui_effect;
		const static std::string skill_projectile;
		const static std::string domineer_site_list_hash;
		const static std::string character_list;
		const static std::string pve_guanqia_characters;
		const static std::string skill_special;
		const static std::string skill_body_effect;
		const static std::string pve_guanqia_hash;
		const static std::string mark_list;
		const static std::string mark_effect;
		const static std::string skill_summon;
		const static std::string skill_summon_attr;
		const static std::string skill_transform;
		const static std::string skill_transform_attr;
		const static std::string skill_effect;
		const static std::string combo_animation;
		const static std::string combo_audio;
		const static std::string combo_hit_num;
		const static std::string combo_trigger;
		const static std::string convert_attr_list;
		const static std::string skill_buff;
#endif

		static int s_aiUpdateTimePerSecond;

		static std::string scriptGetConfigData(int data_type, int data_id);
		
		static std::vector<SkillExec>* getSkillExecVec(int skillId);
		static AnimationExec* getSkillAnimationExec(int execId);

		static SpecialUIEffectExec* getSpecialUIEffectExec(int execId);

		static SkillExecIDType getSkillExecType(int execId);

		static SpecialUIEffectData* getSpecialUIEffectData(const std::string& effName);

		static MoveExec* getMoveExec(int execId);

		static std::string getHurtExecJsonStr(int execId);
		static HurtExec* getHurtExec(int execId);

		static std::string getProjectileExecJsonStr(int execId);
		static ProjectileExec* getProjectileExec(int execId);

		static LabelEffectExec* getLabelEffectExec(int execId);
		
		static AISkill* getSkillData(int skillId);
		static std::string getSkillDataJsonStr(int skid);

		static CharData* getCharacterData(int id);
        static std::string getCharacterDataJsonStr(int cid);

		static std::string getEffectExecJsonStr(int id);
		static EffectExec* getEffectExec(int id);

		static std::string getBaqiSpecialExecJsonStr(int talentID);
		static BaqiSpecialExec* getBaqiSpecialExec(unsigned short talentID, unsigned short talentLV);

        static MarkExec* getMarkExec(int execId);
        static MarkEffectExec* getMarkEffectExec(int markType, int markLevel);
        
        static std::string getSummonExecJsonStr(int execId);
		static SummonExec* getSummonExec(int execId);
		static std::vector<SkillBuffExec*>* getSkillBuffExec(int typeOne,int typeTwo,int typeThree);
		
		static std::string getSummonPropertyJsonStr(int summonId);
        static SummonProperty* getSummonProperty(int summonId);

        static TransformExec* getTransformExec(int execId);

		static SoundExec* getSoundExec(int id);

		static LogicAnimationExec* getLogicAnimationExec(int id);

		static LogicSpecialUIEffectExec* getLogicSpecialUIEffectExec(int id);

		static std::vector<RewardCandidateItem*>* getRewardCandidate(int guanqiaID);

		static pve_guanqia_character* get_pve_guanqia_character(int guanqiaID, int seq);

		static SpecialExec* getSpecialExec(int id);

        static BodyUIEffectExec* getBodyUIEffectExec(int id);

		static int* getEnergyRecover();
		static int* getPositionArray(bool isEven);
		static float* getFightingParameter(int index);
        static int getBattleFieldRangeToCenterX();
        static int getAiUpdateModeAfterWinlose();
        static int getAiCenterXMode();
        static int getAiUltraBrokenMode();
        static int getAiUiEffectMode();
        static int getAiCenterXForCluster(int theClusterCnt);

        static std::unordered_map<int, ComboTrigger*>& checkComboTriggerMap();
		static ComboTrigger* getComboTrigger(int id);

		static ComboAnimationExec* getComboAnimationExec(int id);
		static ComboAudioExec* getComboAudioExec(int id);
		static ComboHitExec* getComboHitExec(int id);
        
        static void getBeHittedAddEnergy(int skillId,int &useSkillAdd,int &beHittedAdd);
        static void loadConverAttrListTable();
        static std::vector<ConvertAttrParams>& getConvertAttrVec();

        /// OP2
        static void parseMeleeCombatBattleLog(const std::string &battle_log_str, MeleeCombatBattleInfo *out_info);
        /// 

		static std::vector<int> getTransformEffectConf(int old);

		static void unload();

	private:
		static int* _character_pos;
		static std::unordered_map<int, MoveExec*> _moveExec_cache;
        static std::unordered_map<int, HurtExec*> _hurtExec_cache;
        static std::unordered_map<int, ProjectileExec*> _projectileExec_cache;
        static std::unordered_map<int, AISkill*> _aiSkill_cache;
        static std::unordered_map<int, CharData*> _characterDataMap;
        static std::unordered_map<int, SpecialUIEffectExec*> _specialEffExecMap;
        static std::unordered_map<int, AnimationExec*> _animationExecMap;
        static std::unordered_map<std::string, SpecialUIEffectData*> _effDataMap;
        static std::unordered_map<int, LabelEffectExec*> _labelEffExecMap;
        static std::unordered_map<int, std::vector<SkillExec>*> _skillExecVecMap;
        static std::unordered_map<int, EffectExec*> _effectExecMap;
        static std::unordered_map<int, MarkExec*> _markExecMap;
        static std::unordered_map<int, MarkEffectExec*> _markEffectExecMap;
        static std::unordered_map<int, std::unordered_map<int, MarkEffectExec*> > _markLevelMap;
        static std::unordered_map<int, SummonExec*> _summonExecMap;
        static std::unordered_map<int, SummonProperty*> _summonPropertyMap;
        static std::unordered_map<int, TransformExec*> _transformExecMap;
        static std::unordered_map<int, BodyUIEffectExec*> _bodyEffectExecMap;
        
        static std::unordered_map<std::string, BaqiSpecialExec*> _baqiSpecialExecMap;
        static std::unordered_map<int, SoundExec*> _soundExecMap;
        static std::unordered_map<int, LogicAnimationExec*> _logicAniMap;
        static std::unordered_map<int, LogicSpecialUIEffectExec*> _logicEffMap;
        static std::unordered_map<int, SpecialExec*> _specialExecMap;
        static std::unordered_map<int, ComboTrigger*> _comboTriggerMap;
        static std::unordered_map<int, ComboAnimationExec*> _comboAniMap;
        static std::unordered_map<int, ComboAudioExec*> _comboAudioMap;
        static std::unordered_map<int, ComboHitExec*> _comboHitMap;
        static std::unordered_map<int, std::pair<int, int>> _beHittedAddEnergyMap;

        static std::unordered_map<int, pve_guanqia_character*> _pve_guanqia_character_Map;
        static std::unordered_map<int, std::vector<RewardCandidateItem*>*> _rewardCandidateMap;
        static std::unordered_map<int, std::vector<RewardCandidateItem*>*> _pve_guanqia_character_reward_Map;
        
        static std::vector<ConvertAttrParams> _convertAttrVec;

		static std::unordered_map<std::string, std::vector<SkillBuffExec*>*>_skillBuffExecMap;

		static int energyRecover[2];
		static int evenPosition[5];
		static int oddPosition[5];
		static float* fightingParameter[10];
        static int battleFieldRangeToCenterX;
        static int aiUpdateModeAfterWinlose;
        static int aiCenterXMode;
        static int aiUltraBrokenMode;
        static int aiUiEffectMode;
        static std::vector<int> *aiCenterXCache;
	};
}

#endif /* defined(__OnePiece__CommonType__) */
