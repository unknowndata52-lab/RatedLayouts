#include <Geode/Geode.hpp>
#include <Geode/modify/CreatorLayer.hpp>

#include "../custom/RLCreatorLayer.hpp"

using namespace geode::prelude;

class $modify(RLHCreatorLayer, CreatorLayer) {
      bool init() {
            if (!CreatorLayer::init())
                  return false;
            // add a menu on the left side of the layer
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            auto bottomLeftMenu =
                static_cast<CCMenu*>(this->getChildByID("bottom-left-menu"));
            // leaderboard buttons
            // most stars button
            auto leaderboardSpr = CCSprite::create("rlStarIconBig.png"_spr);
            auto lbCircleButton = AccountButtonSprite::create(
                leaderboardSpr, AccountBaseColor::Blue, AccountBaseSize::Normal);
            auto lbButton = CCMenuItemSpriteExtra::create(
                lbCircleButton, this, menu_selector(RLHCreatorLayer::onRatedLayoutLayer));
            bottomLeftMenu->addChild(lbButton);
            bottomLeftMenu->updateLayout();
            return true;
      }
      void onRatedLayoutLayer(CCObject* sender) {
            auto layer = RLCreatorLayer::create();
            auto scene = CCScene::create();
            scene->addChild(layer);
            auto transitionFade = CCTransitionFade::create(0.5f, scene);
            CCDirector::sharedDirector()->pushScene(transitionFade);
      }
};