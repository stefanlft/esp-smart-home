package lft.stefan.espsmarthome

import android.content.pm.ActivityInfo
import android.graphics.Color
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.enableEdgeToEdge
import com.github.mikephil.charting.charts.LineChart
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import java.time.Instant
import java.util.Locale

class GraphActivity : ComponentActivity() {

    private lateinit var chart: LineChart
    private var dataSet: LineDataSet? = null // Make nullable to handle initialization

    private val handler = Handler(Looper.getMainLooper())
    private val updateInterval = 1000L // 1 second

    private lateinit var roomName: String
    private lateinit var followedAspect: String

    // Store fetched history here separately from main JSON data
    private var currentHistory: MutableMap<String, List<Map<String, String>>> = mutableMapOf()

    // Flag to prevent multiple simultaneous API calls
    private var isUpdating = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContentView(R.layout.graph_layout)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE

        chart = findViewById(R.id.graph)

        // Get intent extras with null checks
        followedAspect = intent.getStringExtra("aspect") ?: "temperature"
        roomName = intent.getStringExtra("room") ?: "Unknown Room"

        chart.description.text = roomName

        // Configure chart
        setupChart()

        // Initial empty data set
        refreshDataSet()

        // Start periodic updates
        handler.post(updateRunnable)
    }

    private fun setupChart() {
        chart.apply {
            setTouchEnabled(true)
            isDragEnabled = true
            setScaleEnabled(true)
            setPinchZoom(true)

            // Configure axes
            xAxis.apply {
                granularity = 1f
                setDrawGridLines(true)
            }

            axisLeft.apply {
                setDrawGridLines(true)
            }

            axisRight.isEnabled = false

            // Set description
            description.apply {
                text = "$roomName - ${followedAspect.replaceFirstChar {
                    if (it.isLowerCase()) it.titlecase(Locale.ROOT) else it.toString()
                }}"
                textSize = 12f
            }
        }
    }

    private val updateRunnable = object : Runnable {
        override fun run() {
            // Prevent multiple simultaneous updates
            if (isUpdating) {
                handler.postDelayed(this, updateInterval)
                return
            }

            isUpdating = true

            try {
                // Fetch new data
                universalApiWrapper.fetchRoomHistory(roomName, object : HistoryCallback {
                    override fun onSuccess(history: HistoryResponse?) {
                        try {
                            if (history != null && !isDestroyed) {
                                // Update history data
                                history.humidity?.let { currentHistory["humidity"] = it }
                                history.lightness?.let { currentHistory["lightness"] = it }
                                history.temperature?.let { currentHistory["temperature"] = it }
                                history.motion?.let { currentHistory["motion"] = it }

                                // Update UI on main thread
                                runOnUiThread {
                                    if (!isDestroyed) {
                                        updateChart()
                                    }
                                }
                            }
                        } catch (e: Exception) {
                            Log.e("GraphActivity", "Error processing history data", e)
                        } finally {
                            isUpdating = false
                        }
                    }

                    override fun onFailure(error: String?) {
                        Log.e("GraphActivity", "History fetch failed: $error")
                        isUpdating = false
                    }
                })
            } catch (e: Exception) {
                Log.e("GraphActivity", "Error in update runnable", e)
                isUpdating = false
            }

            // Schedule next update
            handler.postDelayed(this, updateInterval)
        }
    }

    private fun updateChart() {
        try {
            refreshDataSet()

            dataSet?.let { ds ->
                if (ds.entryCount > 0) {
                    chart.data = LineData(ds)
                    chart.data.notifyDataChanged()
                    chart.notifyDataSetChanged()
                    chart.invalidate()

                    // Auto-scroll to show latest data
                    chart.setVisibleXRangeMaximum(120F)
                    chart.setVisibleXRangeMinimum(10f) // Show at least 10 points
                    chart.moveViewToX(ds.xMax)
                } else {
                    Log.d("GraphActivity", "No data entries to display")
                }
            }
        } catch (e: Exception) {
            Log.e("GraphActivity", "Error updating chart", e)
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        handler.removeCallbacks(updateRunnable)
    }

    private fun refreshDataSet() {
        try {
            val entries = mutableListOf<Entry>()
            val aspectRecords: List<Map<String, String>> = currentHistory[followedAspect] ?: emptyList()

            Log.d("GraphActivity", "Refreshing dataset for aspect: $followedAspect")
            Log.d("GraphActivity", "Records count: ${aspectRecords.size}")

            if (aspectRecords.isEmpty()) {
                Log.d("GraphActivity", "No records available for aspect: $followedAspect")
                dataSet = createEmptyDataSet()
                return
            }

            val now = Instant.now().epochSecond

            for ((index, record) in aspectRecords.withIndex()) {
                val timeString = record["time"]
                val valueString = record["value"]

                val time = timeString?.toLongOrNull()
                val value = valueString?.toFloatOrNull()

                if (time == null || value == null) {
                    Log.w("GraphActivity", "Skipping record due to null time or value: $record")
                    continue
                }

                // Use relative time (seconds from now) or index-based X values
                val xValue = (time - now).toFloat()
                entries.add(Entry(xValue, value))

                Log.d("GraphActivity", "Added entry: x=$xValue, y=$value")
            }

            Log.d("GraphActivity", "Total entries created: ${entries.size}")

            // Sort entries by X value to ensure proper line drawing
            entries.sortBy { it.x }

            dataSet = LineDataSet(entries,
                followedAspect.replaceFirstChar {
                    if (it.isLowerCase()) it.titlecase(Locale.ROOT) else it.toString()
                }
            ).apply {
                color = Color.BLUE
                valueTextColor = Color.BLACK
                lineWidth = 2f
                circleRadius = 4f
                setDrawValues(false)
                setDrawCircles(true)
                mode = LineDataSet.Mode.LINEAR
            }

        } catch (e: Exception) {
            Log.e("GraphActivity", "Error in refreshDataSet", e)
            dataSet = createEmptyDataSet()
        }
    }

    private fun createEmptyDataSet(): LineDataSet {
        return LineDataSet(mutableListOf(),
            followedAspect.replaceFirstChar {
                if (it.isLowerCase()) it.titlecase(Locale.ROOT) else it.toString()
            }
        ).apply {
            color = Color.BLUE
            valueTextColor = Color.BLACK
            lineWidth = 2f
            circleRadius = 4f
            setDrawValues(false)
        }
    }
}