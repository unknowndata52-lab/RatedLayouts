#include "RLUserControl.hpp"

#include <Geode/Geode.hpp>
#include <argon/argon.hpp>

using namespace geode::prelude;

RLUserControl* RLUserControl::create() {
      auto ret = new RLUserControl();

      if (ret && ret->initAnchored(380.f, 240.f, "GJ_square02.png")) {
            ret->autorelease();
            return ret;
      }

      CC_SAFE_DELETE(ret);
      return nullptr;
};

RLUserControl* RLUserControl::create(int accountId) {
      auto ret = new RLUserControl();
      ret->m_targetAccountId = accountId;

      if (ret && ret->initAnchored(380.f, 240.f, "GJ_square02.png")) {
            ret->autorelease();
            return ret;
      }

      CC_SAFE_DELETE(ret);
      return nullptr;
};

bool RLUserControl::setup() {
      setTitle("Rated Layouts User Control");

      auto menu = CCMenu::create();
      menu->setPosition({0, 0});

      auto optionsMenu = CCMenu::create();
      optionsMenu->setPosition({m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 5});
      optionsMenu->setContentSize({m_mainLayer->getContentSize().width - 40, 170});
      optionsMenu->setLayout(RowLayout::create()
                                 ->setGap(6.f)
                                 ->setGrowCrossAxis(true)
                                 ->setCrossAxisOverflow(false));

      m_optionsLayout = static_cast<RowLayout*>(optionsMenu->getLayout());

      // this is very messy code but ill clean it up later on
      auto excludeBtnSpr = ButtonSprite::create("Leaderboard Exclude", 0.8f);
      if (excludeBtnSpr) excludeBtnSpr->updateBGImage("GJ_button_01.png");
      auto excludeBtnItem = CCMenuItemSpriteExtra::create(excludeBtnSpr, this, menu_selector(RLUserControl::onOptionClicked));
      excludeBtnItem->setID("rl-option-exclude");
      excludeBtnItem->setSizeMult(1.5f);
      optionsMenu->addChild(excludeBtnItem);

      UserOption excludeOpt;
      excludeOpt.key = "exclude";
      excludeOpt.persisted = false;
      excludeOpt.desired = false;
      excludeOpt.button = excludeBtnSpr;
      excludeOpt.item = excludeBtnItem;
      m_userOptions.push_back(excludeOpt);
      m_optionsLayout = static_cast<RowLayout*>(optionsMenu->getLayout());
      optionsMenu->updateLayout();

      // Add an apply button to submit the final state
      auto applySprite = ButtonSprite::create("Apply", 1.f);
      m_applySprite = applySprite;
      if (m_applySprite) m_applySprite->updateBGImage("GJ_button_04.png");
      m_applyButton = CCMenuItemSpriteExtra::create(applySprite, this, menu_selector(RLUserControl::onApplyChanges));
      m_applyButton->setPosition({m_mainLayer->getContentSize().width / 2, 0});
      m_applyButton->setEnabled(false);
      // spinner
      m_applySpinner = LoadingSpinner::create(36.f);
      m_applySpinner->setPosition({m_applyButton->getPosition()});
      m_applySpinner->setVisible(false);
      menu->addChild(m_applySpinner);
      menu->addChild(m_applyButton);

      m_mainLayer->addChild(menu);
      m_mainLayer->addChild(optionsMenu);

      // fetch profile data to determine initial excluded state
      if (m_targetAccountId > 0) {
            matjson::Value jsonBody = matjson::Value::object();
            jsonBody["argonToken"] = Mod::get()->getSavedValue<std::string>("argon_token");
            jsonBody["accountId"] = m_targetAccountId;

            auto postReq = web::WebRequest();
            postReq.bodyJSON(jsonBody);
            auto postTask = postReq.post("https://gdrate.arcticwoof.xyz/profile");

            postTask.listen([this](web::WebResponse* response) {
                  if (!response->ok()) {
                        log::warn("Profile fetch returned non-ok status: {}", response->code());
                        return;
                  }

                  auto jsonRes = response->json();
                  if (!jsonRes) {
                        log::warn("Failed to parse JSON response for profile");
                        return;
                  }

                  // might refactor this later
                  auto json = jsonRes.unwrap();
                  bool isExcluded = json["excluded"].asBool().unwrapOrDefault();

                  this->m_isInitializing = true;
                  this->setOptionState("exclude", isExcluded, true);
                  this->m_isInitializing = false;
            });
      }

      return true;
}

void RLUserControl::onOptionClicked(CCObject* sender) {
      if (m_isInitializing) return;

      auto item = static_cast<CCMenuItemSpriteExtra*>(sender);
      for (auto& opt : m_userOptions) {
            if (opt.item == item) {
                  // flip the desired state and update BG
                  opt.desired = !opt.desired;
                  // use helper to update UI and apply-button state
                  setOptionState(opt.key, opt.desired, false);
                  break;
            }
      }

      // enable apply button if any desired != persisted
      bool differs = false;
      for (auto& opt : m_userOptions) {
            if (opt.desired != opt.persisted) {
                  differs = true;
                  break;
            }
      }
      if (m_applyButton) m_applyButton->setEnabled(differs);
}

void RLUserControl::onApplyChanges(CCObject* sender) {
      bool anyChange = false;
      // check if changes were made
      for (auto& opt : m_userOptions) {
            if (opt.desired != opt.persisted) {
                  anyChange = true;
                  break;
            }
      }
      if (!anyChange) {
            Notification::create("No changes to apply", NotificationIcon::Info)->show();
            return;
      }

      // get argon token
      auto token = Mod::get()->getSavedValue<std::string>("argon_token");
      if (token.empty()) {
            Notification::create("Authentication token not found", NotificationIcon::Error)->show();
            return;
      }

      // prepare payload using desired values
      matjson::Value jsonBody = matjson::Value::object();
      jsonBody["accountId"] = GJAccountManager::get()->m_accountID;
      jsonBody["argonToken"] = token;
      jsonBody["targetAccountId"] = m_targetAccountId;
      for (auto& opt : m_userOptions) {
            jsonBody[opt.key] = opt.desired ? 1 : 0;
      }

      // disable UI while request is in-flight
      if (m_applyButton) m_applyButton->setEnabled(false);
      if (m_applySprite) m_applySprite->updateBGImage("GJ_button_04.png");
      setAllOptionsEnabled(false);
      if (m_applySpinner) {
            m_applySpinner->setVisible(true);
            m_applyButton->setEnabled(false);
            m_applyButton->setVisible(false);
      }

      auto postReq = web::WebRequest();
      postReq.bodyJSON(jsonBody);
      log::info("Applying user settings for account {}", m_targetAccountId);
      auto postTask = postReq.post("https://gdrate.arcticwoof.xyz/setUser");

      postTask.listen([this, jsonBody](web::WebResponse* response) {
            // re-enable UI regardless
            this->setAllOptionsEnabled(true);
            // compute whether any option still differs
            bool differs = false;
            for (auto& opt : this->m_userOptions) {
                  if (opt.desired != opt.persisted) {
                        differs = true;
                        break;
                  }
            }
            if (this->m_applyButton) this->m_applyButton->setEnabled(differs);
            if (this->m_applySprite) this->m_applySprite->updateBGImage(differs ? "GJ_button_01.png" : "GJ_button_04.png");
            if (this->m_applySpinner) this->m_applySpinner->setVisible(false);
            if (this->m_applyButton) this->m_applyButton->setVisible(true);

            if (!response->ok()) {
                  log::warn("setUser returned non-ok status: {}", response->code());
                  Notification::create("Failed to update user", NotificationIcon::Error)->show();
                  // revert each option to persisted state
                  this->m_isInitializing = true;
                  for (auto& opt : this->m_userOptions) {
                        opt.desired = opt.persisted;
                        if (opt.button) opt.button->updateBGImage(opt.persisted ? "GJ_button_02.png" : "GJ_button_01.png");
                  }
                  this->m_isInitializing = false;
                  if (this->m_applyButton) this->m_applyButton->setEnabled(false);
                  if (this->m_applySprite) this->m_applySprite->updateBGImage("GJ_button_04.png");
                  if (this->m_applyButton) this->m_applyButton->setVisible(true);
                  if (this->m_applySpinner) this->m_applySpinner->setVisible(false);
                  return;
            }

            auto jsonRes = response->json();
            if (!jsonRes) {
                  log::warn("Failed to parse setUser response");
                  Notification::create("Invalid server response", NotificationIcon::Error)->show();
                  this->m_isInitializing = true;
                  for (auto& opt : this->m_userOptions) {
                        opt.desired = opt.persisted;
                        if (opt.button) opt.button->updateBGImage(opt.persisted ? "GJ_button_02.png" : "GJ_button_01.png");
                  }
                  this->m_isInitializing = false;
                  if (this->m_applyButton) this->m_applyButton->setEnabled(false);
                  if (this->m_applySprite) this->m_applySprite->updateBGImage("GJ_button_04.png");
                  if (this->m_applyButton) this->m_applyButton->setVisible(true);
                  if (this->m_applySpinner) this->m_applySpinner->setVisible(false);
                  return;
            }

            auto json = jsonRes.unwrap();
            bool success = json["success"].asBool().unwrapOrDefault();
            if (success) {
                  for (auto& opt : this->m_userOptions) {
                        opt.persisted = opt.desired;
                        if (opt.button) opt.button->updateBGImage(opt.persisted ? "GJ_button_02.png" : "GJ_button_01.png");
                  }
                  Notification::create("User has been updated!", NotificationIcon::Success)->show();
                  if (this->m_applyButton) this->m_applyButton->setEnabled(false);
                  if (this->m_applySprite) this->m_applySprite->updateBGImage("GJ_button_04.png");
                  this->onClose(nullptr);
            } else {
                  Notification::create("Failed to update user", NotificationIcon::Error)->show();
                  // revert each option to persisted state
                  this->m_isInitializing = true;
                  for (auto& opt : this->m_userOptions) {
                        opt.desired = opt.persisted;
                        if (opt.button) opt.button->updateBGImage(opt.persisted ? "GJ_button_02.png" : "GJ_button_01.png");
                  }
                  this->m_isInitializing = false;
            }
      });
}

RLUserControl::UserOption* RLUserControl::getOptionByKey(const std::string& key) {
      for (auto& opt : m_userOptions) {
            if (opt.key == key) return &opt;
      }
      return nullptr;
}

void RLUserControl::setOptionState(const std::string& key, bool desired, bool updatePersisted) {
      auto opt = getOptionByKey(key);
      if (!opt) return;
      opt->desired = desired;
      if (updatePersisted) opt->persisted = desired;
      if (opt->button) {
            opt->button->updateBGImage(desired ? "GJ_button_02.png" : "GJ_button_01.png");
      }
      // update the global apply button enabled state
      bool differs = false;
      for (auto& o : m_userOptions) {
            if (o.desired != o.persisted) {
                  differs = true;
                  break;
            }
      }
      if (m_applyButton) m_applyButton->setEnabled(differs);
      if (m_applySprite) m_applySprite->updateBGImage(differs ? "GJ_button_01.png" : "GJ_button_04.png");
}

void RLUserControl::setOptionEnabled(const std::string& key, bool enabled) {
      auto opt = getOptionByKey(key);
      if (!opt) return;
      if (opt->item) opt->item->setEnabled(enabled);
}

void RLUserControl::setAllOptionsEnabled(bool enabled) {
      for (auto& opt : m_userOptions)
            if (opt.item) opt.item->setEnabled(enabled);
}