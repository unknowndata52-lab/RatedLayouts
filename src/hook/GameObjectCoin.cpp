#include <Geode/Geode.hpp>
#include <Geode/modify/EffectGameObject.hpp>

using namespace geode::prelude;

const int USER_COIN = 1329;

class $modify(EffectGameObject) {
    struct Fields {
        utils::web::WebTask m_fetchTask;
        bool m_isSuggested = false;
        // Clean up task when object is destroyed
        ~Fields() { m_fetchTask.cancel(); }
    };

    void customSetup() {
        EffectGameObject::customSetup();

        if (this->m_objectID != USER_COIN) return;
        
        // Prevent applying twice if customSetup runs multiple times
        if (this->getChildByID("rl-blue-coin-anim")) return;

        auto playLayer = PlayLayer::get();
        if (!playLayer || !playLayer->m_level || playLayer->m_level->m_levelID == 0) {
            return;
        }

        int levelId = playLayer->m_level->m_levelID;
        auto url = fmt::format("https://gdrate.arcticwoof.xyz/fetch?levelId={}", levelId);
        
        Ref<EffectGameObject> selfRef = this;
        
        // Start the web request
        m_fields->m_fetchTask = web::WebRequest().get(url);
        
        // Listen for response
        m_fields->m_fetchTask.listen([selfRef, levelId](web::WebResponse* res) {
            if (!selfRef) return;

            if (!res || !res->ok()) {
                log::debug("GameObjectCoin: fetch failed for level {}", levelId);
                return; 
            }

            auto jsonRes = res->json();
            if (!jsonRes) return;

            auto json = jsonRes.unwrap();
            // We only really care if the server responds; we can check suggested/rated here
            // to decide WHICH animation to play, but for now let's apply the Blue Coin.
            
            // --- ANIMATION LOGIC STARTS HERE ---
            
            // 1. Create the Animation Frame Array
            auto spriteCache = CCSpriteFrameCache::sharedSpriteFrameCache();
            auto animFrames = CCArray::create();

            // Loop through your 4 images. 
            // IMPORTANT: Ensure these names match exactly what is in your .plist or file system
            for (int i = 1; i <= 4; i++) {
                auto frameName = fmt::format("RL_BlueCoin{}.png", i);
                auto frame = spriteCache->spriteFrameByName(frameName.c_str());
                
                if (frame) {
                    animFrames->addObject(frame);
                } else {
                    log::error("Could not find sprite frame: {}", frameName);
                    // If you haven't loaded a .plist, you might need to try creating from file:
                    // animFrames->addObject(CCSprite::create(frameName.c_str())->getDisplayFrame());
                }
            }

            // Only proceed if we found frames
            if (animFrames->count() > 0) {
                auto animation = CCAnimation::createWithSpriteFrames(animFrames, 0.10f); // 0.10f is the speed per frame
                auto animate = CCAnimate::create(animation);
                auto loop = CCRepeatForever::create(animate);

                // 2. Find the actual visual node to apply this to.
                // EffectGameObject is a container. The visual coin is usually a child CCSprite.
                
                CCSprite* targetSprite = nullptr;

                // If the object acts as the sprite itself (sometimes true in GD)
                if (auto asSprite = typeinfo_cast<CCSprite*>(selfRef.operator->())) {
                    targetSprite = asSprite;
                }
                // Otherwise look for children (Standard for User Coins)
                else if (auto children = selfRef->getChildren()) {
                    for (auto node : CCArrayExt<CCNode>(children)) {
                        if (auto s = typeinfo_cast<CCSprite*>(node)) {
                            // GD Coins sometimes have a "glow" sprite and a "main" sprite.
                            // The main sprite usually has a specific Z order or texture.
                            // We will simply apply it to the first valid Sprite we find that looks like the coin.
                            targetSprite = s;
                            break; 
                        }
                    }
                }

                if (targetSprite) {
                    // Mark this so we don't do it twice (using ID system)
                    selfRef->setID("rl-blue-coin-anim");
                    
                    // Reset color to white so the blue texture shows correctly
                    // (If the coin was bronze (tinted), it would turn the blue texture brown-ish)
                    targetSprite->setColor({255, 255, 255}); 

                    // Set the initial texture to Frame 1 so it changes immediately
                    auto firstFrame = static_cast<CCSpriteFrame*>(animFrames->objectAtIndex(0));
                    targetSprite->setDisplayFrame(firstFrame);

                    // Run the loop
                    targetSprite->runAction(loop);
                }
            }
            
            selfRef->m_addToNodeContainer = true;
        });
    }
};
