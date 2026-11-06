//
//  AIDataManager.h
//  OnePiece
//
//  Created by Beileng Shan on 4/14/14.
//
//

#ifndef __OnePiece__AIDataManager__
#define __OnePiece__AIDataManager__

#include "json/document.h"
#include <unordered_map>
#include <string>

namespace OnePiece {	
	class AIDataManager
	{
	public:
		static rapidjson::Document* getJsonFromFile(const char *intfilename);
		static void unload();
		static void unload(const char *intfilename);
		static void setConfigDataPath(const std::string &path);
        static const std::string& getConfigDataPath() { return _configdata_path; }
	private:
        static std::unordered_map<const char*, rapidjson::Document*> _cache;
		static std::string _configdata_path;
	};
};

#endif /* defined(__OnePiece__AIDataManager__) */
