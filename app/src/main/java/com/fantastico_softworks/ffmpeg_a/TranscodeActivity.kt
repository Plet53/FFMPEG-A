package com.fantastico_softworks.ffmpeg_a

import androidx.appcompat.app.AppCompatActivity
import java.io.InputStream

class TranscodeActivity : AppCompatActivity() {
  
  companion object {
    fun newInstance() = TranscodeActivity()
    init {
      System.loadLibrary("ffmpeginterface")
      nativeInit()
    }
    
    external fun nativeInit()
    external fun probeVid(IOStream: InputStream): VidMeta
    
  }
  
  data class VidMeta(var dur: String, var vidbitrate: Int)
}