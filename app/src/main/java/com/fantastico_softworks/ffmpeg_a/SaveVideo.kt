package com.fantastico_softworks.ffmpeg_a

import android.content.Context
import android.content.Intent
import androidx.activity.result.contract.ActivityResultContracts

class SaveVideo : ActivityResultContracts.CreateDocument() {
  override fun createIntent(context: Context, input: String): Intent {
    val ext = input.substringAfterLast(".", "webm")
    return super.createIntent(context, input)
      .setType("video/$ext")
      .putExtra(Intent.EXTRA_TITLE, input)
  }
}