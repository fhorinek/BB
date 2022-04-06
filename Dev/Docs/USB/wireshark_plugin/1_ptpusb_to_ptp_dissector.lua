local PTP_PACKETTYPE = {
	UNDEF = 0,
	CMD   = 1,
	DATA  = 2,
	ACK   = 3,
	EVENT = 4
}
local PTP_PACKETTYPENAMES = {
	[PTP_PACKETTYPE.UNDEF]="Undef",
	[PTP_PACKETTYPE.CMD]  ="Command",
	[PTP_PACKETTYPE.DATA] ="Data",
	[PTP_PACKETTYPE.ACK]  ="Ack",
	[PTP_PACKETTYPE.EVENT]="Event"
}

-- interface to the mtp post-dissector:
local PTPIP_PACKETTYPE = {

    INVALID                = 0,
    CMD_REQUEST            = 6,  -- possibly Operation request in [1] 2.3.6 agrees with [3]
    CMD_RESPONSE           = 7,  -- possibly Operation response in [1] 2.3 .7  agrees with [3]
    EVENT                  = 8,
    START_DATA_PACKET      = 9,
    DATA_PACKET            = 10,
    CANCEL_TRANSACTION     = 11,
    END_DATA_PACKET        = 12
} 

ptpusb_proto = Proto("ptpusb","PTP/USB")
local ptpusb = {
    packet_code        = ProtoField.uint16("ptp.pktCode","Code",base.HEX), -- one of opcode, reponsecode or eventcode
    transaction_id     = ProtoField.uint32("ptp.transactionID","Transaction ID",base.HEX),
	length             = ProtoField.uint32("ptp.length","PTP Packet Length"),
	-- internal info as interface fot mtp post-dissector:
	header_tid_offset  = ProtoField.uint32("ptp.headerTidOffset","Transaction ID Offset",base.DEC), -- necessary due to different packet layouts in different transport layers 
	packet_type        = ProtoField.uint32("ptp.pktType","Packet Type in PTPIP",base.HEX),
    data_length        = ProtoField.uint64("ptp.dataLen","Data Length",base.HEX), -- extracted from start data packet
    -- standalone usage:
	ptype  = ProtoField.uint16("ptpusb.ptype", "Type"),
	param  = ProtoField.uint32("ptpusb.param", "Param", base.HEX),
	resp   = ProtoField.uint32("ptpusb.resp",  "Response", base.HEX),
}
ptpusb_proto.fields = {
	ptpusb.param, 
	ptpusb.resp, 
	ptpusb.ptype,
	ptpusb.length,
	ptpusb.header_tid_offset,
	ptpusb.packet_type,
	ptpusb.transaction_id,
	ptpusb.data_length,
	ptpusb.packet_code
}

--usb_idvendor = Field.new("usb.idVendor")
usb_capdata  = Field.new("usb.capdata")
usb_iclass   = Field.new("usb.bInterfaceClass")
usb_source   = Field.new("usb.addr")

function ptpusb_proto.dissector(tvb,pinfo,tree)
	--local idvendor = usb_idvendor()
	local ptp_dissector_table = DissectorTable.get("ptp.data")
	local packet_dissector = ptp_dissector_table:get_dissector('ptp') 
	if not  packet_dissector then
		-- dprint('Can not find ptp dissector' )
	end
	
	local capdata = usb_capdata()
	local iclass  = usb_iclass()
	if capdata and iclass and (iclass.value == 255 or iclass.value == 6) then -- Imaging
		local size = capdata.len -- remaining captured data
		local start = tvb:len() - size 
		
		local ptp_tvb = tvb( start )
		
		local plen_tvb  = ptp_tvb(0,4)
		local ptype_tvb = ptp_tvb(4,2)
		local code_tvb  = ptp_tvb(6,2)
		local tid_tvb   = ptp_tvb(8,4)
		local plen  = plen_tvb:le_uint()
		local ptype = ptype_tvb:le_uint()
		
		local is_from_host = (usb_source().display == "host") 
		
		if size > 512 or (size == 512 and ptype ~= 2) then -- @bug this is buggy. bulk data are after a packet "plen > size" until ACK.
			local subtree = tree:add(ptpusb_proto, tvb(start, size), "PTP/USB: Bulk Data")
			subtree:add_le(ptpusb.packet_type,  PTPIP_PACKETTYPE.DATA_PACKET)
			return
		end
		
		optype = ""
		
		if ptype > 4 then
			optype = ptype .. " "
			ptype = 0
		end
		local subtree = tree:add(ptpusb_proto, ptp_tvb, "PTP/USB: " .. optype .. PTP_PACKETTYPENAMES[ptype])
		subtree:add_le(ptpusb.ptype, ptype_tvb)
		
		-- mtp postdissector:
		subtree:add(ptpusb.header_tid_offset, 8)
		subtree:add_le(ptpusb.length, plen_tvb)
		subtree:add_le(ptpusb.transaction_id,   tid_tvb)
		subtree:add_le(ptpusb.packet_code, code_tvb)
		if is_from_host then 
			local request_types = {
				[PTP_PACKETTYPE.CMD]  = PTPIP_PACKETTYPE.CMD_REQUEST,
				[PTP_PACKETTYPE.DATA] =	PTPIP_PACKETTYPE.START_DATA_PACKET,
				[PTP_PACKETTYPE.ACK]  = PTPIP_PACKETTYPE.INVALID,
				[PTP_PACKETTYPE.EVENT]= PTPIP_PACKETTYPE.INVALID
			}
			subtree:add_le(ptpusb.packet_type,   request_types[ptype] )
		else
			local response_types = {
				[PTP_PACKETTYPE.CMD]  = PTPIP_PACKETTYPE.INVALID,
				[PTP_PACKETTYPE.DATA] =	PTPIP_PACKETTYPE.START_DATA_PACKET,
				[PTP_PACKETTYPE.ACK]  = PTPIP_PACKETTYPE.CMD_RESPONSE,
				[PTP_PACKETTYPE.EVENT]= PTPIP_PACKETTYPE.EVENT
			}
			subtree:add_le(ptpusb.packet_type,   response_types[ptype] )
		end
					
		-- dump remaining parameters:
		local offset = 12
		local stop  = size
		if offset < stop then
			if ptype == PTP_PACKETTYPE.CMD then
				while (offset < stop) do
					subtree:add_le(ptpusb.param, ptp_tvb(offset,4))
					offset = offset+4
				end
			elseif ptype == PTP_PACKETTYPE.DATA then
				----@TODO parse after field
			elseif ptype == PTP_PACKETTYPE.ACK then
				subtree:add_le(ptpusb.resp, ptp_tvb(offset,4))
			elseif ptype == PTP_PACKETTYPE.EVENT then
				subtree:add_le(ptpusb.param, ptp_tvb(offset,4))
			end
		end
		
		if packet_dissector then
			local sub_tvb = tvb:range(start):tvb()
   			packet_dissector(sub_tvb,pinfo,tree)
   		end
	end
end

register_postdissector(ptpusb_proto)
