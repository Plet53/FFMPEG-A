package com.fantastico_softworks.ffmpeg_a

import androidx.appcompat.app.AppCompatActivity
import java.io.InputStream
import java.io.OutputStream

class TranscodeActivity : AppCompatActivity() {
  
  companion object {
    fun newInstance() = TranscodeActivity()
    init {
      System.loadLibrary("ffmpeginterface")
      nativeInit()
    }
    
    external fun nativeInit()
    external fun probeVid(IOStream: InputStream): VidMeta
    external fun transcode(InputStream: InputStream, OutputStream: OutputStream, Type: Int, Bitrate: Int, Height: Int, Width: Int)
    
  }
  
  data class VidMeta (var dur: Long = 0, var wid: Int = 0, var hei: Int = 0)
}