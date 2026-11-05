-- projectilehelper

local function projectilecaster_throw_projectile(projectilecaster, shouldturn, shouldthrow)
    projectilecaster:impl_throw_projectile(shouldturn, shouldthrow)
end

return {
    projectilecaster_throw_projectile = projectilecaster_throw_projectile
}