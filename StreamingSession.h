/**
 * @file StreamingSession.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef STREAMINGSESSION_H
#define STREAMINGSESSION_H

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
    uint8_t sessionId[16];
    HAPCharacteristicValue_StreamingStatus status;
    controllerAddressStruct controller_address;
    srtpParameters videoParams;
    srtpParameters audioParams;
    controllerAddressStruct accessory_address;
    uint32_t ssrcVideo;
    uint32_t ssrcAudio;
} streamingSession;

HAPError handleRead(HAPTLVWriterRef* responseWriter, streamingSession* session);

HAPError handleWrite(HAPTLVReaderRef* responseReader, streamingSession* settings);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif