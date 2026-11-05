local HurtExec = {
    duration                = function (t) return t.conf[6] end, --持续时间
    freq                    = function (t) return t.conf[7] end, --伤害频率
    type                    = function (t) return t.conf[8] end, --伤害类型
    prop                    = function (t) return t.conf[9] end, --伤害属性
    baseapfactor            = function (t) return t.conf[10] end, --攻击力系数
    apbonus                 = function (t) return t.conf[11] end, --攻击力系数
    baseextrahurt           = function (t) return t.conf[12] end, --额外伤害值
    extrahurtbonus          = function (t) return t.conf[13] end, --额外伤害值
    hitexeccondition        = function (t) return t.conf[14] end, --击中伴随执行条件
    hitexecparam            = function (t) return t.conf[15] end, --击中伴随执行参数
    hitstunp                = function (t) return t.conf[19] end, --击中硬值
    hitspecialeffect        = function (t) return t.conf[20] end, --击中特效
    hitaudio                = function (t) return t.conf[21] end, --击中音效
    deadtype                = function (t) return t.conf[30] end, --死亡效果
    dtrate                  = function (t) return t.conf[31] end, --死亡效果概率
    accuinterval            = function (t) return t.conf[32] end, --蓄力时间间隔
    accuattaddperinterval   = function (t) return t.conf[33] end, --单位蓄力间隔增加的攻击力百分比
    hitdeadexecids          = function (t) return t.conf[40] end, --击杀执行ID列表
    hitlevelup              = function (t) return t.conf[41] end -- 击中伴随执行成长值
}

local mt = {
    __index = function(t, key)
        -- print("__index:"..key)
        assert(HurtExec[key] ~= nil, "hurtdata method("..key..") not found!")
        return HurtExec[key](t)
    end
}

return mt
--[[
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
};
]]