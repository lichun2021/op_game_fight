local EffectExec = {
    --[[
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
    ]]
    target = function (t) return t.conf[1] end,
    period = function (t) return t.conf[6] end,
    ifreact = function (t) return t.conf[11] end -- 被攻击能否改变状态
}

local mt = {
    __index = function(t, key)
        -- print("__index:"..key)
        assert(EffectExec[key] ~= nil, "effectexecdata method("..key..") not found!")
        return EffectExec[key](t)
    end
}

return mt