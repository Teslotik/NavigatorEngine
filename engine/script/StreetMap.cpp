#include "StreetMap.h"
#include "Easing.h"

using namespace script::street;

State::State(Scene *scene, Context *context): scene(scene), context(context) {
    subscribe(STRINGIFY(IProcessable), bind(&State::update, this, placeholders::_1));
}

WelcomeState::WelcomeState(struct Scene *scene, Context *context): State(scene, context) {
    animation.onStart = [this]() {
        this->context->scriptablePipeline->subscribe(this, bind(&WelcomeState::renderGui, this, placeholders::_1), enums::Priority::ProcessPriority::Gui - 10);
    };

    animation.onEnd = [this]() {
        this->context->delay([this]() {
            this->context->scriptablePipeline->unsubscribe(this);
            this->context->defaultAction->invoke();
            this->context->tutorialAction->invoke();
        });
    };

    fading.onFrameChanged = [this](float progress) {
        value = progress;
    };
    fading.duration = 3.0f;
    fading.easing = enums::Easing::EaseInOutCubic;
    animation.children.push_back(&fading);

    hiding.onFrameChanged = [this](float progress) {
        offset = -progress;
    };
    hiding.duration = 0.6f;
    hiding.easing = enums::Easing::EaseOutCubic;
    animation.children.push_back(&hiding);

    context->pushAnimation(&animation);
}

void WelcomeState::renderGui(Pipeline *pipeline) {
    using engine::ui::Ui;
    Ui::Handle *handle = handle = scene->gui.begin(context->w, context->h, true);

    scene->gui.setAtlas(scene->welcome.id);
    handle = handle->floating(0, offset * context->h, context->w, context->h);
    handle->setColor(0, 0, 0, 1)->rect();
    handle = handle->anchorRelSize(0, 0, scene->welcome.w, scene->welcome.h)->setColor(1, 1, 1, value)->setImage(0, 1, 1, -1)->rect();

    scene->gui.end();
}


FreeFlyState::FreeFlyState(Scene *scene, Context *context): State(scene, context) {
        
}

bool FreeFlyState::update(IListenable *e) {
    Pipeline *pipeline = static_cast<IProcessable*>(e)->pipeline;
    if (scene->gui.down() && !scene->gui.over) {
        /// @todo рефактор
        glfwSetInputMode(context->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        float dx = (glfwGetKey(context->window, GLFW_KEY_W) == GLFW_PRESS ? 1 : 0) + (glfwGetKey(context->window, GLFW_KEY_S) == GLFW_PRESS ? -1 : 0);
        float dy = (glfwGetKey(context->window, GLFW_KEY_SPACE) == GLFW_PRESS ? 1 : 0) + (glfwGetKey(context->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? -1 : 0);
        float dz = (glfwGetKey(context->window, GLFW_KEY_A) == GLFW_PRESS ? -1 : 0) + (glfwGetKey(context->window, GLFW_KEY_D) == GLFW_PRESS ? 1 : 0);
        vec3 movement = ((dx || dz ? glm::normalize(vec3(dx, 0, dz)) : vec3(0.0)) + vec3(0, dy, 0)) * speed * static_cast<float>(pipeline->deltaTime);
        scene->camera.localTransform(movement.x, movement.y, movement.z, scene->gui.delta.x * sensivity, scene->gui.delta.y * sensivity);
    } else {
        glfwSetInputMode(context->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    return true;
}


RouteState::RouteState(Scene *scene, Context *context): State(scene, context) {
    
}

void RouteState::onParent() {
    pos = vec3(0.0f, 10.0f, 0.0f);
    rot = getRotation(glm::eulerAngleXYZ(glm::radians(90.0f), glm::radians(0.0f), glm::radians(0.0f)));
}

bool RouteState::update(IListenable *e) {
    Pipeline *pipeline = static_cast<IProcessable*>(e)->pipeline;

    if (scene->gui.down()) {
        glfwSetInputMode(context->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        pos.x -= scene->gui.delta.x * sensivity;
        pos.z -= scene->gui.delta.y * sensivity;
        pos.z += ((glfwGetKey(context->window, GLFW_KEY_W) == GLFW_PRESS ? -1 : 0) + (glfwGetKey(context->window, GLFW_KEY_S) == GLFW_PRESS ? 1 : 0)) * speed;
        pos.x += ((glfwGetKey(context->window, GLFW_KEY_A) == GLFW_PRESS ? -1 : 0) + (glfwGetKey(context->window, GLFW_KEY_D) == GLFW_PRESS ? 1 : 0)) * speed;
    } else {
        glfwSetInputMode(context->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    scene->camera.view = lerp(scene->camera.view, pos, rot, vec3(1.0f, 1.0f, 1.0f), pipeline->deltaTime * 12.0f);
    
    return true;
}

void RouteState::select(ItemEntity item) {
    trail.segment = 0;
    if (context->trail == nullptr) {
        context->trail = &trail;
        trail.start = item.index;
        trail.end = item.index;
    } else {
        trail.start = trail.end;
        trail.end = item.index;
    }
}


PanoramaState::PanoramaState(Scene *scene, Context *context): State(scene, context) {
    context->pushAnimation(&teleportAnimation);
    teleportAnimation.isLooped = false;
    teleportAnimation.duration = 0.5f;
    teleportAnimation.easing = engine::enums::Easing::EaseOutCubic;
}

void PanoramaState::onParent() {
    
}

bool PanoramaState::update(IListenable *e) {
    Pipeline *pipeline = static_cast<IProcessable*>(e)->pipeline;
    
    if (
        scene->gui.down() && (
        glfwGetKey(context->window, GLFW_KEY_W) == GLFW_PRESS ||
        glfwGetKey(context->window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(context->window, GLFW_KEY_S) == GLFW_PRESS ||
        glfwGetKey(context->window, GLFW_KEY_D) == GLFW_PRESS ||
        glfwGetKey(context->window, GLFW_KEY_SPACE) == GLFW_PRESS ||
        glfwGetKey(context->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
    )) {
        context->defaultAction->invoke();
    } else if (scene->gui.down()) {
        glfwSetInputMode(context->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mat4 view = scene->camera.view;
        glm::mat4 horizontal = glm::rotate(glm::mat4(glm::mat3(view)), glm::radians(scene->gui.delta.x * sensivity), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 right = glm::normalize(glm::row(horizontal, 0));
        glm::mat4 vertical = glm::rotate(glm::mat4(glm::mat3(horizontal)), glm::radians(scene->gui.delta.y * sensivity), glm::vec3(right));
        
        rot = glm::conjugate(glm::normalize(glm::quat_cast(vertical)));
        scene->camera.view = glm::mat4_cast(glm::conjugate(rot)) * glm::translate(glm::mat4(1.0f), -pos);
    } else {
        glfwSetInputMode(context->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    return true;
}

void PanoramaState::load(ItemEntity panorama) {
    this->pos = panorama.pos;
    this->rot = panorama.rot;
    
    mat4 src = scene->camera.view;
    vec3 p = pos;
    quat r = rot;
    teleportAnimation.onFrameChanged = [this, src, p, r](float progress) {
        scene->camera.view = lerp(src, p, r, vec3(1.0f, 1.0f, 1.0f), progress);
    };
    teleportAnimation.play();
}


NavigationState::NavigationState(Scene *scene, Context *context): State(scene, context) {
    context->pushAnimation(&moveAnimation);
    moveAnimation.isLooped = false;
    moveAnimation.speed = speed;
}

void NavigationState::onParent() {
    moveAnimation.isFreezed = false;

    glfwSetInputMode(context->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    context->scriptablePipeline->subscribe(this, bind(&NavigationState::renderGizmo, this, placeholders::_1), enums::Priority::ProcessPriority::Gizmo);
    
    assert(context->trail != nullptr);

    float height = 2.0f;    // how far camera from the ground
    float offset = 5.0f;    // camera offset from the path marker

    {
        // Align and position camera towards beginning of the path
        context->trail->pos = getPos(context->trail->segment);
        vec3 dir = glm::normalize(getPos(context->trail->segment + 1) - getPos(context->trail->segment));
        pos = context->trail->pos - dir * offset;
        pos.y = height;
        rot = getRotation(glm::lookAt(pos, vec3(context->trail->pos.x, 0.0f, context->trail->pos.z), vec3(0.0f, 1.0f, 0.0f)));
    }

    // Total length of the path
    float length = 0;
    for (int i = 0; i < context->trail->path.size() - 1; i++) {
        length += glm::distance(scene->roads->items[context->trail->path[i]].pos, scene->roads->items[context->trail->path[i + 1]].pos);
    }
    moveAnimation.duration = length;

    moveAnimation.onFrameChanged = [this, length, height, offset](float progress) {
        assert(context->trail != nullptr);

        // Searching for a current segment of the path based on progress
        float total = 0;
        int segment = 0;
        for (int i = 0; i < context->trail->path.size() - 1; i++) {
            float distance = glm::distance(getPos(i), getPos(i + 1));
            if ((total + distance) / length >= progress) break;
            total += distance;
            segment++;
        }
        
        // When current segment is found we need to calculate a progress of it
        float distance = glm::distance(getPos(segment), getPos(segment + 1));
        float segmentProgress = (progress - total / length) * (length / distance);

        context->trail->pos = glm::mix(getPos(segment), getPos(segment + 1), segmentProgress);
        
        // Align and position camera towards the current segment
        vec3 dir = glm::normalize(getPos(segment + 1) - getPos(segment));
        pos = context->trail->pos - dir * offset;
        pos.y = height;
        rot = getRotation(glm::lookAt(pos, vec3(context->trail->pos.x, 0.0f, context->trail->pos.z), vec3(0.0f, 1.0f, 0.0f)));
    };

    moveAnimation.onEnd = [this]() {
        context->delay([this]() {
            context->defaultAction->invoke();
        });
    };
}

void NavigationState::onRemove() {
    moveAnimation.stop(false);
    context->delay([this]() {
        context->scriptablePipeline->unsubscribe(this);
    });
}

bool NavigationState::update(IListenable *e) {
    Pipeline *pipeline = static_cast<IProcessable*>(e)->pipeline;

    bool hasMovement =
        glfwGetKey(context->window, GLFW_KEY_W) == GLFW_PRESS ||
        glfwGetKey(context->window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(context->window, GLFW_KEY_S) == GLFW_PRESS ||
        glfwGetKey(context->window, GLFW_KEY_D) == GLFW_PRESS ||
        glfwGetKey(context->window, GLFW_KEY_SPACE) == GLFW_PRESS ||
        glfwGetKey(context->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    if (
        !scene->gui.over && (
        scene->gui.clicked() ||
        hasMovement
    )) {
        pause();
        scene->actionGo.enable();
    }

    if ((moveAnimation.isFreezed || !moveAnimation.isPlaying) && (!scene->gui.over && scene->gui.down() || hasMovement)) {
        int dx = (glfwGetKey(context->window, GLFW_KEY_W) == GLFW_PRESS ? 1 : 0) + (glfwGetKey(context->window, GLFW_KEY_S) == GLFW_PRESS ? -1 : 0);
        int dz = (glfwGetKey(context->window, GLFW_KEY_A) == GLFW_PRESS ? -1 : 0) + (glfwGetKey(context->window, GLFW_KEY_D) == GLFW_PRESS ? 1 : 0);

        auto forward = getUp(scene->camera.view);
        auto right = getRight(scene->camera.view);
        forward.y = 0;
        right.y = 0;
        forward = glm::normalize(forward);
        right = glm::normalize(right);
        pos -= forward * sensivity * (scene->gui.delta.y + dx) + right * sensivity * (scene->gui.delta.x - dz);
    }

    scene->camera.view = lerp(scene->camera.view, pos, rot, vec3(1.0f, 1.0f, 1.0f), 10.0f * pipeline->deltaTime);
    return true;
}

void NavigationState::renderGizmo(Pipeline *pipeline) {
    using engine::ui::Ui;
    Ui::Handle *handle = scene->gui.begin(context->w, context->h, false);
    scene->gui.setAtlas(scene->icons.id, 15);
    {
        auto marker = scene->camera.projection * scene->camera.view * vec4(context->trail->pos, 1.0f);
        marker /= marker.w;
        handle = handle->floating((marker.x * 0.5f + 0.5f) * (int)context->w - 12, (marker.y * 0.5f + 0.5f) * (int)context->h - 12, -marker.z, 24, 24);
        handle->setColor(1, 0, 0, 1);
        handle->setIcon(Icon::Pin)->rect();
        handle = handle->pop();
    }
    scene->gui.end();
}

inline vec3 NavigationState::getPos(int segment) {
    return scene->roads->items[context->trail->path[segment]].pos;
}

void NavigationState::play() {
    if (moveAnimation.isFreezed) {
        moveAnimation.isFreezed = false;
    } else {
        moveAnimation.play();
    }
}

void NavigationState::pause() {
    if (moveAnimation.isPlaying) moveAnimation.isFreezed = true;
}

void NavigationState::stop() {
    moveAnimation.stop();
}


ItemList::ItemList(Scene *scene, Context *context, Image image, int icon): State(scene, context), atlas(image), icon(icon) {
    
}

bool ItemList::update(IListenable *e) {
    if (!isEnabled) return true;
    
    // if (!scene->gui.over && scene->gui.clicked() && !scene->gui.event) {
    if (!scene->gui.over && scene->gui.released() && !scene->gui.event) {
        // Item pick
        for (auto item: items) {
            vec4 pos = scene->camera.projection * scene->camera.view * vec4(item.pos, 1.0f);
            pos /= pos.w;

            vec2 mouse = vec2(scene->gui.mouse.x / scene->gui.width * 2.0f - 1.0f, -(scene->gui.mouse.y / scene->gui.height * 2.0f - 1.0f));

            float hitRadius = 24.0f * 1.3;
            float d = glm::distance(
                vec2(mouse.x * scene->gui.width, mouse.y * scene->gui.height),
                vec2(pos.x * scene->gui.width, pos.y * scene->gui.height)
            );
            if (d < hitRadius) {
                onClick.emit(item);
                scene->gui.event = true;
                break;
            }
        }

        // Closest pick
        auto it = min_element(items.begin(), items.end(), [this](const ItemEntity e, const ItemEntity r) {
            vec2 mouse = vec2(scene->gui.mouse.x / scene->gui.width * 2.0f - 1.0f, -(scene->gui.mouse.y / scene->gui.height * 2.0f - 1.0f));

            vec4 pos1 = scene->camera.projection * scene->camera.view * vec4(e.pos, 1.0f);
            pos1 /= pos1.w;

            vec4 pos2 = scene->camera.projection * scene->camera.view * vec4(r.pos, 1.0f);
            pos2 /= pos2.w;

            return glm::distance(
                vec2(mouse.x * scene->gui.width, mouse.y * scene->gui.height),
                vec2(pos1.x * scene->gui.width, pos1.y * scene->gui.height)
            ) < glm::distance(
                vec2(mouse.x * scene->gui.width, mouse.y * scene->gui.height),
                vec2(pos2.x * scene->gui.width, pos2.y * scene->gui.height)
            );
        });
        onClosestClick.emit(*it);
    }
    return true;
}

void ItemList::renderGizmo(Pipeline *pipeline) {
    using engine::ui::Ui;
    if (isEnabled) {
        Ui::Handle *handle = scene->gui.begin(context->w, context->h, false);
        scene->gui.setAtlas(atlas.id, 15);
        for (auto &item: items) {
            auto marker = scene->camera.projection * scene->camera.view * vec4(item.pos, 1.0f);
            marker /= marker.w;
            handle = handle->floating((marker.x * 0.5f + 0.5f) * (int)context->w - 12, (marker.y * 0.5f + 0.5f) * (int)context->h - 12, -marker.z, 24, 24);
            handle->setColor(0, 0, 0, 1)->rect();
            handle = handle->setColor(item.col)->setIcon(icon)->rect()->pop();
        }
        scene->gui.end();
    }
}


Tutorial::Tutorial(Scene *scene, Context *context): State(scene, context) {
    animation.onStart = [this]() {
        start = pos;
        page = (page + 1) % 7;
    };
    animation.onFrameChanged = [this](float progress) {
        pos = glm::mix(start, end, progress);
        // 0.0: 1.0
        // 0.5: 0.0
        // 1.0: 1.0
        opacity = fabs(progress - 0.5f) * 2.0f;
        transition = progress;
    };
    animation.duration = 0.8f;
    animation.easing = enums::Easing::EaseInOutCubic;

    context->pushAnimation(&animation);
}

void Tutorial::renderGui(Pipeline *pipeline) {
    using engine::ui::Ui;
    Ui::Handle *handle = scene->gui.begin(context->w, context->h, true);

    if (transition > 0.5) prev = image;

    float padding = 28 + 20;
    scene->gui.setAtlas(prev->id);
    handle = handle->anchorAbs(padding, padding, padding, padding);
    handle->anchorRelSize(pos.x, pos.y, prev->w, prev->h)->setColor(1, 1, 1, opacity)->setImage(0, 1, 1, -1)->rect();

    if (handle->clicked()) {
        switch (page) {
            case About:
                end = vec2(0.0f, 0.0f);
                image = &scene->tutorialAbout;
                break;
            case Layers:
                end = vec2(-1.0f, -1.0f);
                image = &scene->tutorialLayers;
                break;
            case Modes:
                end = vec2(0.0f, -1.0f);
                image = &scene->tutorialModes;
                break;
            case Controls:
                end = vec2(1.0f, -1.0f);
                image = &scene->tutorialControls;
                break;
            case Info:
                end = vec2(0.0f, 1.0f);
                image = &scene->tutorialInfo;
                break;
            case Help:
                end = vec2(-1.0f, 1.0f);
                image = &scene->tutorialHelp;
                break;
            
            default:
                context->delay([this]() {
                    parent->removeChild(this);
                });
        }
        animation.play();
    }

    scene->gui.end();
}

void Tutorial::onParent() {
    pos = start = end = vec2(0.0f, 0.0f);
    page = About;
    image = prev = &scene->tutorialAbout;
    context->delay([this]() {
        context->scriptablePipeline->subscribe(this, bind(&Tutorial::renderGui, this, placeholders::_1), enums::Priority::ProcessPriority::Gui);
        animation.play();
        animation.setProgress(0.5f);
    });
}

void Tutorial::onRemove() {
    context->delay([this]() {
        context->scriptablePipeline->unsubscribe(this);
        animation.stop();
    });
}


ProfileState::ProfileState(Scene *scene, Context *context): State(scene, context) {
    appearAnimation.onFrameChanged = [this](float progress) {
        transition = progress;
    };
    appearAnimation.duration = 0.2f;

    disappearAnimation.onFrameChanged = [this](float progress) {
        transition = 1.0f - progress;
    };
    disappearAnimation.onEnd = [this]() {
        /// @note I think delay is not necessary, because animation
        // is in different collection. Needs to test
        this->context->delay([this]() {
            parent->removeChild(this);
        });
    };
    disappearAnimation.duration = 0.2f;

    context->pushAnimation(&appearAnimation);
    context->pushAnimation(&disappearAnimation);
}

void ProfileState::renderGui(Pipeline *pipeline) {
    using engine::ui::Ui;
    Ui::Handle *handle = scene->gui.begin(context->w, context->h, true);

    scene->gui.setAtlas(scene->profileScreen.id);
    handle = handle->anchorRelSize(0, mix(transition, -1.0f, 0.0f), scene->profileScreen.w, scene->profileScreen.h);
    handle->setColor(1, 1, 1, transition)->setImage(0, 1, 1, -1)->rect();

    if (handle->clicked()) {
        disappearAnimation.play();
    }

    scene->gui.end();
}

void ProfileState::onParent() {
    context->delay([this]() {
        context->scriptablePipeline->subscribe(this, bind(&ProfileState::renderGui, this, placeholders::_1), enums::Priority::ProcessPriority::Gui);
        appearAnimation.play();
    });
}

void ProfileState::onRemove() {
    context->delay([this]() {
        context->scriptablePipeline->unsubscribe(this);
        appearAnimation.stop();
        disappearAnimation.stop();
    });
}