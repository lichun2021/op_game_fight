//
//  AIContinuousTransformCaster.h
//  op_game
//
//  Created by h7ing on 15/9/25.
//
//

#ifndef __op_game__AIContinuousTransformCaster__
#define __op_game__AIContinuousTransformCaster__

#include <memory>
#include <set>
#include "AINode.h"

namespace OnePiece {

    struct TransformExec;

    class AIContext;
    class CharacterData;
    class AITargetContext;

    class AIContinuousTransformCaster;
    typedef std::shared_ptr<AIContinuousTransformCaster> AIContinuousTransformCasterPtr;


    class AIContinuousTransformCaster : public AINode {
    public:
        static AIContinuousTransformCasterPtr create(int transformID, AIContext& aiContext, CharacterData* source, CharacterData* target, int skillID, int skillLevel);
        
        AIContinuousTransformCaster(int transformID, AIContext& aiContext, CharacterData* source, CharacterData* target, int skillID, int skillLevel);
        virtual ~AIContinuousTransformCaster();

        virtual void update(AIContext& aiContext) override;

        void stop();

    protected:
        AIContinuousTransformCaster();
        AIContinuousTransformCaster(const AIContinuousTransformCaster&);
        AIContinuousTransformCaster(const AIContinuousTransformCaster&&);
        
    private:
        static void initTransformTargetContext(AITargetContext &ctx, AIContinuousTransformCaster *caster);
        void castOnTarget(AIContext &aiContext, CharacterData *aTarget);

    private:
        int _transformID = 0;
        TransformExec* _staticData = nullptr;
        AIContext& _aiContext;
        CharacterData* _pSource = nullptr;
        CharacterData* _pTarget = nullptr;
        CharacterData* _pOwner = nullptr;
        int _skillID = 0;
        int _skillLevel = 0;
        std::set<CharacterData*> _affectedTargets;
        bool _isStopped = false;
    };

} // namespace OnePiece

#endif /* defined(__op_game__AIContinuousTransformCaster__) */
