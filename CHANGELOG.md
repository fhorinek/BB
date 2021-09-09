# 09.09.2021 - Build 149

## Added
 * Bootloader wipe will preserve the calibration
 * Store configuration after exiting the settings menu #65
 * Vario bar now support fpm & knots units #77
 * Altitude 2 (QNH2) widget
 * G-meter widget
 * Easier way to enter Bootloader
 
## Fixed
 * retun to last page #82
 * QNH change trigger auto-takeoff #83
 * Correct menu position when escaping submenu #66
 * OTA update stability improved
 * MEMS i2c recovery

# 31.08.2021 - Build 148

## Added
 * Odometer
 
## Fixed
 * Arrow widgets crash 

# 29.08.2021 - Build 146

## Added
 * Widget Glide ratio
 * Height above take-off
 * Compass arrow, points
 * Ground heading arrow, points, number
 * Battery

## Changed
 * system i2c recovery (should also help with #64, #62, #60)

# 27.08.2021 - Build 145

## Changed
 * power off sequence handler (should fix #64, #62, #60)
 
## Fixed
 * "Back"-Button in Device settings/System/Time #61
 * Powering off with debug to serial disabled
 

# 11.08.2021 - Build 143

## Added
 * G-record for IGC
 * GNSS spoofing 
 
## Fixed
 * Fanet distance display
 * Bootloader will only change the drive name if it is not set


# 05.08.2021 - Build 140

## Changed
 * Development menu is hidden, click 5 times on serial number in device info to enable it #40
 * Vario bar widget will change offset to show lift above 3m/s
 
## Fixed
 * Audio Vario freezes #52, #15
 * Wifi menu #43, #25, #44

# 30.07.2021 - Build 139

## Added
 * Audio profile editor on device, connect directly to strato ip via web browser
 * Profile selection in vario settings
 * FANET will switch between flying and hiking based on flight status34 

## Fixed
 * Audio vario respose
 * bluetooth playback #28

# 23.07.2021 - Build 138

## Added
 * Abiliti to downgrade/change firmware via menu

## Fixes
 * Audio stops randomly during flight #22
 * QNH change causes device crash #30
 * Crash while in active flight and opening menu #23
 * Vario freeze on high volume #5
 * Vario sound "all the time rising" #15
 

# 20.07.2021 - Build 133

## Added
 * Automatic power-down when 3.1 V on battery is reached - this will prevent triggering battery protection
 
## Fixes
 * OGN Tracking #16
 
## Note
 * Bootloader update recomended

# 12.07.2021 - Build 132

## Added
 * Settings for units selection

## Added (under the hood)
 * First impementation of File managemer, not yet used
 * First impementation of Context menu, not yet used
 * Configuration entity callbacks
  
## Fixes
 * All strato devices have the same serial number #13
 * FW125, cant see stratos on Fanet machines #11
 * Pilot setting can handle only 1 letter #12, #14
 * Wrong date from GNSS sync #10
 * IGC logger records wrong date #8
 

# 12.07.2021 - Build 125

## Added

### Widgets
 * Altitude1
 * Average vario
 * Compass arrow
 * Digital vario
 * Flight time
 * GNSS altitude
 * Ground heading
 * Ground speed
 * Vario bar
 
### Functions
 * Vario, with configurable acclerometer gain
 * GNSS
 * Audio vario
 * FANET / FLARM beacon
 * IGC logger (no G-record yet)
 * OTA Updates
  
## Fixes
 * Back button is not working #6
 


