#pragma once
#include "pch.h"
#include "../utils/Utils.h"

template <GLenum TFeature>
class FeatureFlag {
    inline static std::stack<bool> flagStack;
    inline static bool currentState = false;
    void set_state(bool newState) {
        if (currentState != newState) {
            if (newState) {
                glEnable(TFeature);
            } else {
                glDisable(TFeature);
            }
        }
        currentState = newState;
    }

   public:
    NO_COPY_CLASS(FeatureFlag);
    FeatureFlag(bool enable) {
        flagStack.push(currentState);
        set_state(enable);
    }

    ~FeatureFlag() {
        bool last = flagStack.top();
        flagStack.pop();
        set_state(last);
    }
};

typedef FeatureFlag<GL_DEPTH_TEST> DepthTest;
typedef FeatureFlag<GL_BLEND> Blend;

class Viewport {
    inline static std::stack<std::pair<int, int>> sizeStack;
    void set_state(int w, int h) { glViewport(0, 0, w, h); }

   public:
    NO_COPY_CLASS(Viewport);
    Viewport(int width, int height) {
        set_state(width, height);
        sizeStack.push(std::make_pair(width, height));
    }

    ~Viewport() {
        sizeStack.pop();
        if (!sizeStack.empty()) {
            set_state(sizeStack.top().first, sizeStack.top().second);
        }
    }
};

class DepthFunc {
    inline static std::stack<GLenum> funcStack;
    inline static GLenum currentState = GL_LESS;
    void set_state(GLenum newState) {
        if (currentState != newState) {
            glDepthFunc(newState);
        }
        currentState = newState;
    }

   public:
    NO_COPY_CLASS(DepthFunc);
    DepthFunc(GLenum func) {
        funcStack.push(currentState);
        set_state(func);
    }

    ~DepthFunc() {
        bool last = funcStack.top();
        funcStack.pop();
        set_state(last);
    }
};