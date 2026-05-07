#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#ifdef GEODE_IS_ANDROID
#include <aaudio/AAudio.h>
#include <android/log.h>
#endif

#include <atomic>
#include <chrono>
#include <cmath>
#include <thread>

using namespace geode::prelude;
using namespace std::chrono;

// ─── Microphone state ───────────────────────────────────────────────────────

static std::atomic<float>  s_volume{0.f};       // current RMS volume
static std::atomic<bool>   s_micRunning{false};
static std::atomic<bool>   s_soundActive{false}; // is sound currently above threshold?

#ifdef GEODE_IS_ANDROID
static AAudioStream* s_stream = nullptr;

// AAudio callback — runs on audio thread
static aaudio_data_callback_result_t audioCallback(
    AAudioStream* /*stream*/,
    void* /*userData*/,
    void* audioData,
    int32_t numFrames
) {
    auto* samples = static_cast<int16_t*>(audioData);
    double sum = 0.0;
    for (int i = 0; i < numFrames; i++) {
        double s = samples[i] / 32768.0;
        sum += s * s;
    }
    float rms = static_cast<float>(std::sqrt(sum / numFrames));
    s_volume.store(rms, std::memory_order_relaxed);
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

    if (result != AAUDIO_OK) {
        log::warn("MicControl: Failed to open AAudio stream: {}", AAudio_convertResultToText(result));
        return false;
    }

    result = AAudioStream_requestStart(s_stream);
    if (result != AAUDIO_OK) {
        AAudioStream_close(s_stream);
        s_stream = nullptr;
        log::warn("MicControl: Failed to start AAudio stream: {}", AAudio_convertResultToText(result));
        return false;
    }

    s_micRunning = true;
    log::info("MicControl: Microphone started!");
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
    log::info("MicControl: Microphone stopped.");
}
#else
// Stub for non-Android builds
static bool startMicrophone() { return false; }
static void stopMicrophone() {}
#endif

// ─── Input injection ─────────────────────────────────────────────────────────

// Tracks state for hold detection
static bool     s_wasActive = false;
static time_point<steady_clock> s_soundStart;

// Called every frame from PlayLayer::update
static void processMicInput(PlayLayer* pl) {
    float sensitivity = Mod::get()->getSettingValue<double>("sensitivity");
    int holdThresholdMs = Mod::get()->getSettingValue<int64_t>("hold-threshold");

    float vol = s_volume.load(std::memory_order_relaxed);
    bool active = (vol >= sensitivity);

    if (active && !s_wasActive) {
        // Sound just started
        s_soundStart = steady_clock::now();
        s_soundActive = true;
        // Immediately trigger press
        pl->pushButton(1, true);
        log::debug("MicControl: press (vol={:.3f})", vol);
    } else if (!active && s_wasActive) {
        // Sound just ended
        s_soundActive = false;
        auto elapsed = duration_cast<milliseconds>(steady_clock::now() - s_soundStart).count();
        // Release
        pl->pushButton(1, false);
        log::debug("MicControl: release after {}ms (vol={:.3f})", elapsed, vol);
    }
    // If still active — hold is maintained (pushButton(1,true) stays pressed)

    s_wasActive = active;
}

// ─── PlayLayer hook ──────────────────────────────────────────────────────────

class $modify(MicPlayLayer, PlayLayer) {

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

#ifdef GEODE_IS_ANDROID
        // Request microphone — AAudio handles permission on modern Android
        // but we show a notification if it fails
        if (!startMicrophone()) {
            Notification::create(
                "MicControl: Failed to access microphone!\nCheck app permissions.",
                NotificationIcon::Error,
                3.f
            )->show();
        } else {
            Notification::create(
                "MicControl: Microphone active 🎙",
                NotificationIcon::Success,
                2.f
            )->show();
        }
#else
        Notification::create(
            "MicControl: Only supported on Android!",
            NotificationIcon::Warning,
            3.f
        )->show();
#endif
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (s_micRunning) {
            processMicInput(this);
        }
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
