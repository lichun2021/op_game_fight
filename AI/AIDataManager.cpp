//
//  AIDataManager.cpp
//  OnePiece
//
//  Created by Beileng Shan on 4/14/14.
//
//

#include "AIDataManager.h"
#ifndef ONE_PIECE_SERVER
#include "cocos2d.h"
#endif
#ifdef SERVER_VERIFYING
  #include "json/filestream.h"
#else
  #include "json/filereadstream.h"
  #include "json/filewritestream.h"
#endif

namespace OnePiece {
    std::unordered_map<const char*, rapidjson::Document*> AIDataManager::_cache;
	std::string AIDataManager::_configdata_path;

	void AIDataManager::unload()
	{
		for (auto iter = _cache.begin(); iter != _cache.end(); ++iter)
		{
			delete iter->second;
		}
		_cache.clear();
	}

	void AIDataManager::unload(const char *fn)
	{
        std::unordered_map<const char*, rapidjson::Document*>::iterator ui = _cache.find(fn);
        if (ui != _cache.end())
        {
            rapidjson::Document* tmp = ui->second;
            _cache.erase(ui);
            delete tmp;
        }
	}

	void AIDataManager::setConfigDataPath(const std::string &path)
	{
		_configdata_path = path;
	}

	rapidjson::Document* AIDataManager::getJsonFromFile(const char *fn)
	{
        std::unordered_map<const char*, rapidjson::Document*>::iterator ui = _cache.find(fn);
        if (ui != _cache.end())
		{
            return ui->second;
		}

#ifdef SERVER_VERIFYING
		char fnbuffer[128];
		size_t n = sprintf(fnbuffer, PATH, fn);
		if (_configdata_path.length() > 0)
		{
			std::string custom_path = _configdata_path + "%s.txt";
			n = sprintf(fnbuffer, custom_path.c_str(), fn);
		}
		fnbuffer[n] = '\0';
		FILE * pFile = fopen(fnbuffer, "r");

		rapidjson::FileStream f(pFile);
		rapidjson::Document *doc = new rapidjson::Document();
		doc->ParseStream<0>(f);
		fclose(pFile);
#else
		char fnbuffer[128];
		size_t n = sprintf(fnbuffer, "src/config_data/%s.txt", fn);
		fnbuffer[n] = '\0';
		std::string str = cocos2d::FileUtils::getInstance()->getStringFromFile(fnbuffer);
		rapidjson::Document *doc = new rapidjson::Document();
		doc->Parse<0>(str.c_str());
#endif

        rapidjson::Document* pResult = nullptr;
		if (doc->HasParseError())
		{
            delete doc;
		}
		else
		{
            pResult = doc;
			_cache[fn] = doc;
		}
        return pResult;
    }
}

