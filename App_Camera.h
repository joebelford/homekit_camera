/*
 * App_Camera.h
 *
 *  Created on: Dec 29, 2020
 *      Author: joe
 */

#ifndef APPLICATIONS_CAMERA_APP_CAMERA_H_
#define APPLICATIONS_CAMERA_APP_CAMERA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"
#include "HAPBase.h"
// #include "libavformat/avformat.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

typedef struct {
    uint8_t audioChannels;
    uint8_t bitRate;
    uint8_t sampleRate;
} audioCodecParamsStruct;

typedef struct {
    uint16_t audioCodecType;
    audioCodecParamsStruct audioCodecParams;
} audioCodecConfigStruct;

typedef struct {
    audioCodecConfigStruct audioCodecConfig;
    uint8_t comfortNoiseSupport;

} supportedAudioConfigStruct;

typedef struct {
    uint8_t profileID;
    uint8_t level;
    uint8_t packetizationMode;
    uint8_t CVOEnabled;
} videoCodecParamsStruct;

typedef struct {
    uint16_t imageWidth;
    uint16_t imageHeight;
    uint8_t frameRate;
} videoAttributesStruct;

typedef struct {
    uint8_t videoCodecType;
    videoCodecParamsStruct videoCodecParams;
    videoAttributesStruct videoAttributes;

} videoCodecConfigStruct;

typedef struct {
    videoCodecConfigStruct videoCodecConfig;
} supportedVideoConfigStruct;

typedef struct {
        HAPIPAddressVersion ipAddrVersion;
        char* ipAddress;
        HAPNetworkPort videoPort;
        HAPNetworkPort audioPort;
} controllerAddress;

typedef struct {
        HAPCharacteristicValue_SupportedRTPConfiguration srtpCryptoSuite;
        char srtpMasterKey [16]; //16 for 128, 32 for 256 crypto suite
        char srtpMasterSalt [14];

} srtpParameters;

typedef struct {
        HAPUUID sessionId;
        HAPCharacteristicValue_StreamingStatus status;
        controllerAddress address;
        srtpParameters videoParams;
        srtpParameters audioParams;
        uint32_t ssrcVideo;
        uint32_t ssrcAudio;
}setupEndpointsStruct;


/**
 * Handle read request to the 'Streaming Status' characteristic of the IP camera service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleStreamingStatusRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context);

/**
 * Handle read request to the 'Supported Audio' characteristic of the IP camera service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleSupportedAudioRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context);

/**
 * Handle read request to the 'Supported Video' characteristic of the IP camera service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleSupportedVideoRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context);

/**
 * Handle read request to the 'Supported Video' characteristic of the IP camera service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleSupportedRTPConfigRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context);

/**
 * Handle read request to the 'Selected RTP' characteristic of the IP camera service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleSelectedRTPConfigRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context);

/**
 * Handle write request to the 'Selected RTP' characteristic of the IP camera service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleSelectedRTPConfigWrite(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicWriteRequest* request,
        HAPTLVReaderRef* requestReader,
        void* _Nullable context);

/**
 * Handle read request to the 'Setup Endpoints' characteristic of the IP camera service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleSetupEndpointsRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context);

/**
 * Handle write request to the 'Setup Endpoints' characteristic of the IP camera service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleSetupEndpointsWrite(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicWriteRequest* request,
        HAPTLVReaderRef* requestReader,
        void* _Nullable context);

HAP_RESULT_USE_CHECK
HAPError HandleMicMuteRead(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicReadRequest* request,
        bool* value,
        void* _Nullable context);

HAP_RESULT_USE_CHECK
HAPError HandleMicMuteWrite(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicWriteRequest* request,
        bool value,
        void* _Nullable context);

HAP_RESULT_USE_CHECK
HAPError HandleMotionDetectedRead(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicReadRequest* request,
        bool* value,
        void* _Nullable context);

HAP_RESULT_USE_CHECK
HAPError HandleRTPActiveRead(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicReadRequest* request,
        HAPCharacteristicValue_Active* value,
        void* _Nullable context);

HAP_RESULT_USE_CHECK
HAPError HandleRTPActiveWrite(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicWriteRequest* request,
        HAPCharacteristicValue_Active value,
        void* _Nullable context);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif /* APPLICATIONS_CAMERA_APP_CAMERA_H_ */
