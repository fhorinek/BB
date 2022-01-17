# Enviroment tools setup

The Strato is powered by two microcontrolers of different kind.

 - **STM32H7A3** is used as main application processor. It is used to gather data from the 
 sensors and it power the user interface
 - **ESP32** is used as media and conectivity co-processor. It generates the sound and provide 
 bluetooth and wifi conectivity.
 
They are connected via UART, used for the primary conectivity and also SPI bus for transfering 
large data chunks. The main reason to use two interfaces is that with UART, both devices can start 
the transmittion independently, second reason is that the STM microctroller can access the ESP 
bootloader for the firmware update.

Thease specific microcontrollers are also used because they are well-known in opensource comunity 
and both platforms provide developer tools for free and with wide range of operating system in mind.

## Hardware

The development can be done on the standard Strato, hovewer the much better option is to use our
dev-kit avalible form our shop. It is essentialy the mainboard of the strato connected to the 
standard st-link and esp32-prog. This setup enables you to use the debbuger for the both 
microcontrollers and also give you access to the tespoint on the pcb.

## Get the code

 Clone the repositary from github 
 
 `git clone https://github.com/fhorinek/BB.git` 

## Setup for the STM32

 1. Download and install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html#get-software)
 2. Start the STM32CubeIDE, close the welcome screen and select **Import projects...**
 3. Select **General** / **Projects from Folder or Archive**
 4. Set cloned repository path as your **Import source** *(eg. ~/git/BB)*
 5. Select **BB3** and **BB3_loader**
 6. Try to build both projects
 7. Remember that the st-link will only connect when the device is **NOT** powered off.
 
## Setup for the ESP32

 1. Install Eclipse for ESP32 via **IDF-eclipse-plugin**
 2. [Install Prerequisites](https://github.com/espressif/idf-eclipse-plugin/blob/master/README.md#Prerequisites)
 3. [Install IDF Plugin](https://github.com/espressif/idf-eclipse-plugin/blob/master/README.md#installing-idf-plugin-using-update-site-url)
 4. [Install ESP-IDF](https://strato.skybean.eu/dev/esp.zip) copy this modified version this to ~/esp/
 5. [Install ESP-IDF Tools](https://github.com/espressif/idf-eclipse-plugin/blob/master/README.md#installing-esp-idf-tools)
 7. [Set path to ESP-ADF](https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html#step-3-set-up-path-to-esp-adf)
 8. Select **Import projects**. Select **Espressif** / **Existing IDF Project**
 9. Set **Existing project Location** to the ESP fw prooject path *(eg. ~/git/BB/BB_esp_fw)*
 10. Try to build the projects
 11. If the **ADF_PATH** is not definef in the CMake go to 
     **Window** / **Prefrences** / **C / C++** / **Build** / **Enviroment**
     Add Variabile **ADF_PATH** to **~/esp/esp-adf** and make sure that you select 
     **Replace native enviroment with speciefied one**

 
 
 
