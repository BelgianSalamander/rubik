//
// Created by Anatol on 27/12/2022.
//

#ifndef RUBIK_EASING_H
#define RUBIK_EASING_H

#include <cmath>
#include <vector>
#include <utility>

static float PI = 3.14159265358979323846f;

static inline float linear(float t) {
    return t;
}

static inline float easeInSine(float t) {
    return 1 - cos((t * PI) / 2);
}

static inline float easeOutSine(float t) {
    return sin((t * PI) / 2);
}

static inline float easeInOutSine(float t) {
    return -(cos(PI * t) - 1) / 2;
}

static inline float easeInQuad(float t) {
    return t * t;
}

static inline float easeOutQuad(float t) {
    return 1 - (1 - t) * (1 - t);
}

static inline float easeInOutQuad(float t) {
    return t < 0.5 ? 2 * t * t : 1 - pow(-2 * t + 2, 2) / 2;
}

static inline float easeInCubic(float t) {
    return t * t * t;
}

static inline float easeOutCubic(float t) {
    return 1 - pow(1 - t, 3);
}

static inline float easeInOutCubic(float t) {
    return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
}

static inline float easeInQuart(float t) {
    return t * t * t * t;
}

static inline float easeOutQuart(float t) {
    return 1 - pow(1 - t, 4);
}

static inline float easeInOutQuart(float t) {
    return t < 0.5 ? 8 * t * t * t * t : 1 - pow(-2 * t + 2, 4) / 2;
}

static inline float easeInQuint(float t) {
    return t * t * t * t * t;
}

static inline float easeOutQuint(float t) {
    return 1 - pow(1 - t, 5);
}

static inline float easeInOutQuint(float t) {
    return t < 0.5 ? 16 * t * t * t * t * t : 1 - pow(-2 * t + 2, 5) / 2;
}

static inline float easeInExpo(float t) {
    return t == 0 ? 0 : pow(2, 10 * t - 10);
}

static inline float easeOutExpo(float t) {
    return t == 1 ? 1 : 1 - pow(2, -10 * t);
}

static inline float easeInOutExpo(float t) {
    return t == 0 ? 0 : t == 1 ? 1 : t < 0.5 ? pow(2, 20 * t - 10) / 2 : (2 - pow(2, -20 * t + 10)) / 2;
}

static inline float easeInCirc(float t) {
    return 1 - sqrt(1 - pow(t, 2));
}

static inline float easeOutCirc(float t) {
    return sqrt(1 - pow(t - 1, 2));
}

static inline float easeInOutCirc(float t) {
    return t < 0.5 ? (1 - sqrt(1 - pow(2 * t, 2))) / 2 : (sqrt(1 - pow(-2 * t + 2, 2)) + 1) / 2;
}

static inline float easeInBack(float t) {
    return 2.70158 * t * t * t - 1.70158 * t * t;
}

static inline float easeOutBack(float t) {
    return 1 + 2.70158 * pow(t - 1, 3) + 1.70158 * pow(t - 1, 2);
}

static inline float easeInOutBack(float t) {
    return t < 0.5 ? (pow(2 * t, 2) * ((1.525 + 1) * 2 * t - 1.525)) / 2 : (pow(2 * t - 2, 2) * ((1.525 + 1) * (t * 2 - 2) + 1.525) + 2) / 2;
}

static inline float easeInElastic(float t) {
    const float c4 = (2 * PI) / 3;

    return t == 0 ? 0 : t == 1 ? 1 : -pow(2, 10 * t - 10) * sin((t * 10 - 10.75) * c4);
}

static inline float easeOutElastic(float t) {
    const float c4 = (2 * PI) / 3;

    return t == 0 ? 0 : t == 1 ? 1 : pow(2, -10 * t) * sin((t * 10 - 0.75) * c4) + 1;
}

static inline float easeInOutElastic(float t) {
    const float c5 = (2 * PI) / 4.5;

    return t == 0 ? 0 : t == 1 ? 1 : t < 0.5 ? -(pow(2, 20 * t - 10) * sin((20 * t - 11.125) * c5)) / 2 : (pow(2, -20 * t + 10) * sin((20 * t - 11.125) * c5)) / 2 + 1;
}

static inline float easeOutBounce(float t) {
    const float n1 = 7.5625;
    const float d1 = 2.75;

    if (t < 1 / d1) {
        return n1 * t * t;
    } else if (t < 2 / d1) {
        return n1 * (t -= 1.5 / d1) * t + 0.75;
    } else if (t < 2.5 / d1) {
        return n1 * (t -= 2.25 / d1) * t + 0.9375;
    } else {
        return n1 * (t -= 2.625 / d1) * t + 0.984375;
    }
}

static inline float easeInBounce(float t) {
    return 1 - easeOutBounce(1 - t);
}

static inline float easeInOutBounce(float t) {
    return t < 0.5 ? (1 - easeOutBounce(1 - 2 * t)) / 2 : (1 + easeOutBounce(2 * t - 1)) / 2;
}

//List of all easing functions and their names
static std::vector<std::pair<std::string, float(*)(float)>> EASINGS = {
    {"Linear", linear},
    {"EaseInSine", easeInSine},
    {"EaseOutSine", easeOutSine},
    {"EaseInOutSine", easeInOutSine},
    {"EaseInQuad", easeInQuad},
    {"EaseOutQuad", easeOutQuad},
    {"EaseInOutQuad", easeInOutQuad},
    {"EaseInCubic", easeInCubic},
    {"EaseOutCubic", easeOutCubic},
    {"EaseInOutCubic", easeInOutCubic},
    {"EaseInQuart", easeInQuart},
    {"EaseOutQuart", easeOutQuart},
    {"EaseInOutQuart", easeInOutQuart},
    {"EaseInQuint", easeInQuint},
    {"EaseOutQuint", easeOutQuint},
    {"EaseInOutQuint", easeInOutQuint},
    {"EaseInExpo", easeInExpo},
    {"EaseOutExpo", easeOutExpo},
    {"EaseInOutExpo", easeInOutExpo},
    {"EaseInCirc", easeInCirc},
    {"EaseOutCirc", easeOutCirc},
    {"EaseInOutCirc", easeInOutCirc},
    {"EaseInBack", easeInBack},
    {"EaseOutBack", easeOutBack},
    {"EaseInOutBack", easeInOutBack},
    {"EaseInElastic", easeInElastic},
    {"EaseOutElastic", easeOutElastic},
    {"EaseInOutElastic", easeInOutElastic},
    {"EaseOutBounce", easeOutBounce},
    {"EaseInBounce", easeInBounce},
    {"EaseInOutBounce", easeInOutBounce}
};

#endif //RUBIK_EASING_H
