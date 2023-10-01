#pragma once
#include "GuiFrame.h"
#include "GuiImage.h"
#include "GuiText.h"
#include "Timer.h"
#include "utils/logger.h"
#include <notifications/notification_defines.h>

typedef enum {
    OVERLAY_STATUS_INFO,
    OVERLAY_STATUS_ERROR,
    OVERLAY_STATUS_IN_PROGRESS,
} OverlayStatus;

typedef enum {
    OVERLAY_STATUS_NOTHING,
    OVERLAY_STATUS_WAIT,
    OVERLAY_STATUS_REQUESTED_SHAKE,
    OVERLAY_STATUS_EFFECT,
    OVERLAY_STATUS_REQUESTED_FADE_OUT_AND_EXIT,
    OVERLAY_STATUS_REQUESTED_EXIT,
} OverlayInternalStatus;

class OverlayFrame;

class Overlay : public GuiFrame {

public:
    friend class OverlayFrame;
    explicit Overlay(GX2Color backgroundColor                             = {100, 100, 100, 255});

    ~Overlay() override;

    void process() override;
    void draw(bool SRGBConversion) override;

    void updateText(const char *text) {
        mNotificationText.setText(text);
        mTextDirty = true;
        OSMemoryBarrier();
    }

    void updateBackgroundColor(GX2Color color) {
        mBackground.setImageColor(color);
        OSMemoryBarrier();
    }

    void updateTextColor(GX2Color textColor) {
        mNotificationText.setColor({textColor.r / 255.0f, textColor.g / 255.0f, textColor.b / 255.0f, textColor.a / 255.0f});
        OSMemoryBarrier();
    }

    void updateStatus(OverlayStatus newStatus);

    OverlayStatus getStatus() {
        return mStatus;
    }

    uint32_t getHandle() {
        return (uint32_t) this;
    }

    void updateWaitDuration(float duration) {
        this->mDelayBeforeFadeoutInSeconds = duration;
    }

    void updateShakeDuration(float duration) {
        this->mShakeDurationInSeconds = duration;
    }

    void callDeleteCallback() {
        if (mRemovedFromOverlayCallback != nullptr) {
            mRemovedFromOverlayCallback(this);
            mRemovedFromOverlayCallback = nullptr;
        }
    }

    void setPosition(float x, float y) override {
        GuiElement::setPosition(x, y);
        mPositionSet = true;
    }

private:
    std::function<void(NotificationModuleHandle, void *)> mFinishFunction;
    std::function<void(Overlay *)> mRemovedFromOverlayCallback;

    void *mFinishFunctionContext;
    GuiImage mBackground;
    GuiText mNotificationText;
    Timer mTimer;
    float mDelayBeforeFadeoutInSeconds = 1.0f;
    float mShakeDurationInSeconds = 1.0f;
    bool mFinishFunctionCalled = false;
    bool mWaitForReset         = false;

    bool mTextDirty   = false;
    bool mPositionSet = false;

    OverlayStatus mStatus                 = OVERLAY_STATUS_INFO;
    OverlayInternalStatus mInternalStatus = OVERLAY_STATUS_NOTHING;
};
