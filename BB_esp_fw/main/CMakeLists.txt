# Edit following two lines to set component requirements (see docs)
#set(COMPONENT_REQUIRES )
#set(COMPONENT_PRIV_REQUIRES )
#
#set(COMPONENT_SRCS "main.c")
#set(COMPONENT_ADD_INCLUDEDIRS "")
#
#register_component()

# Edit following two lines to set component requirements (see docs)

idf_component_register(SRCS
	"bluetooth/a2dp.c"
	"bluetooth/ble.c"
	"bluetooth/bluetooth.c"
	"bluetooth/spp.c"
	"drivers/i2c.c"
	"drivers/spi.c"
	"drivers/tas5720.c"
	"drivers/uart.c"
	"etc/stream.c"
	"pipeline/bluetooth.c"
	"pipeline/output.c"
	"pipeline/pipeline.c"
	"pipeline/sound.c"
	"pipeline/vario.c"
	"server/api.c"
	"server/file.c"
	"server/ob.c"
	"server/render.c"
	"server/server.c"
	"common.c"
	"debug.c"
	"download.c"
	"linked_list.c"
	"main.c"
	"protocol.c"
	"wifi.c"
	INCLUDE_DIRS "."
)

spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
#spiffs_create_partition_image(storage ../spiffs_image)