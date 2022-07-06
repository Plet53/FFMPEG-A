package com.fantastico_softworks.ffmpeg_a.ui.main

import android.annotation.SuppressLint
import android.content.Context
import android.net.Uri
import android.util.Log
import androidx.databinding.ObservableField
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.fantastico_softworks.ffmpeg_a.TranscodeActivity
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlin.math.min

@SuppressLint("StaticFieldLeak")
class MainViewModel : ViewModel() {
  // 8 MegaBytes = 64 MegaBits
  val discordMax = 64000000
  val AV_TIME_SCALE = 1000000
  val inFileName: ObservableField<String> = ObservableField<String>("")
  val outFileName: ObservableField<String> = ObservableField<String>("")
  var outFileType: String = "webm"
  var duration: Long = 0
  var inFileURI: Uri? = null
  var outFileURI: Uri? = null
  var vidBitRateAvg: Int = 0
  var inFileSize: Int = 0
  var invidwidth: Int = 0
  var invidheight: Int = 0
  var outvidwidth: Int = 0
  var outvidheight: Int = 0
  var preset: Int = 0
  var audio: Boolean = true
  val act = TranscodeActivity()
  
  val defDispatch = Dispatchers.IO
  
  fun probe(appcontext: Context) {
    if (inFileURI != null) {
      val stream = appcontext.contentResolver.openInputStream(inFileURI!!)
      if (stream != null) {
        viewModelScope.launch(defDispatch) {
        val data = act.probeVid(stream)
          duration = data.dur
          invidheight = data.hei
          outvidheight = data.hei
          invidwidth = data.wid
          outvidwidth = data.wid
          stream.close()
          // bits/second coming from Bytes in the file / (Duration in seconds * 8 (bits in a byte)
          vidBitRateAvg = ((inFileSize / ((duration * 8) / AV_TIME_SCALE)).toInt())
        }
      } else { Log.d("probe", "could not open file") }
    }
    else { Log.d("probe", "no video selected") }
  }
    
  fun transcode(appcontext: Context) {
    if ((inFileURI != null) && (outFileURI != null)) {
      val istream = appcontext.contentResolver.openInputStream(inFileURI!!)
      if (istream != null) {
        val ostream = appcontext.contentResolver.openOutputStream(outFileURI!!)
        if (ostream != null) {
          var type = 0
          var bitrate = vidBitRateAvg
          when (preset) {
            1 -> {
              type = 1
              bitrate = 2500000
              smallres(720)
              if (duration > 140000000) {
                // TODO: twitter doesn't accept videos longer than 2:20, inform user
                return
              }
            }
            2 -> {}
            3 -> {
              type = 2
            }
            4 -> {
              type = 1
            }
            else -> {
              // Trillions exceed the limits of an Int, which, fair.
              // If input bitrate is already low we don't have to worry too much about compression.
              bitrate = min(((discordMax.toLong() * AV_TIME_SCALE) / duration).toInt(), vidBitRateAvg)
              when (bitrate){
                in 4500000..8000000 -> {
                  smallres(1080)
                }
                in 2500000..4499999 -> {
                  smallres(720)
                }
                in 1000000..1999999 -> {
                  smallres(480)
                }
                in 400000..999999 -> {
                  smallres(360)
                }
                in 200000..399999 -> {
                  smallres(240)
                }
                in 0..199999 -> {
                  // TODO: Not enough bitrate for a meaningful video, inform user
                  return
                }
              }
            }
          }
          val success = act.transcode(istream, ostream, type, outvidwidth, outvidheight, bitrate, audio)
          istream.close()
          ostream.close()
          if (!success) {
            // Clean it up on failure
            val fstream = appcontext.contentResolver.openOutputStream(outFileURI!!, "wt")
            fstream?.close()
          }
          return
        } else {
          Log.d("transcode", "could not open outstream")
          return
        }
      } else {
        Log.d("transcode", "could not open instream")
        return
      }
    } else { Log.d("transcode","video setup is not complete")
    return }
  }
  
  fun smallres(res: Int) {
    // smallres only ever acts if we *need* to ensmallen the vid
    if (invidheight > res && invidwidth > res) {
      if (invidheight < invidwidth) {
        outvidheight = res
        outvidwidth = (res * invidwidth) / invidheight
        // All resolutions for videos must be even, always.
        outvidwidth += (outvidwidth % 2)
      } else {
        // also covers equal, so not worrying about it
        outvidwidth = res
        outvidheight = (res * invidheight) / invidwidth
        outvidheight += (outvidheight % 2)
      }
    }
  }
  
  
  
}