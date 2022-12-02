/**
 * @file streaming.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2021-01-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "snapshot.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLECharacteristic" };

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*) userp;

    unsigned char* ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        /* out of memory! */
        HAPLogError(&logObject, "not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int GetSnapshot(u_int64_t* jpegSize, uint8_t* jpegBuf, int width, int height) {
    CURL* curl_handle;
    CURLcode res;
    struct curl_slist* list = NULL;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
    chunk.size = 0;           /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, "http://10.0.1.252/action/snapshot");
    list = curl_slist_append(list, "Authorization: Basic YWRtaW46YWRtaW4=");
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*) &chunk);

    /* some servers do not like requests that are made without a user-agent
       field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* get it! */
    res = curl_easy_perform(curl_handle);

    /* check for errors */
    if (res != CURLE_OK) {
        HAPLogError(&logObject, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        HAPLogDebug(&logObject, "%lu bytes retrieved\n", (unsigned long) chunk.size);

        tjhandle tjInstanceIn = NULL;
        int flags = 0, snapWidth, snapHeight, snapSubSamp, snapColorspace;
        int pixelFormat = TJPF_BGRX;

        if ((tjInstanceIn = tjInitDecompress()) == NULL)
            HAPLogError(&logObject, "%s", tjGetErrorStr2(tjInstanceIn));

        tjDecompressHeader3(
                tjInstanceIn, chunk.memory, chunk.size, &snapWidth, &snapHeight, &snapSubSamp, &snapColorspace);
        HAPLogDebug(
                &logObject,
                "Snapshot image: Width: %d, Height: %d, Sub: %d, Colorspace: %d",
                snapWidth,
                snapHeight,
                snapSubSamp,     // TJSAMP_420
                snapColorspace); // TJCS_YCbCr
// Don't resize if requested size is greater than snapshot from camera.
        if (height >= snapHeight) {
            HAPLogDebug(&logObject, "%s", "Snapshot not scaled.");
            HAPRawBufferCopyBytes(jpegBuf, chunk.memory, chunk.size);
            *jpegSize = chunk.size;
        } else {
            tjhandle tjInstanceOut = NULL;
            unsigned char* dstBuf = NULL; /* Dynamically allocate the JPEG buffer */
            unsigned long dstSize = 0;

            dstSize = width * height * tjPixelSize[pixelFormat];

            if ((dstBuf = (unsigned char*) tjAlloc(dstSize)) == NULL)
                HAPLogError(&logObject, "%s", "Couldn't allocate dstBuf.");
            // tjDecompress2 also handles the resizing.
            if (tjDecompress2(tjInstanceIn, chunk.memory, chunk.size, dstBuf, width, 0, height, pixelFormat, flags) < 0)
                HAPLogError(&logObject, "%s", tjGetErrorStr2(tjInstanceIn));

            tjDecompressHeader3(tjInstanceIn, dstBuf, dstSize, &snapWidth, &snapHeight, &snapSubSamp, &snapColorspace);
            HAPLogDebug(
                    &logObject,
                    "Snapshot image resized: Width: %d, Height: %d, Sub: %d, Colorspace: %d",
                    snapWidth,
                    snapHeight,
                    snapSubSamp,     // TJSAMP_420
                    snapColorspace); // TJCS_YCbCr

            if ((tjInstanceOut = tjInitCompress()) == NULL)
                HAPLogError(&logObject, "%s", tjGetErrorStr2(tjInstanceOut));
            if (tjCompress2(
                        tjInstanceOut,
                        dstBuf,
                        width,
                        0,
                        height,
                        pixelFormat,
                        &jpegBuf,
                        jpegSize,
                        snapSubSamp,
                        95,
                        flags |= TJFLAG_NOREALLOC) < 0)
                HAPLogError(&logObject, "%s", tjGetErrorStr2(tjInstanceOut));
            tjFree(dstBuf);
            dstBuf = NULL;
            tjDestroy(tjInstanceOut);
            tjInstanceOut = NULL;
        }

        tjDestroy(tjInstanceIn);
        tjInstanceIn = NULL;
    }
    curl_slist_free_all(list); /* free the list */
    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    /* we are done with libcurl, so clean it up */
    curl_global_cleanup();
    HAPLogInfo(&logObject, "Successful %s.\n\n", __func__);
    return 0;
}
