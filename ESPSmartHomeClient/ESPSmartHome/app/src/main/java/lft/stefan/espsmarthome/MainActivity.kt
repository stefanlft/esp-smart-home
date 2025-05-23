package lft.stefan.espsmarthome

import android.Manifest
import android.R.attr.value
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.LayoutInflater
import android.view.Menu
import android.view.MenuItem
import android.widget.Button
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.google.android.material.bottomsheet.BottomSheetDialog
import com.google.android.material.snackbar.Snackbar
import com.google.android.material.textfield.TextInputEditText
import lft.stefan.espsmarthome.databinding.ActivityMainBinding
import java.util.Locale


class MainActivity : AppCompatActivity() {

//    private lateinit var appBarConfiguration: AppBarConfiguration
//    private var delay by Delegates.notNull<Double>()
    private lateinit var binding: ActivityMainBinding
    private var status = "not detected"
    private val notificationWrapper: NotificationWrapper = NotificationWrapper()

    private lateinit var roomsLayout: LinearLayout


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.POST_NOTIFICATIONS)
                != PackageManager.PERMISSION_GRANTED
            ) {
                requestPermissionLauncher.launch(Manifest.permission.POST_NOTIFICATIONS)
            }
        }


        notificationWrapper.createNotificationChannel(this)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        roomsLayout = findViewById(R.id.rooms_layout)

        setSupportActionBar(binding.toolbar)

        universalApiWrapper.setUrl(getFromPreferences("address", "http://127.0.0.1:8080/"))
//        delay =
//        Log.i("delay", "$delay prefs ${getFromPreferences("delay", "1.5")}")
        universalApiWrapper.setMotionCallback(object : MotionCallback {
            override fun onMotionDetected(room: String) {
                status = "detected"
                notificationWrapper.sendNotification(this@MainActivity,
                    "Motion detected in $room!",
                    "ESP Node detected motion!"
                )
            }
        })

//        universalApiWrapper.sseStart()

        refresh()
        startRefreshingData()

        binding.fab.setOnClickListener { view ->
            val bottomSheetDialog = BottomSheetDialog(this)
            val sheetView = layoutInflater.inflate(R.layout.bottom_sheet_layout, null)
            bottomSheetDialog.setContentView(sheetView)
            bottomSheetDialog.show()

            sheetView.findViewById<TextInputEditText>(R.id.input_server_address)
                        .setText(getFromPreferences("address", ""))

            sheetView.findViewById<Button>(R.id.save_button)?.setOnClickListener {
                var address = sheetView.findViewById<TextInputEditText>(R.id.input_server_address)
                                        .text.toString()

                if (address.isEmpty()) {
                    Toast.makeText(this@MainActivity, "Server address cannot be empty", Toast.LENGTH_SHORT).show()
                    return@setOnClickListener
                }

                if (address[address.length - 1] != '/') {
                    address += "/"
                }
                saveToPreferences("address", address)
                Snackbar.make(view, "Saved new server address", Snackbar.LENGTH_LONG).show()
                universalApiWrapper.setUrl(address)
                bottomSheetDialog.dismiss()
            }


        }

        findViewById<Button>(R.id.button_refresh).setOnClickListener {
            refresh()
        }

    }

    private fun showSensorSelectionMenu(roomName: String) {
        val sensorOptions = arrayOf(
            "Temperature",
            "Humidity",
            "Motion",
            "Luminance"
        )

        val sensorKeys = arrayOf(
            "temperature",
            "humidity",
            "motion",
            "lightness" // or "luminance" - match your API
        )

        AlertDialog.Builder(this)
            .setTitle("Select sensor data for $roomName")
            .setItems(sensorOptions) { dialog, which ->
                val selectedSensor = sensorKeys[which]
                openGraphActivity(roomName, selectedSensor)
                dialog.dismiss()
            }
            .setNegativeButton("Cancel") { dialog, _ ->
                dialog.dismiss()
            }
            .show()
    }

    private fun openGraphActivity(roomName: String, selectedSensor: String) {
        val myIntent: Intent = Intent(
            this@MainActivity,
            GraphActivity::class.java
        )
        myIntent.putExtra("room", roomName) //Optional parameters
        myIntent.putExtra("aspect", selectedSensor) //Optional parameters
        this@MainActivity.startActivity(myIntent)
    }


    private val requestPermissionLauncher =
        registerForActivityResult(ActivityResultContracts.RequestPermission()) { isGranted ->
            if (isGranted) {
                notificationWrapper.sendNotification(this, "Permission Granted!", "You can now receive notifications.")
            }
        }


    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Inflate the menu; this adds items to the action bar if it is present.
        menuInflater.inflate(R.menu.menu_main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        return when (item.itemId) {
            R.id.action_settings -> {
                val myIntent: Intent = Intent(
                    this@MainActivity,
                    SettingsActivity::class.java
                )
//                myIntent.putExtra("key", value) //Optional parameters
                this@MainActivity.startActivity(myIntent)
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }


    private fun refresh() {
        universalApiWrapper.update()

        roomsLayout.removeAllViews()

        universalApiWrapper.getJson().rooms.forEach { (roomName, room) ->
            addRoomView(roomName, room)
        }

//        findViewById<TextView>(R.id.textview_ldr).text = "LDR value: " + (universalApiWrapper.getJson().rooms["dormitor"]?.lightness
//            ?: "10%");
    }

    private fun addRoomView(roomName: String, room: RoomData) {
        val inflater = LayoutInflater.from(this)
        val roomView = inflater.inflate(R.layout.room_layout, roomsLayout, false)

        // Find the TextViews in the room template
        val roomNameTextView: TextView = roomView.findViewById(R.id.room_name)
        val temperatureTextView: TextView = roomView.findViewById(R.id.temperature)
        val humidityTextView: TextView = roomView.findViewById(R.id.humidity)
        val lightnessTextView: TextView = roomView.findViewById(R.id.lightness)
        val motionStatusTextView: TextView = roomView.findViewById(R.id.motion_status)
        val activeNodes: TextView = roomView.findViewById(R.id.active_nodes)

        // Set the data for each room dynamically
        roomNameTextView.text = roomName.replaceFirstChar {
            if (it.isLowerCase()) it.titlecase(
                Locale.ROOT
            ) else it.toString()
        }

        var nodeString : String = ""

        for ((i, node) in room.nodes.withIndex()) {
            nodeString += node
            if (i + 1 != room.nodes.size) {
                nodeString += ", "
            }
        }

        temperatureTextView.text = "Temperature: ${room.temperature}"
        humidityTextView.text = "Humidity: ${room.humidity}"
        lightnessTextView.text = "Lightness: ${room.lightness}"
        motionStatusTextView.text = "Motion: ${if (room.motion == "1") "Detected" else "None"}"
        activeNodes.text = "Active Nodes: $nodeString"

        roomView.setOnClickListener {
            showSensorSelectionMenu(roomName)
        }

        // Add the room view to the parent layout
        roomsLayout.addView(roomView)
    }


    private fun startRefreshingData() {
        val handler = Handler(Looper.getMainLooper()) // To run on the main thread

        val refreshRunnable = object : Runnable {
            override fun run() {
                // Your task here (e.g., send an HTTP request)
                println("Refreshing data...")

                refresh()

                // Post the task again after 1.5 seconds (1500 milliseconds)
                handler.postDelayed(this,
                    (getFromPreferences("delay", "1.5").toDouble() * 1000).toLong()
                )
                universalApiWrapper.setUrl(getFromPreferences("address", "http://127.0.0.1:8080/"))
            }
        }

        // Start the task immediately
        handler.post(refreshRunnable)
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