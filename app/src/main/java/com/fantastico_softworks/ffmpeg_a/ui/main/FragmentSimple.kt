package com.fantastico_softworks.ffmpeg_a.ui.main

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.fantastico_softworks.ffmpeg_a.MainActivity
import com.fantastico_softworks.ffmpeg_a.databinding.FragmentSimpleBinding

class FragmentSimple : Fragment() {
  
  external fun hello(): String
  val mess: String = "hey world"
  var notifid: Int = 0
  var preset: Int = 0
  
  companion object {
    fun newInstance() = FragmentSimple()
  }
  
  lateinit var viewModel: MainViewModel
  lateinit var mainActivity: MainActivity
  
  override fun onCreateView(
    inflater: LayoutInflater, container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    val binding = FragmentSimpleBinding.inflate(inflater, container, false)
    binding.viewmodel = viewModel
    binding.inputButton.setOnClickListener { mainActivity.grabSingleFile.launch(arrayOf("video/*")) }
    //binding.outputButton.setOnClickListener { mainActivity.makeSingleFile.launch("${viewModel.inFileName.get()}.${viewModel.outFileType}") }
    binding.mainButton.setOnClickListener { Log.d("user", "transcoding")
      MainViewModel.probe(viewModel, mainActivity.applicationContext) }
    return binding.root
  }
  
  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)
    // TODO: Use the ViewModel
  }
  
  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
  }
  
  /*
  Keeping here until I need the notification code again.
  fun sendMess(view: View) {
    val builder = NotificationCompat.Builder(view.context, "MAIN")
      .setSmallIcon(R.drawable.ic_notif_dark)
      .setContentTitle(hello())
      .setPriority(NotificationCompat.PRIORITY_DEFAULT)
    with(NotificationManagerCompat.from(view.context)) {
      notify(notifid, builder.build())
      notifid = notifid.inc()
    }
  }
  */
}