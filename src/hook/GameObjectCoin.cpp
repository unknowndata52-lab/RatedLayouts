#include <Geode/Geode.hpp>
#include <Geode/modify/EffectGameObject.hpp>

using namespace geode::prelude;

const int USER_COIN = 1329;

class $modify(EffectGameObject) {
    struct Fields {
        utils::web::WebTask m_fetchTask;
        bool m_isSuggested = false;
        ~Fields() { m_fetchTask.cancel(); }
    };

    void customSetup() {
        EffectGameObject::customSetup();

        if (this->m_objectID != USER_COIN) return;
        
        // Prevent applying twice
        if (this->getChildByID("rl-blue-coin-anim")) return;

        auto playLayer = PlayLayer::get();
        if (!playLayer || !playLayer->m_level || playLayer->m_level->m_levelID == 0) {
            return;
        }

        int levelId = playLayer->m_level->m_levelID;
        auto url = fmt::format("https://gdrate.arcticwoof.xyz/fetch?levelId={}", levelId);
        
        Ref<EffectGameObject> selfRef = this;
        
        m_fields->m_fetchTask = web::WebRequest().get(url);
        
        m_fields->m_fetchTask.listen([selfRef, levelId](web::WebResponse* res) {
            if (!selfRef) return;

            // Optional: Check if request succeeded. 
            // If you want the coin to be blue ONLY on rated levels, keep this check.
            if (!res || !res->ok()) { 
                // log::debug("Fetch failed for level {}", levelId);
                return; 
            }

            // --- ANIMATION LOGIC ---
            
            auto spriteCache = CCSpriteFrameCache::get();

            // 1. CRITICAL: Load the spritesheet defined in mod.json
            // Geode places resources in a folder named after your Mod ID.
            // Your Mod ID is "arcticwoof.rated_layouts" and the sheet key is "RLCoins"
            spriteCache->addSpriteFramesWithFile("arcticwoof.rated_layouts/RLCoins.plist");

            // 2. Create the frames
            auto animFrames = CCArray::create();

            // We loop from 1 to 4 because your files are RL_BlueCoin1.png ... RL_BlueCoin4.png
            for (int i = 1; i <= 4; i++) {
                // The frame name inside the plist usually matches the original filename
                auto frameName = fmt::format("RL_BlueCoin{}.png", i);
                auto frame = spriteCache->spriteFrameByName(frameName.c_str());
                
                if (frame) {
                    animFrames->addObject(frame);
                } else {
                    // This log helps debug if the plist path is wrong or frames are named differently
                    log::warn("Frame not found in RLCoins.plist: {}", frameName);
                }
            }

            if (animFrames->count() > 0) {
                // 0.10f = speed (10 frames per second). Lower = Faster.
                auto animation = CCAnimation::createWithSpriteFrames(animFrames, 0.10f); 
                auto animate = CCAnimate::create(animation);
                auto loop = CCRepeatForever::create(animate);

                // 3. Find the sprite node to apply it to
                CCSprite* targetSprite = nullptr;

                if (auto asSprite = typeinfo_cast<CCSprite*>(selfRef.operator->())) {
                    targetSprite = asSprite;
                }
                else if (auto children = selfRef->getChildren()) {
                    for (auto node : CCArrayExt<CCNode>(children)) {
                        if (auto s = typeinfo_cast<CCSprite*>(node)) {
                            targetSprite = s;
                            break; 
                        }
                    }
                }

                if (targetSprite) {
                    selfRef->setID("rl-blue-coin-anim");
                    
                    // IMPORTANT: Set color to White. 
                    // The original coin is Bronze because GD tints it orange. 
                    // To show your blue texture correctly, we must untint it (White).
                    targetSprite->setColor({255, 255, 255}); 

                    // Set initial frame
                    auto firstFrame = static_cast<CCSpriteFrame*>(animFrames->objectAtIndex(0));
                    targetSprite->setDisplayFrame(firstFrame);

                    // Run animation
                    targetSprite->runAction(loop);
                }
            }
            
            selfRef->m_addToNodeContainer = true;
        });
    }
};
