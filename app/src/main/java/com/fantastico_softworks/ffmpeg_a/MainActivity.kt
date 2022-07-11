package com.fantastico_softworks.ffmpeg_a

import android.annotation.SuppressLint
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.provider.OpenableColumns
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import com.fantastico_softworks.ffmpeg_a.ui.main.FragmentSimple
import com.fantastico_softworks.ffmpeg_a.ui.main.MainViewModel
import kotlinx.coroutines.launch

class MainActivity : AppCompatActivity() {
  
  @SuppressLint("Range")
  val grabSingleFile = registerForActivityResult(ChooseFile()) { uri: Uri? ->
    if (uri != null) {
      viewmodel.inFileURI = uri
      val cursor = contentResolver.query(
        uri, null, null, null, null, null)
      cursor?.use {
        if (it.moveToFirst()) {
          viewmodel.inFileName.set(it.getString(it.getColumnIndex(OpenableColumns.DISPLAY_NAME)))
          viewmodel.inFileSize = it.getInt(it.getColumnIndex(OpenableColumns.SIZE))
          viewmodel.viewModelScope.launch(viewmodel.defDispatch) { viewmodel.probe(applicationContext) }
        }
      }
    }
  }
  
  @SuppressLint("Range")
  val makeSingleFile = registerForActivityResult(SaveVideo()) { uri: Uri? ->
    if (uri != null) {
      viewmodel.outFileURI = uri
      // Normally I'd ask the mediastore what the new filename actually is, but it's not initialized properly.
      viewmodel.outFileName.set("${viewmodel.inFileName.get()?.substringBeforeLast(".") ?: "video"}.${viewmodel.outFileType}")
    }
  }
  
  lateinit var viewmodel: MainViewModel
  @RequiresApi(Build.VERSION_CODES.O)
  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    viewmodel = ViewModelProvider(this).get(MainViewModel::class.java)
    setContentView(R.layout.main_activity)
    if (savedInstanceState == null) {
      val fs = FragmentSimple.newInstance()
      supportFragmentManager.beginTransaction()
        .replace(R.id.fragmentContainerView, fs)
        .commitNow()
      fs.mainActivity = this
      fs.viewModel = viewmodel
    }
    // Notification Channels only exist as of O.
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      val name = getString(R.string.channel_name)
      val importance = NotificationManager.IMPORTANCE_DEFAULT
      val channel = NotificationChannel("MAIN", name, importance).apply {
        description = getString(R.string.channel_description)
      }
      val notificationManager: NotificationManager =
        getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
      notificationManager.createNotificationChannel(channel)
    }
  }
  
}
