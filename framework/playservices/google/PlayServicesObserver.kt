package org.akkord.lib

interface PlayServicesObserver {
    fun onPlayServicesConnected()
    fun onPlayServicesDisconnected()
    fun onSnapshotReceived(array: ByteArray?, size: Int)
}