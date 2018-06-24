#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <unistd.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "ZPlayer", __VA_ARGS__)

/*
 * add for ffmpeg
 */
extern "C"
{
    #include <string.h>
    #include "libavcodec\avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"
    #include "libavutil/opt.h"
    #include "libavutil/imgutils.h"
}

static AVPacket *vPacket; //video packet from demux
static AVFrame *vFrame, *pFrameRGBA;
static AVCodecContext *vCodecCtx;
static AVCodec *vCodec;
struct SwsContext *img_convert_ctx;
static AVFormatContext *pFormatCtx;
ANativeWindow *nativeWindow;
ANativeWindow_Buffer windowBuffer;
uint8_t *v_out_buffer;


extern "C" JNIEXPORT jint
JNICALL
Java_com_example_bangl_zplayer_ZPlayer_pausePlay(
        JNIEnv *env,
        jobject /* this */) {
        return 0;
}

extern "C" JNIEXPORT jint
JNICALL
Java_com_example_bangl_zplayer_ZPlayer_resumePlay(
        JNIEnv *env,
        jobject /* this */) {
    return 0;
}

extern "C" JNIEXPORT jint
JNICALL
Java_com_example_bangl_zplayer_ZPlayer_releasePlay(
        JNIEnv *env,
        jobject /* this */) {

    LOGD("jni release play");
    ANativeWindow_release(nativeWindow);
    sws_freeContext(img_convert_ctx);
    av_packet_free(&vPacket);
    av_frame_free(&vFrame);
    av_frame_free(&pFrameRGBA);
    av_free(v_out_buffer);
    avcodec_close(vCodecCtx);
    avformat_close_input(&pFormatCtx);
    return 0;
}

extern "C" JNIEXPORT jint
JNICALL
Java_com_example_bangl_zplayer_ZPlayer_seekTo(
        JNIEnv *env,
        jobject /* this */,
        jint timeus) {
    return 0;
}

extern "C" JNIEXPORT jint
JNICALL
Java_com_example_bangl_zplayer_ZPlayer_startPlay(
        JNIEnv *env,
        jobject /* this */,
        jstring input_jstr,
        jobject surface) {
    int err;
    const char* input_cstr = (env)->GetStringUTFChars(input_jstr, NULL);
    //register all codec
    avcodec_register_all();
    av_register_all();
    pFormatCtx = avformat_alloc_context();
    if (NULL == pFormatCtx) {
        LOGD("[%s,%d] malloc failed", __FUNCTION__, __LINE__);
        return -1;
    }

    err = avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL);
    if (err < 0) {
        avformat_free_context(pFormatCtx);
        LOGD("[%s,%d] open input %s failed\n", __FUNCTION__, __LINE__, "/data/test.mp4");
        return -1;
    }
    LOGD("[%s,%d] stream num is %d", __FUNCTION__, __LINE__, pFormatCtx->nb_streams);

    vCodecCtx = pFormatCtx->streams[AVMEDIA_TYPE_VIDEO]->codec;
    if (NULL == vCodecCtx) {
        avformat_close_input(&pFormatCtx);
        LOGD("[%s,%d] video codec is NULL, return", __FUNCTION__, __LINE__);
        return -1;
    }
    vCodec = avcodec_find_decoder(vCodecCtx->codec_id);  //find video decoder
    if(NULL == vCodec){
        avformat_close_input(&pFormatCtx);
        LOGD("[%s,%d] Can not find video decoder %d, return", __FUNCTION__, __LINE__, vCodecCtx->codec_id);
        return -1;
    }else {
        LOGD("[%s,%d] find video decoder %d", __FUNCTION__, __LINE__, vCodecCtx->codec_id);
    }

    err = avcodec_open2(vCodecCtx, vCodec, NULL);
    if (err < 0) {
        avformat_close_input(&pFormatCtx);
        LOGD("[%s,%d] Can not open video decoder %d, return", __FUNCTION__, __LINE__, vCodecCtx->codec_id);
        return -1;
    }

    vPacket = av_packet_alloc();
    if (NULL == vPacket) {
        LOGD("[%s,%d] Can not malloc av packet, return", __FUNCTION__, __LINE__);
        avcodec_close(vCodecCtx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    int width = vCodecCtx->width;
    int height = vCodecCtx->height;

    vFrame = av_frame_alloc();
    if (NULL == vFrame) {
        LOGD("[%s,%d] Can not malloc yuv frame, return", __FUNCTION__, __LINE__);
        av_packet_free(&vPacket);
        avcodec_close(vCodecCtx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    pFrameRGBA = av_frame_alloc();
    if (NULL == pFrameRGBA) {
        LOGD("[%s,%d] Can not malloc rgb frame, return", __FUNCTION__, __LINE__);
        av_packet_free(&vPacket);
        av_frame_free(&vFrame);
        avcodec_close(vCodecCtx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    nativeWindow = ANativeWindow_fromSurface(env,surface);
    if (NULL == nativeWindow) {
        LOGD("[%s,%d] Can not get window from surface, return", __FUNCTION__, __LINE__);
        av_packet_free(&vPacket);
        av_frame_free(&vFrame);
        av_frame_free(&pFrameRGBA);
        avcodec_close(vCodecCtx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, width, height, 1);
    v_out_buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    if (NULL == v_out_buffer) {
        LOGD("[%s,%d] Can not get window from surface, return", __FUNCTION__, __LINE__);
        ANativeWindow_release(nativeWindow);
        av_packet_free(&vPacket);
        av_frame_free(&vFrame);
        av_frame_free(&pFrameRGBA);
        avcodec_close(vCodecCtx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, v_out_buffer, AV_PIX_FMT_RGBA, width, height, 1);
    img_convert_ctx = sws_getContext(width, height, AV_PIX_FMT_YUV420P,
                                     width, height, AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
    if (0 > ANativeWindow_setBuffersGeometry(nativeWindow,width,height,WINDOW_FORMAT_RGBA_8888)){
        LOGD("Couldn't set buffers geometry.\n");
        ANativeWindow_release(nativeWindow);
        sws_freeContext(img_convert_ctx);
        av_packet_free(&vPacket);
        av_frame_free(&vFrame);
        av_frame_free(&pFrameRGBA);
        av_free(v_out_buffer);
        avcodec_close(vCodecCtx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    while (!av_read_frame(pFormatCtx, vPacket)) {
        //decoder video packet
        int decoder_result = 0;
        if(avcodec_decode_video2(vCodecCtx, vFrame, &decoder_result, vPacket) < 0)
            continue;
        if (decoder_result > 0) {
            //LOGD("[%s,%d] frame width %d, frame height %d", __FUNCTION__, __LINE__, vFrame->width, vFrame->height);
            sws_scale(img_convert_ctx, (const uint8_t* const*)vFrame->data, vFrame->linesize, 0, vCodecCtx->height,
                      pFrameRGBA->data, pFrameRGBA->linesize);

            if (ANativeWindow_lock(nativeWindow, &windowBuffer, NULL) < 0) {
                LOGD("[%s,%d] cannot lock window", __FUNCTION__, __LINE__);
                break;
            } else {
                uint8_t *dst = (uint8_t *) windowBuffer.bits;
                for (int h = 0; h < vCodecCtx->height; h++)
                {
                    memcpy(dst + h * windowBuffer.stride * 4,
                           v_out_buffer + h * pFrameRGBA->linesize[0],
                           pFrameRGBA->linesize[0]);
                }
                ANativeWindow_unlockAndPost(nativeWindow);
                //usleep(1000 * 16);
            }
        }
    }

free:
    ANativeWindow_release(nativeWindow);
    sws_freeContext(img_convert_ctx);
    av_packet_free(&vPacket);
    av_frame_free(&vFrame);
    av_frame_free(&pFrameRGBA);
    av_free(v_out_buffer);
    avcodec_close(vCodecCtx);
    avformat_close_input(&pFormatCtx);
    return 0;
}
