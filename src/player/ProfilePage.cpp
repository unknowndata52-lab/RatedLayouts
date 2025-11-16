#include <Geode/Geode.hpp>
#include <argon/argon.hpp>
#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

class $modify(RLProfilePage, ProfilePage)
{
    struct Fields
    {
        CCLabelBMFont *blueprintStarsCount = nullptr;
        CCLabelBMFont *layoutPointsCount = nullptr;
        int role = 0;
    };

    bool init(int accountID, bool ownProfile)
    {
        if (!ProfilePage::init(accountID, ownProfile))
            return false;

        auto statsMenu = m_mainLayer->getChildByID("stats-menu");
        statsMenu->updateLayout();

        return true;
    }

    void getUserInfoFinished(GJUserScore *score)
    {
        ProfilePage::getUserInfoFinished(score);

        // find the stats menu
        // im trying to recreate how robtop adds the stats in the menu
        // very hacky way to do it but oh well
        auto statsMenu = m_mainLayer->getChildByID("stats-menu");
        if (!statsMenu)
        {
            log::warn("stats-menu not found");
            return;
        }

        auto blueprintStarsContainer = statsMenu->getChildByID("blueprint-stars-container");
        auto layoutPointsContainer = statsMenu->getChildByID("layout-points-container");

        if (blueprintStarsContainer && layoutPointsContainer)
        {
            log::info("Blueprint stars and layout points already exist, skipping creation");
            if (score)
            {
                fetchProfileData(score->m_accountID);
            }
            return;
        }

        log::info("adding the blueprint stars stats");

        auto blueprintStarsCount = CCLabelBMFont::create(
            numToString(0).c_str(),
            "bigFont.fnt");
        blueprintStarsCount->setID("label");
        blueprintStarsCount->setAnchorPoint({1.f, .5f});

        limitNodeSize(blueprintStarsCount, {blueprintStarsCount->getContentSize()}, .7f, .1f); // geode limit node size thingy blep

        auto container = CCNode::create();
        container->setContentSize({blueprintStarsCount->getContentWidth(), 19.5f});
        container->setID("blueprint-stars-container");

        container->addChildAtPosition(blueprintStarsCount, Anchor::Right);
        container->setAnchorPoint({1.f, .5f});
        statsMenu->addChild(container);

        auto blueprintStars = CCSprite::create("rlStarIcon.png"_spr);
        if (blueprintStars)
        {
            blueprintStars->setID("blueprint-stars-icon");
            statsMenu->addChild(blueprintStars);
        }

        auto layoutPointsCount = CCLabelBMFont::create(
            numToString(0).c_str(),
            "bigFont.fnt");
        layoutPointsCount->setID("label");
        layoutPointsCount->setAnchorPoint({1.f, .5f});

        limitNodeSize(layoutPointsCount, {layoutPointsCount->getContentSize()}, .7f, .1f);

        auto layoutContainer = CCNode::create();
        layoutContainer->setContentSize({layoutPointsCount->getContentWidth(), 19.5f});
        layoutContainer->setID("layout-points-container");

        layoutContainer->addChildAtPosition(layoutPointsCount, Anchor::Right);
        layoutContainer->setAnchorPoint({1.f, .5f});
        statsMenu->addChild(layoutContainer);

        auto layoutIcon = CCSprite::create("rlhammerIcon.png"_spr);
        if (layoutIcon)
        {
            layoutIcon->setID("layout-points-icon");
            statsMenu->addChild(layoutIcon);
        }

        m_fields->blueprintStarsCount = blueprintStarsCount;
        m_fields->layoutPointsCount = layoutPointsCount;

        if (score)
        {
            fetchProfileData(score->m_accountID);
        }

        statsMenu->updateLayout();
    }

    void fetchProfileData(int accountId)
    {
        log::info("Fetching profile data for account ID: {}", accountId);

        auto statsMenu = m_mainLayer->getChildByID("stats-menu");

        // argon my beloved <3
        std::string token;
        auto res = argon::startAuth([](Result<std::string> res)
                                    {
        if (!res) {
            log::warn("Auth failed: {}", res.unwrapErr());
            return;
        }
        auto token = std::move(res).unwrap();
        log::debug("token obtained: {}", token);
        Mod::get()->setSavedValue("argon_token", token); }, [](argon::AuthProgress progress)
                                    { log::debug("auth progress: {}", argon::authProgressToString(progress)); });
        if (!res)
        {
            log::warn("Failed to start auth attempt: {}", res.unwrapErr());
            return;
        }

        matjson::Value jsonBody = matjson::Value::object();
        jsonBody["argonToken"] = Mod::get()->getSavedValue<std::string>("argon_token");
        jsonBody["accountId"] = accountId;

        // web request
        auto postReq = web::WebRequest();
        postReq.bodyJSON(jsonBody);
        auto postTask = postReq.post("https://gdrate.arcticwoof.xyz/profile");

        // Handle the response
        postTask.listen([this, statsMenu](web::WebResponse *response)
                        {
            log::info("Received response from server");
            
            if (!response->ok()) {
                log::warn("Server returned non-ok status: {}", response->code());
                return;
            }
            
            auto jsonRes = response->json();
            if (!jsonRes) {
                log::warn("Failed to parse JSON response");
                return;
            }
            
            auto json = jsonRes.unwrap();
            int points = json["points"].asInt().unwrapOrDefault();
            int stars = json["stars"].asInt().unwrapOrDefault();
            int role = json["role"].asInt().unwrapOrDefault();
            
            log::info("Profile data - points: {}, stars: {}", points, stars);

            // store the values into the saved value
            Mod::get()->setSavedValue("stars", stars);
            Mod::get()->setSavedValue("points", points);
            m_fields->role = role;
            Mod::get()->setSavedValue("points", points);
            
            // existing stats containers, this is so hacky but wanted to keep it at the right side
            auto blueprintStarsContainer = statsMenu->getChildByID("blueprint-stars-container");
            auto blueprintStarsIcon = statsMenu->getChildByID("blueprint-stars-icon");
            auto layoutPointsContainer = statsMenu->getChildByID("layout-points-container");
            auto layoutPointsIcon = statsMenu->getChildByID("layout-points-icon");
            
            if (blueprintStarsContainer) blueprintStarsContainer->removeFromParent();
            if (blueprintStarsIcon) blueprintStarsIcon->removeFromParent();
            if (layoutPointsContainer) layoutPointsContainer->removeFromParent();
            if (layoutPointsIcon) layoutPointsIcon->removeFromParent();
            
            // label stuff
            auto blueprintStarsCount = CCLabelBMFont::create(
                numToString(stars).c_str(),
                "bigFont.fnt");
            blueprintStarsCount->setID("label");
            blueprintStarsCount->setAnchorPoint({1.f, .5f});
            limitNodeSize(blueprintStarsCount, {blueprintStarsCount->getContentSize()}, .7f, .1f);
            
            auto container = CCNode::create();
            container->setContentSize({blueprintStarsCount->getContentWidth(), 19.5f});
            container->setID("blueprint-stars-container");
            container->addChildAtPosition(blueprintStarsCount, Anchor::Right);
            container->setAnchorPoint({1.f, .5f});
            statsMenu->addChild(container);
            
            auto blueprintStars = CCSprite::create("rlStarIcon.png"_spr);
            if (blueprintStars)
            {
                blueprintStars->setID("blueprint-stars-icon");
                statsMenu->addChild(blueprintStars);
            }
            
            auto layoutPointsCount = CCLabelBMFont::create(
                numToString(points).c_str(),
                "bigFont.fnt");
            layoutPointsCount->setID("label");
            layoutPointsCount->setAnchorPoint({1.f, .5f});
            limitNodeSize(layoutPointsCount, {layoutPointsCount->getContentSize()}, .7f, .1f);
            
            auto layoutContainer = CCNode::create();
            layoutContainer->setContentSize({layoutPointsCount->getContentWidth(), 19.5f});
            layoutContainer->setID("layout-points-container");
            layoutContainer->addChildAtPosition(layoutPointsCount, Anchor::Right);
            layoutContainer->setAnchorPoint({1.f, .5f});
            statsMenu->addChild(layoutContainer);
            
            auto layoutIcon = CCSprite::create("rlhammerIcon.png"_spr);
            if (layoutIcon)
            {
                layoutIcon->setID("layout-points-icon");
                statsMenu->addChild(layoutIcon);
            }
            
            m_fields->blueprintStarsCount = blueprintStarsCount;
            m_fields->layoutPointsCount = layoutPointsCount;
            
            statsMenu->updateLayout();
            
            Mod::get()->setSavedValue("stars", stars);
            Mod::get()->setSavedValue("points", points); });
    }
    // badge
    void loadPageFromUserInfo(GJUserScore *a2)
    {
        // couldve done a more appropriate way but eh
        ProfilePage::loadPageFromUserInfo(a2);
        CCMenu *username_menu = static_cast<CCMenu *>(m_mainLayer->getChildByIDRecursive("username-menu"));

        if (m_fields->role == 1)
        {
            auto modBadgeSprite = CCSprite::create("rlBadgeMod.png"_spr);
            auto modBadgeButton = CCMenuItemSpriteExtra::create(
                modBadgeSprite,
                this, menu_selector(RLProfilePage::onModBadge));

            modBadgeButton->setID("rl-mod-badge");
            username_menu->addChild(modBadgeButton);
        }
        else if (m_fields->role == 2)
        {
            auto adminBadgeSprite = CCSprite::create("rlBadgeAdmin.png"_spr);
            auto adminBadgeButton = CCMenuItemSpriteExtra::create(
                adminBadgeSprite,
                this, menu_selector(RLProfilePage::onAdminBadge));

            adminBadgeButton->setID("rl-admin-badge");
            username_menu->addChild(adminBadgeButton);
        }

        username_menu->updateLayout();
    }

    void onModBadge(CCObject *sender)
    {
        FLAlertLayer::create(
            "Layout Moderator",
            "This user can suggest layout levels for Rated Layouts to the Layout Admins.",
            "OK")
            ->show();
    }

    void onAdminBadge(CCObject *sender)
    {
        FLAlertLayer::create(
            "Layout Administrator",
            "This user can rate layout levels for Rated Layouts. They can change the featured ranking on the featured layout levels.",
            "OK")
            ->show();
    }
};