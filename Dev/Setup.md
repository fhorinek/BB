# Enviroment tools setup

The Strato is powered by two microcontrolers of different kind.

 - **STM32H7A3** is used as main application processor. It is used to gather data from the 
 sensors and it powers the user interface
 - **ESP32** is used as media and conectivity co-processor. It generates the sound and provides
 bluetooth and wifi conectivity.
 
They are connected via UART, used for the primary conectivity and also SPI bus for transfering 
large data chunks. The main reason to use two interfaces is that with UART, both devices can start 
the transmittion independently, second reason is that the STM microctroller can access the ESP 
bootloader for the firmware update.

These specific microcontrollers are also used because they are well-known in opensource community 
and both platforms provide developer tools for free and with wide range of operating system in mind.

## Hardware

The development can be done on the standard Strato, hovewer the much better option is to use our
dev-kit avalible form our shop. It is essentialy the mainboard of the strato connected to the 
standard st-link and esp32-prog. This setup enables you to use the debbuger for the both 
microcontrollers and also give you access to the tespoint on the pcb.

## Software

As strato uses two different microcontrollers, you need two different
IDEs to do the development.

The following documentation assumes, that the source code is stored in
~/git/BB. If you are using a different location, then change the
commands accordingly.

## Get the code

Clone the repositary from github 

    mkdir ~/git
    cd ~/git
    git clone https://github.com/fhorinek/BB.git 

### Setup for the STM32

You need STM32CubeIDE with some specific configuration. Please follow
this outline to install:

 1. Download and install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html#get-software), version 1.8.0 is known to work.
 2. Start the STM32CubeIDE, close the welcome screen and select **Import projects...**
 3. Select **General** / **Projects from Folder or Archive**
 4. Set cloned repository path as your **Import source** *(eg. ~/git/BB)*
 5. Select **BB3** and **BB3_loader**
 6. Right click on **BB3** and select "Build Configurations>Set Active>Release". Do the same for **BB3_loader**
 7. Try to build both projects
 8. Remember that the st-link will only connect when the device is **NOT** powered off.
 
### Setup for the ESP32

You need a standard Eclipse IDE with some plugins, especially
**IDF-eclipse-plugin**. They require two additional tools from
Espressif: ESP-IDF (Espressif IoT Development Framework) and ESP-ADF
(Espressif Audio Development Framework). The following documentation
assumes, that they are installed in ~/esp/esp-idf and
~/esp/esp-adf. If you are using a different location, then change the
commands accordingly.

Please follow this outline to install:

 1. [Install Prerequisites](https://github.com/espressif/idf-eclipse-plugin/blob/master/README.md#Prerequisites) as this is included in the following step "Install ESP-IDF/ADF"
 2. [Install IDF Plugin](https://github.com/espressif/idf-eclipse-plugin/blob/master/README.md#installing-idf-plugin-using-update-site-url)
 3. [Install ESP-IDF/ADF](https://strato.skybean.eu/dev/esp.zip) extract this modified version to ~/esp/ so that you get ~/esp/esp-idf and ~/esp/esp-adf.
 4. [Install ESP-IDF Tools](https://github.com/espressif/idf-eclipse-plugin/blob/master/README.md#installing-esp-idf-tools)
 5. [Set path to ESP-ADF](https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html#step-3-set-up-path-to-esp-adf)
 6. Select **Import projects**. Select **Espressif** / **Existing IDF Project**
 7. Set **Existing project Location** to the ESP fw prooject path *(eg. ~/git/BB/BB_esp_fw)*
 8. Try to build the projects
 9. If the **ADF_PATH** is not defined in the CMake go to 
     **Window** / **Preferences** / **C / C++** / **Build** / **Enviroment**
     Add Variabile **ADF_PATH** to `~/esp/esp-adf` and make sure that you select 
     **Replace native enviroment with speciefied one**
 
## Typical Development Roundtrip

Make changes to

  - "BB3" (main vario functionality executed by STM32) using STM32CubeIDE
  - "BB_esp_fw" (media/connectivity executed by ESP32) using Eclipse

Use "Build" on the project using the associated IDE to build the
changed parts.

Finally go to "Utilities/Bundle/" and execute `./pack_fw.py` which
will take the above two parts and merge them into "strato.fw".

Follow instructions in manual on copying this to strato and do the
final update.

# Typical development tasks

We decribe some typical tasks and how to do them.

## Add a widget

Create a new source file under `BB3/App/gui/widgets/types/` typically by
copying an existing file, like `widget_battery.c`.

Change values in REGISTER_WIDGET_IU especially the first value, which
is the variable name of the new widget. For this example, we change it
to "TimeDate". This implicitly means, that the callback functions for
this widget must start with "TimeDate", e.g. "TimeDate_init()".

Then add the new widget to `BB3/App/gui/widgets/widget_list.c` as a new
DECLARE_WIDGET and LIST_WIDGET at the end of the existings
declarations.

## Change release notes

Describe your changes in `BB3/Assets/release_note.txt` (displayed on
device after update) and `./CHANGELOG.md` (displayed in GitHub).

