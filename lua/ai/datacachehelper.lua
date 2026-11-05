-- datacachehelper
local cache = {}

local function clear_cache(self)
    self.cache = {}
end

return {
    cache = cache,
    clear_cache = clear_cache
}