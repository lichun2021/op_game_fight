#pragma once
// Protocol/schema definitions, not business configuration.
// Public JS76 and the proven original native/Lua72 are distinct layouts.
#include <string>
#include <stdexcept>
#include <vector>
#include <cstddef>

namespace OnePiece { namespace BattleAttributeSchema {
class Error : public std::runtime_error {
public:
 explicit Error(const std::string& message) : std::runtime_error(message) {}
};

static const std::size_t SharedPrefix=60;
static const std::size_t PublicCount=76;
static const std::size_t RuntimeCount=72;
inline const char* publicSchema(){return "target-js76-v1";}
inline const int* publicTailIndices(){
 static const int indices[12]={62,63,64,65,68,69,70,71,72,73,74,75};
 return indices;
}
inline bool decode(const std::vector<int>& input,const std::string& schema,std::vector<int>& output,std::string& error){
 output.clear();error.clear();
 if(schema.empty()){
  if(input.size()>PublicCount){error="attribute_schema_unknown_length";return false;}
  for(std::size_t i=SharedPrefix;i<input.size();++i)if(input[i]!=0){
   error="attribute_schema_required_at_index_"+std::to_string(i);return false;
  }
  output.assign(RuntimeCount,0);
  for(std::size_t i=0;i<input.size()&&i<SharedPrefix;++i)output[i]=input[i];
  return true;
 }
 if(schema!=publicSchema()){error="attribute_schema_unknown_tag";return false;}
 if(input.size()!=PublicCount){error="attribute_schema_JS76_requires_76_values";return false;}
 const int unimplemented[4]={60,61,66,67};
 for(int index:unimplemented)if(input[index]!=0){
  error="attribute_consumer_unavailable_at_JS76_index_"+std::to_string(index);return false;
 }
 output.assign(RuntimeCount,0);
 for(std::size_t i=0;i<SharedPrefix;++i)output[i]=input[i];
 const int* indices=publicTailIndices();
 for(int i=0;i<12;++i)output[SharedPrefix+i]=input[indices[i]];
 return true;
}
// Only an explicitly requested business-layout export may call this helper.
// Existing Lua origin/after-begin snapshots continue to use their native72 API.
inline bool encodePublic(const std::vector<int>& runtime,std::vector<int>& output,std::string& error){
 output.clear();error.clear();
 if(runtime.size()!=RuntimeCount){error="attribute_schema_native_snapshot_requires_72_values";return false;}
 output.assign(PublicCount,0);
 for(std::size_t i=0;i<SharedPrefix;++i)output[i]=runtime[i];
 const int* indices=publicTailIndices();
 for(int i=0;i<12;++i)output[indices[i]]=runtime[SharedPrefix+i];
 return true;
}
} }
