#include "RLGauntletSelectLayer.hpp"

#include <Geode/Geode.hpp>

#include "RLAnnoucementPopup.hpp"
#include "RLGauntletLevelsLayer.hpp"

using namespace geode::prelude;

RLGauntletSelectLayer* RLGauntletSelectLayer::create() {
      auto ret = new RLGauntletSelectLayer();
      if (ret && ret->init()) {
            ret->autorelease();
            return ret;
      }
      delete ret;
      return nullptr;
}

bool RLGauntletSelectLayer::init() {
      if (!CCLayer::init())
            return false;

      auto bg = createLayerBG();
      addChild(bg, -1);
      bg->setColor({50, 50, 50});

      auto winSize = CCDirector::sharedDirector()->getWinSize();

      // title
      auto titleSprite = CCSprite::create("RL_titleGauntlet.png"_spr);
      titleSprite->setPosition({winSize.width / 2, winSize.height - 30});
      titleSprite->setScale(.7f);
      this->addChild(titleSprite, 10);

      // crap
      addSideArt(this, SideArt::All, SideArtStyle::LayerGray, false);

      auto backMenu = CCMenu::create();
      backMenu->setPosition({0, 0});

      auto backButtonSpr =
          CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
      CCMenuItemSpriteExtra* backButton = CCMenuItemSpriteExtra::create(
          backButtonSpr, this, menu_selector(RLGauntletSelectLayer::onBackButton));
      backButton->setPosition({25, winSize.height - 25});
      backMenu->addChild(backButton);
      this->addChild(backMenu);

      this->setKeypadEnabled(true);

      m_loadingCircle = LoadingSpinner::create({100.f});
      m_loadingCircle->setPosition(winSize / 2);
      this->addChild(m_loadingCircle);

      // popup annouce
      auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
      auto infoBtn = CCMenuItemSpriteExtra::create(
          infoSpr, this, menu_selector(RLGauntletSelectLayer::onInfoButton));
      infoBtn->setPosition({25, 25});
      backMenu->addChild(infoBtn);

      fetchGauntlets();
      return true;
}

void RLGauntletSelectLayer::onInfoButton(CCObject* sender) {
      auto announcement = RLAnnoucementPopup::create();
      announcement->show();
}

void RLGauntletSelectLayer::onBackButton(CCObject* sender) {
      CCDirector::sharedDirector()->popSceneWithTransition(
          0.5f, PopTransition::kPopTransitionFade);
}
void RLGauntletSelectLayer::keyBackClicked() { this->onBackButton(nullptr); }

void RLGauntletSelectLayer::onGauntletButtonClick(CCObject* sender) {
      auto button = static_cast<CCNode*>(sender);
      auto parent = button->getParent();
      int index = parent->getChildren()->indexOfObject(button);

      if (index >= 0 && index < (int)m_gauntletsArray.size()) {
            auto gauntlet = m_gauntletsArray[index];
            auto layer = RLGauntletLevelsLayer::create(gauntlet);
            auto scene = CCScene::create();
            scene->addChild(layer);
            CCDirector::sharedDirector()->pushScene(
                CCTransitionFade::create(0.5f, scene));
      }
}

void RLGauntletSelectLayer::fetchGauntlets() {
      web::WebRequest request;
      m_gauntletsListener = [this](web::WebResponse* response) {
            if (response->ok()) {
                  auto jsonRes = response->json();
                  if (jsonRes.isOk()) {
                        this->onGauntletsFetched(jsonRes.unwrap());
                  } else {
                        log::error("Failed to parse JSON: {}", jsonRes.unwrapErr());
                        Notification::create("Failed to parse gauntlets data", NotificationIcon::Error)->show();
                  }
            } else {
                  log::error("Failed to fetch gauntlets: {}", response->string().unwrapOr("Unknown error"));
                  Notification::create("Failed to fetch gauntlets", NotificationIcon::Error)->show();
            }
      };
      request.get("https://gdrate.arcticwoof.xyz/getGauntlets").listen(m_gauntletsListener);
}

void RLGauntletSelectLayer::onGauntletsFetched(matjson::Value const& json) {
      m_loadingCircle->setVisible(false);
      createGauntletButtons(json);
}

void RLGauntletSelectLayer::createGauntletButtons(matjson::Value const& gauntlets) {
      auto winSize = CCDirector::sharedDirector()->getWinSize();

      m_gauntletsMenu = CCMenu::create();
      m_gauntletsMenu->setPosition({0, 0});
      this->addChild(m_gauntletsMenu);

      if (!gauntlets.isArray()) {
            log::error("Expected gauntlets to be an array");
            return;
      }

      auto gauntletsArray = gauntlets.asArray().unwrap();

      // Store gauntlets for later reference
      m_gauntletsArray.clear();
      for (auto& g : gauntletsArray) {
            m_gauntletsArray.push_back(g);
      }

      const int maxButtonsPerRow = 3;
      const float buttonWidth = 110.0f;
      const float buttonHeight = 240.0f;
      const float spacingX = 30.0f;
      const float spacingY = 80.0f;

      // Calculate total width needed for buttons in a row
      float totalButtonsWidth = (std::min((size_t)maxButtonsPerRow, gauntletsArray.size())) * (buttonWidth + spacingX);
      float startX = (winSize.width - totalButtonsWidth) / 2 + buttonWidth / 2 + spacingX / 2;

      float centerY = winSize.height / 2 - 20;

      for (size_t i = 0; i < gauntletsArray.size(); i++) {
            auto gauntlet = gauntletsArray[i];

            if (!gauntlet.isObject()) continue;

            int gauntletId = gauntlet["id"].asInt().unwrapOr(0);
            std::string gauntletName = gauntlet["name"].asString().unwrapOr("Unknown");

            auto gauntletBg = CCScale9Sprite::create("GJ_squareB_01.png");
            gauntletBg->setContentSize({110, 240});

            // name label with line breaks at spaces
            std::string formattedName = gauntletName;
            std::replace(formattedName.begin(), formattedName.end(), ' ', '\n');
            auto nameLabel = CCLabelBMFont::create(formattedName.c_str(), "bigFont.fnt");
            auto nameLabelShadow = CCLabelBMFont::create(formattedName.c_str(), "bigFont.fnt");
            nameLabel->setAlignment(kCCTextAlignmentCenter);
            nameLabel->setPosition({gauntletBg->getContentSize().width / 2, gauntletBg->getContentSize().height - 15});
            nameLabel->setAnchorPoint({0.5f, 1.0f});
            nameLabel->setScale(0.5f);

            nameLabelShadow->setAlignment(kCCTextAlignmentCenter);
            nameLabelShadow->setPosition({nameLabel->getPositionX() + 2, nameLabel->getPositionY() - 2});
            nameLabelShadow->setAnchorPoint({0.5f, 1.0f});
            nameLabelShadow->setColor({0, 0, 0});
            nameLabelShadow->setOpacity(60);
            nameLabelShadow->setScale(0.5f);

            gauntletBg->addChild(nameLabel, 3);
            gauntletBg->addChild(nameLabelShadow, 2);

            // difficulty range label (min-max) with star icon to the right
            int minDiff = gauntlet["min_difficulty"].asInt().unwrapOr(0);
            int maxDiff = gauntlet["max_difficulty"].asInt().unwrapOr(0);
            if (minDiff > 0 || maxDiff > 0) {
                  std::string diffText = fmt::format("{}-{}", minDiff, maxDiff);

                  auto diffLabel = CCLabelBMFont::create(diffText.c_str(), "bigFont.fnt");
                  if (diffLabel) {
                        diffLabel->setAlignment(kCCTextAlignmentCenter);
                        diffLabel->setAnchorPoint({0.5f, 0.5f});
                        diffLabel->setScale(0.45f);
                        diffLabel->setPosition({gauntletBg->getContentSize().width / 2 + 10, 50});
                        diffLabel->setAnchorPoint({1.f, 0.5f});
                        gauntletBg->addChild(diffLabel, 3);

                        // star icon to the right of the difficulty label
                        auto diffStar = CCSprite::create("RL_starSmall.png"_spr);
                        if (diffStar) {
                              diffStar->setAnchorPoint({0.f, 0.5f});
                              diffStar->setPosition({gauntletBg->getContentSize().width / 2 + 15, diffLabel->getPositionY()});
                              gauntletBg->addChild(diffStar, 2);
                        }
                  }
            }

            std::string spriteName = fmt::format("RL_gauntlet-{}.png"_spr, gauntletId);
            auto gauntletSprite = CCSprite::create(spriteName.c_str());
            auto gauntletSpriteShadow = CCSprite::create(spriteName.c_str());
            gauntletSpriteShadow->setColor({0, 0, 0});
            gauntletSpriteShadow->setOpacity(50);
            gauntletSpriteShadow->setScaleY(1.2f);

            gauntletBg->addChild(gauntletSprite, 3);
            gauntletBg->addChild(gauntletSpriteShadow, 2);
            gauntletSprite->setPosition(gauntletBg->getContentSize() / 2);
            gauntletSpriteShadow->setPosition({gauntletSprite->getPositionX(), gauntletSprite->getPositionY() - 6});

            auto button = CCMenuItemSpriteExtra::create(
                gauntletBg,
                this,
                menu_selector(RLGauntletSelectLayer::onGauntletButtonClick));

            int row = i / maxButtonsPerRow;
            int col = i % maxButtonsPerRow;
            float buttonX = startX + col * (buttonWidth + spacingX);
            float buttonY = centerY + row * spacingY;

            button->setPosition({buttonX, buttonY});
            m_gauntletsMenu->addChild(button);
      }
}