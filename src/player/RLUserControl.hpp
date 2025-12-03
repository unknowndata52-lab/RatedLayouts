#pragma once

#include <Geode/Geode.hpp>
#include <argon/argon.hpp>
#include <vector>

using namespace geode::prelude;

class RLUserControl : public geode::Popup<> {
     public:
      static RLUserControl* create();
      static RLUserControl* create(int accountId);

     private:
      int m_targetAccountId = 0;
      RowLayout* m_optionsLayout = nullptr;

      struct UserOption {
            std::string key;
            bool persisted = false;
            bool desired = false;
            ButtonSprite* button = nullptr;
            CCMenuItemSpriteExtra* item = nullptr;
      };

      std::vector<UserOption> m_userOptions;
      CCMenuItemSpriteExtra* m_optionsButton = nullptr;
      CCMenuItemSpriteExtra* m_applyButton = nullptr;
      LoadingSpinner* m_applySpinner = nullptr;
      bool m_isInitializing = false;
      bool m_waitingForApplyDialog = false;
      void onApplyChanges(CCObject* sender);
      void onToggleChanged(CCObject* sender);
      void onOptionClicked(CCObject* sender);
      UserOption* getOptionByKey(const std::string& key);
      bool setup() override;
};
