#ifndef STREET_MAP
#define STREET_MAP

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.inl>
#include <glm/gtx/io.hpp>

#include <functional>
#include <vector>
#include <numeric>
#include <memory>

#include "Application.h"
#include "platform.h"
#include "entity/Entity.h"
#include "entity/CameraActor.h"
#include "entity/Shader.h"
#include "tween.h"
#include "Script.h"
#include "data/Material.h"
#include "data/Hit.h"
#include "data/Image.h"
#include "alg/search.h"
#include "interface/IProcessable.h"
#include "interface/IHierarchy.h"
#include "interface/IListenable.h"
#include "shader/sun.inl"
#include "shader/gbuffer.inl"
#include "shader/ssao.inl"
#include "shader/blur.inl"
#include "shader/deffered.inl"
#include "shader/image.inl"
#include "surface.h"
#include "ui/ui.h"
#include "primitive.h"
#include "Pipeline.h"
#include "priority.h"
#include "utils.inl"

namespace script::street {

using glm::mat4;
using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::quat;
using namespace engine::entity;
using engine::Application;
using engine::Script;
using engine::Pipeline;
using engine::data::Material;
using engine::data::Hit;
using engine::data::Image;
using engine::ds::Signal;
using engine::primitive::Quad;
using engine::tween::Animation;
using engine::tween::PropertyAnimation;
using engine::tween::SequenceAnimation;
using namespace engine;
using namespace std;
using namespace engine::utils;
using namespace engine::algorithm;

/// @todo move to data package
struct Trail {
    int segment = 0;
    float progress = 0;
    int start = 0;
    int end = 0;
    vec3 pos;
    vector<int> path;
};

struct Item {
    string label;
    vec3 pos;
    quat rot;
};

struct ItemEntity {
    string label;
    vec3 pos;
    quat rot;
    vec4 col{1.0f, 1.0f, 1.0f, 1.0f};
    int index = 0;
};

namespace Icon {
    enum Icon {
        Road = 0,
        Pin = 1,
        Parking = 2,
        Panorama = 3,
        Shop = 4,
        Person = 5,
        Camera = 6,
        Accident = 7,
        Fly = 8,
        Compass = 9,
        Navigation = 10,
        Minus = 11,
        Plus = 12,
        Tutorial = 13,
        Github = 14
    };
}

struct Context: Entity {
protected:
    Application *app;

    friend struct Scene;
    friend class StreetMap;

public:
    struct State *viewer = nullptr;   /// @todo remove
    struct ModeAction *defaultAction;
    struct ButtonAction *tutorialAction;
    struct Trail *trail = nullptr;
    Signal<Pipeline*> *scriptablePipeline;
    GLFWwindow *window;
    unsigned w, h;

    void delay(std::function<void()> f) {
        app->delay(f);
    }

    void pushAnimation(Animation *animation) {
        app->root.tween.children.push_back(animation);
    }

    // void popAnimation(Animation *animation) {
    //     app->root.tween.children.erase(std::find(app->root.tween.children.begin(), app->root.tween.children.end(), animation));
    // }

    void clearAnimations() {
        /// @todo unique_ptr
        app->root.tween.children = list<Animation*>();
    }

    void clearPipeline() {
        /// @todo unique_ptr
        app->scriptablePipeline = Signal<Pipeline*>();
    }
};


struct State: Entity, IStrongHierarchy<State> {
    State *parent = nullptr;
    std::vector<State*> children;

    struct Scene *scene = nullptr;
    Context *context = nullptr;

    State() = default;

    State(struct Scene *scene, Context *context);

    virtual bool update(IListenable *e) { return true; }
};

struct WelcomeState: State {
    SequenceAnimation animation;
    PropertyAnimation fading;
    PropertyAnimation hiding;
    float value = 0.0f;
    float offset = 0.0f;

    WelcomeState(struct Scene *scene, Context *context);

    // virtual ~WelcomeState() {
    //     context->popAnimation(&animation);
    // }

    void onParent() override {
        animation.play();
    }

    void renderGui(Pipeline *pipeline);
};


struct FreeFlyState: State {
    float speed = 3;
    float sensivity = 0.1f;

    FreeFlyState(struct Scene *scene, Context *context);
    bool update(IListenable *e) override;
};

struct RouteState: State {
    float speed = 0.1f;
    float sensivity = 0.02;
    vec3 pos;
    quat rot;
    Trail trail;
    vec2 dragStart{0.0f, 0.0f};

    RouteState(struct Scene *scene, Context *context);

    void onParent() override;
    bool update(IListenable *e) override;

    void select(ItemEntity item);
};

struct PanoramaState: State {
    float sensivity = 0.1f;
    vec3 pos;
    quat rot;

    PropertyAnimation teleportAnimation;

    PanoramaState(struct Scene *scene, Context *context);

    // virtual ~PanoramaState() {
    //     context->popAnimation(&teleportAnimation);
    // }

    void onParent() override;
    bool update(IListenable *e) override;

    void load(ItemEntity panorama);
};

struct NavigationState: State {
    float sensivity = 0.02;
    float speed = 1.0f;
    vec3 pos;
    quat rot;

    PropertyAnimation moveAnimation;

    NavigationState(struct Scene *scene, Context *context);

    // virtual ~NavigationState() {
    //     context->popAnimation(&moveAnimation);
    // }

    void onParent() override;
    void onRemove() override;
    bool update(IListenable *e) override;
    void renderGizmo(Pipeline *pipeline);

    vec3 getPos(int segment);

    void play();
    void pause();
    void stop();
};

// ----------------------------------- Items -----------------------------------

struct ItemList: State {
    bool isEnabled = true;
    vector<ItemEntity> items;
    Image atlas;
    int icon = -1;

    Signal<ItemEntity> onClick;
    Signal<ItemEntity> onClosestClick;

    ItemList(struct Scene *scene, Context *context, Image image, int icon);

    void onParent() override {
        context->scriptablePipeline->subscribe(this, enums::Priority::ProcessPriority::Gizmo, bind(&ItemList::renderGizmo, this, placeholders::_1));
    }

    void onRemove() override {
        context->scriptablePipeline->unsubscribe(this);
    }

    virtual bool update(IListenable *e) override;

    void renderGizmo(Pipeline *pipeline);

    void addItem(ItemEntity item) {
        items.push_back(item);
    }
};


template<typename T>
struct Action: State {
    Signal<T*> onClick;
    Image image;
    vec4 col{1.0f, 1.0f, 1.0f, 1.0f};

protected:
    Action() = default;

    Action(Image image): image(image) {

    }

public:
    void renderGui(engine::ui::Ui *gui) {
        T *derived = static_cast<T*>(this);
        gui->setAtlas(image.id);
        engine::ui::Ui::Handle *handle = gui->getHandle();
        handle->setColor(derived->enabled(gui) ? derived->active(gui) ? vec4(0.0f, 1.0f, 0.0f, 1.0f) : vec4(1.0f, 1.0f, 1.0f, 1.0f) : vec4(1.0f, 1.0f, 1.0f, 0.2f));
        handle->setImage(0, 1, 1, -1)->rect();
        if (handle->clicked(false) && derived->clickable(gui)) onClick.emit(static_cast<T*>(this));
    }

    void invoke() {
        onClick.emit(static_cast<T*>(this));
    }

    float getWidth() {
        return image.w;
    }

    float getHeight() {
        return image.h;
    }
};

struct OptionAction: Action<OptionAction> {
    const char *label;

    OptionAction() = default;

    OptionAction(Image image, const char *label): Action(image), label(label) {
        
    }

    bool enabled(engine::ui::Ui *gui) {
        return std::get<bool>(gui->getProp(label));
    }

    bool active(engine::ui::Ui *gui) {
        return false;
    }

    bool clickable(engine::ui::Ui *gui) {
        return true;
    }

    void toggle(engine::ui::Ui *gui) {
        gui->setProp(label, !std::get<bool>(gui->getProp(label)));
    }
};

struct ModeAction: Action<ModeAction> {
    bool isEnabled = true;
    bool isActive = false;
    vector<ModeAction> *group;

    ModeAction() = default;

    ModeAction(Image image, vector<ModeAction> *group): Action(image), group(group) {
        
    }

    bool enabled(engine::ui::Ui *gui) {
        return isEnabled;
    }

    bool active(engine::ui::Ui *gui) {
        return isActive;
    }

    void activate() {
        if (!isEnabled) return;
        for (auto &action: *group) {
            action.isActive = false;
        }
        isActive = true;
    }

    bool clickable(engine::ui::Ui *gui) {
        return isEnabled;
    }

    void deactivate() {
        isActive = false;
    }

    void enable() {
        isEnabled = true;
    }

    void disable() {
        isEnabled = false;
    }
};

struct ButtonAction: Action<ButtonAction> {
    bool isEnabled = true;

    ButtonAction() = default;

    ButtonAction(Image image): Action(image) {
        
    }

    bool enabled(engine::ui::Ui *gui) {
        return isEnabled;
    }

    bool active(engine::ui::Ui *gui) {
        return false;
    }

    bool clickable(engine::ui::Ui *gui) {
        return true;
    }

    void enable() {
        isEnabled = true;
    }

    void disable() {
        isEnabled = false;
    }
};



struct Tutorial: State {
    enum Page {
        About,
        Layers,
        Modes,
        Controls,
        Info,
        Help
    };

    PropertyAnimation animation;

    vec2 start{0.0f, 0.0f};
    vec2 end{0.0f, 0.0f};
    vec2 pos{0.0f, 0.0f};
    Image *image = nullptr;
    Image *prev = nullptr;
    float opacity = 0.0f;
    float transition = 0.0f;

    int page = 0;

    Tutorial(struct Scene *scene, Context *context);

    // virtual ~Tutorial() {
    //     context->popAnimation(&animation);
    // }

    void renderGui(Pipeline *pipeline);

    void onParent() override;
    void onRemove() override;
};


struct ProfileState: State {
    PropertyAnimation appearAnimation;
    PropertyAnimation disappearAnimation;

    float transition = 0.0f;

    ProfileState(struct Scene *scene, Context *context);

    // virtual ~ProfileState() {
    //     context->popAnimation(&appearAnimation);
    //     context->popAnimation(&disappearAnimation);
    // }

    void renderGui(Pipeline *pipeline);

    void onParent() override;
    void onRemove() override;
};


// ----------------------------------- Data -----------------------------------


struct Light {
    DepthSurface shadow{6096, 6096};
    mat4 projection;
    mat4 view;
    vec4 col{1.0f, 1.0f, 1.0f, 1.0f};

    static Light Sun() {
        return {
            .projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.2f, 10.0f),
            .view = glm::lookAt(vec3(-2.0f, 4.0f, -1.0f), vec3(-2.0f, 4.0f, -1.0f) + vec3(0.487f, -0.696f, 0.525f), vec3(0.0f, 1.0f,  0.0f)),
            .col = vec4(1.0f, 0.18f, 0.0f, 1.0f) * 1.2f
            // .col = vec4(1.0f, 1.0f, 1.0f, 1.0f)
        };
    }
};

struct Scene: Entity {
    // Shaders
    Shader sunShader{engine::shader::sun::vert, engine::shader::sun::frag};
    Shader gbufferShader{engine::shader::gbuffer::vert, engine::shader::gbuffer::frag};
    Shader ssaoShader{engine::shader::ssao::vert, engine::shader::ssao::frag};
    Shader blurBoxShader{engine::shader::blur::box::vert, engine::shader::blur::box::frag};
    Shader defferedShader{engine::shader::deffered::vert, engine::shader::deffered::frag};
    Shader imageShader{engine::shader::image::vert, engine::shader::image::frag};

    // Lights
    Light sun = Light::Sun();

    // Meshes
    vector<MeshActor*> meshes;
    Quad quad;

    // Materials
    map<string, Material> materials;

    // Surfaces
    GBuffer gbuffer;
    ScreenSurface screen;
    /// @todo RenderStack
    Surface pass1;
    Surface pass2;

    // Hierarchy
    Tag renderable;

    // Cameras
    CameraActor camera;

    // Materials
    vec4 skyColor{0.541914f, 0.894068f, 1.0f, 1.0f};
    vec4 ambientColor{0.541914f * 0.7f, 0.894068f * 0.7f, 1.0f * 0.7f, 1.0f};

    // vec4 skyColor{1.0f, 1.0f, 1.0f, 1.0f};
    // vec4 ambientColor{1.0f, 1.0f, 1.0f, 1.0f};

    // UI
    engine::ui::Ui gui;

    // Items
    ItemList *panoramas;
    ItemList *accidents;
    ItemList *cameras;
    ItemList *parking;
    ItemList *persons;
    ItemList *roads;
    ItemList *shops;

    astar::AStar<astar::Node3D> search{euclideanDistance};

    // Images
    Image icons;
    Image modeFreeFly;
    Image optionAccident;
    Image optionCamera;
    Image optionPanorama;
    Image optionParking;
    Image optionFriends;
    Image welcome;
    Image optionShop;
    Image optionRoads;
    Image optionFavorite;
    Image modePanorama;
    Image modeNavigation;
    Image modeRoute;
    Image buttonZoomIn;
    Image buttonZoomOut;
    Image buttonCompass;
    Image buttonGithub;
    Image buttonTutorial;
    Image buttonGo;
    Image tutorialAbout;
    Image tutorialControls;
    Image tutorialHelp;
    Image tutorialInfo;
    Image tutorialLayers;
    Image tutorialModes;
    Image helpScreen;
    Image profileScreen;

    // Actions
    vector<OptionAction> layers;
    vector<ModeAction> modes;
    vector<ButtonAction> controls;
    vector<ButtonAction> info;
    ButtonAction actionGo;

    Scene(Application *app, Context *context, unsigned maxW, unsigned maxH): gbuffer{
        maxW, maxH
    }, screen{
        maxW, maxH
    }, pass1{
        maxW, maxH
    }, pass2{
        maxW, maxH
    } {
        icons = app->userStorage.image("atlas.png");
        optionAccident = app->userStorage.image("gui/OptionAccident.png");
        optionCamera = app->userStorage.image("gui/OptionCamera.png");
        optionPanorama = app->userStorage.image("gui/OptionPanorama.png");
        optionParking = app->userStorage.image("gui/OptionParking.png");
        optionFriends = app->userStorage.image("gui/OptionFriends.png");
        welcome = app->userStorage.image("gui/WelcomeScreen.png");
        optionShop = app->userStorage.image("gui/OptionShop.png");
        optionRoads = app->userStorage.image("gui/OptionRoads.png");
        optionFavorite = app->userStorage.image("gui/OptionFavorite.png");
        modeFreeFly = app->userStorage.image("gui/ModeFreeFly.png");
        modePanorama = app->userStorage.image("gui/ModePanorama.png");
        modeNavigation = app->userStorage.image("gui/ModeNavigation.png");
        modeRoute = app->userStorage.image("gui/ModeRoute.png");
        buttonZoomIn = app->userStorage.image("gui/ButtonZoomIn.png");
        buttonZoomOut = app->userStorage.image("gui/ButtonZoomOut.png");
        buttonCompass = app->userStorage.image("gui/ButtonCompass.png");
        buttonGithub = app->userStorage.image("gui/ButtonGithub.png");
        buttonTutorial = app->userStorage.image("gui/ButtonTutorial.png");
        buttonGo = app->userStorage.image("gui/ButtonGo.png");
        helpScreen = app->userStorage.image("gui/HelpScreen.png");
        profileScreen = app->userStorage.image("gui/ProfileScreen.png");

        actionGo.image = buttonGo;

        tutorialAbout = app->userStorage.image("gui/TutorialAbout.png");
        tutorialControls = app->userStorage.image("gui/TutorialControls.png");
        tutorialHelp = app->userStorage.image("gui/TutorialHelp.png");
        tutorialInfo = app->userStorage.image("gui/TutorialInfo.png");
        tutorialLayers = app->userStorage.image("gui/TutorialLayers.png");
        tutorialModes = app->userStorage.image("gui/TutorialModes.png");

        panoramas = new ItemList(this, context, icons, Icon::Panorama);
        accidents = new ItemList(this, context, icons, Icon::Accident);
        cameras = new ItemList(this, context, icons, Icon::Camera);
        parking = new ItemList(this, context, icons, Icon::Parking);
        persons = new ItemList(this, context, icons, Icon::Person);
        roads = new ItemList(this, context, icons, Icon::Road);
        shops = new ItemList(this, context, icons, Icon::Shop);
    }

    ~Scene() {
        // for (auto it = meshes.begin(); it != meshes.end(); it++) {
        //     delete *it; 
        //     *it = nullptr;
        // }
        // meshes.clear();
        // delete panoramas;
        // delete accidents;
        // delete cameras;
        // delete parking;
        // delete persons;
        // delete roads;
        // delete shops;
    }
};


class StreetMap: public Script {
public:
    Application *app;
    Platform platform;

    State *state;

    FreeFlyState *freeFlyState;
    RouteState *routeState;
    PanoramaState *panoramaState;
    NavigationState *navigationState;
    Tutorial *tutorial;
    ProfileState *profileState;
    WelcomeState *welcomeState;

    Scene *scene;
    Context *context;

    // Animations
    PropertyAnimation compasssAnimation;

    StreetMap(Application *app): app(app) {
        context = new Context{};
        context->app = app;
        context->window = app->window;
        context->scriptablePipeline = &app->scriptablePipeline;

        scene = new Scene(app, context, app->displayWidth, app->displayHeight);

        scene->materials["wall"] = {
            .albedo = {1.0f, 1.0f, 1.0f, 1.0f}
        };
        scene->materials["roof"] = {
            .albedo = {0.29f, 0.25f, 0.21f, 1.0f}
        };
        scene->materials["paths_footway"] = {
            .albedo = {0.0f, 0.25f, 0.21f, 1.0f}
        };
        scene->materials["Floor"] = {
            .albedo = {0.63f, 1.0f, 0.21f, 1.0f}
        };
        
        // scene->materials["Floor"] = {
        //     .albedo = {1.0f, 1.0f, 1.0f, 1.0f}
        // };

        // scene->materials["wall"] = {
        //     .albedo = {1.0f, 1.0f, 1.0f, 1.0f}
        // };
        // scene->materials["roof"] = {
        //     .albedo = {1.0f, 1.0f, 1.0f, 1.0f}
        // };
        // scene->materials["paths_footway"] = {
        //     .albedo = {1.0f, 1.0f, 1.0f, 1.0f}
        // };
        // scene->materials["Floor"] = {
        //     .albedo = {1.0f, 1.0f, 1.0f, 1.0f}
        // };

        scene->camera.view = glm::lookAt(vec3(0.0f, 2.0f, -8.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f));

        state = new State(scene, context);
        freeFlyState = new FreeFlyState(scene, context);
        routeState = new RouteState(scene, context);
        panoramaState = new PanoramaState(scene, context);
        navigationState = new NavigationState(scene, context);
        welcomeState = new WelcomeState(scene, context);
        tutorial = new Tutorial(scene, context);
        profileState = new ProfileState(scene, context);

        loadChunk();

        // Modes
        scene->modes.reserve(32);
        ModeAction &modeFreeFly = scene->modes.emplace_back(scene->modeFreeFly, &scene->modes);
        ModeAction &modePanorama = scene->modes.emplace_back(scene->modePanorama, &scene->modes);
        ModeAction &modeNavigation = scene->modes.emplace_back(scene->modeNavigation, &scene->modes);
        ModeAction &modeRoute = scene->modes.emplace_back(scene->modeRoute, &scene->modes);

        // Layers
        scene->layers.reserve(32);
        {
            scene->layers.emplace_back(scene->optionAccident, "option-accident").onClick.subscribe([this](OptionAction *action) {
                action->toggle(&scene->gui);
            });
            scene->layers.emplace_back(scene->optionCamera, "option-camera").onClick.subscribe([this](OptionAction *action) {
                action->toggle(&scene->gui);
            });
            scene->layers.emplace_back(scene->optionPanorama, "option-panorama").onClick.subscribe([this](OptionAction *action) {
                action->toggle(&scene->gui);
            });
            scene->layers.emplace_back(scene->optionParking, "option-parking").onClick.subscribe([this](OptionAction *action) {
                action->toggle(&scene->gui);
            });
            scene->layers.emplace_back(scene->optionFriends, "option-friends").onClick.subscribe([this](OptionAction *action) {
                action->toggle(&scene->gui);
            });
            scene->layers.emplace_back(scene->optionRoads, "option-roads").onClick.subscribe([this, &modeNavigation](OptionAction *action) {
                if (modeNavigation.active(&scene->gui)) return;
                action->toggle(&scene->gui);
            });
            scene->layers.emplace_back(scene->optionShop, "option-shop").onClick.subscribe([this](OptionAction *action) {
                action->toggle(&scene->gui);
            });
            scene->layers.emplace_back(scene->optionFavorite, "option-favorite").onClick.subscribe([this](OptionAction *action) {
                action->toggle(&scene->gui);
            });
        }


        // Buttons
        /// @note we use reservation so data will not be moved when items is added
        /// @note 32 should be enough
        scene->controls.reserve(32);

        ButtonAction &buttonZoomIn = scene->controls.emplace_back(scene->buttonZoomIn);
        buttonZoomIn.onClick.subscribe([this](ButtonAction *action) {
            scene->gui.setProp("fov", std::max(std::get<float>(scene->gui.getProp("fov")) - 5.0f, 10.0f));
        });

        ButtonAction &buttonZoomOut = scene->controls.emplace_back(scene->buttonZoomOut);
        buttonZoomOut.onClick.subscribe([this](ButtonAction *action) {
            scene->gui.setProp("fov", std::min(std::get<float>(scene->gui.getProp("fov")) + 5.0f, 90.0f));
        });

        ButtonAction &buttonCompass = scene->controls.emplace_back(scene->buttonCompass);
        buttonCompass.onClick.subscribe([this](ButtonAction *action) {
            mat4 src = scene->camera.view;
            vec4 p = getPosition(scene->camera.view);
            p /= p.w;
            compasssAnimation.onFrameChanged = [this, src, p](float progress) {
                scene->camera.view = lerp(src, p, glm::eulerAngleY(glm::radians(-180.0f)), vec3(1.0f, 1.0f, 1.0f), progress);
            };
            compasssAnimation.play();
        });
        compasssAnimation.duration = 0.5f;
        compasssAnimation.easing = enums::Easing::EaseOutCubic;
        context->pushAnimation(&compasssAnimation);

        // Links
        scene->info.reserve(32);
        
        ButtonAction &buttonGithub = scene->info.emplace_back(scene->buttonGithub);
        buttonGithub.onClick.subscribe([this](ButtonAction *action) {
            platform.openUrl("https://github.com/Teslotik");
        });

        ButtonAction &buttonTutorial = scene->info.emplace_back(scene->buttonTutorial);
        buttonTutorial.onClick.subscribe([this](ButtonAction *action) {
            state->addChild(tutorial);
        });

        // Modes

        modeFreeFly.onClick.subscribe([this, &buttonCompass, &modeNavigation](ModeAction *action) {
            context->trail = nullptr;
            state->removeChild(profileState);
            modeNavigation.disable();
            scene->gui.setProp("option-accident", true);
            scene->gui.setProp("option-camera", true);
            scene->gui.setProp("option-panorama", true);
            scene->gui.setProp("option-parking", true);
            scene->gui.setProp("option-friends", true);
            scene->gui.setProp("option-roads", false);
            scene->gui.setProp("option-shop", true);
            setView(freeFlyState);
            action->activate();
            buttonCompass.enable();
        });

        modePanorama.onClick.subscribe([this, &buttonCompass, &modeNavigation](ModeAction *action) {
            vec3 pos = getPosition(scene->camera.view);
            auto it = min_element(scene->panoramas->items.begin(), scene->panoramas->items.end(), [this, pos](const ItemEntity &i, const ItemEntity &r) {
                return glm::distance(i.pos, pos) < glm::distance(r.pos, pos);
            });
            context->trail = nullptr;
            state->removeChild(profileState);
            modeNavigation.disable();
            scene->gui.setProp("option-accident", true);
            scene->gui.setProp("option-camera", true);
            scene->gui.setProp("option-panorama", true);
            scene->gui.setProp("option-parking", true);
            scene->gui.setProp("option-friends", true);
            scene->gui.setProp("option-roads", false);
            scene->gui.setProp("option-shop", true);
            setView(panoramaState);
            panoramaState->load(*it);
            action->activate();
            buttonCompass.enable();
        });
        
        modeNavigation.onClick.subscribe([this, &buttonCompass](ModeAction *action) {
            state->removeChild(profileState);
            scene->gui.setProp("option-accident", true);
            scene->gui.setProp("option-camera", true);
            scene->gui.setProp("option-panorama", false);
            scene->gui.setProp("option-parking", true);
            scene->gui.setProp("option-friends", false);
            scene->gui.setProp("option-roads", false);
            scene->gui.setProp("option-shop", false);
            setView(navigationState);
            action->activate();
            buttonCompass.disable();
        });

        modeRoute.onClick.subscribe([this, &buttonCompass, &modeNavigation](ModeAction *action) {
            state->removeChild(profileState);
            scene->gui.setProp("option-accident", true);
            scene->gui.setProp("option-camera", false);
            scene->gui.setProp("option-panorama", false);
            scene->gui.setProp("option-parking", false);
            scene->gui.setProp("option-friends", false);
            scene->gui.setProp("option-roads", true);
            scene->gui.setProp("option-shop", false);
            setView(routeState);
            action->activate();
            buttonCompass.disable();
        });

        scene->accidents->onClick.subscribe([this](const ItemEntity &item) {
            state->removeChild(profileState);
            state->addChild(profileState);
        });

        scene->cameras->onClick.subscribe([this](const ItemEntity &item) {
            state->removeChild(profileState);
            state->addChild(profileState);
        });
        
        scene->parking->onClick.subscribe([this](const ItemEntity &item) {
            state->removeChild(profileState);
            state->addChild(profileState);
        });

        scene->persons->onClick.subscribe([this](const ItemEntity &item) {
            state->removeChild(profileState);
            state->addChild(profileState);
        });

        scene->shops->onClick.subscribe([this](const ItemEntity &item) {
            state->removeChild(profileState);
            state->addChild(profileState);
        });

        scene->actionGo.onClick.subscribe([this, &modeNavigation](ButtonAction *action) {
            state->removeChild(profileState);
            modeNavigation.invoke();
            action->disable();
            navigationState->play();
        });

        scene->panoramas->onClick.subscribe([this, &modePanorama](const ItemEntity &item) {
            state->removeChild(profileState);
            modePanorama.invoke();
            panoramaState->load(item);
        });

        scene->roads->onClosestClick.subscribe([this, &modeNavigation](const ItemEntity &item) {
            if (scene->gui.wasDragged) return;
            state->removeChild(profileState);
            routeState->select(item);
            scene->actionGo.enable();
            auto path = scene->search(context->trail->start, context->trail->end);
            context->trail->path.clear();
            for (auto node: path) {
                context->trail->path.push_back(node->label);
            }

            for (auto &item: scene->roads->items) {
                item.col = vec4(1.0f, 1.0f, 1.0f, 1.0f);
            }
            scene->roads->items[context->trail->start].col = vec4(0.0f, 1.0f, 0.0f, 1.0f);
            scene->roads->items[context->trail->end].col = vec4(1.0f, 0.0f, 0.0f, 1.0f);
            modeNavigation.enable();
        });

        context->defaultAction = &modeFreeFly;
        context->tutorialAction = &buttonTutorial;
    }

    ~StreetMap() {
        // context->clearAnimations();
        delete state;
        delete freeFlyState;
        delete routeState;
        delete panoramaState;
        delete navigationState;
        delete tutorial;
        delete profileState;
        delete welcomeState;
        delete scene;
        delete context;
    }

    void enable() override {
        app->root.script = this;
        app->root.addChild(state);

        state->addChild(scene->panoramas);
        state->addChild(scene->accidents);
        state->addChild(scene->cameras);
        state->addChild(scene->parking);
        state->addChild(scene->persons);
        state->addChild(scene->roads);
        state->addChild(scene->shops);
        
        setView(welcomeState);
        
        app->scriptablePipeline.subscribe(enums::Priority::ProcessPriority::Input, bind(&StreetMap::processInput, this, placeholders::_1));
        app->scriptablePipeline.subscribe(enums::Priority::ProcessPriority::Render + 5, bind(&StreetMap::render, this, placeholders::_1));
        app->scriptablePipeline.subscribe(enums::Priority::ProcessPriority::Render + 1, bind(&StreetMap::renderShadows, this, placeholders::_1));
        app->scriptablePipeline.subscribe(enums::Priority::ProcessPriority::Render, bind(&StreetMap::renderDeffered, this, placeholders::_1));
        app->scriptablePipeline.subscribe(enums::Priority::ProcessPriority::Compositor, bind(&StreetMap::renderCompositor, this, placeholders::_1));
        app->scriptablePipeline.subscribe(enums::Priority::ProcessPriority::Gizmo, bind(&StreetMap::renderGizmo, this, placeholders::_1));
        app->scriptablePipeline.subscribe(enums::Priority::ProcessPriority::Gui, bind(&StreetMap::renderGui, this, placeholders::_1));

        scene->gui.setProp("fov", 45.0f);
        scene->gui.setProp("option-accident", true);
        scene->gui.setProp("option-camera", true);
        scene->gui.setProp("option-panorama", true);
        scene->gui.setProp("option-parking", true);
        scene->gui.setProp("option-friends", true);
        scene->gui.setProp("option-roads", false);
        scene->gui.setProp("option-shop", true);
        scene->gui.setProp("option-favorite", true);
    };

    void loadChunk() {
        // Loading meshes
        scene->meshes = app->userStorage.mesh("map.obj", &scene->materials);

        for (auto i: scene->meshes) scene->renderable.addChild(i);

        // Loading data

        nlohmann::json chunk = app->userStorage.json("chunk_1.json");

        /// @todo remove 0.01f

        {
            int i = 0;
            for (auto &node: chunk["nodes"]) {
                string label = node["label"];
                float x = node["pos"][0];
                float z = node["pos"][1];
                scene->search.node(stoi(label), x * 0.01f, 8.919 * 0.01f, z * 0.01f);
                scene->roads->addItem({
                    .label = label,
                    .pos = vec3(x * 0.01f, 8.919 * 0.01f, z * 0.01f),
                    .index = i
                });
                i++;
            }
        }
        
        for (auto &item: chunk["connections"]) {
            int id = item["node"];
            for (auto &connection: item["conn"]) {
                int child = connection["c"];
                float weight = connection["w"];
                scene->search.connection(id, child, weight * 0.01f);
            }
        }

        for (auto &view: chunk["panoramas"]) {
            string label = view["label"];
            float x = view["pos"][0];
            float y = view["pos"][1];
            float z = view["pos"][2];
            float rx = view["rot"][0];
            float ry = view["rot"][1];
            float rz = view["rot"][2];
            scene->panoramas->addItem({
                .label = label,
                .pos = vec3(x * 0.01f, y * 0.01f, z * 0.01f),
                .rot = glm::conjugate(glm::quat_cast(glm::eulerAngleXYZ(-rx, -ry, -rz)))
            });
        }

        for (auto &view: chunk["accidents"]) {
            string label = view["label"];
            float x = view["pos"][0];
            float y = view["pos"][1];
            float z = view["pos"][2];
            scene->accidents->addItem({
                .label = label,
                .pos = vec3(x * 0.01f, y * 0.01f, z * 0.01f)
            });
        }

        for (auto &view: chunk["cameras"]) {
            string label = view["label"];
            float x = view["pos"][0];
            float y = view["pos"][1];
            float z = view["pos"][2];
            scene->cameras->addItem({
                .label = label,
                .pos = vec3(x * 0.01f, y * 0.01f, z * 0.01f)
            });
        }

        for (auto &view: chunk["parking"]) {
            string label = view["label"];
            float x = view["pos"][0];
            float y = view["pos"][1];
            float z = view["pos"][2];
            scene->parking->addItem({
                .label = label,
                .pos = vec3(x * 0.01f, y * 0.01f, z * 0.01f)
            });
        }

        for (auto &view: chunk["persons"]) {
            string label = view["label"];
            float x = view["pos"][0];
            float y = view["pos"][1];
            float z = view["pos"][2];
            scene->persons->addItem({
                .label = label,
                .pos = vec3(x * 0.01f, y * 0.01f, z * 0.01f)
            });
        }

        for (auto &view: chunk["shops"]) {
            string label = view["label"];
            float x = view["pos"][0];
            float y = view["pos"][1];
            float z = view["pos"][2];
            scene->shops->addItem({
                .label = label,
                .pos = vec3(x * 0.01f, y * 0.01f, z * 0.01f)
            });
        }
    }

    bool setView(State *viewer) {
        if (viewer != nullptr && viewer == context->viewer) return false;
        if (context->viewer != nullptr) state->removeChild(context->viewer);
        context->viewer = viewer;
        if (viewer != nullptr) state->addChild(viewer);
        return true;
    }

    void update(Pipeline *pipeline) override {
        IProcessable processable{
            .pipeline = pipeline
        };
        state->dispatch(STRINGIFY(IProcessable), &processable);
    }

    void processInput(Pipeline *pipeline) {
        // cout << "input" << endl;
        float fov = std::get<float>(scene->gui.getProp("fov"));
        scene->camera.projection = glm::perspective(glm::radians(fov), static_cast<float>(app->w) / static_cast<float>(app->h), 0.1f, 100.0f);
        scene->gui.onInput(app->mouse.x, app->mouse.y, app->mouseDown);
    }

    void render(Pipeline *pipeline) {
        // cout << "render" << endl;
        context->scriptablePipeline = &app->scriptablePipeline;
        context->w = app->w;
        context->h = app->h;
    }

    void renderShadows(Pipeline *pipeline) {
        // cout << "shadows" << endl;
        scene->sun.shadow.begin();
        scene->sun.shadow.clear();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        for (auto entity: scene->renderable.children) {
            MeshActor *me = static_cast<MeshActor*>(entity);
            scene->sunShader.use();
            me->beginRenderInGame(nullptr);
            scene->sunShader.setMat4("mvp", scene->sun.projection * scene->sun.view * me->getMatrixWorld());
            me->endRenderInGame(nullptr);
        }
        scene->sun.shadow.end();
        glDisable(GL_CULL_FACE);
    }

    void renderDeffered(Pipeline *pipeline) {
        // cout << "deffered" << endl;
        scene->gbuffer.begin();
        vec4 skyColor{0.541914f, 0.894068f, 1.0f, 1.0f};
        // skyColor *= 0.7f;
        scene->gbuffer.clear(skyColor.x, skyColor.y, skyColor.z, 1.0f); // sky color
        glEnable(GL_DEPTH_TEST);
        for (auto entity: scene->renderable.children) {
            MeshActor *me = static_cast<MeshActor*>(entity);
            scene->gbufferShader.use();
            me->beginRenderInGame(nullptr);

            scene->gbufferShader.setMat4("mvp", scene->camera.projection * scene->camera.view * me->getMatrixWorld());
            scene->gbufferShader.setMat4("lightSpaceMatrix", scene->sun.projection * scene->sun.view);
            scene->gbufferShader.setTex("shadow", scene->sun.shadow.col, 0);

            me->endRenderInGame(nullptr);
        }
        scene->gbuffer.end();
    }

    void renderCompositor(Pipeline *pipeline) {
        // cout << "compositor" << endl;
        scene->pass1.begin();
        scene->pass1.clear();
        scene->quad.begin(-1, 1, 2, -2);
        scene->ssaoShader.use();
        scene->ssaoShader.setMat4("mvp", mat4(1.0));
        scene->ssaoShader.setTex("depth", scene->gbuffer.pos, 0);
        scene->ssaoShader.setTex("normal", scene->gbuffer.normal, 1);
        scene->ssaoShader.setTex("depth", scene->gbuffer.depth, 2);
        scene->ssaoShader.setMat4("proj", scene->camera.projection);
        scene->ssaoShader.setMat4("view", scene->camera.view);
        scene->ssaoShader.setVec3("eye", vec3(0, 0, 1));

        glm::mat4 M = glm::inverse(scene->camera.view);
        M = glm::transpose(M);
        scene->ssaoShader.setMat4("matrixNormal", M);

        scene->quad.end();
        scene->pass1.end();

        scene->pass2.begin();
        scene->pass2.clear();
        scene->quad.begin(-1, 1, 2, -2);
        scene->blurBoxShader.use();
        scene->blurBoxShader.setMat4("mvp", mat4(1.0));
        scene->blurBoxShader.setTex("image", scene->pass1.col, 0);
        scene->quad.end();
        scene->pass2.end();

        scene->screen.begin();
        scene->screen.clear();
        scene->screen.viewport(app->w, app->h);

        scene->quad.begin(-1, 1, 2, -2);
        scene->defferedShader.use();
        scene->defferedShader.setMat4("mvp", mat4(1.0));
        scene->defferedShader.setTex("albedo", scene->gbuffer.albedo, 0);
        scene->defferedShader.setTex("normal", scene->gbuffer.normal, 1);
        scene->defferedShader.setTex("shadow", scene->gbuffer.shadow, 2);
        scene->defferedShader.setTex("ambientOcclusion", scene->pass2.col, 3);

        scene->defferedShader.setVec4("ambient", scene->ambientColor);
        scene->defferedShader.setVec3("sun.pos", vec3(0.0f, 0.0f, 190.0f));
        scene->defferedShader.setVec3("sun.dir", getUp(scene->sun.view));
        scene->defferedShader.setVec4("sun.col", scene->sun.col);

        scene->quad.end();
        scene->screen.end();

        // scene->screen.begin();
        // scene->screen.clear();
        // scene->screen.viewport(app->w, app->h);
        // scene->quad.begin(-1, 1, 2, -2);

        // scene->imageShader.use();
        // scene->imageShader.setMat4("mvp", mat4(1.0));
        // scene->imageShader.setTex("image", scene->pass1.col, 0);
        // // scene->imageShader.setTex("image", scene->sun.shadow.col, 0);

        // scene->quad.end();
        // scene->screen.end();
    }

    void renderGizmo(Pipeline *pipeline) {
        // cout << "gizmo" << endl;
        using engine::ui::Ui;

        scene->screen.begin();
        scene->screen.viewport(app->w, app->h);
        glDisable(GL_DEPTH_TEST);

        Ui::Handle *handle = scene->gui.begin(scene->camera.view, scene->camera.projection, app->w, app->h);
        handle->transform(glm::translate(mat4(1.0f), vec3(-static_cast<int>(app->w) / 2.0f, -static_cast<int>(app->h) / 2.0f, 0.0)));
        
        // handle->drawLine(0, 0, 0, 100, 3.0f, 1, 1, 1, 1);
        // handle->drawLine(0, 0, 79, 452, 3.0f, 1, 1, 1, 1);

        if (context->trail != nullptr && context->trail->path.size() > 1) {
            for (int i = 0; i < context->trail->path.size() - 1; i++) {
                auto a = scene->roads->items[context->trail->path[i]].pos;
                auto b = scene->roads->items[context->trail->path[i + 1]].pos;
                handle->drawLine(a.x, a.z, b.x, b.z, 0.03f, 1, 1, 1, 1);
            }
        }

        scene->gui.end();

        scene->panoramas->isEnabled = std::get<bool>(scene->gui.getProp("option-panorama"));
        scene->accidents->isEnabled = std::get<bool>(scene->gui.getProp("option-accident"));
        scene->cameras->isEnabled = std::get<bool>(scene->gui.getProp("option-camera"));
        scene->parking->isEnabled = std::get<bool>(scene->gui.getProp("option-parking"));
        scene->persons->isEnabled = std::get<bool>(scene->gui.getProp("option-friends"));
        scene->roads->isEnabled = std::get<bool>(scene->gui.getProp("option-roads"));
        scene->shops->isEnabled = std::get<bool>(scene->gui.getProp("option-shop"));
        /// @todo favorite (option-favorite)
    }

    void renderGui(Pipeline *pipeline) {
        // cout << "gui" << endl;
        using engine::ui::Ui;
        Ui::Handle *form = scene->gui.begin(app->w, app->h, true);

        // Make UI 3D! Leaved this just for fun
        // Ui::Handle *form = scene->gui.begin(scene->camera.view, scene->camera.projection, app->w, app->h);
        // form->transform(glm::translate(mat4(1.0f), vec3(-(int)app->w / 2.0f, -(int)app->h / 2.0f, 0.0)))->transform(glm::scale(mat4(1.0f), vec3(0.01, 0.01, 0.01)));

        float padding = 28;
        float spacing = 4;
        vec4 sel{0.654f, 1.0f, 0.368f, 1.0f};
        vec4 norm{1.0f, 1.0f, 1.0f, 1.0f};
        vec4 inactive = vec4(1.0f, 1.0f, 1.0f, 0.2f);

        form = form->anchorAbs(padding, padding, padding, padding);

        if (scene->actionGo.enabled(&scene->gui) && context->trail != nullptr && context->trail->path.size() > 1) {
            Ui::Handle *handle = form->anchorRelSize(1, 1, scene->actionGo.getWidth(), scene->actionGo.getHeight());
            scene->actionGo.renderGui(&scene->gui);
        }

        form->activate();
        {
            // Layers
            Ui::Handle *handle = form->floating(0, 0, 0, 0);

            for (auto &layer: scene->layers) {
                handle = handle->col(layer.getWidth(), layer.getHeight());
                layer.renderGui(&scene->gui);
                handle->setSpacing(4);
            }
        }

        form->activate();
        {
            // Modes
            float width = accumulate(scene->modes.begin(), scene->modes.end(), 0.0f, [](float b, ModeAction &a) {
                return a.image.w + b;
            });
            Ui::Handle *handle = form->anchorRelSize(0, -1, width, scene->modeFreeFly.h)->floating(0, 0, 0, 0);

            for (auto &mode: scene->modes) {
                scene->gui.setAtlas(mode.image.id);
                handle = handle->row(mode.image.w, mode.image.h);
                mode.renderGui(&scene->gui);
                handle->setSpacing(4);
            }
        }

        form->activate();
        {
            // Buttons
            Ui::Handle *handle = form->anchorRelSize(1, -1, scene->buttonZoomIn.w, 0)->floating(0, 0, 0, 0);

            for (auto &control: scene->controls) {
                scene->gui.setAtlas(control.image.id);
                handle = handle->col(control.image.w, control.image.h);
                control.renderGui(&scene->gui);
                handle->setSpacing(4);
            }
        }

        form->activate();
        {
            // Links and info
            float width = accumulate(scene->info.begin(), scene->info.end(), 0.0f, [](float b, ButtonAction &a) {
                return a.image.w + b;
            });
            Ui::Handle *handle = form->anchorRelSize(0, 1, width, scene->buttonTutorial.h)->floating(0, 0, 0, 0);

            for (auto &link: scene->info) {
                scene->gui.setAtlas(link.image.id);
                handle = handle->row(link.image.w, link.image.h);
                link.renderGui(&scene->gui);
                handle->setSpacing(4);
            }
        }

        form->activate();
        {
            // Help
            scene->gui.setAtlas(scene->helpScreen.id);
            Ui::Handle *handle = form->anchorRelSize(-1, 1, scene->helpScreen.w, scene->helpScreen.h);
            handle->setColor(1, 1, 1, 1)->setImage(0, 1, 1, -1)->rect();
        }

        scene->gui.end();
    }
};

} // namespace script

#endif