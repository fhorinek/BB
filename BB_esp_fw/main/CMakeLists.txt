# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 
	"bluetooth/bt_app_av.c"
	"bluetooth/bt_app_core.c"
	"bluetooth/bt.c"
	"drivers/i2c.c"
	"drivers/i2s.c"
	"drivers/spi.c"
	"drivers/tas5720.c"
	"drivers/uart.c"
	"etc/stream.c"
	"common.c"
	"debug.c"
	"main.c"
	"protocol.c"
	"wifi.c"
)

register_component()
