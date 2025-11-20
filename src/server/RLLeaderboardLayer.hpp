#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RLLeaderboardLayer : public CCLayer {
protected:
    GJListLayer* m_listLayer;
    ScrollLayer* m_scrollLayer;
    LoadingSpinner* m_spinner;
    TabButton* m_starsTab;
    TabButton* m_creatorTab;
    
    bool init();
    void onBackButton(CCObject* sender);
    void onLeaderboardTypeButton(CCObject* sender);
    void onAccountClicked(CCObject* sender);
    void fetchLeaderboard(int type, int amount);
    void populateLeaderboard(const std::vector<matjson::Value>& users);
    void onInfoButton(CCObject* sender);
    void keyBackClicked() override;

public:
    static RLLeaderboardLayer* create();
};
