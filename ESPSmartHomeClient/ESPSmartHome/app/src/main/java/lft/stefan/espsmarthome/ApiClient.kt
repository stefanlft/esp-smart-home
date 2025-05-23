package lft.stefan.espsmarthome

import android.util.Log
import com.squareup.moshi.JsonAdapter
import com.squareup.moshi.Moshi
import com.squareup.moshi.Types
import com.squareup.moshi.kotlin.reflect.KotlinJsonAdapterFactory
import okhttp3.OkHttpClient
import okhttp3.Request
import java.io.IOException

interface ApiCallback {
    fun onSuccess(result: String?)
    fun onFailure(error: String?)
}

interface MotionCallback {
    fun onMotionDetected(room: String)
}

interface HistoryCallback {
    fun onSuccess(history: HistoryResponse?)
    fun onFailure(error: String?)
}

data class HistoryResponse(
    val temperature: List<Map<String, String>>?,
    val humidity: List<Map<String, String>>?,
    val lightness: List<Map<String, String>>?,
    val motion: List<Map<String, String>>?
)



data class RoomData(
    val humidity: String,
    val last_update: Double,
    val lightness: String,
    val motion: String,
    val temperature: String,
    val nodes: List<String>,
)


data class JsonResponse(val rooms: Map<String, RoomData>)

public class ApiClient {
    private val client = OkHttpClient()
    private val moshi = Moshi.Builder().add(KotlinJsonAdapterFactory()).build()
    private val jsonAdapter = moshi.adapter(JsonResponse::class.java)
    private val historyJsonAdapter = moshi.adapter(HistoryResponse::class.java)

    private lateinit var motionCallback: MotionCallback

    private lateinit var baseUrl: String

    private var jsonResponse = JsonResponse(emptyMap())

    fun setUrl(url: String) {
        baseUrl = url
        if (baseUrl[baseUrl.length - 1] != '/')
            baseUrl += "/"
    }

    fun getUrl() : String {
        return baseUrl
    }


    fun setMotionCallback(callback: MotionCallback) {
        motionCallback = callback
    }

    fun getJson() : JsonResponse {
        return jsonResponse
    }

    fun update() {
        val jsonRequest = Request.Builder()
            .url(baseUrl + "json")
            .build()


        client.newCall(jsonRequest).enqueue(object : okhttp3.Callback {
            override fun onFailure(call: okhttp3.Call, e: IOException) {
                Log.e("ApiClient", "Request failed: ${e.message}")
            }

            override fun onResponse(call: okhttp3.Call, response: okhttp3.Response) {
                response.use {
                    if (!response.isSuccessful) {
//                        lightnessCallback.onFailure("Unexpected response: ${response.code}")
                        Log.e("ApiClient", "Unexpected response: ${response.code}")
                        return
                    }

                    val responseBody = response.body?.string()

                    jsonResponse = responseBody?.let { it1 -> jsonAdapter.fromJson(it1) }!!


                    for ((room, roomData) in jsonResponse.rooms) {
                        if (roomData.motion == "1") {
                            motionCallback.onMotionDetected(room)
                        }
                    }

                }
            }
        })
    }

    fun fetchRoomHistory(roomName: String, historyCallback: HistoryCallback) {
        val historyUrl = baseUrl + "history/" + roomName
        val jsonRequest = Request.Builder()
            .url(historyUrl)
            .build()

        client.newCall(jsonRequest).enqueue(object : okhttp3.Callback {
            override fun onFailure(call: okhttp3.Call, e: IOException) {
                Log.e("ApiClient", "Request failed: ${e.message}")
                historyCallback.onFailure("Failed to fetch history: ${e.message}")
            }

            override fun onResponse(call: okhttp3.Call, response: okhttp3.Response) {
                response.use {
                    if (!response.isSuccessful) {
                        val errorMessage = "Unexpected response for history: ${response.code}"
                        Log.e("ApiClient", errorMessage)
                        historyCallback.onFailure(errorMessage)
                        return
                    }

                    val responseBody = response.body?.string()
                    if (responseBody.isNullOrEmpty()) {
                        val errorMessage = "Empty response body for history."
                        Log.e("ApiClient", errorMessage)
                        historyCallback.onFailure(errorMessage)
                        return
                    }

                    try {
                        // You'll need a Moshi adapter for RoomHistoryWrapper
                        // For example:
                        val moshi = Moshi.Builder().build()
                        val type = Types.newParameterizedType(Map::class.java, String::class.java, HistoryResponse::class.java)

                        val roomHistory = historyJsonAdapter.fromJson(responseBody)

                        if (roomHistory != null) {
                            historyCallback.onSuccess(roomHistory)
                        } else {
                            historyCallback.onFailure("History data for room '$roomName' not found in response.")
                        }

                    } catch (e: Exception) {
                        Log.e("ApiClient", "Error parsing history JSON: ${e.message}")
                        historyCallback.onFailure("Error parsing history data: ${e.message}")
                    }
                }
            }
        })
    }

}

var universalApiWrapper: ApiClient = ApiClient()