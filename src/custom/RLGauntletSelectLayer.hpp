#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RLGauntletSelectLayer : public CCLayer {
     public:
      static RLGauntletSelectLayer* create();

     protected:
      bool init();
      void onBackButton(CCObject* sender);
      void keyBackClicked() override;
      void fetchGauntlets();
      void onGauntletsFetched(matjson::Value const& json);
      void createGauntletButtons(matjson::Value const& gauntlets);
      void onGauntletButtonClick(CCObject* sender);
      void onInfoButton(CCObject* sender);

     private:
      LoadingSpinner* m_loadingCircle = nullptr;
      CCMenu* m_gauntletsMenu = nullptr;
      matjson::Value m_selectedGauntlet = matjson::Value();
      std::vector<matjson::Value> m_gauntletsArray;
      std::function<void(web::WebResponse*)> m_gauntletsListener;
};