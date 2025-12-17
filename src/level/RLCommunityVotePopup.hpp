#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RLCommunityVotePopup : public geode::Popup<> {
     public:
      static RLCommunityVotePopup* create();

     private:
      bool setup() override;
};