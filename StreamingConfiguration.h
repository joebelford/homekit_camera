/**
 * @file StreamingConfiguration.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef STREAMINGCONFIGURATION_H
#define STREAMINGCONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"
// #include "HAPBase.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

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

HAPError handleAudioRead(HAPTLVWriterRef* responseWriter);
HAPError handleVideoRead(HAPTLVWriterRef* responseWriter);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif /* STREAMINGCONFIGURATION_H */