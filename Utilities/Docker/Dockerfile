FROM ubuntu

ENV TOOLCHAIN_URL=https://dl.espressif.com/dl/xtensa-esp32-elf-gcc8_4_0-esp-2021r2-linux-amd64.tar.gz
ENV ESP_URL=https://strato.skybean.eu/dev/esp.zip

RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install --assume-yes \
    git wget flex bison gperf python3 python3-pip python3-setuptools \
    cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 \
    zip unzip && \
    apt-get clean

RUN mkdir -p /opt/Espressif && \
    cd /opt/Espressif && \
    wget --quiet $TOOLCHAIN_URL && \
    tar xzf $(basename $TOOLCHAIN_URL) && \
    rm $(basename $TOOLCHAIN_URL)

RUN cd /opt && \
    wget --quiet $ESP_URL && \
    unzip -q $(basename $ESP_URL) && \
    rm $(basename $ESP_URL)

ENV PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/opt/Espressif/xtensa-esp32-elf/bin
ENV IDF_PATH=/opt/esp/esp-idf
ENV ADF_PATH=/opt/esp/esp-adf

RUN ln -s /usr/bin/python3 /usr/bin/python && \
    pip install -r $IDF_PATH/requirements.txt
