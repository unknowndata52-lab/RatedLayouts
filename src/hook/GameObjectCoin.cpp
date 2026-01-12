#include <Geode/Geode.hpp>
#include <Geode/modify/CCSprite.hpp>
#include <Geode/modify/EffectGameObject.hpp>
#include <string_view>

using namespace geode::prelude;

const int USER_COIN = 1329;
const ccColor3B BRONZE_COLOR = ccColor3B{255, 175, 75};

// Helper: try multiple possible getters for the sprite frame (getSpriteFrame or getDisplayFrame)
namespace {
      template <typename T>
      auto rl_get_frame_impl(T* s, int) -> decltype(s->getSpriteFrame()) {
            return s->getSpriteFrame();
      }

      template <typename T>
      auto rl_get_frame_impl(T* s, long) -> decltype(s->getDisplayFrame()) {
            return s->getDisplayFrame();
      }

      template <typename T>
      cocos2d::CCSpriteFrame* rl_get_frame_impl(T*, ...) {
            return nullptr;
      }

      static cocos2d::CCSpriteFrame* rl_get_frame(cocos2d::CCSprite* s) {
            return rl_get_frame_impl(s, 0);
      }
}

class $modify(EffectGameObject) {
      struct Fields {
            utils::web::WebTask m_fetchTask;
            bool m_isSuggested = false;
            bool m_loggedFrame = false;
            ~Fields() { m_fetchTask.cancel(); }
      };

      void customSetup() {
            EffectGameObject::customSetup();

            if (this->m_objectID != USER_COIN) return;

            // avoid duplicate
            if (this->getChildByID("rl-blue-coin")) return;

            auto playLayer = PlayLayer::get();
            if (!playLayer || !playLayer->m_level || playLayer->m_level->m_levelID == 0) {
                  return;
            }

            int levelId = playLayer->m_level->m_levelID;

            auto url = fmt::format("https://gdrate.arcticwoof.xyz/fetch?levelId={}", levelId);
            Ref<EffectGameObject> selfRef = this;
            m_fields->m_fetchTask = web::WebRequest().get(url);
            auto fields = m_fields;
            m_fields->m_fetchTask.listen([selfRef, fields, levelId](web::WebResponse* res) {
                  if (!selfRef) return;

                  if (!res || !res->ok()) {
                        log::debug("GameObjectCoin: fetch failed or non-ok for level {}", levelId);
                        return;  // don't apply blue coin if server does not respond OK
                  }

                  auto jsonRes = res->json();
                  if (!jsonRes) {
                        log::debug("GameObjectCoin: invalid JSON response for level {}", levelId);
                        return;
                  }

                  auto json = jsonRes.unwrap();
                  bool isSuggested = json["isSuggested"].asBool().unwrapOrDefault();
                  auto tag = isSuggested ? 69 : 67;  // idk what to name the tags lol

                  if (auto asSprite = typeinfo_cast<CCSprite*>(selfRef.operator->())) {
                        asSprite->setTag(tag);
                        if (!isSuggested) asSprite->setColor({0, 127, 232});
                  }
                  if (auto children = selfRef->getChildren()) {
                        for (auto node : CCArrayExt<CCNode>(children)) {
                              if (auto cs = typeinfo_cast<CCSprite*>(node)) {
                                    cs->setTag(tag);
                                    if (!isSuggested) cs->setColor({0, 127, 232});
                              }
                        }
                  }

                  // btw this is only just makes the coin blue but still uses the default coin sprite.
                  // i still dont know how to replace the actual sprite image lmao

                  selfRef->m_addToNodeContainer = true;
            });
      }

      void update(float dt) {
            EffectGameObject::update(dt);

            if (this->m_objectID != USER_COIN) return;

            // // Only log once to avoid spamming
            // if (m_fields->m_loggedFrame) return;

            CCSprite* sprite = nullptr;
            if (auto asSprite = typeinfo_cast<CCSprite*>(this)) {
                  sprite = asSprite;
            }

            if (!sprite) return;

            // try to get the sprite frame using a compatibility helper
            if (auto frame = rl_get_frame(sprite)) {
                  auto name = frame->getFrameName();
                  std::string_view namev = name;
                  if (!namev.empty() && namev[0]) {
                        log::debug("GameObjectCoin: coin display frame name: {}", name);
                  } else {
                        log::debug("GameObjectCoin: coin display frame name is empty");
                  }
            } else {
                  log::debug("GameObjectCoin: coin has no display frame");
            }

            m_fields->m_loggedFrame = true;
      }
};

class $modify(CCSprite) {
      void setColor(const ccColor3B& color) {
            if (color == BRONZE_COLOR) {
                  int tag = this->getTag();
                  if (tag == 67) {
                        CCSprite::setColor({0, 127, 232});
                  }
            }

            CCSprite::setColor(color);
      }
};