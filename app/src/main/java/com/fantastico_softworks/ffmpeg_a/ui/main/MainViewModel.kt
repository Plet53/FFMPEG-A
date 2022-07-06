package com.fantastico_softworks.ffmpeg_a.ui.main

import android.content.Context
import android.net.Uri
import android.util.Log
import androidx.databinding.ObservableField
import androidx.lifecycle.ViewModel
import com.fantastico_softworks.ffmpeg_a.TranscodeActivity
import kotlinx.coroutines.Dispatchers

class MainViewModel : ViewModel() {
  val discordMax = 8000000
  val AV_TIME_SCALE = 1000000
  val inFileName: ObservableField<String> = ObservableField<String>("")
  var outFileName: ObservableField<String> = ObservableField<String>("")
  var outFileType: String = "webm"
  var duration: Long = 0
  var inFileURI: Uri? = null
  var outFileURI: Uri? = null
  var vidBitRateAvg: Int = 0
  var inFileSize: Int = 0
  var outFileSize: Long = 0
  var invidwidth: Int = 0
  var invidheight: Int = 0
  var smallerRes: Int = 720
  var preset: Int = 0
  
  val defDispatch = Dispatchers.Default
  
  companion object {
    fun probe(mainViewModel: MainViewModel, appcontext: Context) {
      if (mainViewModel.inFileURI != null) {
        val stream = appcontext.contentResolver.openInputStream(mainViewModel.inFileURI!!)
        if (stream != null) {
          val data = TranscodeActivity.probeVid(stream)
          mainViewModel.duration = data.dur
          mainViewModel.invidheight = data.hei
          mainViewModel.invidwidth = data.wid
          stream.close()
          // bits/second coming from Bytes in the file / (Duration in seconds * 8 (bits in a byte)
          mainViewModel.vidBitRateAvg = ((mainViewModel.inFileSize / ((mainViewModel.duration * 8) / mainViewModel.AV_TIME_SCALE)).toInt())
        } else { Log.d("probe", "could not open file") }
      }
      else { Log.d("probe", "no video selected")
        return }
    }
    
    fun transcode(mainViewModel: MainViewModel, appcontext: Context) {
      if ((mainViewModel.inFileURI != null) && (mainViewModel.outFileURI != null)) {
        val istream = appcontext.contentResolver.openInputStream(mainViewModel.inFileURI!!)
        if (istream != null) {
          val ostream = appcontext.contentResolver.openOutputStream(mainViewModel.outFileURI!!)
          if (ostream != null) {
            
            TranscodeActivity.transcode(istream, ostream,,,,)
            istream.close()
            ostream.close()
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
  }
  
}