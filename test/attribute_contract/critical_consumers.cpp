// Diagnostic only: actual owned actors, unchanged frozen HurtExec, natural RNG.
#include "verify/fight.h"
#include "verify/verifyhelper.h"
#include "AI/AIDataManager.h"
#include "AI/AIContext.h"
#include "AI/AICharacterData.h"
#include "AI/HeroHurtcalculate.h"
#include "AI/BattleAttributeSchema.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <set>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
int Dir_index=1;
using namespace OnePiece;
static void check(bool c,const char*m){if(!c)throw std::runtime_error(m);}
static void crashTrace(int s){void*frames[50];int n=backtrace(frames,50);backtrace_symbols_fd(frames,n,2);_exit(128+s);}
int main(int argc,char**argv){try{
 signal(SIGSEGV,crashTrace);
 check(argc==4,"configuration input output required");
 AIDataManager::setConfigDataPath(std::string(argv[1])+"/");
 std::ifstream file(argv[2]);std::ostringstream text;text<<file.rdbuf();
 Json::Value input;Json::Reader reader;check(reader.parse(text.str(),input),"actual input parse failed");
 std::cerr<<"phase baseline begin"<<std::endl;auto baseline=fight(text.str());check(baseline.first==1&&baseline.second,"actual baseline failed");std::cerr<<"phase baseline complete"<<std::endl;
 auto* ctx=AIContext::getInstance();auto* rng=ctx->localRandom;check(rng,"actual RNG missing");
 Json::Value out;out["status"]="running";out["baselineSeed"]=input["seed"];
 out["cases"]=Json::Value(Json::arrayValue);
 // The diagnostic actor keeps the archived accuracy/crit/resistance inputs.
 // Only HP capacity and base crit multiplier/reduction are set as explicit fixtures.
 // Every tested public tail is parsed before the real CharacterData constructor.
 for(int type=0;type<2;++type)for(int mode=0;mode<3;++mode){
  std::cerr<<"phase case "<<type<<","<<mode<<" clear"<<std::endl;ctx->clear();check(ctx->localRandom==rng,"context clear replaced natural RNG");
  Json::Value s=input["teams"][0u][1u],t=input["teams"][1u][0u];
  while(s["attr"].size()<76)s["attr"].append(0);while(t["attr"].size()<76)t["attr"].append(0);
  s["attrSchema"]="target-js76-v1";t["attrSchema"]="target-js76-v1";
  for(int i=60;i<76;++i){s["attr"][i]=0;t["attr"][i]=0;}
  s["attr"][18]=300;s["attr"][19]=300;t["attr"][36]=50;t["attr"][37]=50;
  s["attr"][1]=1000000;s["attr"][11]=1000000;t["attr"][1]=1000000;t["attr"][11]=1000000;
  int pubAdd=type==0?72:74,pubCut=type==0?73:75,runtimeAdd=type==0?68:70,runtimeCut=type==0?69:71;
  if(mode==1)s["attr"][pubAdd]=10000;if(mode==2)t["attr"][pubCut]=10000;
  std::list<std::pair<int,int>> emptySkills;
  auto sid=ctx->addHeroCharacters(createBattleAttr(s),emptySkills,0,1,1,0,1,TeamSide::kOur,0);
  auto tid=ctx->addHeroCharacters(createBattleAttr(t),emptySkills,0,1,0,0,1,TeamSide::kEnemy,0);
  auto* source=ctx->getCharacterByID(sid);auto* target=ctx->getCharacterByID(tid);check(source&&target,"actual actors missing");std::cerr<<"phase actors created"<<std::endl;
  // Match the active-cluster relationships normally installed by toNextCluster.
  // This offline fixture does not start scheduling or cast any opening skill.
  ctx->current_ClusterNumOur=1;ctx->current_ClusterNumEnemy=1;
  source->m_pTmpFriends=ctx->getClusterOrigin(TeamSide::kOur,1);source->m_pTmpRivals=ctx->getClusterOrigin(TeamSide::kEnemy,1);
  target->m_pTmpFriends=ctx->getClusterOrigin(TeamSide::kEnemy,1);target->m_pTmpRivals=ctx->getClusterOrigin(TeamSide::kOur,1);
  check(source->getCurrentRivalsList()&&target->getCurrentRivalsList(),"active relationships missing");
  check(source->attributes[runtimeAdd]==(mode==1?10000:0),"source public mapping failed");
  check(target->attributes[runtimeCut]==(mode==2?10000:0),"target public mapping failed");
  int hurtId=type==0?21754:20748;auto* hurt=AIDataHelper::getHurtExec(hurtId);check(hurt&&hurt->type==type,"unchanged frozen hurt missing");
  check(hurt->accuInterval==0&&hurt->hitSpecialEffect==0&&hurt->hitAudio==0&&hurt->result==0,"selected hurt is not minimal");
  for(auto id:hurt->hitExecs)check(id<=0,"selected hurt carries effect");
  check(source->m_aiEffectMap.empty()&&target->m_aiEffectMap.empty(),"fixture unexpectedly has effects");
  Json::Value row;row["type"]=type;row["mode"]=mode;row["hurtId"]=hurtId;row["hurtName"]=hurt->name;
  row["publicAddIndex"]=pubAdd;row["publicCutIndex"]=pubCut;row["runtimeAddIndex"]=runtimeAdd;row["runtimeCutIndex"]=runtimeCut;
  row["sourcePublic"]=s["attr"];row["targetPublic"]=t["attr"];
  row["missProbability"]=HeroHurtcalculate::calcMissProb(*ctx,source,target,type);
  row["escapeProbability"]=HeroHurtcalculate::calcEscProb(*ctx,source,target,type);
  row["criticalProbability"]=HeroHurtcalculate::calcCHProb(*ctx,source,target,type);
  float multiplier=source->getRealAttriByAttriType(type==0?18:19)/100.f-target->getRealAttriByAttriType(type==0?36:37)/100.f;
  if(multiplier<1.f)multiplier=1.f;row["derivedCriticalMultiplier"]=multiplier;
  int ap=type==0?source->getBAPower():source->getMAPower();
  int initial=ap*(float(hurt->baseAPFactor+hurt->APBonus)/100.f)+(float(hurt->baseExtraHurt+hurt->extraHurtBonus)/100.f);
  int multiplied=initial*multiplier;
  HurtCalcParam p(nullptr,0,1,multiplied,type,HeroHurtcalculate::HurtFuncGetRealtimeHurtProperty(*source,hurt->property),CRITICAL_HIT,hurt,false,hurtId);
  p.isSimulateCalc=true;HitResult simulated;HeroHurtcalculate::HurtFuncHurtTargetNative(*ctx,source,target,p,&simulated);
  row["initialHurt"]=initial;row["preMitigationCriticalHurt"]=multiplied;row["publicSimulationExpectedCriticalDamage"]=p.HPDelta;
  int critCount=0,normalCount=0,missCount=0;std::set<int>critDamage,normalDamage;
  row["naturalSamples"]=Json::Value(Json::arrayValue);
  std::cerr<<"phase sampling"<<std::endl;for(int n=0;n<1024&&(critCount<4||normalCount<4);++n){
   source->updateHP(source->getHPTop());target->updateHP(target->getHPTop());
   int before=target->getHP();int hit=HeroHurtcalculate::ExertHurtNative(*ctx,source,nullptr,hurtId,0,1,target,0);int damage=before-target->getHP();
   Json::Value sample;sample["n"]=n;sample["hitType"]=hit;sample["damage"]=damage;row["naturalSamples"].append(sample);
   if(hit==CRITICAL_HIT){++critCount;critDamage.insert(damage);check(damage==p.HPDelta,"public Exert critical damage differs from expected native pipeline");}
   else if(hit==NORMAL_HIT){++normalCount;normalDamage.insert(damage);}else ++missCount;
   check(source->m_aiEffectMap.empty()&&target->m_aiEffectMap.empty(),"sample created hidden effects");
  }
  row["criticalSamples"]=critCount;row["normalSamples"]=normalCount;row["missOrEscapeSamples"]=missCount;
  row["criticalDamages"]=Json::Value(Json::arrayValue);for(auto x:critDamage)row["criticalDamages"].append(x);
  row["normalDamages"]=Json::Value(Json::arrayValue);for(auto x:normalDamage)row["normalDamages"].append(x);
  check(critCount>=4&&normalCount>=4&&critDamage.size()==1&&normalDamage.size()==1,"natural samples incomplete or nonstationary");
  out["cases"].append(row);
 }
 for(int type=0;type<2;++type){auto b=out["cases"][type*3];auto a=out["cases"][type*3+1];auto c=out["cases"][type*3+2];
  check(a["criticalDamages"][0u].asInt()>b["criticalDamages"][0u].asInt(),"add tail did not change public critical damage");
  check(c["criticalDamages"][0u].asInt()<b["criticalDamages"][0u].asInt(),"cut tail did not change public critical damage");
  check(a["normalDamages"]==b["normalDamages"]&&c["normalDamages"]==b["normalDamages"],"critical tail changed normal damage");
 }
 out["status"]="passed_public_ExertHurtNative_natural_critical_SWEET_RUN_consumption";
 out["rngForced"]=false;out["rngResetAfterBaseline"]=false;out["configChanged"]=false;out["privateAccessChanged"]=false;
 std::ofstream output(argv[3]);output<<Json::FastWriter().write(out);return 0;
 }catch(const std::exception&e){std::cerr<<"critical public probe failed: "<<e.what()<<std::endl;return 2;}}
