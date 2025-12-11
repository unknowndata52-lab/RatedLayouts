#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RLCreditsPopup : public geode::Popup<> {
     public:
      static RLCreditsPopup* create();

     private:
      bool setup() override;
      void onAccountClicked(CCObject* sender);
      ScrollLayer* m_scrollLayer = nullptr;
      LoadingSpinner* m_spinner = nullptr;
};