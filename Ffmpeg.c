/**
 * @file Ffmpeg.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2021-01-22
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "HAP.h"
#include "HAPBase.h"
#include "Ffmpeg.h"

#include "App_Camera.h"
#include "App.h"
#include "DB.h"
// #include "streaming.h"
#include <arpa/inet.h>
#include "util_base64.h"
#include "HAPPlatformSystemCommand.h"
#include <libavutil/imgutils.h>

void StreamContextInitialize(void* context) {
    AccessoryContext* myContext = context;
    // Get IP Address
    struct in_addr ia;
    inet_pton(AF_INET, "10.0.1.230", &(ia.s_addr));
    myContext->session.controllerAddress.ipAddress = ia.s_addr;
    // 61673?rtcpport=61673&pkt_size=1316
    myContext->session.controllerAddress.videoPort = 61673;
    myContext->session.videoParameters.vRtpParameters.maxMTU = 1316;
    memset(&myContext->inStreamContext, 0, sizeof(rtsp_context));
    myContext->inStreamContext.format_context = NULL;
    myContext->inStreamContext.packet = av_packet_alloc();
    myContext->inStreamContext.opts = NULL;
    myContext->inStreamContext.url = "rtsp://admin:admin@10.0.1.252:554/ch01/0";
    av_dict_set(&myContext->inStreamContext.opts, "rtsp_transport", "tcp", 0);
    // av_dict_set(&myContext->inStreamContext.opts, "framerate", "30", 0);
    myContext->outStreamContext.format_context = NULL;
    myContext->outStreamContext.opts = NULL;
}

void StreamContextDeintialize(void* context) {
    AccessoryContext* myContext = context;
    HAPLogDebug(&kHAPLog_Default, "Freeing memory and exiting.\n");
    av_packet_free(&myContext->inStreamContext.packet);
    av_dict_free(&myContext->inStreamContext.opts);
    avformat_close_input(&myContext->inStreamContext.format_context);
}

void* startInStream(void* context) {
    // start rtsp stream
    rtsp_context* rtspStream = &((AccessoryContext*) context)->inStreamContext;
    int ret, stream_index = 0;
    if (avformat_open_input(&rtspStream->format_context, rtspStream->url, NULL, &rtspStream->opts) < 0) {
        fprintf(stderr, "Failed to open input stream");
    }
    if (avformat_find_stream_info(rtspStream->format_context, NULL) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
    }
    av_dump_format(rtspStream->format_context, 0, rtspStream->url, 0);

    ret = av_find_best_stream(rtspStream->format_context, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr,
                "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO),
                rtspStream->url);
    } else {
        stream_index = ret;
        rtspStream->strm = rtspStream->format_context->streams[stream_index];

        /* find decoder for the stream */
        rtspStream->decoder = avcodec_find_decoder(rtspStream->strm->codecpar->codec_id);
        if (!rtspStream->decoder) {
            fprintf(stderr, "failed to find %s codec\n", av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        }

        /* Allocate a codec context for the decoder */
        rtspStream->codec_context = avcodec_alloc_context3(rtspStream->decoder);
        if (!rtspStream->codec_context) {
            fprintf(stderr, "Failed to allocate the %s codec context\n", av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        }

        /* Copy codec parameters from input stream to codec context */
        if ((ret = avcodec_parameters_to_context(rtspStream->codec_context, rtspStream->strm->codecpar)) < 0) {
            fprintf(stderr,
                    "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        }

        /* Init the decoders */
        if ((ret = avcodec_open2(rtspStream->codec_context, rtspStream->decoder, &rtspStream->opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n", av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        }
        // *stream_idx = stream_index;
    }
    rtspStream->width = rtspStream->codec_context->width;
    rtspStream->height = rtspStream->codec_context->height;
    rtspStream->pix_fmt = rtspStream->codec_context->pix_fmt;
    pthread_exit(NULL);
}

void* startOutStream(void* context HAP_UNUSED) {
    AccessoryContext* myContext = context;
    streamingSession* mySession = &myContext->session;
    rtsp_context* rtspStream = &myContext->inStreamContext;
    AVPacket* pkt = myContext->inStreamContext.packet;
    int ret;
    // int stream_index = 0;
    int* stream_mapping = NULL;
    int stream_mapping_size = 0;
    srtp_context* srtpStream = &myContext->outStreamContext;

    HAPError err;
    char controllerAddress[INET_ADDRSTRLEN];
    HAPRawBufferZero(controllerAddress, INET_ADDRSTRLEN);
    (void) inet_ntop(AF_INET, &mySession->controllerAddress.ipAddress, controllerAddress, INET_ADDRSTRLEN);
    // Encode64 ssrc
    uint8_t ssrcBuffer[KEYLENGTH + SALTLENGTH];
    HAPRawBufferZero(ssrcBuffer, KEYLENGTH + SALTLENGTH);
    HAPRawBufferCopyBytes(&ssrcBuffer, mySession->videoParams.srtpMasterKey, KEYLENGTH);
    HAPRawBufferCopyBytes(&ssrcBuffer[UUIDLENGTH], mySession->videoParams.srtpMasterSalt, SALTLENGTH);
    size_t ssrc64bufferLength = util_base64_encoded_len(sizeof(ssrcBuffer));
    size_t ssrc64length;
    char ssrc64[ssrc64bufferLength];
    HAPRawBufferZero(ssrc64, ssrc64bufferLength);
    util_base64_encode(ssrcBuffer, KEYLENGTH + SALTLENGTH, ssrc64, ssrc64bufferLength, &ssrc64length);
    ssrc64[ssrc64length] = '\0';

    // create strings from values
    char framerateArg[4];
    err = HAPStringWithFormat(
            framerateArg, sizeof framerateArg, "%d", mySession->videoParameters.codecConfig.videoAttributes.frameRate);
    char scaleArg[16];
    err = HAPStringWithFormat(
            scaleArg,
            sizeof scaleArg,
            "scale=%u:%u",
            mySession->videoParameters.codecConfig.videoAttributes.imageWidth,
            mySession->videoParameters.codecConfig.videoAttributes.imageHeight);
    char bitrateArg[5];
    err = HAPStringWithFormat(
            bitrateArg,
            sizeof bitrateArg,
            "%uk",
            mySession->videoParameters.vRtpParameters.vRtpParameters.maximumBitrate);
    char payloadTypeArg[4];
    err = HAPStringWithFormat(
            payloadTypeArg,
            sizeof payloadTypeArg,
            "%d",
            mySession->videoParameters.vRtpParameters.vRtpParameters.payloadType);
    char ssrcArg[11];
    err = HAPStringWithFormat(
            ssrcArg, sizeof ssrcArg, "%u", mySession->videoParameters.vRtpParameters.vRtpParameters.ssrc);
    HAPLogDebug(&kHAPLog_Default, "%u, %s", mySession->videoParameters.vRtpParameters.vRtpParameters.ssrc, ssrcArg);
    char controllerUrl[256];
    err = HAPStringWithFormat(
            controllerUrl,
            sizeof controllerUrl,
            "srtp://%s:%u?rtcpport=%u&localrtcpport=%u&pkt_size=%u",
            controllerAddress,
            mySession->controllerAddress.videoPort,
            mySession->controllerAddress.videoPort,
            mySession->controllerAddress.videoPort,
            mySession->videoParameters.vRtpParameters.maxMTU);

    // pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "Could not allocate AVPacket\n");
        goto end;
    }
    // startInStream(rtspStream);
    // fprintf(stderr, "Started In Stream.\n");
    uint8_t* video_dst_data[4] = { NULL };
    int video_dst_linesize[4];
    int video_dst_bufsize;
    ret = av_image_alloc( // TODO - Make sure this is freed
            video_dst_data,
            video_dst_linesize,
            rtspStream->width,
            rtspStream->height,
            rtspStream->pix_fmt,
            1);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw video buffer\n");
        goto end;
    }
    video_dst_bufsize = ret;
    av_dict_set(&srtpStream->opts, "srtp_out_suite", "AES_CM_128_HMAC_SHA1_80", 0);
    av_dict_set(&srtpStream->opts, "srtp_out_params", ssrc64, 0);
    avformat_alloc_output_context2(&srtpStream->format_context, NULL, "rtp", controllerUrl);
    if (!srtpStream->format_context) {
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    stream_mapping_size = rtspStream->format_context->nb_streams;
    stream_mapping = av_calloc(stream_mapping_size, sizeof(*stream_mapping));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    srtpStream->encoder = avcodec_find_encoder(rtspStream->strm->codecpar->codec_id);
    srtpStream->codec_context = avcodec_alloc_context3(srtpStream->encoder);
    srtpStream->strm = avformat_new_stream(srtpStream->format_context, srtpStream->encoder);
    if (!srtpStream->strm) {
        fprintf(stderr, "No output stream.\n");
        goto end;
    }
    avcodec_parameters_copy(srtpStream->strm->codecpar, rtspStream->strm->codecpar);
    avcodec_parameters_to_context(srtpStream->codec_context, srtpStream->strm->codecpar);
    if (!(srtpStream->format_context->flags & AVFMT_NOFILE)) {
        ret = avio_open(&srtpStream->format_context->pb, controllerUrl, AVIO_FLAG_WRITE);
        if (ret < 0) {
            char err_buff[256];
            av_strerror(ret, err_buff, sizeof(err_buff));
            printf("Could not open output URL '%s'", err_buff);
            goto end;
        }
    }

    av_dump_format(srtpStream->format_context, 0, controllerUrl, 1);

    ret = avformat_init_output(srtpStream->format_context, &srtpStream->opts);
    if (ret < 0) {
        printf("Error occurred when opening output URL\n");
        goto end;
    }
    ret = avformat_write_header(srtpStream->format_context, &srtpStream->opts);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        goto end;
    }

    while (1) {
        ret = av_read_frame(rtspStream->format_context, pkt);
        if (ret < 0)
            break;

        // in_stream  = ifmt_ctx->streams[pkt->stream_index];
        if (pkt->stream_index >= stream_mapping_size || stream_mapping[pkt->stream_index] < 0) {
            av_packet_unref(pkt);
            continue;
        }
        pkt->stream_index = stream_mapping[pkt->stream_index];
        // out_stream = ofmt_ctx->streams[pkt->stream_index];
        // log_packet(rtspStream->format_context, pkt, "in");

        /* copy packet */
        av_packet_rescale_ts(pkt, rtspStream->strm->time_base, srtpStream->strm->time_base);
        pkt->pos = -1;
        // log_packet(ofmt_ctx, pkt, "out");

        ret = av_interleaved_write_frame(srtpStream->format_context, pkt);
        /* pkt is now blank (av_interleaved_write_frame() takes ownership of
         * its contents and resets pkt), so that no unreferencing is necessary.
         * This would be different if one used av_write_frame(). */
        if (ret < 0) {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }
    }

    av_write_trailer(srtpStream->format_context);

end:
    HAPLogDebug(&kHAPLog_Default, "Freeing memory and exiting.\n");
    // TODO - Close stuff and fix the goto end stuff
    av_dict_free(&myContext->outStreamContext.opts);
    // if (&myContext->outStreamContext.format_context && !(myContext->outStreamContext.format_context->flags &
    // AVFMT_NOFILE))
    avio_close(myContext->outStreamContext.format_context->pb);
    avformat_free_context(myContext->outStreamContext.format_context);

    pthread_exit(NULL);
    /*
        char* const ffmpegCommand[] = { "/usr/local/bin/ffmpeg", // static
                                        "-report",               // static
                                        "-rtsp_transport",
                                        "tcp", // static
                                        "-r",
                                        framerateArg, // need framerate here
                                        "-i",
                                        "rtsp://admin:admin@10.0.1.234:554/ch01/0", // static for now
                                        "-an",                                      // static
                                        "-sn",                                      // static
                                        "-dn",                                      // static
                                        "-codec:v",
                                        "libx264", // static
                                        // "-f", "avfoundation", //static
                                        // "-threads", "0", //static
                                        // "-vcodec", "libx264", //static
                                        "-pix_fmt",
                                        "yuvj420p", // static
                                        // "-color_range", "mpeg", //static
                                        "-f",
                                        "rawvideo", // static
                                        // "-preset", "ultrafast", //static
                                        "-profile:v",
                                        "main", // static
                                        "-level:v",
                                        "4.0", // static
                                        // "-tune", "zerolatency", //static
                                        // "-filter:v", scaleArg,
                                        "-vf",
                                        scaleArg, // need scale arg here!
                                        "-b:v",
                                        bitrateArg, // Need buffer size here!
                                        // "-bufsize", bitrateArg, // Need buffer size here too!
                                        "-payload_type",
                                        payloadTypeArg, // Need payload type arg here
                                        "-ssrc",
                                        ssrcArg, // need ssrc arg here
                                        "-f",
                                        "rtp", // static
                                        "-srtp_out_suite",
                                        "AES_CM_128_HMAC_SHA1_80", // static for now
                                        "-srtp_out_params",
                                        ssrc64,        // need out_params arg
                                        controllerUrl, // Need to format this arg
                                        "-loglevel",
                                        "level+verbose",
                                        NULL };

        char commandBuffer[0xFFF];
        size_t outputSize;
        err = HAPPlatformSystemCommandRun(
                ffmpegCommand, commandBuffer, 0xFFF, &outputSize); // TODO - test with snapshot command.
        HAPLogDebug(&kHAPLog_Default, "%s", commandBuffer);
        HAPLogDebug(&kHAPLog_Default, "%lu", outputSize); */
}

/*
ffmpeg -rtsp_transport tcp -r 30 -i rtsp://admin:admin@10.0.1.234/ch01/1
-an -sn -dn -codec:v libx264 -pix_fmt yuv420p -color_range mpeg
-f rawvideo -preset ultrafast -tune zerolatency -b:v 299k -payload_type 99
-ssrc 366912 -f rtp -srtp_out_suite AES_CM_128_HMAC_SHA1_80
-srtp_out_params 806MgmMtLfeV4a5TZfGFMz7rwSO4/WU+WDIMxb7t
srtp://10.0.1.210:61673?rtcpport=61673&pkt_size=1316 -loglevel level+verbose
*/

/* Borrowed from HAP-python until I figure it out
        :param session_info: Contains information about the current session. Can be used
            for session storage. Available keys:
            - id - The session ID.
        :type session_info: ``dict``
        :param stream_config: Stream configuration, as negotiated with the HAP client.
            Implementations can only use part of these. Available keys:
            General configuration:
                - address - The IP address from which the camera will stream
                - v_port - Remote port to which to stream video
                - v_srtp_key - Base64-encoded key and salt value for the
                    AES_CM_128_HMAC_SHA1_80 cipher to use when streaming video.
                    The key and the salt are concatenated before encoding
                - a_port - Remote audio port to which to stream audio
                - a_srtp_key - As v_srtp_params, but for the audio stream.
            Video configuration:
                - v_profile_id - The profile ID for the H.264 codec, e.g. baseline.
                    Refer to ``VIDEO_CODEC_PARAM_PROFILE_ID_TYPES``.
                - v_level - The level in the profile ID, e.g. 3:1.
                    Refer to ``VIDEO_CODEC_PARAM_LEVEL_TYPES``.
                - width - Video width
                - height - Video height
                - fps - Video frame rate
                - v_ssrc - Video synchronisation source
                - v_payload_type - Type of the video codec
                - v_max_bitrate - Maximum bit rate generated by the codec in kbps
                    and averaged over 1 second
                - v_rtcp_interval - Minimum RTCP interval in seconds
                - v_max_mtu - MTU that the IP camera must use to transmit
                    Video RTP packets.
            Audio configuration:
                - a_bitrate - Whether the bitrate is variable or constant
                - a_codec - Audio codec
                - a_comfort_noise - Wheter to use a comfort noise codec
                - a_channel - Number of audio channels
                - a_sample_rate - Audio sample rate in KHz
                - a_packet_time - Length of time represented by the media in a packet
                - a_ssrc - Audio synchronisation source
                - a_payload_type - Type of the audio codec
                - a_max_bitrate - Maximum bit rate generated by the codec in kbps
                    and averaged over 1 second
                - a_rtcp_interval - Minimum RTCP interval in seconds
                - a_comfort_payload_type - The type of codec for comfort noise

FFMPEG_CMD = (
    # pylint: disable=bad-continuation
    'ffmpeg -re -f avfoundation -framerate {fps} -i 0:0 -threads 0 '
    '-vcodec libx264 -an -pix_fmt yuv420p -r {fps} -f rawvideo -tune zerolatency '
    '-vf scale={width}:{height} -b:v {v_max_bitrate}k -bufsize {v_max_bitrate}k '
    '-payload_type 99 -ssrc {v_ssrc} -f rtp '
    '-srtp_out_suite AES_CM_128_HMAC_SHA1_80 -srtp_out_params {v_srtp_key} '
    'srtp://{address}:{v_port}?rtcpport={v_port}&'
    'localrtcpport={v_port}&pkt_size=1378'
)
'''Template for the ffmpeg command.'''
*/