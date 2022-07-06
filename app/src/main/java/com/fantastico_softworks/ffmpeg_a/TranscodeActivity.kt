package com.fantastico_softworks.ffmpeg_a

import androidx.appcompat.app.AppCompatActivity
import java.io.InputStream
import java.io.OutputStream

class TranscodeActivity : AppCompatActivity() {
  
  external fun probeVid(IOStream: InputStream): VidMeta
  external fun transcode(InputStream: InputStream, OutputStream: OutputStream, type: Int, width: Int, height: Int, bitrate: Int, audio: Boolean): Boolean
  
  companion object {
    fun newInstance() = TranscodeActivity()
    init {
      System.loadLibrary("ffmpeginterface")
      nativeInit()
    }
    
    private external fun nativeInit()
    
  }
  
  data class VidMeta (var dur: Long = 0, var wid: Int = 0, var hei: Int = 0)
}