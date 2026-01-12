#include <Geode/Geode.hpp>
#include <Geode/modify/CCSprite.hpp>
#include <Geode/modify/EffectGameObject.hpp>

using namespace geode::prelude;

const int USER_COIN = 1329;
const ccColor3B BRONZE_COLOR = ccColor3B{255, 175, 75};

// Replace coin visuals when GameObjects are set up
class $modify(EffectGameObject) {
      struct Fields {
            utils::web::WebTask m_fetchTask;
            bool m_isSuggested = false;
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

                  if (isSuggested) {
                        if (!selfRef->getChildByID("rl-suggested")) {
                              auto marker = CCNode::create();
                              marker->setID("rl-suggested");
                              selfRef->addChild(marker);
                        }

                        log::debug("skipping replacement for level {}", levelId);
                        return;
                  }

                  if (auto marker = selfRef->getChildByID("rl-suggested")) {
                        marker->removeFromParent();
                  }

                  // TODO: change the coin gameobject sprite to blue coin with animation pls

                  selfRef->m_addToNodeContainer = true;
            });
      }
};

// detect and remove the bronze tint on unverified coins
class $modify(CCSprite) {
      void setColor(ccColor3B const& col) {
            if (col == BRONZE_COLOR) {
                  GameObject* gameObj = typeinfo_cast<GameObject*>(this);  // check for user coin
                  if (gameObj && gameObj->m_objectID == USER_COIN) {
                        auto eff = typeinfo_cast<EffectGameObject*>(gameObj);
                        if (eff && eff->getChildByID("rl-suggested")) {
                              CCSprite::setColor(col);
                              return;
                        }

                        CCSprite::setColor({0, 127, 232});
                        return;
                  }
            }

            CCSprite::setColor(col);
      }
};