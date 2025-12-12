#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RLStarsTotalPopup : public geode::Popup<> {
     public:
      static RLStarsTotalPopup* create(int accountId);

     private:
      bool setup() override;
      int m_accountId = 0;
      CCLabelBMFont* m_resultsLabel = nullptr;
      LoadingSpinner* m_spinner = nullptr;
      CCMenu* m_facesContainer = nullptr;
      RowLayout* m_facesLayout = nullptr;
      std::vector<CCLabelBMFont*> m_countLabels;
      std::vector<GJDifficultySprite*> m_difficultySprites;

      bool m_demonModeActive = false;
      void buildDifficultyUI(const std::unordered_map<int, int>& counts);
      void onDemonToggle(CCObject* sender);
      std::unordered_map<int, int> m_counts;
};