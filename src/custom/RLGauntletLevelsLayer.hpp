#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RLGauntletLevelsLayer : public CCLayer {
     public:
      static RLGauntletLevelsLayer* create(matjson::Value const& gauntletData);

     protected:
      bool init(matjson::Value const& gauntletData);
      void onBackButton(CCObject* sender);
      void keyBackClicked() override;
      void fetchLevelDetails(int gauntletId);
      void onLevelDetailsFetched(matjson::Value const& json);
      void createLevelButtons(matjson::Value const& levelsData, int gauntletId);
      
     private:
      std::string m_gauntletName;
      CCMenu* m_levelsMenu = nullptr;
      LoadingSpinner* m_loadingCircle = nullptr;
      int m_gauntletId = 0;
};
