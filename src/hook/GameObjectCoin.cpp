#include <Geode/Geode.hpp>
#include <Geode/modify/EffectGameObject.hpp>

using namespace geode::prelude;

const int USER_COIN = 1329;

class $modify(EffectGameObject) {
    struct Fields {
        utils::web::WebTask m_fetchTask;
        ~Fields() { m_fetchTask.cancel(); }
    };

    void customSetup() {
        EffectGameObject::customSetup();

        if (this->m_objectID != USER_COIN) return;
        
        // 1. Check if the user disabled fetch requests in settings
        if (Mod::get()->getSettingValue<bool>("compatibilityMode")) return;

        // Prevent duplicate animation logic
        if (this->getChildByID("rl-blue-coin-anim")) return;

        auto playLayer = PlayLayer::get();
        if (!playLayer || !playLayer->m_level || playLayer->m_level->m_levelID == 0) return;

        int levelId = playLayer->m_level->m_levelID;
        auto url = fmt::format("https://gdrate.arcticwoof.xyz/fetch?levelId={}", levelId);
        
        Ref<EffectGameObject> selfRef = this;
        m_fields->m_fetchTask = web::WebRequest().get(url);
        
        m_fields->m_fetchTask.listen([selfRef, levelId](web::WebResponse* res) {
            if (!selfRef || !res || !res->ok()) return;

            // --- ANIMATION LOGIC ---
            auto spriteCache = CCSpriteFrameCache::get();

            // Load the sheet defined in mod.json: "ModID/SheetKey.plist"
            spriteCache->addSpriteFramesWithFile("arcticwoof.rated_layouts/RLCoins.plist");

            auto animFrames = CCArray::create();
            for (int i = 1; i <= 4; i++) {
                auto frameName = fmt::format("RL_BlueCoin{}.png", i);
                auto frame = spriteCache->spriteFrameByName(frameName.c_str());
                if (frame) animFrames->addObject(frame);
            }

            if (animFrames->count() > 0) {
                auto animation = CCAnimation::createWithSpriteFrames(animFrames, 0.10f); 
                auto loop = CCRepeatForever::create(CCAnimate::create(animation));

                CCSprite* targetSprite = nullptr;
                if (auto asSprite = typeinfo_cast<CCSprite*>(selfRef.operator->())) {
                    targetSprite = asSprite;
                } else if (auto children = selfRef->getChildren()) {
                    for (auto node : CCArrayExt<CCNode>(children)) {
                        if (auto s = typeinfo_cast<CCSprite*>(node)) {
                            targetSprite = s;
                            break; 
                        }
                    }
                }

                if (targetSprite) {
                    selfRef->setID("rl-blue-coin-anim");
                    // Untint the coin so the blue texture shows correctly
                    targetSprite->setColor({255, 255, 255}); 
                    targetSprite->setDisplayFrame(static_cast<CCSpriteFrame*>(animFrames->objectAtIndex(0)));
                    targetSprite->runAction(loop);
                }
            }
            selfRef->m_addToNodeContainer = true;
        });
    }
};
