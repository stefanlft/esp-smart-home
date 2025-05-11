package lft.stefan.espsmarthome

import android.util.Log
import com.squareup.moshi.Moshi
import com.squareup.moshi.kotlin.reflect.KotlinJsonAdapterFactory
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.Response
import okhttp3.sse.EventSource
import okhttp3.sse.EventSourceListener
import okhttp3.sse.EventSources
import java.io.IOException

interface ApiCallback {
    fun onSuccess(result: String?)
    fun onFailure(error: String?)
}

interface MotionCallback {
    fun onMotionDetected(room: String)
}

data class RoomData(
    val humidity: String,
    val last_update: Double,
    val lightness: String,
    val motion: String,
    val temperature: String,
    val nodes: List<String>
)

data class JsonResponse(val rooms: Map<String, RoomData>)


public class ApiClient {
    private val client = OkHttpClient()
    private val moshi = Moshi.Builder().add(KotlinJsonAdapterFactory()).build()
    private val jsonAdapter = moshi.adapter(JsonResponse::class.java)

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
}