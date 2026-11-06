// Offline entry. Uses the unchanged full fight verifier and returns state without grants.
#include "verify/fight.h"
#include "AI/AIDataManager.h"
#include "AI/AIContext.h"
#include "AI/AICharacterData.h"
#include "jsonx/json.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
int Dir_index=1;
int main(int argc,char**argv) {
 try {
  if(argc!=4 && argc!=5) throw std::runtime_error("config input output [diagnosticSeed] required");
  std::ifstream file(argv[2]); std::ostringstream content; content<<file.rdbuf();
  Json::Value input; Json::Reader reader;
  if(!reader.parse(content.str(),input)) throw std::runtime_error("Invalid complete input");
  OnePiece::AIDataManager::setConfigDataPath(std::string(argv[1])+"/");
  auto result=fight(content.str(),argc==5,argc==5?std::stoi(argv[4]):0);
  if(!result.second) throw std::runtime_error("Original fight verifier returned no result");
  auto* ctx=OnePiece::AIContext::getInstance(); Json::Value out;
  out["status"]="actual_offline_same_source_engine_executed";
  out["gkid"]=input["gkid"]; out["inputSeed"]=input["seed"];
  out["executionSeed"]=argc==5?std::stoi(argv[4]):input["seed"].asInt();
  out["explicitDiagnosticSeedOverride"]=argc==5;
  out["engineWinCode"]=result.first; out["nativeAttributeCount"]=ATT_NUM;
  out["tick"]=int(ctx->currentTick); out["waveCount"]=ctx->currentWaveCount;
  out["characters"]=Json::Value(Json::arrayValue);
  for(unsigned team=0;team<input["teams"].size();++team) {
   auto side=team==0?OnePiece::TeamSide::kOur:OnePiece::TeamSide::kEnemy;
   auto cluster=ctx->getClusterOrigin(side,team==0?1:team); if(!cluster)continue;
   for(auto* c:*cluster) {
    Json::Value row; row["team"]=team; row["id"]=c->getID();row["myID"]=c->myID;
    row["hp"]=c->getHP();row["mp"]=c->getEnergy();
    row["attributes"]=Json::Value(Json::arrayValue);
    for(int i=0;i<ATT_NUM;++i)row["attributes"].append(c->attributes[i]);
    row["skills"]=Json::Value(Json::arrayValue);
    for(auto pair:c->skillList){Json::Value s;s["i"]=pair.first;s["lv"]=pair.second;row["skills"].append(s);}
    out["characters"].append(row);
   }
  }
  out["rewards"]=Json::Value(Json::arrayValue);
  for(auto pair:*result.second){Json::Value row;row["i"]=pair.first;row["a"]=pair.second;out["rewards"].append(row);}
  out["listenerOrDatabaseStarted"]=false;out["rewardGranted"]=false;
  std::ofstream output(argv[3]); if(!output)throw std::runtime_error("Cannot save result");
  output<<Json::FastWriter().write(out);return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 2;}
}
