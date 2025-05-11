package lft.stefan.espsmarthome

import android.Manifest
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.util.Log
import androidx.core.app.ActivityCompat
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat

class NotificationWrapper {
    fun createNotificationChannel(context: Context) {
        val channel = NotificationChannel(
            "my_channel_id",  // Unique ID for the channel
            "My Notifications",  // Name visible to the user
            NotificationManager.IMPORTANCE_HIGH // Importance level
        ).apply {
            description = "Channel for app notifications"
        }

        // Register the channel with the system
        val notificationManager: NotificationManager =
            context.getSystemService(NotificationManager::class.java)
        notificationManager.createNotificationChannel(channel)
    }

    fun sendNotification(context: Context, title: String, message: String) {
        val builder = NotificationCompat.Builder(context, "my_channel_id")
            .setSmallIcon(android.R.drawable.ic_dialog_info) // Notification icon
            .setContentTitle(title) // Notification title
            .setContentText(message) // Notification message
            .setPriority(NotificationCompat.PRIORITY_DEFAULT) // Priority
            .setAutoCancel(true) // Auto dismiss when tapped

        with(NotificationManagerCompat.from(context)) {
            if (ActivityCompat.checkSelfPermission(
                    context,
                    Manifest.permission.POST_NOTIFICATIONS
                ) != PackageManager.PERMISSION_GRANTED
            ) {
                // TODO: Consider calling
                //    ActivityCompat#requestPermissions
                // here to request the missing permissions, and then overriding
                //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                //                                          int[] grantResults)
                // to handle the case where the user grants the permission. See the documentation
                // for ActivityCompat#requestPermissions for more details.
                return
            }
            notify(1, builder.build()) // Unique ID for the notification
            Log.i("NOTIF", "Should notify")
        }
    }


}