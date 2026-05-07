#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#ifdef GEODE_IS_ANDROID
#include <aaudio/AAudio.h>
#endif

#include <atomic>
#include <chrono>
#include <cmath>

using namespace geode::prelude;
using namespace std::chrono;

static std::atomic<float> s_volume{0.f};
static std::atomic<bool>  s_micRunning{false};

#ifdef GEODE_IS_ANDROID
static AAudioStream* s_stream = nullptr;

static aaudio_data_callback_result_t audioCallback(
    AAudioStream*, void*, void* audioData, int32_t numFrames
) {
    auto* samples = static_cast<int16_t*>(audioData);
    double sum = 0.0;
    for (int i = 0; i < numFrames; i++) {
        double s = samples[i] / 32768.0;
        sum += s * s;
    }
    s_volume.store(static_cast<float>(std::sqrt(sum / numFrames)), std::memory_order_relaxed);
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

static bool startMicrophone() {
    if (s_micRunning) return true;
    AAudioStreamBuilder* builder = nullptr;
    if (AAudio_createStreamBuilder(&builder) != AAUDIO_OK) return false;
    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_INPUT);
    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setChannelCount(builder, 1);
    AAudioStreamBuilder_setSampleRate(builder, 16000);
    AAudioStreamBuilder_setFramesPerDataCallback(builder, 512);
    AAudioStreamBuilder_setDataCallback(builder, audioCallback, nullptr);
    AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setInputPreset(builder, AAUDIO_INPUT_PRESET_VOICE_RECOGNITION);
    aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &s_stream);
    AAudioStreamBuilder_delete(builder);
    if (result != AAUDIO_OK) return false;
    if (AAudioStream_requestStart(s_stream) != AAUDIO_OK) {
        AAudioStream_close(s_stream);
        s_stream = nullptr;
        return false;
    }
    s_micRunning = true;
    return true;
}

static void stopMicrophone() {
    if (!s_micRunning) return;
    if (s_stream) {
        AAudioStream_requestStop(s_stream);
        AAudioStream_close(s_stream);
        s_stream = nullptr;
    }
    s_micRunning = false;
    s_volume.store(0.f, std::memory_order_relaxed);
}
#else
static bool startMicrophone() { return false; }
static void stopMicrophone() {}
#endif

static bool s_wasActive = false;

static void processMicInput(GJBaseGameLayer* layer) {
    float sensitivity = (float)Mod::get()->getSettingValue<double>("sensitivity");
    float vol = s_volume.load(std::memory_order_relaxed);
    bool active = (vol >= sensitivity);

    if (active && !s_wasActive) {
        layer->handleButton(true, 1, true);
    } else if (!active && s_wasActive) {
        layer->handleButton(false, 1, true);
    }
    s_wasActive = active;
}

class $modify(MicPlayLayer, PlayLayer) {

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
#ifdef GEODE_IS_ANDROID
        if (!startMicrophone()) {
            Notification::create(
                "MicControl: No mic access! Check permissions.",
                NotificationIcon::Error, 3.f
            )->show();
        } else {
            Notification::create(
                "MicControl: Mic active!",
                NotificationIcon::Success, 2.f
            )->show();
        }
#endif
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (s_micRunning) processMicInput(this);
    }

    void onQuit() {
        stopMicrophone();
        s_wasActive = false;
        PlayLayer::onQuit();
    }

    void levelComplete() {
        stopMicrophone();
        s_wasActive = false;
        PlayLayer::levelComplete();
    }
};
