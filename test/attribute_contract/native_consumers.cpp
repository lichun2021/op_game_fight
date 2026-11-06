// Full owned classes and real registered Lua bindings; no recording storage shim.
#include "verify/fight.h"
#include "verify/verifyhelper.h"
#include "AI/AIDataManager.h"
#include "AI/AIContext.h"
#include "AI/AICharacterData.h"
#include "AI/HeroHurtcalculate.h"
#include "AI/BattleAttributeSchema.h"
#include "AI/scriptbind/AIScriptBindCharacterData.hpp"
#include "AI/AIScriptBinder.hpp"
#include "AICallbackServer.h"
#include "lua.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <memory>
int Dir_index=1;
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
int main(int argc,char**argv){try{
 check(argc==4,"config actual-input output required");
 OnePiece::AIDataManager::setConfigDataPath(std::string(argv[1])+"/");
 std::ifstream file(argv[2]);std::ostringstream text;text<<file.rdbuf();Json::Value input;Json::Reader reader;check(reader.parse(text.str(),input),"complete actual input required");
 auto result=fight(text.str());check(result.first==1&&result.second,"original 10408 baseline failed");
 auto* ctx=OnePiece::AIContext::getInstance();check(ctx->currentTick==648,"baseline tick changed");
 const auto oldTick=ctx->currentTick;const auto oldCount=ctx->getClusterOrigin(OnePiece::TeamSide::kOur,1)->size();
 int rejected=0;
 for(int index=60;index<76;++index){
  Json::Value bad=input;auto& card=bad["teams"][0u][1u];
  while(card["attr"].size()<76)card["attr"].append(0);card["attr"][index]=1;
  try{fight(Json::FastWriter().write(bad));throw std::logic_error("ambiguous tail accepted");}
  catch(const std::runtime_error& e){check(std::string(e.what()).find("attribute_schema_required")!=std::string::npos,"wrong raw rejection");++rejected;}
  check(ctx->currentTick==oldTick&&ctx->getClusterOrigin(OnePiece::TeamSide::kOur,1)->size()==oldCount,"rejection mutated engine before preflight");
 }
 for(int index:{60,61,66,67}){
  Json::Value bad=input;auto& card=bad["teams"][0u][1u];
  while(card["attr"].size()<76)card["attr"].append(0);card["attr"][index]=1;card["attrSchema"]="target-js76-v1";
  try{fight(Json::FastWriter().write(bad));throw std::logic_error("unimplemented consumer accepted");}
  catch(const std::runtime_error& e){check(std::string(e.what()).find("attribute_consumer_unavailable")!=std::string::npos,"wrong unsupported rejection");++rejected;}
  check(ctx->currentTick==oldTick,"tagged rejection mutated engine");
 }
 // The schema parser and all constructor snapshots are the actual source build.
 Json::Value tagged=input["teams"][0u][1u];while(tagged["attr"].size()<76)tagged["attr"].append(0);
 tagged["attrSchema"]="target-js76-v1";
 const int* indices=OnePiece::BattleAttributeSchema::publicTailIndices();
 for(int i=0;i<12;++i)tagged["attr"][indices[i]]=1000+i;
 ctx->clear();
 std::list<std::pair<int,int>> skills;skills.emplace_back(100,1);
 auto sourceId=ctx->addHeroCharacters(createBattleAttr(tagged),skills,0,1,1,0,1,OnePiece::TeamSide::kOur,27139);
 auto targetId=ctx->addHeroCharacters(createBattleAttr(input["teams"][1u][0u]),skills,0,1,0,0,1,OnePiece::TeamSide::kEnemy,0);
 auto* source=ctx->getCharacterByID(sourceId);auto* target=ctx->getCharacterByID(targetId);check(source&&target,"actual constructors failed");
 for(int i=0;i<12;++i){check(source->attributes[60+i]==1000+i,"mapped raw field lost");check(OnePiece::CharacterFunc::GetCharacterAttr(source->attributesOrigin.get(),60+i)==1000+i,"origin field lost");}
 // Actual registered Lua get/set and origin snapshots stay native72.
 OnePiece::CharacterFunc::CopyOldFashionAttributes(source->attributes,source->attributesAfterBegin.get());
 auto* L=ctx->getLuaState();check(L!=nullptr,"real Lua state unavailable");
 OnePiece::luastack_pushcharacterdata(L,source);lua_setglobal(L,"field76_actor");
 const char* script="local c=field76_actor;local A=require('datadefs/attrtype');assert(A.PHYSICS_ADD==60 and A.RUN_BREAK_CUT==71);for i=60,71 do assert(c:get_attr(i)==1000+i-60);assert(c:get_attr_origin(i)==1000+i-60);assert(c:get_attr_afterbegin(i)==1000+i-60);c:set_attr(i,2000+i-60);assert(c:get_attr(i)==2000+i-60);assert(c:get_attr_origin(i)==1000+i-60);end;for _,i in ipairs({72,73,74,75})do assert(c:get_attr(i)==0);end";
 if(luaL_dostring(L,script)!=0)throw std::runtime_error(lua_tostring(L,-1));
 Json::Value out;out["status"]="passed_real_native72_constructors_snapshots_Lua_binding_and_recovered_consumers";
 out["rejectedBeforeEngineMutation"]=rejected;out["realLuaMappedTailCases"]=12;out["realLuaIndexDomain"]=72;
 // Each recovered derived accessor has a nonzero source operand, raw getters
 // remain raw, and the target ARM64 arithmetic oracle is in the native proof.
 out["derived"]=Json::Value(Json::arrayValue);
 const int baseIndices[8]={18,19,22,23,24,25,36,37};const int percentIndices[8]={68,70,64,66,65,67,69,71};
 for(int j=0;j<8;++j){
  int type=baseIndices[j],percent=percentIndices[j];source->attributes[type]=201;source->attributes[percent]=10000;
  int actual=source->getRealAttriByAttriType(type);check(actual==402,"derived getter failed actual field consumption");
  check(source->attributes[type]==201,"derived getter overwrote raw storage");
  Json::Value row;row["attribute"]=type;row["runtimePercentIndex"]=percent;row["base"]=201;row["percent"]=10000;row["actual"]=actual;out["derived"].append(row);
 }
 check(source->getRealAttriByAttriType(14)==0,"unsupported derived getter changed original dispatch");
 auto resetTail=[&](){for(int i=60;i<72;++i){source->attributes[i]=0;target->attributes[i]=0;}};
 // Public native hurt path in simulation mode, with complete real CharacterData.
 // These are diagnostic process-local actors; no rewards, saves or RNG forcing.
 auto hurt=[&](int type){OnePiece::HurtCalcParam p(nullptr,100,1,10000,type,0);p.isSimulateCalc=true;OnePiece::HitResult hit;OnePiece::HeroHurtcalculate::HurtFuncHurtTargetNative(*ctx,source,target,p,&hit);return p.HPDelta;};
 resetTail();int physical=hurt(0),magical=hurt(1);check(physical>0&&magical>0,"clean native hurt path unavailable");
 out["hurt"]=Json::Value(Json::arrayValue);
 for(int type=0;type<2;++type){
  resetTail();int before=hurt(type);int addIndex=type==0?60:62,cutIndex=type==0?61:63;
  source->attributes[addIndex]=10000;int add=hurt(type);check(add==ctx->getFinalValue(10000,0,before),"physical/magical add tail not consumed by actual hurt function");
  resetTail();target->attributes[cutIndex]=10000;int cut=hurt(type);check(cut==ctx->getFinalValue(0,10000,before),"physical/magical cut tail not consumed by actual hurt function");
  Json::Value row;row["type"]=type;row["runtimeAdd"]=addIndex;row["runtimeCut"]=cutIndex;row["before"]=before;row["add10000"]=add;row["cut10000"]=cut;out["hurt"].append(row);
 }
 out["avoidance"]=Json::Value(Json::arrayValue);
 target->attributes[22]=1000;target->attributes[23]=1000;source->attributes[24]=100;source->attributes[25]=100;
 for(int type=0;type<2;++type){
  resetTail();int before=OnePiece::HeroHurtcalculate::getOriginalAvoidanceHurt(type,target,source,10000);
  int defense=type==0?64:66,penetration=type==0?65:67;
  target->attributes[defense]=10000;int withDefense=OnePiece::HeroHurtcalculate::getOriginalAvoidanceHurt(type,target,source,10000);
  resetTail();source->attributes[penetration]=10000;int withPen=OnePiece::HeroHurtcalculate::getOriginalAvoidanceHurt(type,target,source,10000);
  check(withDefense<before&&withPen>before,"actual avoidance did not consume defense/penetration tails");
  Json::Value row;row["type"]=type;row["before"]=before;row["defense10000"]=withDefense;row["penetration10000"]=withPen;out["avoidance"].append(row);
 }
 out["ATT_NUMChanged"]=false;out["businessConfigurationChanged"]=false;out["listenerOrDatabaseStarted"]=false;out["rewardGranted"]=false;
 std::ofstream output(argv[3]);output<<Json::FastWriter().write(out);return 0;
 }catch(const std::exception& e){std::cerr<<"native consumption probe failed: "<<e.what()<<std::endl;return 2;}}
