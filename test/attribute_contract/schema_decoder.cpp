#include "verify/verifyhelper.h"
#include "AI/BattleAttributeSchema.h"
#include <iostream>
#include <memory>
#include <stdexcept>
int Dir_index=1;
int main(){
 std::string line;
 while(std::getline(std::cin,line)){
  Json::Value input,out;Json::Reader reader;
  if(!reader.parse(line,input))return 2;
  try{
   std::unique_ptr<int[]> values(createAttr(input["encoded"].toStyledString()));
   out["returned"]=true;out["error"]="";out["runtime72"]=Json::Value(Json::arrayValue);
   std::vector<int> runtime;
   for(int i=0;i<72;++i){out["runtime72"].append(values[i]);runtime.push_back(values[i]);}
   std::vector<int> business;std::string error;
   if(!OnePiece::BattleAttributeSchema::encodePublic(runtime,business,error))throw std::runtime_error(error);
   out["explicitPublic76"]=Json::Value(Json::arrayValue);for(int value:business)out["explicitPublic76"].append(value);
  }catch(const std::exception& e){out["returned"]=false;out["error"]=e.what();}
  std::cout<<Json::FastWriter().write(out);
 }
 return 0;
}
