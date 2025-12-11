#include "RLSearchLayer.hpp"

using namespace geode::prelude;

// helper to map rating to difficulty level
static int rl_mapRatingToLevel(int rating) {
      switch (rating) {
            case 1:
                  return -1;
            case 2:
                  return 1;
            case 3:
                  return 2;
            case 4:
            case 5:
                  return 3;
            case 6:
            case 7:
                  return 4;
            case 8:
            case 9:
                  return 5;
            case 10:
                  return 7;
            case 15:
                  return 8;
            case 20:
                  return 6;
            case 25:
                  return 9;
            case 30:
                  return 10;
            default:
                  return 0;
      }
}

bool RLSearchLayer::init() {
      if (!CCLayer::init()) return false;

      auto winSize = CCDirector::sharedDirector()->getWinSize();

      addSideArt(this, SideArt::All, SideArtStyle::Layer, false);

      // create if moving bg disabled
      if (Mod::get()->getSettingValue<bool>("disableBackground") == true) {
            auto bg = createLayerBG();
            addChild(bg, -1);
      }

      // test the ground moving thingy :o
      // idk how gd actually does it correctly but this is close enough i guess
      if (Mod::get()->getSettingValue<bool>("disableBackground") == false) {
            m_bgContainer = CCNode::create();
            m_bgContainer->setContentSize(winSize);
            this->addChild(m_bgContainer, -7);
            std::string bgName = "game_bg_01_001.png";
            auto testBg = CCSprite::create(bgName.c_str());
            if (!testBg) {
                  testBg = CCSprite::create("game_bg_01_001.png");
            }
            if (testBg) {
                  float tileW = testBg->getContentSize().width;
                  int tiles = static_cast<int>(ceil(winSize.width / tileW)) + 2;
                  for (int i = 0; i < tiles; ++i) {
                        auto bgSpr = CCSprite::create(bgName.c_str());
                        if (!bgSpr) bgSpr = CCSprite::create("game_bg_01_001.png");
                        if (!bgSpr) continue;
                        bgSpr->setAnchorPoint({0.f, 0.f});
                        bgSpr->setPosition({i * tileW, 0.f});
                        bgSpr->setColor({40, 125, 255});
                        m_bgContainer->addChild(bgSpr);
                        m_bgTiles.push_back(bgSpr);
                  }
            }

            m_groundContainer = CCNode::create();
            m_groundContainer->setContentSize(winSize);
            this->addChild(m_groundContainer, -5);

            std::string groundName = "groundSquare_01_001.png";
            auto testGround = CCSprite::create(groundName.c_str());
            if (!testGround) testGround = CCSprite::create("groundSquare_01_001.png");
            if (testGround) {
                  float tileW = testGround->getContentSize().width;
                  int tiles = static_cast<int>(ceil(winSize.width / tileW)) + 2;
                  for (int i = 0; i < tiles; ++i) {
                        auto gSpr = CCSprite::create(groundName.c_str());
                        if (!gSpr) gSpr = CCSprite::create("groundSquare_01_001.png");
                        if (!gSpr) continue;
                        gSpr->setAnchorPoint({0.f, 0.f});
                        gSpr->setPosition({i * tileW, -70.f});
                        gSpr->setColor({0, 102, 255});
                        m_groundContainer->addChild(gSpr);
                        m_groundTiles.push_back(gSpr);
                  }
            }

            auto floorLineSpr = CCSprite::createWithSpriteFrameName("floorLine_01_001.png");
            floorLineSpr->setPosition({winSize.width / 2, 58});
            m_groundContainer->addChild(floorLineSpr, -4);
      }

      // back menu
      auto backMenu = CCMenu::create();
      backMenu->setPosition({0, 0});

      auto backSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
      auto backButton = CCMenuItemSpriteExtra::create(
          backSpr, this, menu_selector(RLSearchLayer::onBackButton));
      backButton->setPosition({25, winSize.height - 25});
      backMenu->addChild(backButton);
      this->addChild(backMenu);

      // search input stuff would go here
      m_searchInputMenu = CCMenu::create();
      m_searchInputMenu->setPosition({winSize.width / 2, winSize.height - 30});
      m_searchInputMenu->setContentSize({365.f, 40.f});

      auto searchInputBg = CCScale9Sprite::create("square02_001.png");
      searchInputBg->setContentSize(m_searchInputMenu->getContentSize());
      searchInputBg->setOpacity(100);
      m_searchInputMenu->addChild(searchInputBg, -1);

      m_searchInput = TextInput::create(245.f, "Search Layouts...");
      m_searchInput->setID("search-input");
      m_searchInputMenu->addChild(m_searchInput);
      m_searchInput->setPosition({-50.f, 0});

      auto searchButtonSpr = CCSprite::createWithSpriteFrameName("GJ_longBtn06_001.png");
      auto searchButton = CCMenuItemSpriteExtra::create(
          searchButtonSpr, this, menu_selector(RLSearchLayer::onSearchButton));
      searchButton->setPosition({105.f, 0});
      m_searchInputMenu->addChild(searchButton);

      auto clearButtonSpr = CCSprite::createWithSpriteFrameName("GJ_longBtn07_001.png");
      auto clearButton = CCMenuItemSpriteExtra::create(
          clearButtonSpr, this, menu_selector(RLSearchLayer::onClearButton));
      clearButton->setPosition({155.f, 0});
      m_searchInputMenu->addChild(clearButton);

      this->addChild(m_searchInputMenu);

      // difficulty selection
      m_difficultyFilterMenu = CCMenu::create();
      m_difficultyFilterMenu->setPosition({winSize.width / 2, winSize.height - 110});
      m_difficultyFilterMenu->setContentSize({400.f, 80.f});
      // difficulty groups: singletons and grouped ratings
      std::vector<std::vector<int>> groups = {{1}, {2}, {3}, {4, 5}, {6, 7}, {8, 9}};
      m_difficultyGroups = groups;
      // initialize selection vector with number of groups
      m_difficultySelected.assign(static_cast<int>(groups.size()), false);

      // helper to map rating values to engine difficultyLevel
      auto mapRatingToLevel = [](int rating) -> int {
            switch (rating) {
                  case 1:
                        return -1;
                  case 2:
                        return 1;
                  case 3:
                        return 2;
                  case 4:
                  case 5:
                        return 3;
                  case 6:
                  case 7:
                        return 4;
                  case 8:
                  case 9:
                        return 5;
                  default:
                        return 0;
            }
      };

      int groupCount = static_cast<int>(groups.size());
      float spacing = 60.0f;
      float startX = -spacing * (groupCount - 1) / 2.0f;
      for (int gi = 0; gi < groupCount; ++gi) {
            auto& group = groups[gi];
            if (group.empty()) continue;
            int rep = group[0];
            int difficultyLevel = mapRatingToLevel(rep);

            // Use Long variant for normal difficulties by default (per user request)
            auto difficultySprite = GJDifficultySprite::create(difficultyLevel, GJDifficultyName::Long);
            if (!difficultySprite) continue;
            difficultySprite->updateDifficultyFrame(difficultyLevel, GJDifficultyName::Long);
            // unselected default color
            difficultySprite->setColor({125, 125, 125});

            std::string id;
            if (group.size() == 1) {
                  id = "rl-difficulty-" + numToString(group[0]);
            } else {
                  id = "rl-difficulty-" + numToString(group.front()) + "-" + numToString(group.back());
            }

            // position the menu item
            float x = startX + gi * spacing;
            difficultySprite->setPosition({x, 0});
            difficultySprite->setScale(1.0f);
            difficultySprite->setID(id.c_str());

            auto menuItem = CCMenuItemSpriteExtra::create(difficultySprite, this, menu_selector(RLSearchLayer::onDifficultyButton));
            menuItem->setPosition({x, 0});
            menuItem->setTag(gi + 1);
            menuItem->setScale(1.0f);
            menuItem->setID(id.c_str());
            m_difficultyFilterMenu->addChild(menuItem);
            m_difficultyMenuItems.push_back(menuItem);
            m_difficultySprites.push_back(difficultySprite);
      }
      auto difficultyMenuBg = CCScale9Sprite::create("square02_001.png");
      difficultyMenuBg->setContentSize(m_difficultyFilterMenu->getContentSize());
      difficultyMenuBg->setPosition(m_difficultyFilterMenu->getPosition());
      difficultyMenuBg->setOpacity(100);
      this->addChild(difficultyMenuBg, -1);

      // Create demon difficulties menu (10, 15, 20, 25, 30)
      m_demonFilterMenu = CCMenu::create();
      m_demonFilterMenu->setPosition(m_difficultyFilterMenu->getPosition());
      m_demonFilterMenu->setContentSize(m_difficultyFilterMenu->getContentSize());

      std::vector<int> demonRatings = {10, 15, 20, 25, 30};
      m_demonSelected.assign(static_cast<int>(demonRatings.size()), false);
      float demonSpacing = 60.0f;
      float demonStartX = -demonSpacing * (static_cast<int>(demonRatings.size()) - 1) / 2.0f;
      for (int di = 0; di < static_cast<int>(demonRatings.size()); ++di) {
            int rating = demonRatings[di];
            int difficultyLevel = 0;
            bool isDemon = false;
            switch (rating) {
                  case 10:
                        difficultyLevel = 7;
                        isDemon = true;
                        break;
                  case 15:
                        difficultyLevel = 8;
                        isDemon = true;
                        break;
                  case 20:
                        difficultyLevel = 6;
                        isDemon = true;
                        break;
                  case 25:
                        difficultyLevel = 9;
                        isDemon = true;
                        break;
                  case 30:
                        difficultyLevel = 10;
                        isDemon = true;
                        break;
                  default:
                        difficultyLevel = 0;
                        break;
            }
            auto demonSprite = GJDifficultySprite::create(difficultyLevel, GJDifficultyName::Long);
            if (!demonSprite) continue;
            demonSprite->updateDifficultyFrame(difficultyLevel, GJDifficultyName::Long);
            demonSprite->setColor({125, 125, 125});
            float x = demonStartX + di * demonSpacing;
            demonSprite->setPosition({x, 0});
            demonSprite->setScale(1.0f);
            std::string id = "rl-demon-difficulty-" + numToString(rating);
            demonSprite->setID(id.c_str());
            auto demonItem = CCMenuItemSpriteExtra::create(demonSprite, this, menu_selector(RLSearchLayer::onDemonDifficultyButton));
            demonItem->setPosition({x, 0});
            demonItem->setTag(di + 1);
            demonItem->setScale(1.0f);
            demonItem->setID(id.c_str());
            m_demonFilterMenu->addChild(demonItem);
            m_demonMenuItems.push_back(demonItem);
            m_demonSprites.push_back(demonSprite);
      }
      m_demonFilterMenu->setVisible(false);
      this->addChild(m_demonFilterMenu);

      // Demon toggle button
      auto demonOff = CCSpriteGrayscale::createWithSpriteFrameName("GJ_demonIcon_001.png");
      auto demonOn = CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png");
      auto demonToggle = CCMenuItemToggler::create(demonOff, demonOn, this, menu_selector(RLSearchLayer::onDemonToggle));
      demonToggle->setScale(1.0f);
      demonToggle->setID("demon-toggle");

      auto toggleMenu = CCMenu::create();
      toggleMenu->setPosition(m_difficultyFilterMenu->getPosition());
      // position at the right edge
      float toggleX = m_difficultyFilterMenu->getContentSize().width / 2 + 30.0f;
      demonToggle->setPosition({toggleX, 0});
      toggleMenu->addChild(demonToggle);
      this->addChild(toggleMenu);

      this->addChild(m_difficultyFilterMenu);

      // options filter menu
      auto optionsMenu = CCMenu::create();
      optionsMenu->setPosition({winSize.width / 2, 20});
      optionsMenu->setAnchorPoint({0.f, 0.5f});
      optionsMenu->setContentSize({400.f, 140.f});
      this->addChild(optionsMenu);
      auto optionsMenuBg = CCScale9Sprite::create("square02_001.png");
      optionsMenuBg->setContentSize(optionsMenu->getContentSize());
      optionsMenuBg->setPosition({0, optionsMenu->getContentSize().height / 2});
      optionsMenuBg->setOpacity(100);
      optionsMenu->addChild(optionsMenuBg);

      this->setKeypadEnabled(true);
      this->scheduleUpdate();

      return true;
}

void RLSearchLayer::onSearchButton(CCObject* sender) {
      // implement search functionality here
}

void RLSearchLayer::onDemonToggle(CCObject* sender) {
      auto toggler = static_cast<CCMenuItemToggler*>(sender);
      if (!toggler) return;
      // toggle state
      m_demonModeActive = !m_demonModeActive;
      // show/hide menus
      if (m_demonModeActive) {
            m_demonFilterMenu->setVisible(true);
            m_difficultyFilterMenu->setVisible(false);
            // clear normal selection state entirely
            for (size_t _i = 0; _i < m_difficultySelected.size(); ++_i) m_difficultySelected[_i] = false;
            for (size_t _i = 0; _i < m_difficultySprites.size(); ++_i) {
                  auto sprite = m_difficultySprites[_i];
                  auto mi = (_i < m_difficultyMenuItems.size() ? m_difficultyMenuItems[_i] : nullptr);
                  if (sprite) {
                        sprite->setColor({125, 125, 125});
                  }
            }
      } else {
            m_demonFilterMenu->setVisible(false);
            m_difficultyFilterMenu->setVisible(true);
            // clear demon selection state entirely
            for (size_t _i = 0; _i < m_demonSelected.size(); ++_i) m_demonSelected[_i] = false;
            for (size_t _i = 0; _i < m_demonSprites.size(); ++_i) {
                  auto sprite = m_demonSprites[_i];
                  if (sprite) {
                        sprite->setColor({125, 125, 125});
                  }
            }
      }
      // if switching back to normal, update the normal sprites to long
      if (!m_demonModeActive) {
            for (int gi = 0; gi < static_cast<int>(m_difficultySprites.size()); ++gi) {
                  auto sprite = m_difficultySprites[gi];
                  if (!sprite) continue;
                  auto& group = m_difficultyGroups[gi];
                  if (group.empty()) continue;
                  int rep = group[0];
                  int difficultyLevel = rl_mapRatingToLevel(rep);
                  sprite->updateDifficultyFrame(difficultyLevel, GJDifficultyName::Long);
                  sprite->setColor({125, 125, 125});
            }
      } else {
            // when switching to demon we might want to clear normal selection
            for (int gi = 0; gi < static_cast<int>(m_difficultySprites.size()); ++gi) {
                  auto sprite = m_difficultySprites[gi];
                  if (!sprite) continue;
                  // revert to short or leave as-is
                  auto& group = m_difficultyGroups[gi];
                  if (group.empty()) continue;
                  int rep = group[0];
                  int difficultyLevel = rl_mapRatingToLevel(rep);
                  sprite->updateDifficultyFrame(difficultyLevel, GJDifficultyName::Short);
                  sprite->setColor({125, 125, 125});
            }
      }
}

void RLSearchLayer::onDemonDifficultyButton(CCObject* sender) {
      auto item = static_cast<CCMenuItemSpriteExtra*>(sender);
      if (!item) return;
      int tag = item->getTag();
      if (tag < 1 || tag > static_cast<int>(m_demonSelected.size())) return;
      int idx = tag - 1;
      m_demonSelected[idx] = !m_demonSelected[idx];
      auto sprite = m_demonSprites[idx];
      if (!sprite) return;
      if (m_demonSelected[idx]) {
            sprite->setColor({255, 255, 255});
      } else {
            sprite->setColor({125, 125, 125});
      }
}

void RLSearchLayer::onDifficultyButton(CCObject* sender) {
      auto item = static_cast<CCMenuItemSpriteExtra*>(sender);
      if (!item) return;
      int tag = item->getTag();
      if (tag < 1 || tag > static_cast<int>(m_difficultySelected.size())) return;
      int idx = tag - 1;
      // toggle state
      m_difficultySelected[idx] = !m_difficultySelected[idx];
      auto sprite = m_difficultySprites[idx];
      if (!sprite) return;
      if (m_difficultySelected[idx]) {
            // selected, highlight
            sprite->setColor({255, 255, 255});
      } else {
            // unselected, gray out
            sprite->setColor({125, 125, 125});
      }
}

void RLSearchLayer::onClearButton(CCObject* sender) {
      // clear the text input
      if (m_searchInput) {
            m_searchInput->setString("");
      }
}

void RLSearchLayer::keyBackClicked() { onBackButton(nullptr); }

void RLSearchLayer::onBackButton(CCObject* sender) {
      CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

void RLSearchLayer::update(float dt) {
      if (m_bgTiles.size()) {
            float move = m_bgSpeed * dt;
            int num = static_cast<int>(m_bgTiles.size());
            for (auto spr : m_bgTiles) {
                  if (!spr) continue;
                  float tileW = spr->getContentSize().width;
                  float x = spr->getPositionX();
                  x -= move;
                  if (x <= -tileW) {
                        x += tileW * num;
                  }
                  spr->setPositionX(x);
            }
      }

      if (m_groundTiles.size()) {
            float move = m_groundSpeed * dt;
            int num = static_cast<int>(m_groundTiles.size());
            for (auto spr : m_groundTiles) {
                  if (!spr) continue;
                  float tileW = spr->getContentSize().width;
                  float x = spr->getPositionX();
                  x -= move;
                  if (x <= -tileW) {
                        x += tileW * num;
                  }
                  spr->setPositionX(x);
            }
      }
}

RLSearchLayer* RLSearchLayer::create() {
      RLSearchLayer* layer = new RLSearchLayer();
      if (layer && layer->init()) {
            layer->autorelease();
            return layer;
      }
      CC_SAFE_DELETE(layer);
      return nullptr;
}
