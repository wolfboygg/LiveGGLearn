package com.putong.media

import android.util.Log
import android.view.Surface

class JNIHelper {
  init {
    System.loadLibrary("media")
  }

  external fun getFFmpegInfo(path: String, surface: Surface): IntArray

  companion object {
    fun printInfo(path: String, surface: Surface): IntArray {
      val helper = JNIHelper()
      val info = helper.getFFmpegInfo(path, surface)
      Log.i("GUO", "info:${info.toString()}")
      return info
    }
  }
}
