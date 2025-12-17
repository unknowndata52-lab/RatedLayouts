#include "RLCommunityVotePopup.hpp"

using namespace geode::prelude;

RLCommunityVotePopup* RLCommunityVotePopup::create() {
      auto popup = new RLCommunityVotePopup();
      // Use initAnchored to avoid calling FLAlertLayer::init() with no args
      if (popup && popup->initAnchored(360.f, 180.f, "GJ_square02.png")) {
            popup->autorelease();
            return popup;
      }
      CC_SAFE_DELETE(popup);
      return nullptr;
}

bool RLCommunityVotePopup::setup() {
      // Minimal setup so the popup compiles and can be extended later
      // You can add buttons/labels here when implementing the feature
      return true;
}

