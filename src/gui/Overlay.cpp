#include "Overlay.h"

Overlay::Overlay(GX2Color backgroundColor) : GuiFrame(0, 0), mBackground(0, 0, backgroundColor) {
    mBackground.setImageColor(backgroundColor);

    mNotificationText.setColor({1.0f, 1.0f, 1.0f, 1.0f});
    mNotificationText.setPosition(0, 0);
    mNotificationText.setFontSize(20);
    mNotificationText.setAlignment(ALIGN_CENTERED);

    setPosition(0, 0);

    append(&mBackground);
    append(&mNotificationText);
}

Overlay::~Overlay() {
    remove(&mNotificationText);
    remove(&mBackground);
}

void Overlay::process() {
    GuiFrame::process();

    if (mWaitForReset) {
        mTimer.reset();
        mWaitForReset = false;
        return;
    }

    if (mInternalStatus == OVERLAY_STATUS_WAIT) {
        if (mTimer.elapsed() >= mDelayBeforeFadeoutInSeconds) {
            mInternalStatus = OVERLAY_STATUS_REQUESTED_FADE_OUT_AND_EXIT;
        }
    }
}

void Overlay::draw(bool SRGBConversion) {
    if (!mPositionSet) {
        return;
    }
    if (mTextDirty) {
        mNotificationText.updateTextSize();
        mTextDirty = false;
    }
    width  = 1280.0f;
    height = 720.0f;

    mBackground.setSize(width, height);
    GuiFrame::draw(SRGBConversion);
}

void Overlay::updateStatus(OverlayStatus newStatus) {
    if (newStatus == OVERLAY_STATUS_INFO) {
        mInternalStatus = OVERLAY_STATUS_WAIT;
    } else {
        return;
    }
    mWaitForReset = true;
    this->mStatus = newStatus;
}