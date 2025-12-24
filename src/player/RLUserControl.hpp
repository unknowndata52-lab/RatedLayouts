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
      ButtonSprite* m_applySprite = nullptr;
      LoadingSpinner* m_applySpinner = nullptr;
      bool m_isInitializing = false;
      bool m_waitingForApplyDialog = false;
      void onApplyChanges(CCObject* sender);
      void onToggleChanged(CCObject* sender);
      void onOptionClicked(CCObject* sender);

      UserOption* getOptionByKey(const std::string& key);
      void setOptionState(const std::string& key, bool desired, bool updatePersisted = false);
      void setOptionEnabled(const std::string& key, bool enabled);
      void setAllOptionsEnabled(bool enabled);
      
      bool setup() override;
};
