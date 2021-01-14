/*
 * App_Camera.h
 *
 *  Created on: Dec 29, 2020
 *      Author: joe
 */

#ifndef APP_CAMERA_H
#define APP_CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"
// #include "HAPBase.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

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

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif /* APP_CAMERA_H */
