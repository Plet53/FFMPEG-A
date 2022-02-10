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
  val mess: String = "hey world"
  var notifid: Int = 0
  
  companion object {
    fun newInstance() = FragmentSimple()
  }
  
  private lateinit var viewModel: MainViewModel
  
  override fun onCreateView(
    inflater: LayoutInflater, container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.fragment_simple, container, false)
  }
  
  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)
    viewModel = ViewModelProvider(this).get(MainViewModel::class.java)
    // TODO: Use the ViewModel
    val binding = FragmentSimpleBinding.inflate(this.layoutInflater)
    binding.mainButton.setOnClickListener(View.OnClickListener {
      fun onClick(view: View) {
        sendMess()
      }
    } )
  }
  
  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
  }
  
  fun sendMess() {
    val builder = NotificationCompat.Builder(this.requireContext(), "MAIN")
      .setSmallIcon(R.drawable.ic_notif_dark)
      .setContentTitle(mess)
      .setPriority(NotificationCompat.PRIORITY_DEFAULT)
    with(NotificationManagerCompat.from(this.requireContext())) {
      notify(notifid, builder.build())
      notifid = notifid.inc()
    }
  }
  
}