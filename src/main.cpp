#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#ifdef GEODE_IS_ANDROID
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#endif

#include <atomic>
#include <chrono>
#include <cmath>

using namespace geode::prelude;
using namespace std::chrono;

static std::atomic<float> s_volume{0.f};
static std::atomic<bool>  s_micRunning{false};

#ifdef GEODE_IS_ANDROID

#define BUFFER_SIZE 1024
static SLObjectItf s_engineObj = nullptr;
static SLEngineItf s_engine = nullptr;
static SLObjectItf s_recorderObj = nullptr;
static SLRecordItf s_recorder = nullptr;
static SLAndroidSimpleBufferQueueItf s_bufferQueue = nullptr;
static int16_t s_buffer[BUFFER_SIZE];

static void bufferQueueCallback(SLAndroidSimpleBufferQueueItf bq, void*) {
    double sum = 0.0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        double s = s_buffer[i] / 32768.0;
        sum += s * s;
    }
    float rms = static_cast<float>(std::sqrt(sum / BUFFER_SIZE));
    s_volume.store(rms, std::memory_order_relaxed);
    // Логуємо кожен фрейм щоб побачити реальні значення
    log::debug("MicControl RMS: {:.5f}", rms);
    (*bq)->Enqueue(bq, s_buffer, sizeof(s_buffer));
}

static bool startMicrophone() {
    if (s_micRunning) return true;
    SLDataLocator_IODevice ioDevice = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT, SL_DEFAULTDEVICEID_AUDIOINPUT, nullptr};
    SLDataSource audioSrc = {&ioDevice, nullptr};
    SLDataLocator_AndroidSimpleBufferQueue bufQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSink audioSnk = {&bufQueue, &format};
    const SLInterfaceID ids[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    if (slCreateEngine(&s_engineObj, 0, nullptr, 0, nullptr, nullptr) != SL_RESULT_SUCCESS) return false;
    if ((*s_engineObj)->Realize(s_engineObj, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS) return false;
    if ((*s_engineObj)->GetInterface(s_engineObj, SL_IID_ENGINE, &s_engine) != SL_RESULT_SUCCESS) return false;
    if ((*s_engine)->CreateAudioRecorder(s_engine, &s_recorderObj, &audioSrc, &audioSnk, 1, ids, req) != SL_RESULT_SUCCESS) return false;
    if ((*s_recorderObj)->Realize(s_recorderObj, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS) return false;
    if ((*s_recorderObj)->GetInterface(s_recorderObj, SL_IID_RECORD, &s_recorder) != SL_RESULT_SUCCESS) return false;
    if ((*s_recorderObj)->GetInterface(s_recorderObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &s_bufferQueue) != SL_RESULT_SUCCESS) return false;

    (*s_bufferQueue)->RegisterCallback(s_bufferQueue, bufferQueueCallback, nullptr);
    (*s_bufferQueue)->Enqueue(s_bufferQueue, s_buffer, sizeof(s_buffer));
    (*s_recorder)->SetRecordState(s_recorder, SL_RECORDSTATE_RECORDING);

    s_micRunning = true;
    return true;
}

static void stopMicrophone() {
    if (!s_micRunning) return;
    if (s_recorder) (*s_recorder)->SetRecordState(s_recorder, SL_RECORDSTATE_STOPPED);
    if (s_recorderObj) { (*s_recorderObj)->Destroy(s_recorderObj); s_recorderObj = nullptr; }
    if (s_engineObj) { (*s_engineObj)->Destroy(s_engineObj); s_engineObj = nullptr; }
    s_engine = nullptr; s_recorder = nullptr; s_bufferQueue = nullptr;
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
        log::info("MicControl: JUMP! vol={:.5f} sensitivity={:.5f}", vol, sensitivity);
        layer->handleButton(true, 1, true);
    } else if (!active && s_wasActive) {
        log::info("MicControl: RELEASE vol={:.5f}", vol);
        layer->handleButton(false, 1, true);
    }
    s_wasActive = active;
}

class $modify(MicPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
#ifdef GEODE_IS_ANDROID
        if (!startMicrophone()) {
            Notification::create("MicControl: No mic access!", NotificationIcon::Error, 3.f)->show();
        } else {
            Notification::create("MicControl: Mic active!", NotificationIcon::Success, 2.f)->show();
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
