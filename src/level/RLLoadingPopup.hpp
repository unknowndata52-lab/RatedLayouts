#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RLLoadingPopup : public geode::Popup<> {
     public:
      static RLLoadingPopup* create();

     private:
      bool setup() override;
};