#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RLBadgeRequestPopup : public geode::Popup<> {
     public:
      static RLBadgeRequestPopup* create();

     private:
      bool setup() override;

      void onSubmit(CCObject* sender);

      TextInput* m_discordInput = nullptr;
};