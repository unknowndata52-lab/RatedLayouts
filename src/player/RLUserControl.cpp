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

      // add moderator toggle if current user is an admin
      int userRole = Mod::get()->getSavedValue<int>("role", 0);
      if (userRole == 2) {
            auto modBtnSpr = ButtonSprite::create("Set Moderator", 0.8f);
            if (modBtnSpr) modBtnSpr->updateBGImage("GJ_button_01.png");
            auto modBtnItem = CCMenuItemSpriteExtra::create(modBtnSpr, this, menu_selector(RLUserControl::onOptionClicked));
            modBtnItem->setID("rl-option-make-mod");
            modBtnItem->setSizeMult(1.5f);
            optionsMenu->addChild(modBtnItem);

            UserOption modOpt;
            modOpt.key = "giveMod";
            modOpt.persisted = false;
            modOpt.desired = false;
            modOpt.button = modBtnSpr;
            modOpt.item = modBtnItem;
            m_userOptions.push_back(modOpt);
            optionsMenu->updateLayout();
      }
      // Add an apply button to submit the final state
      auto applySprite = ButtonSprite::create("Apply", 1.f);
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

            Ref<RLUserControl> thisRef = this;
            postTask.listen([thisRef](web::WebResponse* response) {
                  if (!thisRef || !thisRef->m_mainLayer) return;

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
                  bool isMod = json["role"].asInt().unwrapOrDefault() == 1;

                  if (thisRef) {
                        thisRef->m_isInitializing = true;
                        for (auto& opt : thisRef->m_userOptions) {
                              if (opt.key == "exclude") {
                                    opt.persisted = isExcluded;
                                    opt.desired = isExcluded;
                                    if (opt.button) opt.button->updateBGImage(isExcluded ? "GJ_button_02.png" : "GJ_button_01.png");
                              } else if (opt.key == "giveMod") {
                                    opt.persisted = isMod;
                                    opt.desired = isMod;
                                    if (opt.button) opt.button->updateBGImage(isMod ? "GJ_button_02.png" : "GJ_button_01.png");
                              }
                        }
                        thisRef->m_isInitializing = false;
                  }
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
                  if (opt.button) {
                        opt.button->updateBGImage(opt.desired ? "GJ_button_02.png" : "GJ_button_01.png");
                  }
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
      for (auto& opt : m_userOptions) {
            if (opt.item) opt.item->setEnabled(false);
      }
      if (m_applySpinner) {
            m_applySpinner->setVisible(true);
            m_applyButton->setEnabled(false);
            m_applyButton->setVisible(false);
      }

      auto postReq = web::WebRequest();
      postReq.bodyJSON(jsonBody);
      log::info("Applying user settings for account {}", m_targetAccountId);
      auto postTask = postReq.post("https://gdrate.arcticwoof.xyz/setUser");

      Ref<RLUserControl> thisRef = this;
      postTask.listen([thisRef, jsonBody](web::WebResponse* response) {
            if (!thisRef) return;

            // re-enable UI regardless
            for (auto& opt : thisRef->m_userOptions) {
                  if (opt.item) opt.item->setEnabled(true);
            }
            if (thisRef->m_applyButton) thisRef->m_applyButton->setEnabled(true);
            if (thisRef->m_applySpinner) thisRef->m_applySpinner->setVisible(false);
            if (thisRef->m_applyButton) thisRef->m_applyButton->setVisible(true);

            if (!response->ok()) {
                  log::warn("setUser returned non-ok status: {}", response->code());
                  Notification::create("Failed to update user", NotificationIcon::Error)->show();
                  // revert each option to persisted state
                  thisRef->m_isInitializing = true;
                  for (auto& opt : thisRef->m_userOptions) {
                        opt.desired = opt.persisted;
                        if (opt.button) opt.button->updateBGImage(opt.persisted ? "GJ_button_02.png" : "GJ_button_01.png");
                  }
                  thisRef->m_isInitializing = false;
                  if (thisRef->m_applyButton) thisRef->m_applyButton->setEnabled(false);
                  if (thisRef->m_applyButton) thisRef->m_applyButton->setVisible(true);
                  if (thisRef->m_applySpinner) thisRef->m_applySpinner->setVisible(false);
                  return;
            }

            auto jsonRes = response->json();
            if (!jsonRes) {
                  log::warn("Failed to parse setUser response");
                  Notification::create("Invalid server response", NotificationIcon::Error)->show();
                  thisRef->m_isInitializing = true;
                  for (auto& opt : thisRef->m_userOptions) {
                        opt.desired = opt.persisted;
                        if (opt.button) opt.button->updateBGImage(opt.persisted ? "GJ_button_02.png" : "GJ_button_01.png");
                  }
                  thisRef->m_isInitializing = false;
                  if (thisRef->m_applyButton) thisRef->m_applyButton->setEnabled(false);
                  if (thisRef->m_applyButton) thisRef->m_applyButton->setVisible(true);
                  if (thisRef->m_applySpinner) thisRef->m_applySpinner->setVisible(false);
                  return;
            }

            auto json = jsonRes.unwrap();
            bool success = json["success"].asBool().unwrapOrDefault();
            if (success) {
                  for (auto& opt : thisRef->m_userOptions) {
                        opt.persisted = opt.desired;
                        if (opt.button) opt.button->updateBGImage(opt.persisted ? "GJ_button_02.png" : "GJ_button_01.png");
                  }
                  Notification::create("User has been updated!", NotificationIcon::Success)->show();
                  if (thisRef->m_applyButton) thisRef->m_applyButton->setEnabled(false);
                  thisRef->onClose(nullptr);
            } else {
                  Notification::create("Failed to update user", NotificationIcon::Error)->show();
                  // revert each option to persisted state
                  thisRef->m_isInitializing = true;
                  for (auto& opt : thisRef->m_userOptions) {
                        opt.desired = opt.persisted;
                        if (opt.button) opt.button->updateBGImage(opt.persisted ? "GJ_button_02.png" : "GJ_button_01.png");
                  }
                  thisRef->m_isInitializing = false;
            }
      });
}

RLUserControl::UserOption* RLUserControl::getOptionByKey(const std::string& key) {
      for (auto& opt : m_userOptions) {
            if (opt.key == key) return &opt;
      }
      return nullptr;
}