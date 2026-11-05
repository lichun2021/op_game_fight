//
//  ExternalDelegateCombo.h
//  OnePiece
//
//  Created by Beileng Shan on 12/12/14.
//
//

#ifndef __OnePiece__ExternalDelegateCombo__
#define __OnePiece__ExternalDelegateCombo__

namespace OnePiece {

	class ExternalDelegateCombo
	{
	public:
		//执行入口
		virtual void executeExec(int comboID, int internalID, int targetID, int X, int Y, int totalTick) = 0;
	};

}

#endif /* defined(__OnePiece__ExternalDelegateCombo__) */
