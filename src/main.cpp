#include <Geode/Geode.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class ModManagerPopup : public geode::Popup
{
    CCMenuItemToggler *m_checkbox;

protected:
    bool init(float w, float h)
    {
        if (!Popup::init(w, h))
            return false;

        this->setTitle("Denabler");

        auto disableBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Disable All", "goldFont.fnt", "GJ_button_01.png", 1.0f),
            this,
            menu_selector(ModManagerPopup::onDisableAll));

        auto enableBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Enable All", "goldFont.fnt", "GJ_button_01.png", 1.0f),
            this,
            menu_selector(ModManagerPopup::onEnableAll));

        float margin = w * 0.06f;
        if (margin < 12.f)
            margin = 12.f;
        else if (margin > 40.f)
            margin = 40.f;
        float available = w - (margin * 2);
        if (available < 80.f)
            available = 80.f;
        float gap = w * 0.05f;
        if (gap < 12.f)
            gap = 12.f;
        else if (gap > 30.f)
            gap = 30.f;
        float btnW = disableBtn->getContentSize().width;
        float combined = (btnW * 2.f) + gap;
        float scale = available / combined;
        if (scale > 1.f)
            scale = 1.f;
        else if (scale < 0.5f)
            scale = 0.5f;
        disableBtn->setScale(scale);
        enableBtn->setScale(scale);

        auto menu = CCMenu::create();
        menu->addChild(disableBtn);
        menu->addChild(enableBtn);
        menu->setLayout(RowLayout::create()->setGap(gap));
        menu->updateLayout();

        auto menu2 = CCMenu::create();
        menu2->setLayout(RowLayout::create()->setGap(5)->setAutoGrowAxis(true));
        menu2->setAnchorPoint({0.5f, 0.5f});

        m_checkbox = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(ModManagerPopup::onToggle), 1);
        m_checkbox->setCascadeColorEnabled(true);
        menu2->setContentSize(m_checkbox->getContentSize());
        menu2->addChild(m_checkbox);

        auto label = CCLabelBMFont::create("Only Pinned Mods", "bigFont.fnt");
        label->setScale(0.7f);
        menu2->addChild(label);

        m_mainLayer->addChild(menu2);
        m_mainLayer->addChildAtPosition(menu, Anchor::Center);

        menu2->setPosition({m_size.width / 2,
                            menu->getPositionY() - menu->getScaledContentHeight() - 10});
        menu2->updateLayout();

        return true;
    }

    void onToggle(CCObject *)
    {
        log::debug("Toggled: {}", m_checkbox->isToggled());
    }

    void onDisableAll(CCObject *)
    {
        this->onClose(nullptr);

        auto allMods = Loader::get()->getAllMods();
        int disabledCount = 0;
        int failedCount = 0;

        bool disableSelf = Mod::get()->getSettingValue<bool>("disable-self");

        for (auto mod : allMods)
        {
            if ((mod->isInternal() || !mod->isOrWillBeEnabled()) && (m_checkbox->isToggled() && (m_checkbox->isToggled() && !mod->isPinned())))
            {
                continue;
            }

            if (mod->getID() == Mod::get()->getID() && !disableSelf)
            {
                continue;
            }

            auto result = mod->disable();
            if (result.isOk())
            {
                disabledCount++;
            }
            else
            {
                failedCount++;
                log::warn("Failed to disable mod {}: {}", mod->getID(), result.unwrapErr());
            }
        }

        std::string message;
        if (failedCount == 0)
        {
            message = fmt::format("Successfully disabled {} mod(s)!\nRestart required for changes to take effect.", disabledCount);
        }
        else
        {
            message = fmt::format("Disabled {} mod(s), {} failed.\nRestart required for changes to take effect.", disabledCount, failedCount);
        }

        FLAlertLayer::create("Disable All Mods", message, "OK")->show();
    }

    void onEnableAll(CCObject *)
    {
        this->onClose(nullptr);

        auto allMods = Loader::get()->getAllMods();
        int enabledCount = 0;
        int failedCount = 0;

        for (auto mod : allMods)
        {
            if (mod->isInternal() || mod->isOrWillBeEnabled() || (m_checkbox->isToggled() && !mod->isPinned()))
            {
                continue;
            }

            auto result = mod->enable();
            if (result.isOk())
            {
                enabledCount++;
            }
            else
            {
                failedCount++;
                log::warn("Failed to enable mod {}: {}", mod->getID(), result.unwrapErr());
            }
        }

        std::string message;
        if (failedCount == 0)
        {
            message = fmt::format("Successfully enabled {} mod(s)!\nRestart required for changes to take effect.", enabledCount);
        }
        else
        {
            message = fmt::format("Enabled {} mod(s), {} failed.\nRestart required for changes to take effect.", enabledCount, failedCount);
        }

        FLAlertLayer::create("Enable All Mods", message, "OK")->show();
    }

public:
    static ModManagerPopup *create()
    {
        auto ret = new ModManagerPopup();
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float w = winSize.width * 0.75f;
        if (w < 320.f)
            w = 320.f;
        else if (w > 700.f)
            w = 700.f;
        float h = winSize.height * 0.45f;
        if (h < 160.f)
            h = 160.f;
        else if (h > 420.f)
            h = 420.f;
        if (ret->init(w, h))
        {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

#include <Geode/modify/MenuLayer.hpp>
class $modify(MyMenuLayer, MenuLayer)
{
    bool init()
    {
        if (!MenuLayer::init())
        {
            return false;
        }

        auto modButton = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_optionsBtn02_001.png"),
            this,
            menu_selector(MyMenuLayer::onModManagerButton));
        modButton->setScale(1.0f);

        auto bottomMenu = this->getChildByID("bottom-menu");
        if (bottomMenu)
        {
            bottomMenu->addChild(modButton);
            modButton->setID("mod-manager-button"_spr);
            bottomMenu->updateLayout();
        }

        return true;
    }

    void onModManagerButton(CCObject *)
    {
        ModManagerPopup::create()->show();
    }

    void enableAllMods(CCObject *)
    {
        auto allMods = Loader::get()->getAllMods();
        int enabledCount = 0;
        int failedCount = 0;

        for (auto mod : allMods)
        {
            if (mod->isInternal() || mod->isOrWillBeEnabled())
            {
                continue;
            }

            auto result = mod->enable();
            if (result.isOk())
            {
                enabledCount++;
            }
            else
            {
                failedCount++;
                log::warn("Failed to enable mod {}: {}", mod->getID(), result.unwrapErr());
            }
        }

        std::string message;
        if (failedCount == 0)
        {
            message = fmt::format("Successfully enabled {} mod(s)!\nRestart required for changes to take effect.", enabledCount);
        }
        else
        {
            message = fmt::format("Enabled {} mod(s), {} failed.\nRestart required for changes to take effect.", enabledCount, failedCount);
        }

        FLAlertLayer::create("Enable All Mods", message, "OK")->show();
    }

    void disableAllMods(CCObject *)
    {
        auto allMods = Loader::get()->getAllMods();
        int disabledCount = 0;
        int failedCount = 0;

        bool disableSelf = Mod::get()->getSettingValue<bool>("disable-self");

        for (auto mod : allMods)
        {
            if (mod->isInternal() || !mod->isOrWillBeEnabled())
            {
                continue;
            }

            if (mod->getID() == Mod::get()->getID() && !disableSelf)
            {
                continue;
            }

            auto result = mod->disable();
            if (result.isOk())
            {
                disabledCount++;
            }
            else
            {
                failedCount++;
                log::warn("Failed to disable mod {}: {}", mod->getID(), result.unwrapErr());
            }
        }

        std::string message;
        if (failedCount == 0)
        {
            message = fmt::format("Successfully disabled {} mod(s)!\nRestart required for changes to take effect.", disabledCount);
        }
        else
        {
            message = fmt::format("Disabled {} mod(s), {} failed.\nRestart required for changes to take effect.", disabledCount, failedCount);
        }

        FLAlertLayer::create("Disable All Mods", message, "OK")->show();
    }
};