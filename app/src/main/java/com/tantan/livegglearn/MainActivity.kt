package com.tantan.livegglearn

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Environment
import android.view.SurfaceHolder
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.putong.media.JNIHelper
import com.tantan.livegglearn.util.CommonUtils
import com.tantan.livegglearn.view.LiveSurfaceView

open class MainActivity : AppCompatActivity(), SurfaceHolder.Callback {

  private val REQUEST_PERMISSIONS =
    arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE)
  private val PERMISSION_REQUEST_CODE = 1

  private val mVideoPath =
    Environment.getExternalStorageDirectory().absolutePath + "/byteflow/one_piece.mp4"

  lateinit var surfaceView: LiveSurfaceView
  var holder: SurfaceHolder? = null

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    setContentView(R.layout.activity_main)
    surfaceView = findViewById(R.id.surface_view)
    surfaceView.holder.addCallback(this)
  }

  fun onClick(view: View) {
    when (view.id) {
      R.id.decoder -> {
        holder?.let {
          val array = JNIHelper.printInfo(mVideoPath, it.surface)
          surfaceView.setAspectRatio(array[0], array[1])
        }
      }
    }
  }

  override fun onResume() {
    super.onResume()
    if (!hasPermissionsGranted(REQUEST_PERMISSIONS)) {
      ActivityCompat.requestPermissions(this, REQUEST_PERMISSIONS, PERMISSION_REQUEST_CODE)
    }
    CommonUtils.copyAssetsDirToSDCard(this, "byteflow", "/sdcard")
  }

  override fun onRequestPermissionsResult(
    requestCode: Int,
    permissions: Array<String?>,
    grantResults: IntArray
  ) {
    if (requestCode == PERMISSION_REQUEST_CODE) {
      if (!hasPermissionsGranted(REQUEST_PERMISSIONS)) {
        Toast.makeText(this, "We need the permission: WRITE_EXTERNAL_STORAGE", Toast.LENGTH_SHORT)
          .show()
      }
    } else {
      super.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }
  }

  private fun hasPermissionsGranted(permissions: Array<String>): Boolean {
    for (permission in permissions) {
      if (
        ActivityCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED
      ) {
        return false
      }
    }
    return true
  }

  override fun surfaceCreated(holder: SurfaceHolder) {
    this.holder = holder
  }

  override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {}

  override fun surfaceDestroyed(holder: SurfaceHolder) {}
}
