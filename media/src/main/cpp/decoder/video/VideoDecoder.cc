//
// Created by 郭磊 on 2024/1/3.
//

#include "VideoDecoder.h"

VideoDecoder::VideoDecoder() {}

VideoDecoder::~VideoDecoder() {}

void VideoDecoder::Init(const char *url) {
  strcpy(resource_url, url);
  LOGD("resource url:%s\n", resource_url);
  // 1.构造解封装上下文
  m_AVFormatContext = avformat_alloc_context();
  // 2.打开对应的资源文件
  int ret = avformat_open_input(&m_AVFormatContext, resource_url, NULL, NULL);
  if (ret != 0) {
    char buf[1024];
    av_strerror(ret, buf, 1024);
    LOGD("avformat open input is failed:%d\n", ret);
    LOGD("avformat can not open reason:%s\n", buf);
    CleanResource();
    return;
  }
  // 3.找到对应的流信息，所有的信息都在上下文中
  ret = avformat_find_stream_info(m_AVFormatContext, NULL);
  if (ret < 0) {
    LOGD("avformat_find_stream_info is  failed\n");
    CleanResource();
    return;
  }
  // 4.获取视频流索引
  for (int i = 0; i < m_AVFormatContext->nb_streams; i++) {
    if (m_AVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      m_StreamIndex = i;
      break;
    }
  }
  if (m_StreamIndex == -1) {
    CleanResource();
    LOGD("can not found video stream\n");
    return;
  }
  // 5.获取这个流信息的解码器
  AVCodecParameters *codecParameters = m_AVFormatContext->streams[m_StreamIndex]->codecpar;
  m_StreamTimeBase = m_AVFormatContext->streams[m_StreamIndex]->time_base;
  m_AVCodec = avcodec_find_decoder(codecParameters->codec_id);
  if (m_AVCodec == NULL) {
    CleanResource();
    LOGD("avcodec find decoder is failed\n");
    return;
  }
  // 找到解码器，进行循环解码
  // 7.创建解码上下文
  m_AVCodecContext = avcodec_alloc_context3(m_AVCodec);
  if (m_AVCodecContext == NULL) {
    CleanResource();
    LOGD("avcodec context create is failed\n");
    return;
  }
  m_AVCodecContext->thread_count = 8;
  // 将参数信息给到codec上下文
  ret = avcodec_parameters_to_context(m_AVCodecContext, codecParameters);
  if (ret < 0) {
    CleanResource();
    LOGD("avcodec_parameters_to_context is failed\n");
    return;
  }

  // 8.构建AVDictionary 通过字典设置一些信息
  AVDictionary *pAVDictionary = NULL;
  av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
  av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
  av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
  av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);

  // 9.打开解码器
  ret = avcodec_open2(m_AVCodecContext, m_AVCodec, &pAVDictionary);
  if (ret != 0) {
    CleanResource();
    LOGD("av codec open is failed\n");
    return;
  }
  LOGD("ffmpeg init success");
  // 计算一下帧率
  m_fps = av_q2d(m_AVFormatContext->streams[m_StreamIndex]->avg_frame_rate);
  if (isnan(m_fps) || m_fps == 0) {
    // 换一种获取的方式
    m_fps = av_q2d(m_AVFormatContext->streams[m_StreamIndex]->r_frame_rate);
  }
  if (isnan(m_fps) || m_fps == 0) {
    m_fps = av_q2d(av_guess_frame_rate(m_AVFormatContext,
                                       m_AVFormatContext->streams[m_StreamIndex],
                                       NULL));
  }
  LOGD("current video fps:%f\n", m_fps);
  // 通过不停的读取pack，然后将pack解码成frame信息即可
  m_AVPack = av_packet_alloc();
  m_AVFrame = av_frame_alloc();
  OnDecoderReady();
  detach_thread_create(NULL, (void *) StartVideoDecoder, this);
}

/**
 * 用来设置一些视频需要的一些参数信息，width height等等
 */
void VideoDecoder::OnDecoderReady() {
  LOGD("VideoDecoder onDecoderReady");
  m_VideoWidth = m_AVCodecContext->width;
  m_VideoHeight = m_AVCodecContext->height;
  if (m_Render != NULL) {
    int dstSize[2] = {0};
    m_Render->Init(m_VideoWidth, m_VideoHeight, dstSize);
    // 获取真是的渲染的宽高
    m_RenderWidth = dstSize[0];
    m_RenderHeight = dstSize[1];
    // 这里需要分配对应的NativeImage
    m_RGBAFrame = av_frame_alloc();
    // 构建填充大小
    int buffer_size = av_image_get_buffer_size(DST_PIXEL_FORMAT, m_RenderWidth, m_RenderHeight, 1);
    // 分配空间进行填充
    m_FrameBuffer = static_cast<uint8_t *>(malloc(buffer_size * sizeof(uint8_t)));
    /**
     * av_image_fill_arrays() 函数用于将图像数据的指针按照特定格式填充到 AVFrame 结构体中。
     * 它通常在处理视频编码或解码时使用，主要作用是将一维数组转换成适合于FFmpeg处理的二维数组，
     * 同时也会更新AVFrame结构体的一些参数，如linesize、width和height等
     * 所以这里的设置是必须的。我理解就是用来初始化raga的AVFrame
     */
    av_image_fill_arrays(m_RGBAFrame->data,
                         m_RGBAFrame->linesize,
                         m_FrameBuffer,
                         DST_PIXEL_FORMAT,
                         m_RenderWidth,
                         m_RenderHeight,
                         1);
    // 构建一个转换器
    m_SwsContext = sws_getContext(m_VideoWidth,
                                  m_VideoHeight,
                                  m_AVCodecContext->pix_fmt,
                                  m_RenderWidth,
                                  m_RenderHeight,
                                  DST_PIXEL_FORMAT,
                                  SWS_FAST_BILINEAR,
                                  NULL,
                                  NULL,
                                  NULL);
  }
}

void VideoDecoder::StartVideoDecoder(void *args) {
  VideoDecoder *videoDecoder = static_cast<VideoDecoder *>(args);
  if (videoDecoder != NULL) {
    videoDecoder->RunVideoDecoder();
  }
}

void VideoDecoder::RunVideoDecoder() {
  LOGD("RunVideoDecoder\n");
  int ret = -1;
  int m_AVFrameCount = 0;
  while (true) {
    ret = av_read_frame(m_AVFormatContext, m_AVPack);
    if (ret < 0) {
      LOGD("读取数据包失败，返回:%d 错误描述:%s", ret, av_err2str(ret));
      break;
    }
    // while 知道读取pack包完成
    if (m_AVPack->stream_index == m_StreamIndex) {// 保证是一个流，可能读到其他的比如音频流
      ret = avcodec_send_packet(m_AVCodecContext, m_AVPack);
      if (ret != 0) {
        LOGD("avcodec_send_packet is error\n");
        break;
      }
      //去接收所有包里的frame
      int frameCount = 0;
      while (avcodec_receive_frame(m_AVCodecContext, m_AVFrame) == 0) {
        LOGE("VideoDecoder::OnFrameAvailable frame=%p,total:%d, count:%d",
             m_AVFrame,
             m_AVFrameCount++,
             frameCount++);
        double delay = AVSyncFirst();
//        int delay = 1000 / m_fps;
        av_usleep(1000000 * delay);
        LOGD("VideoDecoder delay:%f\n", delay);
        OnFrameAvailable(m_AVFrame);
      }
    }
    // 读取成功之后送到解码器
    av_packet_unref(m_AVPack);
  }
}

void VideoDecoder::OnFrameAvailable(AVFrame *avFrame) {
  sws_scale(m_SwsContext,
            avFrame->data,
            avFrame->linesize,
            0,
            m_VideoHeight,
            m_RGBAFrame->data,
            m_RGBAFrame->linesize);
  // 数据都在m_RGBAFrame中，构建NativeImage调用render进行渲染
  NativeImage image;
  memset(&image, 0, sizeof(NativeImage));
  image.format = DST_PIXEL_FORMAT;
  image.width = m_RenderWidth;
  image.height = m_RenderHeight;
  image.ppPlane[0] = m_RGBAFrame->data[0];
  image.pLineSize[0] = m_RGBAFrame->linesize[0];
  m_Render->RenderVideoFrame(&image);

}

void VideoDecoder::CleanResource() {
  if (m_AVFormatContext != NULL) {
    avformat_free_context(m_AVFormatContext);
  }
  if (m_AVCodecContext != NULL) {
    avcodec_free_context(&m_AVCodecContext);
  }
  if (m_AVPack != NULL) {
    av_packet_free(&m_AVPack);
  }
  if (m_AVFrame != NULL) {
    av_frame_free(&m_AVFrame);
  }
}

void VideoDecoder::SetVideoRender(NativeRender *nativeRender) {
  m_Render = nativeRender;
}

int VideoDecoder::GetRenderWidth() {
  return m_RenderWidth;
}

int VideoDecoder::GetRenderHeight() {
  return m_RenderHeight;
}

double VideoDecoder::AVSyncFirst() {
  // 视频同步音频 那就需要在解码视频的时候获取对应的音频播放时间来进行处理
  // 可以通过回调函数处理 也可以通过友元函数进行获取
  double frame_delay = 1.0 / m_fps;
  double extra_delay = m_AVFrame->repeat_pict / (2 * m_fps);
  double delay = extra_delay + frame_delay;
  double audio_clock = m_AudioDecoder->GetAudioClock();
  LOGD("VideoDecoder::AVSyncFirst delay11111:%f", delay);
  double video_clock = m_AVFrame->best_effort_timestamp * av_q2d(m_StreamTimeBase);
  LOGD("VideoDecoder::AVSyncFirst audio_clock:%f", audio_clock);
  LOGD("VideoDecoder::AVSyncFirst video_clock:%f", video_clock);
  // 定义一个同步的阈值
  /**
  * 1、delay < 0.04, 同步阈值就是0.04
  * 2、delay > 0.1 同步阈值就是0.1
  * 3、0.04 < delay < 0.1 ，同步阈值是delay
  */
  LOGD("VideoDecoder::AVSyncFirst delay11111:%f", delay);
  double diff = video_clock - audio_clock;
  LOGD("VideoDecoder::AVSyncFirst diff:%f", diff);
  double sync = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
  // 然后开始休眠
  if (diff <= -sync) {
    // 减小时间进行同步
    delay = FFMAX(0, delay + diff);
  } else if (diff > sync) {
    delay = delay + diff;
  }
  LOGD("VideoDecoder::AVSyncFirst delay22222:%f", delay);
  return delay;
}

double VideoDecoder::AVSyncSecond() {}

double VideoDecoder::AVSyncThird() {

}

void VideoDecoder::SetAudioDecoder(AudioDecoder *decoder) {
  this->m_AudioDecoder = decoder;
}
