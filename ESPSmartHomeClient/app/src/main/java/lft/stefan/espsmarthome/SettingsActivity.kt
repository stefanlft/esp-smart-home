package lft.stefan.espsmarthome

import android.content.Context
import android.content.Intent
import android.content.SharedPreferences
import android.os.Bundle
import android.util.Log
import android.widget.Button
import android.widget.EditText
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.textfield.TextInputEditText


class SettingsActivity : AppCompatActivity() {
    override fun onStart() {
        super.onStart()
    }

    private lateinit var settings: SharedPreferences

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.settings_layout)

        findViewById<EditText>(R.id.input_server_address).setText(getFromPreferences("address", "http://127.0.0.1:5555/"))
        findViewById<EditText>(R.id.input_delay).setText(getFromPreferences("delay", "1.5"))


        findViewById<Button>(R.id.save_button).setOnClickListener {
            val address = findViewById<EditText>(R.id.input_server_address).text.toString()
            val delay = findViewById<EditText>(R.id.input_delay).text.toString()
            Log.i("Settings", "Delay $delay")

            saveToPreferences("address", address)
            saveToPreferences("delay", delay)

            Toast.makeText(this@SettingsActivity, "Settings saved!", Toast.LENGTH_SHORT).show()
        }
    }

    override fun onStop() {
        super.onStop()
    }

    override fun onResume() {
        super.onResume()
    }

    override fun onPause() {
        super.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
    }

    private fun restartApp(context: Context) {
        val intent = context.packageManager.getLaunchIntentForPackage(context.packageName)
        intent?.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP or Intent.FLAG_ACTIVITY_NEW_TASK)
        context.startActivity(intent)
        Runtime.getRuntime().exit(0)  // Use exit(0) instead of finishAffinity() for a clean process kill
    }


    private fun Context.saveToPreferences(key: String, value: String) {
        val sharedPreferences = getSharedPreferences("MyPrefs", Context.MODE_PRIVATE)
        sharedPreferences.edit().putString(key, value).apply()
    }

    private fun Context.getFromPreferences(key: String, defaultValue: String): String {
        val sharedPreferences = getSharedPreferences("MyPrefs", Context.MODE_PRIVATE)
        return sharedPreferences.getString(key, defaultValue) ?: defaultValue
    }
}