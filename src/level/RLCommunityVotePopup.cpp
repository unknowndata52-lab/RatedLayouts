#include "RLCommunityVotePopup.hpp"

using namespace geode::prelude;
#include <algorithm>  // for std::clamp

// Note: up/down CircleButton UI removed in favor of numeric TextInput controls.

RLCommunityVotePopup* RLCommunityVotePopup::create() {
      return RLCommunityVotePopup::create(0);
}

RLCommunityVotePopup* RLCommunityVotePopup::create(int levelId) {
      auto popup = new RLCommunityVotePopup();
      popup->m_levelId = levelId;
      if (popup && popup->initAnchored(420.f, 240.f, "GJ_square02.png")) {
            popup->autorelease();
            return popup;
      }
      CC_SAFE_DELETE(popup);
      return nullptr;
}

void RLCommunityVotePopup::onSubmit(CCObject*) {
      // prepare payload (plays payload by dex)
      int accountId = GJAccountManager::get()->m_accountID;
      auto argonToken = Mod::get()->getSavedValue<std::string>("argon_token");
      if (argonToken.empty()) {
            Notification::create("Auth required to submit vote", NotificationIcon::Error)->show();
            return;
      }
      int gameplayVote = 0;
      int designVote = 0;
      int difficultyVote = 0;

      if (m_gameplayInput) {
            gameplayVote = numFromString<int>(m_gameplayInput->getString()).unwrapOr(0);
            gameplayVote = std::clamp(gameplayVote, 1, 30);
      }
      if (m_designInput) {
            designVote = numFromString<int>(m_designInput->getString()).unwrapOr(0);
            designVote = std::clamp(designVote, 1, 10);
      }
      if (m_difficultyInput) {
            difficultyVote = numFromString<int>(m_difficultyInput->getString()).unwrapOr(0);
            difficultyVote = std::clamp(difficultyVote, 1, 10);
      }

      matjson::Value body = matjson::Value::object();
      body["accountId"] = accountId;
      body["argonToken"] = argonToken;
      body["levelId"] = m_levelId;
      body["gameplayScore"] = gameplayVote;
      body["designScore"] = designVote;
      body["difficultyScore"] = difficultyVote;

      Ref<RLCommunityVotePopup> selfRef = this;
      auto req = web::WebRequest();
      req.bodyJSON(body);
      req.post("https://gdrate.arcticwoof.xyz/setSuggestScore").listen([selfRef](web::WebResponse* res) {
            if (!selfRef) return;
            if (!res || !res->ok()) {
                  Notification::create("Failed to submit vote", NotificationIcon::Error)->show();
                  return;
            }
            auto j = res->json();
            if (!j) {
                  Notification::create("Invalid submit response", NotificationIcon::Warning)->show();
                  return;
            }
            auto json = j.unwrap();
            bool success = json["success"].asBool().unwrapOrDefault();
            if (success) {
                  Notification::create("Vote submitted!", NotificationIcon::Success)->show();
                  selfRef->removeFromParent();
            } else {
                  Notification::create("Vote submission failed", NotificationIcon::Error)->show();
            }
      });
}

bool RLCommunityVotePopup::setup() {
      setTitle("Community Vote");
      addSideArt(m_mainLayer, SideArt::All, SideArtStyle::PopupBlue, false);
      float startY = m_mainLayer->getContentSize().height - 70.f;
      float rowH = 40.f;

      // design row
      auto designRow = CCLayer::create();
      designRow->setContentSize({m_mainLayer->getContentSize().width - 40.f, rowH});
      designRow->setPosition({20.f, startY});
      designRow->setAnchorPoint({0.f, 0.5f});
      m_mainLayer->addChild(designRow);

      auto designLabel = CCLabelBMFont::create("Design", "chatFont.fnt");
      designLabel->setAnchorPoint({0.f, 0.5f});
      designLabel->setPosition({0.f, rowH / 2.f});
      designRow->addChild(designLabel);

      m_designScoreLabel = CCLabelBMFont::create("-", "bigFont.fnt");
      m_designScoreLabel->setAnchorPoint({0.5f, 0.5f});
      m_designScoreLabel->setPosition({designRow->getContentSize().width / 2.f, rowH / 2.f});
      m_designScoreLabel->setScale(0.7f);
      designRow->addChild(m_designScoreLabel);

      // replace up/down buttons with a numeric text input (expects -1,0,1)
      m_designInput = TextInput::create(80.f, "1-10");
      m_designInput->setPosition({designRow->getContentSize().width - 40.f, rowH / 2.f});
      m_designInput->setID("design-vote-input");
      m_designInput->setCommonFilter(CommonFilter::Int);
      designRow->addChild(m_designInput);

      m_designVote = numFromString<int>(m_designInput->getString()).unwrapOr(0);

      // difficulty row
      auto difficultyRow = CCLayer::create();
      difficultyRow->setContentSize({m_mainLayer->getContentSize().width - 40.f, rowH});
      difficultyRow->setPosition({20.f, startY - rowH - 8.f});
      difficultyRow->setAnchorPoint({0.f, 0.5f});
      m_mainLayer->addChild(difficultyRow);

      auto diffLabel = CCLabelBMFont::create("Difficulty", "chatFont.fnt");
      diffLabel->setAnchorPoint({0.f, 0.5f});
      diffLabel->setPosition({0.f, rowH / 2.f});
      difficultyRow->addChild(diffLabel);

      m_difficultyScoreLabel = CCLabelBMFont::create("-", "bigFont.fnt");
      m_difficultyScoreLabel->setAnchorPoint({0.5f, 0.5f});
      m_difficultyScoreLabel->setPosition({difficultyRow->getContentSize().width / 2.f, rowH / 2.f});
      m_difficultyScoreLabel->setScale(0.7f);
      difficultyRow->addChild(m_difficultyScoreLabel);

      // numeric input for difficulty vote
      m_difficultyInput = TextInput::create(80.f, "1-30");
      m_difficultyInput->setPosition({difficultyRow->getContentSize().width - 40.f, rowH / 2.f});
      m_difficultyInput->setID("difficulty-vote-input");
      m_difficultyInput->setCommonFilter(CommonFilter::Int);
      difficultyRow->addChild(m_difficultyInput);

      m_difficultyVote = numFromString<int>(m_difficultyInput->getString()).unwrapOr(0);

      // gameplay row
      auto gameplayRow = CCLayer::create();
      gameplayRow->setContentSize({m_mainLayer->getContentSize().width - 40.f, rowH});
      gameplayRow->setPosition({20.f, startY - (rowH + 8.f) * 2.f});
      gameplayRow->setAnchorPoint({0.f, 0.5f});
      m_mainLayer->addChild(gameplayRow);

      auto gpLabel = CCLabelBMFont::create("Gameplay", "chatFont.fnt");
      gpLabel->setAnchorPoint({0.f, 0.5f});
      gpLabel->setPosition({0.f, rowH / 2.f});
      gameplayRow->addChild(gpLabel);

      m_gameplayScoreLabel = CCLabelBMFont::create("-", "bigFont.fnt");
      m_gameplayScoreLabel->setAnchorPoint({0.5f, 0.5f});
      m_gameplayScoreLabel->setPosition({gameplayRow->getContentSize().width / 2.f, rowH / 2.f});
      m_gameplayScoreLabel->setScale(0.7f);
      gameplayRow->addChild(m_gameplayScoreLabel);

      // numeric input for gameplay vote
      m_gameplayInput = TextInput::create(80.f, "1-10");
      m_gameplayInput->setPosition({gameplayRow->getContentSize().width - 40.f, rowH / 2.f});
      m_gameplayInput->setID("gameplay-vote-input");
      m_gameplayInput->setCommonFilter(CommonFilter::Int);
      gameplayRow->addChild(m_gameplayInput);

      m_gameplayVote = numFromString<int>(m_gameplayInput->getString()).unwrapOr(0);

      // moderators' average difficulty
      auto modRow = CCLayer::create();
      modRow->setContentSize({m_mainLayer->getContentSize().width - 40.f, rowH});
      modRow->setPosition({20.f, startY - (rowH + 8.f) * 3.f});
      modRow->setAnchorPoint({0.f, 0.5f});
      m_mainLayer->addChild(modRow);

      auto modLabel = CCLabelBMFont::create("Moderators' Average Difficulty", "chatFont.fnt");
      modLabel->setAnchorPoint({0.f, 0.5f});
      modLabel->setPosition({0.f, rowH / 2.f});
      modLabel->setScale(0.8f);
      modRow->addChild(modLabel);

      m_modDifficultyLabel = CCLabelBMFont::create("-", "bigFont.fnt");
      m_modDifficultyLabel->setAnchorPoint({0.5f, 0.5f});
      m_modDifficultyLabel->setPosition({modRow->getContentSize().width / 2.f, rowH / 2.f});
      m_modDifficultyLabel->setScale(0.7f);
      modRow->addChild(m_modDifficultyLabel);

      // submit button
      auto submitSpr = ButtonSprite::create("Submit", "goldFont.fnt", "GJ_button_01.png");
      auto submitBtn = CCMenuItemSpriteExtra::create(submitSpr, this, menu_selector(RLCommunityVotePopup::onSubmit));
      submitBtn->setPosition({m_mainLayer->getContentSize().width / 2.f, 0});
      auto submitMenu = CCMenu::create();
      submitMenu->setPosition({0, 0});
      submitMenu->addChild(submitBtn);
      m_mainLayer->addChild(submitMenu, 3);

      // info button
      auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
      auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr, this, menu_selector(RLCommunityVotePopup::onInfo));
      infoBtn->setPosition({25.f, 25.f});
      m_buttonMenu->addChild(infoBtn, 3);

      // fetch current scores from server
      if (m_levelId > 0) {
            Ref<RLCommunityVotePopup> selfRef = this;
            web::WebRequest().get(fmt::format("https://gdrate.arcticwoof.xyz/fetch?levelId={}", m_levelId)).listen([selfRef, submitBtn](web::WebResponse* res) {
                  if (!selfRef) return;
                  if (!res || !res->ok()) {
                        Notification::create("Failed to fetch community vote info", NotificationIcon::Error)->show();
                        return;
                  }
                  auto jres = res->json();
                  if (!jres) {
                        Notification::create("Invalid response for community vote", NotificationIcon::Warning)->show();
                        return;
                  }
                  auto json = jres.unwrap();
                  // update score labels
                  int designScore = json["designScore"].asInt().unwrapOrDefault();
                  int difficultyScore = json["difficultyScore"].asInt().unwrapOrDefault();
                  int gameplayScore = json["gameplayScore"].asInt().unwrapOrDefault();

                  if (selfRef->m_designScoreLabel) selfRef->m_designScoreLabel->setString(numToString(designScore).c_str());
                  if (selfRef->m_difficultyScoreLabel) selfRef->m_difficultyScoreLabel->setString(numToString(difficultyScore).c_str());
                  if (selfRef->m_gameplayScoreLabel) selfRef->m_gameplayScoreLabel->setString(numToString(gameplayScore).c_str());

                  // moderator difficulty (read-only)
                  double avg = -1.0;
                  if (json.contains("averageDifficulty")) {
                        auto avgRes = json["averageDifficulty"].asDouble();
                        if (avgRes) avg = avgRes.unwrap();
                  }

                  if (selfRef->m_modDifficultyLabel) {
                        if (avg >= 0.0) {
                              if (std::floor(avg) == avg) {
                                    selfRef->m_modDifficultyLabel->setString(numToString((int)avg).c_str());
                              } else {
                                    selfRef->m_modDifficultyLabel->setString(fmt::format("{:.1f}", avg).c_str());
                              }
                        } else {
                              selfRef->m_modDifficultyLabel->setString("-");
                        }
                  }

                  // Query whether this account has already voted on this level and
                  int accountId = GJAccountManager::get()->m_accountID;
                  auto argonToken = Mod::get()->getSavedValue<std::string>("argon_token");
                  matjson::Value voteBody = matjson::Value::object();
                  voteBody["accountId"] = accountId;
                  voteBody["argonToken"] = argonToken;
                  voteBody["levelId"] = selfRef->m_levelId;

                  web::WebRequest voteReq;
                  voteReq.bodyJSON(voteBody);
                  voteReq.post("https://gdrate.arcticwoof.xyz/getVote").listen([selfRef, submitBtn](web::WebResponse* vres) {
                        if (!selfRef) return;
                        if (!vres || !vres->ok()) {
                              return;
                        }
                        auto vj = vres->json();
                        if (!vj) return;
                        auto vjson = vj.unwrap();

                        bool hasGameplay = vjson["hasGameplayVoted"].asBool().unwrapOrDefault();
                        bool hasDesign = vjson["hasDesignVoted"].asBool().unwrapOrDefault();
                        bool hasDifficulty = vjson["hasDifficultyVoted"].asBool().unwrapOrDefault();

                        // helpers to create disabled up/down images
                        auto makeDisabledUp = []() -> CircleButtonSprite* {
                              auto sprite = CCSpriteGrayscale::createWithSpriteFrameName("GJ_downloadsIcon_001.png");
                              if (sprite) sprite->setFlipY(true);
                              return CircleButtonSprite::create(static_cast<CCNode*>(sprite), CircleBaseColor::Gray, CircleBaseSize::Small);
                        };
                        auto makeDisabledDown = []() -> CircleButtonSprite* {
                              auto sprite = CCSpriteGrayscale::createWithSpriteFrameName("GJ_downloadsIcon_001.png");
                              if (sprite) {
                                    sprite->setColor({255, 0, 0});
                                    sprite->setFlipX(true);
                              }
                              return CircleButtonSprite::create(static_cast<CCNode*>(sprite), CircleBaseColor::Gray, CircleBaseSize::Small);
                        };

                        if (hasGameplay) {
                              if (selfRef->m_gameplayInput) {
                                    selfRef->m_gameplayInput->setString("VOTED");
                                    selfRef->m_gameplayInput->setEnabled(false);
                              }
                        }

                        if (hasDesign) {
                              if (selfRef->m_designInput) {
                                    selfRef->m_designInput->setString("VOTED");
                                    selfRef->m_designInput->setEnabled(false);
                              }
                        }

                        if (hasDifficulty) {
                              if (selfRef->m_difficultyInput) {
                                    selfRef->m_difficultyInput->setString("VOTED");
                                    selfRef->m_difficultyInput->setEnabled(false);
                              }
                        }

                        // If all three categories have already been voted on, disable submit
                        if (hasGameplay && hasDesign && hasDifficulty) {
                              if (submitBtn) {
                                    submitBtn->setEnabled(false);
                                    submitBtn->setNormalImage(ButtonSprite::create("Submit", "goldFont.fnt", "GJ_button_04.png"));
                              }
                        }
                  });

                  // if the server indicates this level is not suggested, close or remove submit UI
                  bool isSuggested = json["isSuggested"].asBool().unwrapOrDefault();
                  if (!isSuggested) {
                        selfRef->onClose(nullptr);
                        Notification::create("Level is not suggested", NotificationIcon::Warning)->show();
                  }
            });
      }

      return true;
}

void RLCommunityVotePopup::onInfo(CCObject*) {
      MDPopup::create(
          "Community Voting Info",
          "You can vote on <cl>suggested/sent layouts</c> based on three categories:\n\n"
          "<cg>Design</c>: How well-designed and visually appealing the layout is.\n\n"
          "<cg>Difficulty</c>: The difficulty the level is according to your experience.\n\n"
          "<cg>Gameplay</c>: How fun and enjoyable the layout is overall.\n\n"
          "Each category has its own score range:\n"
          "- Design: 1 to 10\n"
          "- Difficulty: 1 to 30\n"
          "- Gameplay: 1 to 10\n\n"
          "Please rate suggested layouts <cg>honestly and fairly</c> based on your experience playing them!",
          "OK")
          ->show();
}
