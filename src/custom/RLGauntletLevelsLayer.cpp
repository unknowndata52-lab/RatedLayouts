#include "RLGauntletLevelsLayer.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

RLGauntletLevelsLayer* RLGauntletLevelsLayer::create(matjson::Value const& gauntletData) {
      auto ret = new RLGauntletLevelsLayer();
      if (ret && ret->init(gauntletData)) {
            ret->autorelease();
            return ret;
      }
      delete ret;
      return nullptr;
}

bool RLGauntletLevelsLayer::init(matjson::Value const& gauntletData) {
      if (!CCLayer::init())
            return false;

      auto bg = createLayerBG();
      addChild(bg, -1);
      bg->setColor({50, 50, 50});

      auto winSize = CCDirector::sharedDirector()->getWinSize();

      m_gauntletName = gauntletData["name"].asString().unwrapOr("Unknown");
      m_gauntletId = gauntletData["id"].asInt().unwrapOr(0);

      auto titleLabel = CCLabelBMFont::create(m_gauntletName.c_str(), "goldFont.fnt");
      titleLabel->setPosition({winSize.width / 2, winSize.height - 20});
      this->addChild(titleLabel);

      auto backMenu = CCMenu::create();
      backMenu->setPosition({0, 0});

      auto backButtonSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
      CCMenuItemSpriteExtra* backButton = CCMenuItemSpriteExtra::create(
          backButtonSpr, this, menu_selector(RLGauntletLevelsLayer::onBackButton));
      backButton->setPosition({25, winSize.height - 25});
      backMenu->addChild(backButton);
      this->addChild(backMenu);

      this->setKeypadEnabled(true);

      m_loadingCircle = LoadingSpinner::create({100.f});
      m_loadingCircle->setPosition(winSize / 2);
      this->addChild(m_loadingCircle);

      // Fetch level details
      fetchLevelDetails(m_gauntletId);

      return true;
}

void RLGauntletLevelsLayer::fetchLevelDetails(int gauntletId) {
      web::WebRequest request;
      matjson::Value postData;
      postData["id"] = gauntletId;

      request.bodyJSON(postData);
      auto task = request.post("https://gdrate.arcticwoof.xyz/getLevelsGauntlets");
      task.listen([this](web::WebResponse* response) {
            if (response->ok()) {
                  auto jsonRes = response->json();
                  if (jsonRes.isOk()) {
                        onLevelDetailsFetched(jsonRes.unwrap());
                  } else {
                        log::error("Failed to parse level details JSON: {}", jsonRes.unwrapErr());
                        Notification::create("Failed to parse level data", NotificationIcon::Error)->show();
                        if (m_loadingCircle) {
                              m_loadingCircle->setVisible(false);
                        }
                  }
            } else {
                  log::error("Failed to fetch level details: {}", response->string().unwrapOr("Unknown error"));
                  Notification::create("Failed to fetch level details", NotificationIcon::Error)->show();
                  if (m_loadingCircle) {
                        m_loadingCircle->setVisible(false);
                  }
            }
      });
}

void RLGauntletLevelsLayer::onLevelDetailsFetched(matjson::Value const& json) {
      if (m_loadingCircle) {
            m_loadingCircle->setVisible(false);
      }

      createLevelButtons(json, m_gauntletId);
}

void RLGauntletLevelsLayer::createLevelButtons(matjson::Value const& levelsData, int gauntletId) {
      auto winSize = CCDirector::sharedDirector()->getWinSize();

      m_levelsMenu = CCMenu::create();
      m_levelsMenu->setPosition({0, 0});
      this->addChild(m_levelsMenu);

      if (!levelsData.isArray()) {
            log::error("Expected levels data to be an array");
            return;
      }

      auto levelsArray = levelsData.asArray().unwrap();

      const float buttonWidth = 120.0f;
      const float spacingX = 40.0f;

      size_t validCount = 0;
      for (auto& v : levelsArray)
            if (v.isObject()) ++validCount;
      if (validCount == 0) return;

      float totalWidth = validCount * buttonWidth + (validCount - 1) * spacingX;
      float startX = (winSize.width - totalWidth) / 2 + buttonWidth / 2;
      float centerY = winSize.height / 2;

      size_t placedIndex = 0;
      for (size_t i = 0; i < levelsArray.size(); i++) {
            auto level = levelsArray[i];

            if (!level.isObject()) continue;

            int levelId = level["levelid"].asInt().unwrapOr(0);
            std::string levelName = level["levelname"].asString().unwrapOr("Unknown");
            int difficulty = level["difficulty"].asInt().unwrapOr(0);

            std::string gauntletName = fmt::format("RL_gauntlet-{}.png"_spr, gauntletId);
            auto gauntletSprite = CCSprite::create(gauntletName.c_str());

            auto nameLabel = CCLabelBMFont::create(levelName.c_str(), "bigFont.fnt");
            nameLabel->setAlignment(kCCTextAlignmentCenter);
            nameLabel->setAnchorPoint({0.5f, 1.0f});
            nameLabel->setPosition({gauntletSprite->getContentSize().width / 2, gauntletSprite->getContentSize().height + 15});
            nameLabel->limitLabelWidth(100.0f, 0.35f, 0.1f);
            gauntletSprite->addChild(nameLabel);

            auto difficultyLabel = CCLabelBMFont::create(numToString(difficulty).c_str(), "bigFont.fnt");
            difficultyLabel->setAnchorPoint({1.0f, 0.5f});
            difficultyLabel->setScale(0.625f);
            difficultyLabel->setPosition({gauntletSprite->getContentSize().width / 2, -10});
            gauntletSprite->addChild(difficultyLabel);

            auto starSpr = CCSprite::create("RL_starSmall.png"_spr);
            starSpr->setAnchorPoint({0.0f, 0.5f});
            starSpr->setPosition({difficultyLabel->getPositionX() + 6, -10});
            gauntletSprite->addChild(starSpr);

            auto button = CCMenuItemSpriteExtra::create(
                gauntletSprite,
                this,
                nullptr  // TODO: Add callback to play level
            );

            float buttonX = startX + placedIndex * (buttonWidth + spacingX);
            float buttonY = centerY;

            button->setPosition({buttonX, buttonY});
            m_levelsMenu->addChild(button);

            ++placedIndex;
      }
}

void RLGauntletLevelsLayer::onBackButton(CCObject* sender) {
      CCDirector::sharedDirector()->popSceneWithTransition(
          0.5f, PopTransition::kPopTransitionFade);
}

void RLGauntletLevelsLayer::keyBackClicked() { this->onBackButton(nullptr); }
