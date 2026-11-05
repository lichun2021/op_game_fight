####################################################
# Generic makefile - 万能Makefile
# for compiling and linking C++ projects on Linux 
# Author: George Foot  Modified:Jackie Lee
####################################################
### Customising
#
# Adjust the following if necessary; EXECUTABLE is the target
# executable's filename, and LIBS is a list of libraries to link in
# (e.g. alleg, stdcx, iostr, etc). You can override these on make's
# command line of course, if you prefer to do it that way.
#
#
EXECUTABLE := main    # 可执行文件名
LIBDIR:=./lua/libs               # 静态库目录
LIBS := lua cjson socket mime               # 静态库文件名
INCLUDES:=. ./lua/include ./lua/luacjson ./lua/luasocket      # 头文件目录
SRCDIR:=./AI/action_hero ./AI/projectile ./AI/scriptbind ./jsonx ./AI ./verify          # 除了当前目录外，其他的源代码文件目录
#
# # Now alter any implicit rules' variables if you like, e.g.:

CC:=g++
CFLAGS := -g -O3 -std=c++11 
CPPFLAGS := $(CFLAGS)
CPPFLAGS += $(addprefix -I,$(INCLUDES))
CPPFLAGS += -MMD
#
# # The next bit checks to see whether rm is in your djgpp bin
# # directory; if not it uses del instead, but this can cause (harmless)
# # `File not found' error messages. If you are not using DOS at all,
# # set the variable to something which will unquestioningly remove
# # files.
#

RM-F := rm -f -r


# # You shouldn't need to change anything below this point.
#
SRCS := $(wildcard *.cpp) $(wildcard $(addsuffix /*.cpp, $(SRCDIR)))
OBJS := $(patsubst %.cpp,%.o,$(SRCS))
DEPS := $(patsubst %.o,%.d,$(OBJS))
MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.cpp,$(MISSING_DEPS)))


.PHONY : all deps objs clean veryclean rebuild info lua

all: lua executableSystemLib executableUserLocalLib

lua:
#	cd lua/lua-5.1.4 && $(MAKE) generic
#	cd lua/luajit-2.0.5 && $(MAKE)
	cd lua/lua && $(MAKE) a
	cd lua/luacjson && $(MAKE) a
	cd lua/luasocket && $(MAKE) a
	if [ ! -d "lua/libs" ]; then mkdir lua/libs; fi
#	cp lua/lua-5.1.4/src/liblua.a lua/libs
#	cp lua/luajit-2.0.5/src/libluajit.a lua/libs
	cp lua/lua/liblua.a lua/libs
	cp lua/luacjson/libcjson.a lua/libs
	cp lua/luasocket/libsocket.a lua/libs
	cp lua/luasocket/libmime.a lua/libs

executableSystemLib:  $(OBJS)
	$(CC) -o main $(OBJS) $(addprefix -L,$(LIBDIR)) $(addprefix -l,$(LIBS))
executableUserLocalLib:  $(OBJS)
	$(CC) -Wl,-rpath,/usr/local/gcc/lib64 -o main.alt $(OBJS) $(addprefix -L,$(LIBDIR)) $(addprefix -l,$(LIBS))

deps : $(DEPS)

objs : $(OBJS)

clean :
	@$(RM-F) *.o
	@$(RM-F) *.d
	@$(RM-F) ./AI/action_hero/*.o ./AI/projectile/*.o ./AI/scriptbind/*.o ./jsonx/*.o ./AI/*.o ./verify/*.o
	@$(RM-F) ./AI/action_hero/*.d ./AI/projectile/*.d ./AI/scriptbind/*.d ./jsonx/*.d ./AI/*.d ./verify/*.d
	cd lua/lua && $(MAKE) clean
	cd lua/luacjson && $(MAKE) clean
	cd lua/luasocket && $(MAKE) clean
	@$(RM-F) lua/libs
veryclean: clean
	@$(RM-F) $(EXECUTABLE)

rebuild: veryclean all
ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) :
	@$(RM-F) $(patsubst %.d,%.o,$@)
endif
-include $(DEPS)
$(EXECUTABLE) : $(OBJS)
	$(CC) -o $(EXECUTABLE) $(OBJS) $(addprefix -L,$(LIBDIR)) $(addprefix -l,$(LIBS))

info:
	@echo $(SRCS)
	@echo $(OBJS)
	@echo $(DEPS)
	@echo $(MISSING_DEPS)
	@echo $(MISSING_DEPS_SOURCES)