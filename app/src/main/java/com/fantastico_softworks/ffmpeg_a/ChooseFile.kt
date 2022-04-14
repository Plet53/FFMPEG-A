package com.fantastico_softworks.ffmpeg_a

import android.content.Context
import android.content.Intent
import androidx.activity.result.contract.ActivityResultContracts

class ChooseFile : ActivityResultContracts.OpenDocument() {
  override fun createIntent(context: Context, input: Array<String>): Intent {
    val intent = super.createIntent(context, input)
    return Intent.createChooser(intent, R.string.openvid.toString())
  }
}