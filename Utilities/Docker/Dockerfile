FROM ubuntu

ENV ESP_TOOLCHAIN_URL=https://strato.skybean.eu/dev/xtensa-esp32-elf-gcc8_4_0-esp-2021r2-linux-amd64.tar.gz
ENV ESP_URL=https://strato.skybean.eu/dev/esp.zip

ENV STM32CUBEIDE_URL=https://strato.skybean.eu/dev/en.st-stm32cubeide_1.9.0_12015_20220302_0855_amd64.deb_bundle.sh
ENV STM32CUBEIDE_VERSION=1.9.0

ENV DEBIAN_FRONTEND=noninteractive
ENV LICENSE_ALREADY_ACCEPTED=1
ENV PATH="${PATH}:/opt/Espressif/xtensa-esp32-elf/bin:/opt/st/stm32cubeide_${STM32CUBEIDE_VERSION}"
ENV IDF_PATH=/opt/esp/esp-idf
ENV ADF_PATH=/opt/esp/esp-adf

# Install general dependencies
RUN apt-get update && \
    apt-get install --assume-yes \
    git wget flex bison gperf python3 python3-pip python3-setuptools \
    cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 \
    zip unzip && \
    apt-get clean

# Install STM32 Cube IDE
RUN wget --quiet $STM32CUBEIDE_URL && \
    unzip -qp $(basename $STM32CUBEIDE_URL) > stm32cubeide-installer.sh && \
    chmod +x stm32cubeide-installer.sh && \
    ./stm32cubeide-installer.sh --quiet && \
    rm stm32cubeide-installer.sh && \
    rm $(basename $STM32CUBEIDE_URL)

# Install ESP32 compiler tools (Espressif)
RUN mkdir -p /opt/Espressif && \
    cd /opt/Espressif && \
    wget --quiet $ESP_TOOLCHAIN_URL && \
    tar xzf $(basename $ESP_TOOLCHAIN_URL) && \
    rm $(basename $ESP_TOOLCHAIN_URL)

# Install custom ESP dependencies
RUN cd /opt && \
    wget --quiet $ESP_URL && \
    unzip -q $(basename $ESP_URL) && \
    rm $(basename $ESP_URL)

RUN ln -s /usr/bin/python3 /usr/bin/python && \
    pip install -r $IDF_PATH/requirements.txt

