//
// Created by 郭磊 on 2024/1/8.
//

#include "AudioDecoder.h"

static const char *TAG = "AudioDecoder";

AudioDecoder::AudioDecoder() {}


void AudioDecoder::Init(const char *video_path) {
  // 这里进行字符串的拷贝
  strcpy(resource_url, video_path);
  // 拷贝完成
  LOGD("%s resource url is:%s", TAG, resource_url);

  // 初始化FFMPEG 并找到对应的音频流及对应的解码器
  m_AVFormatContext = avformat_alloc_context();

  // 读入资源信息
  int ret = avformat_open_input(&m_AVFormatContext, resource_url, NULL, NULL);
  if (ret != 0) {
    char buf[1024];
    av_strerror(ret, buf, 1024);
    LOGD("%s avformat open input is failed, reason:%s", TAG, buf);
    CleanResource();
    return;
  }

  ret = avformat_find_stream_info(m_AVFormatContext, NULL);
  if (ret < 0) {
    char buf[1024];
    av_strerror(ret, buf, 1024);
    LOGD("%s avformat find stream info is failed, reason:%s", TAG, buf);
    CleanResource();
    return;
  }

  // 找到流信息
  for (int i = 0; i < m_AVFormatContext->nb_streams; i++) {
    if (m_AVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      m_StreamIndex = i;
      break;
    }
  }

  if (m_StreamIndex == -1) {
    LOGD("%s not found dest stream info", TAG);
    CleanResource();
    return;
  }
  // 拿到对应的解码器参数
  m_AVCodecParameters = m_AVFormatContext->streams[m_StreamIndex]->codecpar;
  m_StreamTimeBase = m_AVFormatContext->streams[m_StreamIndex]->time_base;

  m_AVCodec = avcodec_find_decoder(m_AVCodecParameters->codec_id);
  if (m_AVCodec == NULL) {
    LOGD("%s not found codec", TAG);
    CleanResource();
    return;
  }
  // 构建编码器上下文
  m_AVCodecContext = avcodec_alloc_context3(m_AVCodec);
  // 设置信息
  ret = avcodec_parameters_to_context(m_AVCodecContext, m_AVCodecParameters);
  if (ret < 0) {
    LOGD("%s set params to context is error", TAG);
    CleanResource();
    return;
  }
  // 构建AVDictionary 通过字典设置一些信息
  AVDictionary *pAVDictionary = NULL;
  av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
  av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
  av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
  av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);
  // 打开解码器
  ret = avcodec_open2(m_AVCodecContext, m_AVCodec, &pAVDictionary);
  if (ret != 0) {
    LOGD("%s av codec open is failed", TAG);
    CleanResource();
    return;
  }

  // 设置完成 分配对应的AVPack和AVFrame
  m_AVPacket = av_packet_alloc();
  m_AVFrame = av_frame_alloc();

  OnDecoderReady();
  detach_thread_create(NULL, (void *) StartAudioDecoder, this);

}

void AudioDecoder::OnDecoderReady() {
  // 设置播放音频需要的若干信息
  if (m_AudioRender != NULL) {
    m_AudioRender->Init();
    // 这里进行设置 采样率 声道 位深等等
    m_SwrContext = swr_alloc();
    av_opt_set_int(m_SwrContext, "in_channel_layout", m_AVCodecContext->channel_layout, 0);
    av_opt_set_int(m_SwrContext, "out_channel_layout", AUDIO_DST_CHANNEL_LAYOUT, 0);

    av_opt_set_int(m_SwrContext, "in_sample_rate", m_AVCodecContext->sample_rate, 0);
    av_opt_set_int(m_SwrContext, "out_sample_rate", AUDIO_DST_SAMPLE_RATE, 0);

    av_opt_set_sample_fmt(m_SwrContext, "in_sample_fmt", m_AVCodecContext->sample_fmt, 0);
    av_opt_set_sample_fmt(m_SwrContext, "out_sample_fmt", DST_SAMPLT_FORMAT, 0);

    swr_init(m_SwrContext);

    LOGD(
        "AudioDecoder::OnDecoderReady audio metadata sample rate: %d, channel: %d, format: %d, frame_size: %d, layout: %lld",
        m_AVCodecContext->sample_rate,
        m_AVCodecContext->channels,
        m_AVCodecContext->sample_fmt,
        m_AVCodecContext->frame_size,
        m_AVCodecContext->channel_layout);
    // 获取现在音频的
    m_nbSamples = (int) av_rescale_rnd(ACC_NB_SAMPLES,
                                       AUDIO_DST_SAMPLE_RATE,
                                       m_AVCodecContext->sample_rate,
                                       AV_ROUND_UP);
    m_DstFrameDataSze = av_samples_get_buffer_size(NULL,
                                                   AUDIO_DST_CHANNEL_COUNTS,
                                                   m_nbSamples,
                                                   DST_SAMPLT_FORMAT,
                                                   1);

    LOGD("AudioDecoder::OnDecoderReady [m_nbSamples, m_DstFrameDataSze]=[%d, %d]",
         m_nbSamples,
         m_DstFrameDataSze);

    // 分配好可以填充的大小
    m_AudioOutBuffer = (uint8_t *) malloc(m_DstFrameDataSze);

    m_AudioRender->Init();

  }
}

void AudioDecoder::StartAudioDecoder(void *args) {
  AudioDecoder *decoder = static_cast<AudioDecoder *>(args);
  if (decoder != NULL) {
    decoder->RunAudioDecoder();
  }
}

void AudioDecoder::RunAudioDecoder() {
  int ret = -1;
  int packCount = 0;
  while (true) {
    ret = av_read_frame(m_AVFormatContext, m_AVPacket);
    if (ret < 0) {
      LOGD("%s read file end.....", TAG);
      break;
    }
    LOGD("%s read packet count:%d\n", TAG, packCount++);
    // 找到我们需要流信息
    if (m_AVPacket->stream_index == m_StreamIndex) {
      // 将读取出来信息发送非解码器进行解码
      avcodec_send_packet(m_AVCodecContext, m_AVPacket);
      // 然后使用一个循环进行读取
      int frameCount = 0;
      while (avcodec_receive_frame(m_AVCodecContext, m_AVFrame) == 0) {
        LOGD("%s receive frame count:%d\n", TAG, frameCount++);
        // 这里有问题 应该
//        av_usleep(1000 * 45);
        OnFrameAvailable(m_AVFrame);
        av_frame_unref(m_AVFrame);
      }
    }
    av_packet_unref(m_AVPacket);
  }
}

void AudioDecoder::OnFrameAvailable(AVFrame *avFrame) {
  if (m_AudioRender) {
    int result = swr_convert(m_SwrContext,
                             &m_AudioOutBuffer,
                             m_DstFrameDataSze / 2,
                             (const uint8_t **) avFrame->data,
                             avFrame->nb_samples);
    if (result > 0) {
      LOGD("AudioDecoder::OnFrameAvailable pts:%d, time_base:%f",
           avFrame->pts,
           av_q2d(avFrame->time_base));
      m_Clock = avFrame->pts * av_q2d(m_StreamTimeBase);
      m_AudioRender->RenderAudioFrame(m_AudioOutBuffer, m_DstFrameDataSze);
    }
  }
}

void AudioDecoder::SetAudioRender(AudioRender *render) {
  this->m_AudioRender = render;
}

void AudioDecoder::CleanResource() {
  if (m_AVFormatContext) {
    avformat_free_context(m_AVFormatContext);
  }
  if (m_AVCodec) {
    avcodec_close(m_AVCodecContext);
  }
  if (m_AVCodecContext) {
    avcodec_free_context(&m_AVCodecContext);
  }
  if (m_AVPacket) {
    av_packet_free(&m_AVPacket);
  }
  if (m_AVFrame) {
    av_frame_free(&m_AVFrame);
  }

}

double AudioDecoder::GetAudioClock() {
  return m_Clock;
}

AudioDecoder::~AudioDecoder() {}
