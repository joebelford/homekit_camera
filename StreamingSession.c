/**
 * @file StreamingSession.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "StreamingSession.h"
#include <string.h>

static bool isValid(void* unsused HAP_UNUSED) {
    return true;
};

/**
 * The callback used to decode a value.
 *
 * @param[out] value                Decoded value.
 * @param      bytes                Encoded value buffer. May be modified.
 * @param      numBytes             Length of encoded value buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If invalid data was encountered while parsing.
 */
HAP_RESULT_USE_CHECK
HAPError decodeIpAddress(in_addr_t* value, void* bytes, size_t numBytes) {
    in_addr_t* ia = value;
    int s;
    char ipAddr[numBytes + 1]; // Not sure why unless char needs \0
    HAPRawBufferZero(ipAddr, numBytes + 1);
    HAPRawBufferCopyBytes(&ipAddr, bytes, numBytes);
    s = inet_pton(AF_INET, ipAddr, ia);
    if (s <= 0) {
        HAPLogError(&kHAPLog_Default, "%s\n", "Invalid address");
        return kHAPError_InvalidData;
    };
    return kHAPError_None;
};

HAP_RESULT_USE_CHECK
HAPError decodeUInt8value(uint8_t* value, void* bytes, size_t numBytes) {
    HAPRawBufferCopyBytes((uint8_t*) value, (uint8_t*) bytes, numBytes);
    return kHAPError_None;
};

/**
 * The callback used to encode a value.
 *
 * @param      value                Value to encode.
 * @param[out] bytes                Encoded value buffer.
 * @param      maxBytes             Capacity of encoded value buffer.
 * @param[out] numBytes             Length of encoded value buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an unknown error occurred while serializing.
 * @return kHAPError_InvalidState   If serialization is not possible in the current state.
 * @return kHAPError_OutOfResources If out of resources while serializing.
 * @return kHAPError_Busy           If serialization is temporarily not possible.
 */
HAPError encodeIpAddress(in_addr_t* value, void* bytes, size_t maxBytes, size_t* numBytes) {
    if (maxBytes < INET_ADDRSTRLEN) {
        return kHAPError_OutOfResources;
    };
    in_addr_t* ia = value;
    inet_ntop(AF_INET, ia, bytes, INET_ADDRSTRLEN);
    if (bytes == NULL) {
        return kHAPError_Unknown;
    }
    *numBytes = strlen(bytes);
    return kHAPError_None;
};

HAPError encodeUInt8UUIDValue(uint8_t* value, void* bytes, size_t maxBytes HAP_UNUSED, size_t* numBytes) {
    HAPRawBufferCopyBytes((uint8_t*) bytes, (uint8_t*) value, UUIDLENGTH);
    *numBytes = UUIDLENGTH;
    return kHAPError_None;
};

HAPError encodeUInt8KeyValue(uint8_t* value, void* bytes, size_t maxBytes HAP_UNUSED, size_t* numBytes) {
    HAPRawBufferCopyBytes((uint8_t*) bytes, (uint8_t*) value, KEYLENGTH);
    *numBytes = KEYLENGTH;
    return kHAPError_None;
};

HAPError encodeUInt8SaltValue(uint8_t* value, void* bytes, size_t maxBytes HAP_UNUSED, size_t* numBytes) {
    HAPRawBufferCopyBytes((uint8_t*) bytes, (uint8_t*) value, SALTLENGTH);
    *numBytes = SALTLENGTH;
    return kHAPError_None;
};

/**
 * The callback used to get the description of a value.
 *
 * @param      value                Valid value.
 * @param[out] bytes                Buffer to fill with the value's description. Will be NULL-terminated.
 * @param      maxBytes             Capacity of buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 */

HAPError getIpAddressDescription(in_addr_t* value, char* bytes, size_t maxBytes) {
    if (maxBytes < INET_ADDRSTRLEN) {
        return kHAPError_OutOfResources;
    };
    in_addr_t* ia = value;
    inet_ntop(AF_INET, ia, bytes, INET_ADDRSTRLEN);
    return kHAPError_None;
};

HAPError getUInt8ValueDescription(uint8_t* value HAP_UNUSED, char* bytes, size_t maxBytes HAP_UNUSED) {
    char description[] = "UInt8 Value Description";
    HAPRawBufferCopyBytes(bytes, description, sizeof(description));
    return kHAPError_None;
};

/* ---------------------------------------------------------------------------------------------*/

HAP_STRUCT_TLV_SUPPORT(void, AddressTypeFormat);
HAP_STRUCT_TLV_SUPPORT(void, SrtpParametersFormat);
HAP_STRUCT_TLV_SUPPORT(void, StreamingSessionFormat);
HAP_STRUCT_TLV_SUPPORT(void, SelectedRTPFormat);
HAP_STRUCT_TLV_SUPPORT(void, RTPParametersFormat);
HAP_STRUCT_TLV_SUPPORT(void, VideoRTPParametersTypeFormat);
HAP_STRUCT_TLV_SUPPORT(void, AudioRTPParametersTypeFormat);
HAP_STRUCT_TLV_SUPPORT(void, SelectedVideoParametersFormat);
HAP_STRUCT_TLV_SUPPORT(void, SelectedAudioParametersFormat);
HAP_STRUCT_TLV_SUPPORT(void, SessionControlTypeFormat);
HAP_VALUE_TLV_SUPPORT(in_addr_t, IpAddressTypeFormat);
HAP_VALUE_TLV_SUPPORT(uint8_t, SessionIdTypeFormat);
HAP_VALUE_TLV_SUPPORT(uint8_t, SrtpMasterKeyTypeFormat);
HAP_VALUE_TLV_SUPPORT(uint8_t, SrtpMasterSaltTypeFormat);

/* ---------------------------------------------------------------------------------------------*/

const HAPUInt8TLVFormat ipAddrVersionTypeFormat = { .type = kHAPTLVFormatType_UInt8,
                                                    .constraints = { .minimumValue = 0, .maximumValue = 1 },
                                                    .callbacks = { .getBitDescription = NULL,
                                                                   .getDescription = NULL } };

const HAPStructTLVMember ipAddrVersionTypeMember = { .valueOffset =
                                                             HAP_OFFSETOF(controllerAddressStruct, ipAddrVersion),
                                                     .isSetOffset = 0,
                                                     .tlvType = 1,
                                                     .debugDescription = "Controller IP Address Type",
                                                     .format = &ipAddrVersionTypeFormat,
                                                     .isOptional = false,
                                                     .isFlat = false };

const IpAddressTypeFormat ipAddressTypeFormat = {
    .type = kHAPTLVFormatType_Value,
    .callbacks = { .decode = decodeIpAddress, .encode = encodeIpAddress, .getDescription = getIpAddressDescription }
};

const HAPStructTLVMember ipAddressTypeMember = { .valueOffset = HAP_OFFSETOF(controllerAddressStruct, ipAddress),
                                                 .isSetOffset = 0,
                                                 .tlvType = 2,
                                                 .debugDescription = "IP Address Type",
                                                 .format = &ipAddressTypeFormat,
                                                 .isOptional = false,
                                                 .isFlat = false };

const HAPUInt16TLVFormat rtpPortTypeFormat = { .type = kHAPTLVFormatType_UInt16,
                                               .constraints = { .minimumValue = 49152, .maximumValue = 65535 },
                                               .callbacks = { .getBitDescription = NULL, .getDescription = NULL } };

const HAPStructTLVMember videoPortTypeMember = { .valueOffset = HAP_OFFSETOF(controllerAddressStruct, videoPort),
                                                 .isSetOffset = 0,
                                                 .tlvType = 3,
                                                 .debugDescription = "Video RTP Port Type",
                                                 .format = &rtpPortTypeFormat,
                                                 .isOptional = false,
                                                 .isFlat = false };

const HAPStructTLVMember audioPortTypeMember = { .valueOffset = HAP_OFFSETOF(controllerAddressStruct, audioPort),
                                                 .isSetOffset = 0,
                                                 .tlvType = 4,
                                                 .debugDescription = "Audio RTP Port Type",
                                                 .format = &rtpPortTypeFormat,
                                                 .isOptional = false,
                                                 .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const AddressTypeFormat addressTypeFormat = { .type = kHAPTLVFormatType_Struct,
                                              .members = (const HAPStructTLVMember* const[]) { &ipAddrVersionTypeMember,
                                                                                               &ipAddressTypeMember,
                                                                                               &videoPortTypeMember,
                                                                                               &audioPortTypeMember,
                                                                                               NULL },
                                              .callbacks = { .isValid = isValid } };

const HAPStructTLVMember controllerAddressTypeMember = { .valueOffset =
                                                                 HAP_OFFSETOF(streamingSession, controllerAddress),
                                                         .isSetOffset = 0,
                                                         .tlvType = 3,
                                                         .debugDescription = "Controller Address Type",
                                                         .format = &addressTypeFormat,
                                                         .isOptional = false,
                                                         .isFlat = false };

const HAPStructTLVMember accessoryAddressTypeMember = { .valueOffset = HAP_OFFSETOF(streamingSession, accessoryAddress),
                                                        .isSetOffset = 0,
                                                        .tlvType = 3,
                                                        .debugDescription = "Controller Address Type",
                                                        .format = &addressTypeFormat,
                                                        .isOptional = false,
                                                        .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const SessionIdTypeFormat sessionIdTypeFormat = { .type = kHAPTLVFormatType_Value,
                                                  .callbacks = { .decode = decodeUInt8value,
                                                                 .encode = encodeUInt8UUIDValue,
                                                                 .getDescription = getUInt8ValueDescription } };

const HAPStructTLVMember sessionIdTypeMember = { .valueOffset = HAP_OFFSETOF(streamingSession, sessionId),
                                                 .isSetOffset = 0,
                                                 .tlvType = 1,
                                                 .debugDescription = "Session ID Type",
                                                 .format = &sessionIdTypeFormat,
                                                 .isOptional = false,
                                                 .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const HAPUInt8TLVFormat streamingStatusTypeFormat = { .type = kHAPTLVFormatType_UInt8,
                                                      .constraints = { .minimumValue = 0, .maximumValue = 2 },
                                                      .callbacks = { .getDescription = NULL,
                                                                     .getBitDescription = NULL } };

const HAPStructTLVMember streamingStatusTypeMember = { .valueOffset = HAP_OFFSETOF(streamingSession, status),
                                                       .isSetOffset = 0,
                                                       .tlvType = 2,
                                                       .debugDescription = "Streaming Status Type",
                                                       .format = &streamingStatusTypeFormat,
                                                       .isOptional = false,
                                                       .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const HAPUInt8TLVFormat srtpCryptoSuiteTypeFormat = { .type = kHAPTLVFormatType_UInt8,
                                                      .constraints = { .minimumValue = 0, .maximumValue = 2 },
                                                      .callbacks = { .getBitDescription = NULL,
                                                                     .getDescription = NULL } };

const SrtpMasterKeyTypeFormat srtpMasterKeyTypeFormat = { .type = kHAPTLVFormatType_Value,
                                                          .callbacks = { .decode = decodeUInt8value,
                                                                         .encode = encodeUInt8KeyValue,
                                                                         .getDescription = getUInt8ValueDescription } };

const SrtpMasterSaltTypeFormat srtpMasterSaltTypeFormat = { .type = kHAPTLVFormatType_Value,
                                                            .callbacks = { .decode = decodeUInt8value,
                                                                           .encode = encodeUInt8SaltValue,
                                                                           .getDescription =
                                                                                   getUInt8ValueDescription } };

const HAPStructTLVMember srtpCryptoSuiteTypeMember = { .valueOffset = HAP_OFFSETOF(srtpParameters, srtpCryptoSuite),
                                                       .isSetOffset = 0,
                                                       .tlvType = 1,
                                                       .debugDescription = "SRTP Crypto Suite Type",
                                                       .format = &srtpCryptoSuiteTypeFormat,
                                                       .isOptional = false,
                                                       .isFlat = false };

const HAPStructTLVMember srtpMasterKeyTypeMember = { .valueOffset = HAP_OFFSETOF(srtpParameters, srtpMasterKey),
                                                     .isSetOffset = 0,
                                                     .tlvType = 2,
                                                     .debugDescription = "Master Key Type",
                                                     .format = &srtpMasterKeyTypeFormat,
                                                     .isOptional = false,
                                                     .isFlat = false };

const HAPStructTLVMember srtpMasterSaltTypeMember = { .valueOffset = HAP_OFFSETOF(srtpParameters, srtpMasterSalt),
                                                      .isSetOffset = 0,
                                                      .tlvType = 3,
                                                      .debugDescription = "Master Salt Type",
                                                      .format = &srtpMasterSaltTypeFormat,
                                                      .isOptional = false,
                                                      .isFlat = false };

const SrtpParametersFormat srtpParametersFormat = {
    .type = kHAPTLVFormatType_Struct,
    .members = (const HAPStructTLVMember* const[]) { &srtpMasterKeyTypeMember,
                                                     &srtpMasterSaltTypeMember,
                                                     &srtpCryptoSuiteTypeMember,
                                                     NULL },
    .callbacks = { .isValid = isValid }
};

/* ---------------------------------------------------------------------------------------------*/

const HAPStructTLVMember videoParamsTypeMember = { .valueOffset = HAP_OFFSETOF(streamingSession, videoParams),
                                                   .isSetOffset = 0,
                                                   .tlvType = 4,
                                                   .debugDescription = "Video Params Type",
                                                   .format = &srtpParametersFormat,
                                                   .isOptional = false,
                                                   .isFlat = false };

const HAPStructTLVMember audioParamsTypeMember = { .valueOffset = HAP_OFFSETOF(streamingSession, audioParams),
                                                   .isSetOffset = 0,
                                                   .tlvType = 5,
                                                   .debugDescription = "Video Params Type",
                                                   .format = &srtpParametersFormat,
                                                   .isOptional = false,
                                                   .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const HAPUInt32TLVFormat ssrcTypeFormat = { .type = kHAPTLVFormatType_UInt32,
                                            .constraints = { .minimumValue = 0, .maximumValue = 0xFFFFFFFF },
                                            .callbacks = { .getBitDescription = NULL, .getDescription = NULL } };

const HAPStructTLVMember ssrcVideoTypeMember = { .valueOffset = HAP_OFFSETOF(streamingSession, ssrcVideo),
                                                 .isSetOffset = 0,
                                                 .tlvType = 6,
                                                 .debugDescription = "SSRC Video Type",
                                                 .format = &ssrcTypeFormat,
                                                 .isOptional = false,
                                                 .isFlat = false };

const HAPStructTLVMember ssrcAudioTypeMember = { .valueOffset = HAP_OFFSETOF(streamingSession, ssrcAudio),
                                                 .isSetOffset = 0,
                                                 .tlvType = 7,
                                                 .debugDescription = "SSRC Audio Type",
                                                 .format = &ssrcTypeFormat,
                                                 .isOptional = false,
                                                 .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const StreamingSessionFormat streamingSessionFormatWrite = {
    .type = kHAPTLVFormatType_Struct,
    .members = (const HAPStructTLVMember* const[]) { &sessionIdTypeMember,
                                                     &controllerAddressTypeMember,
                                                     &videoParamsTypeMember,
                                                     &audioParamsTypeMember,
                                                     NULL },
    .callbacks = { .isValid = isValid }
};

const StreamingSessionFormat streamingSessionFormatRead = {
    .type = kHAPTLVFormatType_Struct,
    .members = (const HAPStructTLVMember* const[]) { &sessionIdTypeMember,
                                                     &streamingStatusTypeMember,
                                                     &accessoryAddressTypeMember,
                                                     &videoParamsTypeMember,
                                                     &audioParamsTypeMember,
                                                     &ssrcVideoTypeMember,
                                                     &ssrcAudioTypeMember,
                                                     NULL },
    .callbacks = { .isValid = isValid }
};

/* ---------------------------------------------------------------------------------------------*/

const HAPUInt8TLVFormat payloadTypeFormat = { .type = kHAPTLVFormatType_UInt8,
                                              .constraints = { .minimumValue = 0, .maximumValue = 0xFF },
                                              .callbacks = { .getBitDescription = NULL, .getDescription = NULL } };

const HAPStructTLVMember payloadTypeMember = { .valueOffset = HAP_OFFSETOF(rtpParameters, payloadType),
                                               .isSetOffset = 0,
                                               .tlvType = 1,
                                               .debugDescription = "Payload Type",
                                               .format = &payloadTypeFormat,
                                               .isOptional = false,
                                               .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const HAPStructTLVMember selectedSsrcTypeMember = { .valueOffset = HAP_OFFSETOF(rtpParameters, ssrc),
                                                    .isSetOffset = 0,
                                                    .tlvType = 2,
                                                    .debugDescription = "Selected SSRC Type",
                                                    .format = &ssrcTypeFormat,
                                                    .isOptional = false,
                                                    .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const HAPUInt16TLVFormat maximumBitrateTypeFormat = { .type = kHAPTLVFormatType_UInt16,
                                                      .constraints = { .minimumValue = 0, .maximumValue = 0xFFFF },
                                                      .callbacks = { .getBitDescription = NULL,
                                                                     .getDescription = NULL } };

const HAPStructTLVMember maximumBitrateTypeMember = { .valueOffset = HAP_OFFSETOF(rtpParameters, maximumBitrate),
                                                      .isSetOffset = 0,
                                                      .tlvType = 3,
                                                      .debugDescription = "Maximum Bit Rate Type",
                                                      .format = &maximumBitrateTypeFormat,
                                                      .isOptional = false,
                                                      .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const HAPUInt32TLVFormat minRTCPIntervalTypeFormat = { .type = kHAPTLVFormatType_UInt32,
                                                       .constraints = { .minimumValue = 0, .maximumValue = 0xFFFFFFFF },
                                                       .callbacks = { .getBitDescription = NULL,
                                                                      .getDescription = NULL } };

const HAPStructTLVMember minRTCPIntervalTypeMember = { .valueOffset = HAP_OFFSETOF(rtpParameters, minRTCPinterval),
                                                       .isSetOffset = 0,
                                                       .tlvType = 4,
                                                       .debugDescription = "Min RTCP Interval Type",
                                                       .format = &minRTCPIntervalTypeFormat,
                                                       .isOptional = false,
                                                       .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

/* const RTPParametersFormat rtpParametersFormat = {
    .type = kHAPTLVFormatType_Struct,
    .members = (const HAPStructTLVMember* const[]) { &payloadTypeMember,
                                                     &selectedSsrcTypeMember,
                                                     &maximumBitrateTypeMember,
                                                     &minRTCPIntervalTypeMember,
                                                     NULL },
    .callbacks = { .isValid = isValid }
}; */

/* const HAPStructTLVMember videoRtpParametersMember = { .valueOffset = HAP_OFFSETOF(videoRtpParameters,
   vRtpParameters), .isSetOffset = 0, .tlvType = 1, .debugDescription = "Video RTP Parameters Type", .format =
   &rtpParametersFormat, .isOptional = false, .isFlat = true }; */

/* const HAPStructTLVMember audioRtpParametersMember = { .valueOffset = HAP_OFFSETOF(audioRtpParameters, rtpParameters),
                                                      .isSetOffset = 0,
                                                      .tlvType = 1,
                                                      .debugDescription = "Audio RTP Parameters Type",
                                                      .format = &rtpParametersFormat,
                                                      .isOptional = false,
                                                      .isFlat = true }; */

/* ---------------------------------------------------------------------------------------------*/

const HAPUInt16TLVFormat maximumMTUTypeFormat = { .type = kHAPTLVFormatType_UInt16,
                                                  .constraints = { .minimumValue = 0, .maximumValue = 0xFFFF },
                                                  .callbacks = { .getBitDescription = NULL, .getDescription = NULL } };

const HAPStructTLVMember maximumMTUTypeMember = { .valueOffset = HAP_OFFSETOF(videoRtpParameters, maxMTU),
                                                  .isSetOffset = 0,
                                                  .tlvType = 5,
                                                  .debugDescription = "Maximum MTU Type",
                                                  .format = &maximumMTUTypeFormat,
                                                  .isOptional = false,
                                                  .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const VideoRTPParametersTypeFormat videoRTPParametersTypeFormat = { .type = kHAPTLVFormatType_Struct,
                                                                    .members =
                                                                            (const HAPStructTLVMember* const[]) {
                                                                                    // &videoRtpParametersMember,
                                                                                    &payloadTypeMember,
                                                                                    &selectedSsrcTypeMember,
                                                                                    &maximumBitrateTypeMember,
                                                                                    &minRTCPIntervalTypeMember,
                                                                                    &maximumMTUTypeMember,
                                                                                    NULL },
                                                                    .callbacks = { .isValid = isValid } };

const HAPStructTLVMember videoRTPParametersTypeMember = { .valueOffset =
                                                                  HAP_OFFSETOF(selectedVideoParameters, vRtpParameters),
                                                          .isSetOffset = 0,
                                                          .tlvType = 4,
                                                          .debugDescription = "Selected Video Parameters",
                                                          .format = &videoRTPParametersTypeFormat,
                                                          .isOptional = false,
                                                          .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const HAPUInt8TLVFormat comfortNoisePayloadTypeFormat = { .type = kHAPTLVFormatType_UInt8,
                                                          .constraints = { .minimumValue = 0, .maximumValue = 0xFF },
                                                          .callbacks = { .getBitDescription = NULL,
                                                                         .getDescription = NULL } };

const HAPStructTLVMember comfortNoisePayloadTypeMember = {
    .valueOffset = HAP_OFFSETOF(audioRtpParameters, comfortNoisePayload),
    .isSetOffset = 0,
    .tlvType = 6,
    .debugDescription = "Comfort Noise Payload Type",
    .format = &comfortNoisePayloadTypeFormat,
    .isOptional = false,
    .isFlat = false
};

/* ---------------------------------------------------------------------------------------------*/

const AudioRTPParametersTypeFormat audioRTPParametersTypeFormat = { .type = kHAPTLVFormatType_Struct,
                                                                    .members =
                                                                            (const HAPStructTLVMember* const[]) {
                                                                                    // &audioRtpParametersMember,
                                                                                    &payloadTypeMember,
                                                                                    &selectedSsrcTypeMember,
                                                                                    &maximumBitrateTypeMember,
                                                                                    &minRTCPIntervalTypeMember,
                                                                                    &comfortNoisePayloadTypeMember,
                                                                                    NULL },
                                                                    .callbacks = { .isValid = isValid } };

const HAPStructTLVMember audioRTPParametersTypeMember = { .valueOffset =
                                                                  HAP_OFFSETOF(selectedAudioParameters, rtpParameters),
                                                          .isSetOffset = 0,
                                                          .tlvType = 3,
                                                          .debugDescription = "Selected Audio RTP Parameters Type",
                                                          .format = &audioRTPParametersTypeFormat,
                                                          .isOptional = false,
                                                          .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

/* const HAPStructTLVMember streamingVideoCodecConfigMember = { .valueOffset = HAP_OFFSETOF(selectedVideoParameters,
   codecConfig), .isSetOffset = 0, .tlvType = 1, .debugDescription = "Video Codec Config", .format =
   &videoCodecConfigFormat, .isOptional = false, .isFlat = true }; */

const SelectedVideoParametersFormat selectedVideoParametersFormat = {
    .type = kHAPTLVFormatType_Struct,
    .members =
            (const HAPStructTLVMember* const[]) { // &streamingVideoCodecConfigMember,
                                                  &videoCodecTypeMember,
                                                  &videoCodecParamsMember,
                                                  &videoAttributesMember,
                                                  &videoRTPParametersTypeMember,
                                                  NULL },
    .callbacks = { .isValid = isValid }
};

const HAPStructTLVMember SelectedVideoRtpParametersMember = { .valueOffset =
                                                                      HAP_OFFSETOF(selectedRTPStruct, videoParameters),
                                                              .isSetOffset = 0,
                                                              .tlvType = 2,
                                                              .debugDescription = "Selected Video RTP Parameters Type",
                                                              .format = &selectedVideoParametersFormat,
                                                              .isOptional = false,
                                                              .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const HAPUInt8TLVFormat comfortNoiseSelectedTypeFormat = { .type = kHAPTLVFormatType_UInt8,
                                                           .constraints = { .minimumValue = 0, .maximumValue = 1 },
                                                           .callbacks = { .getBitDescription = NULL,
                                                                          .getDescription = NULL } };

const HAPStructTLVMember comfortNoiseSelectedTypeMember = { .valueOffset =
                                                                    HAP_OFFSETOF(selectedAudioParameters, comfortNoise),
                                                            .isSetOffset = 0,
                                                            .tlvType = 4,
                                                            .debugDescription = "Comfort Noise Type",
                                                            .format = &comfortNoiseSelectedTypeFormat,
                                                            .isOptional = false,
                                                            .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

/* const HAPStructTLVMember streamingAudioCodecConfigMember = { .valueOffset = HAP_OFFSETOF(selectedAudioParameters,
   codecConfig), .isSetOffset = 0, .tlvType = 1, .debugDescription = "Audio Codec Config", .format =
   &audioCodecConfigFormat, .isOptional = false, .isFlat = true }; */

const SelectedAudioParametersFormat selectedAudioParametersFormat = {
    .type = kHAPTLVFormatType_Struct,
    .members =
            (const HAPStructTLVMember* const[]) { // &streamingAudioCodecConfigMember,
                                                  &audioCodecTypeMember,
                                                  &audioCodecParamsMember,
                                                  &audioRTPParametersTypeMember,
                                                  &comfortNoiseSelectedTypeMember,
                                                  NULL },
    .callbacks = { .isValid = isValid }
};

const HAPStructTLVMember SelectedAudioRtpParametersMember = { .valueOffset =
                                                                      HAP_OFFSETOF(selectedRTPStruct, audioParameters),
                                                              .isSetOffset = 0,
                                                              .tlvType = 3,
                                                              .debugDescription = "Selected Audio RTP Parameters Type",
                                                              .format = &selectedAudioParametersFormat,
                                                              .isOptional = false,
                                                              .isFlat = false };

/* ---------------------------------------------------------------------------------------------*/

const HAPUInt8TLVFormat controlSessionCommandTypeFormat = { .type = kHAPTLVFormatType_UInt8,
                                                            .constraints = { .minimumValue = 0, .maximumValue = 4 },
                                                            .callbacks = { .getBitDescription = NULL,
                                                                           .getDescription = NULL } };

const HAPStructTLVMember controlSessionCommandTypeMember = { .valueOffset = HAP_OFFSETOF(sessionControl, command),
                                                             .isSetOffset = 0,
                                                             .tlvType = 2,
                                                             .debugDescription = "Command Type",
                                                             .format = &controlSessionCommandTypeFormat,
                                                             .isOptional = false,
                                                             .isFlat = false };

const HAPStructTLVMember controlSessionIdTypeMember = { .valueOffset = HAP_OFFSETOF(sessionControl, sessionId),
                                                        .isSetOffset = 0,
                                                        .tlvType = 1,
                                                        .debugDescription = "Control Session ID Type",
                                                        .format = &sessionIdTypeFormat,
                                                        .isOptional = false,
                                                        .isFlat = false };

const SessionControlTypeFormat sessionControlTypeFormat = {
    .type = kHAPTLVFormatType_Struct,
    .members =
            (const HAPStructTLVMember* const[]) { &controlSessionCommandTypeMember, &controlSessionIdTypeMember, NULL },
    .callbacks = { .isValid = isValid }
};

const HAPStructTLVMember sessionControlTypeMember = { .valueOffset = HAP_OFFSETOF(selectedRTPStruct, control),
                                                      .isSetOffset = 0,
                                                      .tlvType = 1,
                                                      .debugDescription = "Session Control Type",
                                                      .format = &sessionControlTypeFormat,
                                                      .isOptional = false,
                                                      .isFlat = false };
/* ---------------------------------------------------------------------------------------------*/

const SelectedRTPFormat selectedRTPFormatWrite = {
    .type = kHAPTLVFormatType_Struct,
    .members = (const HAPStructTLVMember* const[]) { &sessionControlTypeMember,
                                                     &SelectedVideoRtpParametersMember,
                                                     &SelectedAudioRtpParametersMember,
                                                     NULL },
    .callbacks = { .isValid = isValid }
};

/* ---------------------------------------------------------------------------------------------*/

HAPError handleSessionRead(HAPTLVWriterRef* responseWriter, streamingSession* session) {
    HAPError err;
    err = HAPTLVWriterEncode(responseWriter, &streamingSessionFormatRead, session);
    return err;
};

HAPError handleSessionWrite(HAPTLVReaderRef* responseReader, streamingSession* session) {
    HAPError err;
    err = HAPTLVReaderDecode(responseReader, &streamingSessionFormatWrite, session);
    return err;
};

HAPError handleSelectedWrite(HAPTLVReaderRef* responseReader, selectedRTPStruct* selectedRTP) {
    HAPError err;
    err = HAPTLVReaderDecode(responseReader, &selectedRTPFormatWrite, selectedRTP);
    return err;
};

void checkFormats() {

    const HAPTLVFormat* formats[] = {
        &sessionControlTypeFormat, &selectedVideoParametersFormat, &selectedAudioParametersFormat, NULL
    };
    for (size_t i = 0; i < 3; i++) {
        bool valid;
        valid = HAPTLVFormatIsValid(formats[i]);
        HAPLogDebug(&kHAPLog_Default, "%zu: %d", i, valid);
    }
}