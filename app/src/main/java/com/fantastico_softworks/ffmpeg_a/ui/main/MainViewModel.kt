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
  val inFileName: ObservableField<String> = ObservableField<String>("")
  var outFileName: ObservableField<String> = ObservableField<String>("")
  var outFileType: String = "webm"
  var inFileURI: Uri? = null
  var outFileURI: Uri? = null
  var vidBitRateAvg: Int = 0
  var vidBitRateVar: Int = 0
  var inFileSize: Int = 0
  var outFileSize: Int = 0
  var smallerRes: Int = 720
  var preset: Int = 0
  
  val defDispatch = Dispatchers.Default
  
  companion object {
    fun probe(mainViewModel: MainViewModel, appcontext: Context) {
      if(mainViewModel.inFileURI != null) {
        val stream = appcontext.contentResolver.openInputStream(mainViewModel.inFileURI!!)
        if (stream != null) {
          val data = TranscodeActivity.probeVid(stream!!)
          stream.close()
          Log.d("probe", data.dur)
          Log.d("probe", data.vidbitrate.toString())
        } else { Log.d("probe", "could not open file") }
      }
      else { Log.d("probe", "no video selected")
        return }
    }
  }
  
}