#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#ifdef GEODE_IS_ANDROID
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#endif

#include <atomic>
#include <cmath>

using namespace geode::prelude;

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
    (*bq)->Enqueue(bq, s_buffer, sizeof(s_buffer));
}

static bool startMicrophone() {
    if (s_micRunning) return true;

    log::info("MicControl: startMicrophone() called");

    SLresult r;
    r = slCreateEngine(&s_engineObj, 0, nullptr, 0, nullptr, nullptr);
    if (r != SL_RESULT_SUCCESS) { log::error("MicControl: slCreateEngine failed {}", r); return false; }

    r = (*s_engineObj)->Realize(s_engineObj, SL_BOOLEAN_FALSE);
    if (r != SL_RESULT_SUCCESS) { log::error("MicControl: Realize engine failed {}", r); return false; }

    r = (*s_engineObj)->GetInterface(s_engineObj, SL_IID_ENGINE, &s_engine);
    if (r != SL_RESULT_SUCCESS) { log::error("MicControl: GetInterface engine failed {}", r); return false; }

    SLDataLocator_IODevice ioDevice = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT, SL_DEFAULTDEVICEID_AUDIOINPUT, nullptr};
    SLDataSource audioSrc = {&ioDevice, nullptr};
    SLDataLocator_AndroidSimpleBufferQueue bufQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSink audioSnk = {&bufQueue, &format};
    const SLInterfaceID ids[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    r = (*s_engine)->CreateAudioRecorder(s_engine, &s_recorderObj, &audioSrc, &audioSnk, 1, ids, req);
    if (r != SL_RESULT_SUCCESS) { log::error("MicControl: CreateAudioRecorder failed {}", r); return false; }

    r = (*s_recorderObj)->Realize(s_recorderObj, SL_BOOLEAN_FALSE);
    if (r != SL_RESULT_SUCCESS) { log::error("MicControl: Realize recorder failed {}", r); return false; }

    r = (*s_recorderObj)->GetInterface(s_recorderObj, SL_IID_RECORD, &s_recorder);
    if (r != SL_RESULT_SUCCESS) { log::error("MicControl: GetInterface record failed {}", r); return false; }

    r = (*s_recorderObj)->GetInterface(s_recorderObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &s_bufferQueue);
    if (r != SL_RESULT_SUCCESS) { log::error("MicControl: GetInterface bufferqueue failed {}", r); return false; }

    (*s_bufferQueue)->RegisterCallback(s_bufferQueue, bufferQueueCallback, nullptr);
    (*s_bufferQueue)->Enqueue(s_bufferQueue, s_buffer, sizeof(s_buffer));

    r = (*s_recorder)->SetRecordState(s_recorder, SL_RECORDSTATE_RECORDING);
    if (r != SL_RESULT_SUCCESS) { log::error("MicControl: SetRecordState failed {}", r); return false; }

    s_micRunning = true;
    log::info("MicControl: Microphone started successfully!");
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
    log::info("MicControl: Microphone stopped.");
}

#else
static bool startMicrophone() { return false; }
static void stopMicrophone() {}
#endif

static bool s_wasActive = false;
static float s_maxVolume = 0.f;
static int s_frameCount = 0;

static void processMicInput(GJBaseGameLayer* layer) {
    float sensitivity = (float)Mod::get()->getSettingValue<double>("sensitivity");
    float vol = s_volume.load(std::memory_order_relaxed);

    s_frameCount++;
    if (vol > s_maxVolume) s_maxVolume = vol;
    if (s_frameCount >= 60) {
        log::info("MicControl: RMS max={:.5f} sens={:.5f}", s_maxVolume, sensitivity);
        Notification::create(
            fmt::format("RMS: {:.4f} | sens: {:.4f}", s_maxVolume, sensitivity),
            NotificationIcon::None, 1.5f
        )->show();
        s_maxVolume = 0.f;
        s_frameCount = 0;
    }

    bool active = (vol >= sensitivity);
    if (active && !s_wasActive) layer->handleButton(true, 1, true);
    else if (!active && s_wasActive) layer->handleButton(false, 1, true);
    s_wasActive = active;
}

class $modify(MicPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
#ifdef GEODE_IS_ANDROID
        log::info("MicControl: PlayLayer::init called");
        if (!startMicrophone()) {
            Notification::create("MicControl: Mic FAILED!", NotificationIcon::Error, 5.f)->show();
        } else {
            Notification::create("MicControl: Mic OK!", NotificationIcon::Success, 2.f)->show();
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
        s_frameCount = 0;
        s_maxVolume = 0.f;
        PlayLayer::onQuit();
    }

    void levelComplete() {
        stopMicrophone();
        s_wasActive = false;
        PlayLayer::levelComplete();
    }
};
