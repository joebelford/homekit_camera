/**
 * @file streaming.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef STREAMING_H
#define STREAMING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"
#include "HAPTLV+Internal.h"
#include <arpa/inet.h>

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

#define KEYLENGTH  16
#define SALTLENGTH 14
#define UUIDLENGTH 16

typedef struct {
    uint8_t audioChannels;
    uint8_t bitRate;
    uint8_t sampleRate;
    uint8_t rtpTime;
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


HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_RTPCommand) { /** Inactive. */
                                                            kHAPCharacteristicValue_RTPCommand_End = 0,

                                                            /** Active. */
                                                            kHAPCharacteristicValue_RTPCommand_Start,

                                                            /** Active. */
                                                            kHAPCharacteristicValue_RTPCommand_Suspend,

                                                            /** Active. */
                                                            kHAPCharacteristicValue_RTPCommand_Resume,

                                                            /** Active. */
                                                            kHAPCharacteristicValue_RTPCommand_Reconfigure
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_RTPCommand);

typedef struct {
    uint8_t payloadType; //type of video codec
    uint32_t ssrc; //ssrc for video stream
    uint16_t maximumBitrate; //in kbps and averaged over 1 sec
    uint32_t minRTCPinterval; //Minimum RTCP interval in seconds formatted as a 4 byte little endian ieee754 floating point value
} rtpParameters;

typedef struct {
    rtpParameters vRtpParameters;
    uint16_t maxMTU; //MTU accessory must use to transmit video RTP packets  Only populated for non-default?? value.
} videoRtpParameters;

typedef struct {
    rtpParameters rtpParameters;
    uint8_t comfortNoisePayload; //Only required when Comfort Noise is chosen in selected audio parameters TLV
} audioRtpParameters;

typedef struct {
    videoCodecConfigStruct codecConfig;
    videoRtpParameters vRtpParameters;
} selectedVideoParameters;

typedef struct {
    audioCodecConfigStruct codecConfig;
    audioRtpParameters rtpParameters;
    uint8_t comfortNoise; //1 = Comfort Noise selected
} selectedAudioParameters;

typedef struct {
    uint8_t ipAddrVersion; // Tried to use HAPIPAddressVersion but it says v4 = 1 but docs say 0.
    in_addr_t ipAddress;
    HAPNetworkPort videoPort;
    HAPNetworkPort audioPort;
} controllerAddressStruct;

typedef struct {
    HAPCharacteristicValue_SupportedRTPConfiguration srtpCryptoSuite;
    uint8_t srtpMasterKey[16]; // 16 for 128, 32 for 256 crypto suite
    uint8_t srtpMasterSalt[14];
} srtpParameters;

typedef struct {
    uint8_t sessionId[UUIDLENGTH];
    HAPCharacteristicValue_StreamingStatus status;
    controllerAddressStruct controllerAddress;
    srtpParameters videoParams;
    srtpParameters audioParams;
    controllerAddressStruct accessoryAddress;
    uint32_t ssrcVideo;
    uint32_t ssrcAudio;
    selectedVideoParameters videoParameters;
    selectedAudioParameters audioParameters;
} streamingSession;

typedef struct {
    uint8_t sessionId[UUIDLENGTH];
    HAPCharacteristicValue_RTPCommand command;
} sessionControl;


typedef struct {
    sessionControl control;
    selectedVideoParameters videoParameters;
    selectedAudioParameters audioParameters;
} selectedRTPStruct;

HAPError handleSessionRead(HAPTLVWriterRef* responseWriter, streamingSession* session);

HAPError handleSessionWrite(HAPTLVReaderRef* responseReader, streamingSession* settings);

HAPError handleSelectedWrite(HAPTLVReaderRef* responseReader, selectedRTPStruct* selectedRTP);

void checkFormats();

HAPError handleAudioRead(HAPTLVWriterRef* responseWriter);
HAPError handleVideoRead(HAPTLVWriterRef* responseWriter);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif /* STREAMING_H */