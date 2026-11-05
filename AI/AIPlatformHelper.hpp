//
//  AIPlatformHelper.hpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#ifndef AIPlatformHelper_hpp
#define AIPlatformHelper_hpp

#include <string>

namespace OnePiece {
    
    class AIPlatformHelper {
    public:
        static std::string getPlatformName();
        static std::string getExecutablePath();
    };
    
}

#endif /* AIPlatformHelper_hpp */
