package com.fantastico_softworks.ffmpeg_a.ui.main

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.PopupMenu
import androidx.fragment.app.Fragment
import androidx.lifecycle.viewModelScope
import com.fantastico_softworks.ffmpeg_a.MainActivity
import com.fantastico_softworks.ffmpeg_a.R
import com.fantastico_softworks.ffmpeg_a.databinding.FragmentSimpleBinding
import kotlinx.coroutines.launch

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
    // Select a File
    binding.inputButton.setOnClickListener { mainActivity.grabSingleFile.launch(arrayOf("video/*")) }
    // Create a File
    binding.outputButton.setOnClickListener { mainActivity.makeSingleFile.launch(
      "${viewModel.inFileName.get()?.substringBeforeLast(".", "video")}.${viewModel.outFileType}") }
    // Transcode video from input to output, on the IO threadpool
    binding.mainButton.setOnClickListener { viewModel.viewModelScope.launch(viewModel.defDispatch) {
      viewModel.transcode(mainActivity.applicationContext) }}
    // Create the preset menu
    val popup = PopupMenu(context, binding.dropDown)
    popup.inflate(R.menu.preset_menu)
    // Respond to the selection of a preset
    popup.setOnMenuItemClickListener { item ->
      // Yes my presets are 0 indexed
      preset = item.numericShortcut.digitToInt() - 1
      viewModel.outFileURI = null
      viewModel.outFileName.set("")
      // R stores the strings one after the other, exactly as I'd hoped, nyehe
      binding.modeName.setText(R.string.mode_name0 + preset)
      binding.modeDescription.setText(R.string.mode_desc0 + preset)
      when (preset) {
        3 -> {viewModel.outFileType = "avi"}
        1, 4 -> {viewModel.outFileType = "mp4"}
        else -> {viewModel.outFileType = "webm"}
      }
      // I have to return a value here. But the return keyword is illegal in a lambda. *shrug*
      true }
    // Show the preset menu
    binding.dropDown.setOnClickListener { popup.show() }
    // Determine whether we're dealing with audio
    binding.audswitch.setOnCheckedChangeListener { _, isChecked ->
      viewModel.audio = !isChecked }
    return binding.root
  }
  
  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)
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