#include <Geode/Geode.hpp>
#include <Geode/modify/CommentCell.hpp>

using namespace geode::prelude;

class $modify(RLCommentCell, CommentCell)
{
    struct Fields
    {
        int role = 0;
    };

    void loadFromComment(GJComment *comment)
    {
        CommentCell::loadFromComment(comment);

        if (!comment)
        {
            log::warn("Comment is null");
            return;
        }

        fetchUserRole(comment->m_accountID);
    }

    void applyCommentTextColor()
    {
        if (m_fields->role == 0)
        {
            return;
        }

        ccColor3B color;
        if (m_fields->role == 1)
        {
            color = ccc3(0, 150, 255); // mod comment color
        }
        else if (m_fields->role == 2)
        {
            color = ccc3(150, 0, 0); // admin comment color
        }
        else
        {
            return;
        }

        log::info("Applying comment text color for role: {}", m_fields->role);

        if (auto commentTextLabel = typeinfo_cast<CCLabelBMFont *>(m_mainLayer->getChildByIDRecursive("comment-text-label")))
        {
            log::info("Found and coloring comment-text-label");
            commentTextLabel->setColor(color);
            return;
        }

        log::warn("comment-text-label not found, searching all children");

        auto children = m_mainLayer->getChildren();
        if (children)
        {
            log::info("Found {} children in m_mainLayer", children->count());
            CCObject *obj = nullptr;
            CCARRAY_FOREACH(children, obj)
            {
                auto node = typeinfo_cast<CCNode *>(obj);
                if (node)
                {
                    log::info("Child node ID: {}", node->getID());
                }
            }
        }
    }

    void fetchUserRole(int accountId)
    {
        log::info("Fetching role for comment user ID: {}", accountId);
        auto getTask = web::WebRequest()
                           .param("accountId", accountId)
                           .get("https://gdrate.arcticwoof.xyz/commentProfile");

        getTask.listen([this](web::WebResponse *response)
                       {
            log::info("Received role response from server for comment");

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
            int role = json["role"].asInt().unwrapOrDefault();
            m_fields->role = role;

            log::info("User comment role: {}", role);

            this->loadBadgeForComment(); });
    }

    void loadBadgeForComment()
    {
        auto userNameMenu = typeinfo_cast<CCMenu *>(m_mainLayer->getChildByIDRecursive("username-menu"));
        if (!userNameMenu)
        {
            log::warn("username-menu not found in comment cell");
            return;
        }

        if (m_fields->role == 1)
        {
            applyCommentTextColor();

            auto modBadgeSprite = CCSprite::create("rlBadgeMod.png"_spr);
            modBadgeSprite->setScale(0.7f);
            auto modBadgeButton = CCMenuItemSpriteExtra::create(
                modBadgeSprite,
                this,
                menu_selector(RLCommentCell::onModBadge));

            modBadgeButton->setID("rl-comment-mod-badge");
            userNameMenu->addChild(modBadgeButton);
            userNameMenu->updateLayout();
        }
        else if (m_fields->role == 2)
        {
            applyCommentTextColor();

            auto adminBadgeSprite = CCSprite::create("rlBadgeAdmin.png"_spr);
            adminBadgeSprite->setScale(0.7f);
            auto adminBadgeButton = CCMenuItemSpriteExtra::create(
                adminBadgeSprite,
                this,
                menu_selector(RLCommentCell::onAdminBadge));

            adminBadgeButton->setID("rl-comment-admin-badge");
            userNameMenu->addChild(adminBadgeButton);
            userNameMenu->updateLayout();
        }
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