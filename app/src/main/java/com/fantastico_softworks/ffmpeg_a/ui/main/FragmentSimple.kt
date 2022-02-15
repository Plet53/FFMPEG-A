package com.fantastico_softworks.ffmpeg_a.ui.main

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import com.fantastico_softworks.ffmpeg_a.R
import com.fantastico_softworks.ffmpeg_a.databinding.FragmentSimpleBinding


class FragmentSimple : Fragment() {
  
  external fun hello(): String
  val mess: String = "hey world"
  var notifid: Int = 0
  
  companion object {
    fun newInstance() = FragmentSimple()
    init {
      System.loadLibrary("hello")
    }
  }
  
  private lateinit var viewModel: MainViewModel
  
  override fun onCreateView(
    inflater: LayoutInflater, container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    val binding = FragmentSimpleBinding.inflate(inflater, container, false)
    binding.mainButton.setOnClickListener { sendMess(binding.root) }
    return binding.root
  }
  
  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)
    viewModel = ViewModelProvider(this).get(MainViewModel::class.java)
    // TODO: Use the ViewModel
  }
  
  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
  }
  
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
}