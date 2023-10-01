#include "OverlayFrame.h"

void OverlayFrame::addNotification(std::shared_ptr<Notification> status) {
    status->setParent(this);
    append(status.get());
    status->setAlignment(ALIGN_TOP_LEFT);
    status->setEffect(EFFECT_FADE, 55, 255);
    {
        std::lock_guard<std::mutex> lock(gNotificationListMutex);
        list.push_front(std::move(status));
    }
}

void OverlayFrame::addOverlay(std::shared_ptr<Overlay> status) {
    status->setParent(this);
    append(status.get());
    status->setAlignment(ALIGN_TOP_LEFT);
    status->setEffect(EFFECT_FADE, 0, 255);
    {
        std::lock_guard<std::mutex> lockOverlay(gOverlayListMutex);
        listOverlay.push_front(std::move(status));
    }
}

void OverlayFrame::OnFadeOutFinished(GuiElement *element) {
    auto *item = dynamic_cast<Notification *>(element);
    if (item) {
        item->mInternalStatus = NOTIFICATION_STATUS_REQUESTED_EXIT;
    } else {
        DEBUG_FUNCTION_LINE_ERR("Failed to fade out item: dynamic cast failed");
    }
}

void OverlayFrame::OnFadeOutOverlayFinished(GuiElement *element) {
    auto *item = dynamic_cast<Overlay *>(element);
    if (item) {
        item->mInternalStatus = OVERLAY_STATUS_REQUESTED_EXIT;
    } else {
        DEBUG_FUNCTION_LINE_ERR("Failed to fade out item: dynamic cast failed");
    }
}

void OverlayFrame::OnShakeFinished(GuiElement *element) {
    auto *item = dynamic_cast<Notification *>(element);
    if (item) {
        item->mInternalStatus = NOTIFICATION_STATUS_WAIT;
    }
}

void OverlayFrame::clearElements() {
    std::lock_guard<std::mutex> lock(gNotificationListMutex);
    for (auto &element : list) {
        remove(element.get());
    }
    list.clear();

    std::lock_guard<std::mutex> lockOverlay(gOverlayListMutex);
    for (auto &element : listOverlay) {
        remove(element.get());
    }
    listOverlay.clear();
}

void OverlayFrame::process() {
    GuiFrame::process();

    std::lock_guard<std::mutex> lock(gNotificationListMutex);

    float offset = -25.0f;
    for (auto &item : list) {
        item->process();
        item->setPosition(25, offset);
        offset -= (item->getHeight() + 10.0f);
        if (item->mInternalStatus == NOTIFICATION_STATUS_REQUESTED_FADE_OUT_AND_EXIT) {
            item->resetEffects();
            item->setEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT | EFFECT_SLIDE_FROM, 30);
            item->effectFinished.connect(this, &OverlayFrame::OnFadeOutFinished);
            item->finishFunction();
            item->mInternalStatus = NOTIFICATION_STATUS_EFFECT;
        } else if (item->mInternalStatus == NOTIFICATION_STATUS_REQUESTED_SHAKE) {
            item->resetEffects();
            // shake for fixed duration, even if we drop frames
            item->setEffect(EFFECT_SHAKE, 0, (int32_t) (item->mShakeDurationInSeconds * 1000));
            item->effectFinished.connect(this, &OverlayFrame::OnShakeFinished);
            item->mInternalStatus = NOTIFICATION_STATUS_EFFECT;
        }
    }
    auto oit = list.before_begin(), it = std::next(oit);
    while (it != list.end()) {
        if ((*it)->mInternalStatus == NOTIFICATION_STATUS_REQUESTED_EXIT) {
            (*it)->callDeleteCallback();
            remove((*it).get());
            list.erase_after(oit);
            break;
        }
        oit = it++;
    }

    std::lock_guard<std::mutex> lockOverlay(gOverlayListMutex);

    for (auto &item : listOverlay) {
        item->process();
        if (item->mInternalStatus == OVERLAY_STATUS_REQUESTED_FADE_OUT_AND_EXIT) {
            item->resetEffects();
            item->setEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT | EFFECT_SLIDE_FROM, 1280);
            item->effectFinished.connect(this, &OverlayFrame::OnFadeOutOverlayFinished);
            item->mInternalStatus = OVERLAY_STATUS_EFFECT;
        }
    }
    auto oitOverlay = listOverlay.before_begin(), itOverlay = std::next(oitOverlay);
    while (itOverlay != listOverlay.end()) {
        if ((*itOverlay)->mInternalStatus == OVERLAY_STATUS_REQUESTED_EXIT) {
            (*itOverlay)->callDeleteCallback();
            remove((*itOverlay).get());
            listOverlay.erase_after(oitOverlay);
            break;
        }
        oitOverlay = itOverlay++;
    }
}
