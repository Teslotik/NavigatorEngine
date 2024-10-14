#include "Easing.h"

using engine::enums::Easing::Easing;

float easeOutBounce(float x) {
    constexpr float n1 = 7.5625;
    constexpr float d1 = 2.75;
    
    if (x < 1 / d1) {
        return n1 * x * x;
    } else if (x < 2 / d1) {
        return n1 * (x -= 1.5 / d1) * x + 0.75;
    } else if (x < 2.5 / d1) {
        return n1 * (x -= 2.25 / d1) * x + 0.9375;
    } else {
        return n1 * (x -= 2.625 / d1) * x + 0.984375;
    }
}

float engine::enums::Easing::ease(Easing easing, float x) {
    // https://easings.net
    switch (easing) {
        case Easing::EaseInOutLinear:
            return x;
        
        case Easing::EaseInSine:
            return 1 - std::cos((x * M_PI) / 2);

        case Easing::EaseOutSine:
            return std::sin((x * M_PI) / 2);

        case Easing::EaseInOutSine:
            return -(std::cos(M_PI * x) - 1) / 2;

        case Easing::EaseInQuad:
            return x * x;

        case Easing::EaseOutQuad:
            return 1 - (1 - x) * (1 - x);

        case Easing::EaseInOutQuad:
            return x < 0.5 ? 2 * x * x : 1 - std::pow(-2 * x + 2, 2) / 2;

        case Easing::EaseInCubic:
            return x * x * x;

        case Easing::EaseOutCubic:
            return 1 - std::pow(1 - x, 3);

        case Easing::EaseInOutCubic:
            return x < 0.5 ? 4 * x * x * x : 1 - std::pow(-2 * x + 2, 3) / 2;

        case Easing::EaseInQuart:
            return x * x * x * x;

        case Easing::EaseOutQuart:
            return 1 - std::pow(1 - x, 4);

        case Easing::EaseInOutQuart:
            return x < 0.5 ? 8 * x * x * x * x : 1 - std::pow(-2 * x + 2, 4) / 2;

        case Easing::EaseInQuint:
            return x * x * x * x * x;

        case Easing::EaseOutQuint:
            return 1 - std::pow(1 - x, 5);

        case Easing::EaseInOutQuint:
            return x < 0.5 ? 16 * x * x * x * x * x : 1 - std::pow(-2 * x + 2, 5) / 2;

        case Easing::EaseInExpo:
            return x == 0 ? 0 : std::pow(2, 10 * x - 10);

        case Easing::EaseOutExpo:
            return x == 1 ? 1 : 1 - std::pow(2, -10 * x);

        case Easing::EaseInOutExpo:
            return x == 0
                ? 0
                : x == 1
                ? 1
                : x < 0.5 ? std::pow(2, 20 * x - 10) / 2
                : (2 - std::pow(2, -20 * x + 10)) / 2;

        case Easing::EaseInCirc:
            return 1 - std::sqrt(1 - std::pow(x, 2));

        case Easing::EaseOutCirc:
            return std::sqrt(1 - std::pow(x - 1, 2));

        case Easing::EaseInOutCirc:
            return x < 0.5
                ? (1 - std::sqrt(1 - std::pow(2 * x, 2))) / 2
                : (std::sqrt(1 - std::pow(-2 * x + 2, 2)) + 1) / 2;

        case Easing::EaseInBack: {
            constexpr float c1 = 1.70158;
            constexpr float c3 = c1 + 1;
            return c3 * x * x * x - c1 * x * x;
        }

        case Easing::EaseOutBack: {
            constexpr float c1 = 1.70158;
            constexpr float c3 = c1 + 1;
            return 1 + c3 * std::pow(x - 1, 3) + c1 * std::pow(x - 1, 2);
        }

        case Easing::EaseInOutBack: {
            constexpr float c1 = 1.70158;
            constexpr float c2 = c1 * 1.525;
            return x < 0.5
                ? (std::pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2
                : (std::pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
        }

        case Easing::EaseInElastic: {
            constexpr float c4 = (2 * M_PI) / 3;
            
            return x == 0
                ? 0
                : x == 1
                ? 1
                : -std::pow(2, 10 * x - 10) * std::sin((x * 10 - 10.75) * c4);
        }

        case Easing::EaseOutElastic: {
            constexpr float c4 = (2 * M_PI) / 3;
            
            return x == 0
                ? 0
                : x == 1
                ? 1
                : std::pow(2, -10 * x) * std::sin((x * 10 - 0.75) * c4) + 1;
        }

        case Easing::EaseInOutElastic: {
            constexpr float c5 = (2 * M_PI) / 4.5;
            
            return x == 0
                ? 0
                : x == 1
                ? 1
                : x < 0.5
                ? -(std::pow(2, 20 * x - 10) * std::sin((20 * x - 11.125) * c5)) / 2
                : (std::pow(2, -20 * x + 10) * std::sin((20 * x - 11.125) * c5)) / 2 + 1;
        }

        case Easing::EaseInBounce:
            return 1 - easeOutBounce(1 - x);

        case Easing::EaseOutBounce:
            return easeOutBounce(x);

        case Easing::EaseInOutBounce:
            return x < 0.5
                ? (1 - easeOutBounce(1 - 2 * x)) / 2
                : (1 + easeOutBounce(2 * x - 1)) / 2;
    }
    return x;
}