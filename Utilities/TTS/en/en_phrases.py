phrases = {
    "bt_conn" : "bluetooth connected",
    "bt_disc" : "bluetooth disconnected",
    "gnss_ok" : "satellite fix acquired",
    "gnss_lost" : "satellite fix lost",
    "takeoff" : "take off",
    "landed" : "landed",
    "bat_low" : "battery low",
}

command = "flite -voice en/cmu_us_clb.flitevox --setf duration_stretch=0.95 --setf int_f0_target_mean=160 -t \"%s\" %s.wav"
