# Original complete service and explicit test tools share one engine source tree.
# Build outputs leave the three preserved original root binaries untouched.
CXX ?= c++
CC ?= cc
BUILD_DIR ?= build
CPPFLAGS += -I. -Ilua/include -Ilua/lua -Ilua/luacjson -Ilua/luasocket -Itest
CXXFLAGS ?= -O2 -g
CXXFLAGS += -std=c++11 -ffp-contract=off
CFLAGS ?= -O2 -g
CFLAGS += -std=gnu99
ENGINE_DIRS := AI/action_hero AI/projectile AI/scriptbind jsonx AI verify
ENGINE_CPP := $(filter-out rankserver.cpp server.cpp rankhandle.cpp rankserialization.cpp testverify.cpp op38_local_main.cpp,$(wildcard *.cpp)) $(foreach d,$(ENGINE_DIRS),$(wildcard $(d)/*.cpp))
LUA_C := $(filter-out lua/lua/lua.c lua/lua/luac.c,$(wildcard lua/lua/*.c))
CJSON_C := $(addprefix lua/luacjson/,lua_cjson.c strbuf.c fpconv.c)
SOCKET_C := $(addprefix lua/luasocket/,auxiliar.c buffer.c except.c inet.c io.c luasocket.c options.c select.c tcp.c timeout.c udp.c usocket.c)
ENGINE_OBJ := $(addprefix $(BUILD_DIR)/obj/,$(ENGINE_CPP:.cpp=.o) $(LUA_C:.c=.o) $(CJSON_C:.c=.o) $(SOCKET_C:.c=.o))
SERVICE_OBJ := $(addprefix $(BUILD_DIR)/obj/,server.o rankserver.o rankhandle.o rankserialization.o)
DEPS := $(ENGINE_OBJ:.o=.d) $(SERVICE_OBJ:.o=.d)

.PHONY: all executableSystemLib executableUserLocalLib guarded-pve-tool engine-probes
# The default keeps the original server/rank entry and every original protocol case.
all: $(BUILD_DIR)/fight_server
executableSystemLib executableUserLocalLib: $(BUILD_DIR)/fight_server

$(BUILD_DIR)/fight_server: $(ENGINE_OBJ) $(SERVICE_OBJ) | $(BUILD_DIR)/lua
	$(CXX) $(CXXFLAGS) $^ -ldl -lm -o $@

# Restricted 34-table ordinary-PVE adapter: a test tool, never the full service.
guarded-pve-tool: $(BUILD_DIR)/guarded_pve_test
$(BUILD_DIR)/guarded_pve_test: $(ENGINE_OBJ) $(BUILD_DIR)/obj/test/guarded_pve_main.o | $(BUILD_DIR)/lua
	$(CXX) $(CXXFLAGS) $^ -ldl -lm -o $@

$(BUILD_DIR)/obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@
$(BUILD_DIR)/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/lua:
	@mkdir -p $(BUILD_DIR)
	@test ! -e $@ || test -L $@
	ln -sfn $(abspath lua) $@

engine-probes:
	python3 test/attribute_contract/build.py --build-dir $(abspath $(BUILD_DIR)/contracts) --cxx $(CXX) --cc $(CC)

-include $(DEPS)
