#include <Geode/Geode.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
      auto packDir = geode::Mod::get()->getResourcesDir();
      auto packPath = packDir.string();
      CCFileUtils::get()->addTexturePack(CCTexturePack{.m_id = Mod::get()->getID(), .m_paths = {packPath}});
      log::info("Added texture pack from {}", packPath);

      // Load sprite frames (plist) for the blue coin if present
      auto plistPath = (packDir / "RL_BlueCoin.plist").string();
      if (std::filesystem::exists(packDir / "RL_BlueCoin.plist")) {
            CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(plistPath.c_str());
            log::info("Loaded sprite frames from {}", plistPath);
      }
}