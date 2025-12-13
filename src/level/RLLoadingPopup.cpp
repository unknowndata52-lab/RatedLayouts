#include <Geode/Geode.hpp>
#include "RLLoadingPopup.hpp"

using namespace geode::prelude;

RLLoadingPopup* RLLoadingPopup::create() {
      auto ret = new RLLoadingPopup();

      if (ret && ret->initAnchored(200.f, 100.f, "GJ_square01.png")) {
            ret->autorelease();
            return ret;
      }

      delete (ret);
      return nullptr;
}

bool RLLoadingPopup::setup() {
      auto loadingLabel = CCLabelBMFont::create("Loading...", "bigFont.fnt");
      loadingLabel->setPosition({m_mainLayer->getContentSize().width / 2,
                                 m_mainLayer->getContentSize().height - 20});
      m_mainLayer->addChild(loadingLabel);
      return true;
};
