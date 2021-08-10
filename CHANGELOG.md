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
 


