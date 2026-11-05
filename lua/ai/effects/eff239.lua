-- 239 开始释放技能(释放条件为0且正常释放)时有概率滑倒并触发执行
local get_innereffdata = require "effects/getinnereffdata"

local function check_do(character)
    return aieffectcaster.check_do_239(character)
end

local function get_slipticklen(self)
    --[[
        // params: [a,b,c,d,e]
        // a-触发概率
        // b-是否无视120效果免晕免击倒
        // c-触发间隔
        // d-滑倒时间  -1不滑倒
        // e-执行id  触发时执行(不管有没有滑倒)
    ]]
    local effdata = get_innereffdata(self)
    return effdata.params[4]
end

return {
    check_do = check_do,
    get_slipticklen = get_slipticklen
}