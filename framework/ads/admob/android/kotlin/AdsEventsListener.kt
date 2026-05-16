package org.akkord.lib

object AdsEventsListener {
    var onAdEvent: ((AdFormat, Int) -> Unit)? = null
}
