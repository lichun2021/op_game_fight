//
//  AIUICallback.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__AIUICallback__
#define __OnePiece__AIUICallback__

namespace OnePiece {
	class AIUICallback
	{
	public:
		virtual ~AIUICallback();
		AIUICallback();
		
		virtual void detach() = 0;

	private:
		//AIUICallback(const AIUICallback&);
		//AIUICallback(AIUICallback&&);
		//AIUICallback& operator=(const AIUICallback&);
	};

}

#endif /* defined(__OnePiece__AIUICallback__) */
