--[[
	PTP Protocol analyzer based on the original wireshark analyzer
	Adjusted by T. Engelmeier
]]

--[[
    References:
   [1] CIPA DC-X005-2005 - PTP-IP
   [2] BS ISO 15740:2008 - Photography Electronic still picture imaging - Picture transfer protocol (PTP)
   for digital still photography devices
   [3] gPhoto's Reversed Engineered PTP/IP documentation - http://gphoto.sourceforge.net/doc/ptpip.php
   [4] gPhoto's ptp2 header file  https://gphoto.svn.sourceforge.net/svnroot/gphoto/trunk/libgphoto2/camlibs/ptp2/ptp.h

   [5] ISO+15740-2008.pdf PTP 1.1 standard
   [6] etl_WPDMTP.events.npl

	To be able to handle various formats of PTP the main dissector has to pass a number of parsed data
	- session_id
	- transaction_id
	- code [opcode, responsecode or eventcode]
	- header_tid_offset [offset of transaction_id].
	-- While it would be more logical to pass e.g. the first byte after tid, here the tid is mapped


	this allows to handle packets which are structured (op = request, resp = response or event)


	op in PTP [5]     op in PTP/IP [1]    resp in PTP        resp in PTP/IP
	                  UINT32 len                              UINT32 len
	                  UINT32 packet_type                      UINT32 packet_type
	                  UINT32 dataphase
    UINT16 opcode     UINT16 opcode        UINT16 code        UINT16 code
    UINT32 sessionid                       UINT32 sessionid
    UINT32 tid        UINT32 tid           UINT32 tid         UINT32 tid

    Additionally, we handle PTP/IP start data and data packets.
    They are formally PTP/IP only but otherwise there is no notion of the data stream in PTP

--]]

--[[
	String Names of packet types [3] & [4]
	PTP/IP definitions - interface to the 'Parent' PTP dissector
--]]

PTPIP_PACKETTYPE = {
    INVALID                = 0,
    INIT_COMMAND_REQUEST   = 1,
    INIT_COMMAND_ACK       = 2,
    INIT_EVENT_REQUEST     = 3,
    INIT_EVENT_ACK         = 4,
    INIT_FAIL              = 5,
    CMD_REQUEST            = 6,  -- possibly Operation request in [1] 2.3.6 agrees with [3]
    CMD_RESPONSE           = 7,  -- possibly Operation response in [1] 2.3.7  agrees with [3]
    EVENT                  = 8,
    START_DATA_PACKET      = 9,
    DATA_PACKET            = 10,
    CANCEL_TRANSACTION     = 11,
    END_DATA_PACKET        = 12,
    PING                   = 13, -- possibly Probe Request in [1] 2.3.13
    PONG                   = 14  -- possibly Probe Response in [1] 2.3.14
}
-----------------------------------------------
-- main MTP definitions - extracted from [6]

-- Requests
    
local MTP_OPERATIONS = {
    UNDEFINED = 0x1000,
    GETDEVICEINFO = 0x1001,
    OPENSESSION = 0x1002,
    CLOSESESSION = 0x1003,
    GETSTORAGEIDS = 0x1004,
    GETSTORAGEINFO = 0x1005,
    GETNUMOBJECTS = 0x1006,
    GETOBJECTHANDLES = 0x1007,
    GETOBJECTINFO = 0x1008,
    GETOBJECT = 0x1009,
    GETTHUMB = 0x100A,
    DELETEOBJECT = 0x100B,
    SENDOBJECTINFO = 0x100C,
    SENDOBJECT = 0x100D,
    INITIATECAPTURE = 0x100E,
    FORMATSTORE = 0x100F,
    RESETDEVICE = 0x1010,
    SELFTEST = 0x1011,
    SETOBJECTPROTECTION = 0x1012,
    POWERDOWN = 0x1013,
    GETDEVICEPROPDESC = 0x1014,
    GETDEVICEPROPVALUE = 0x1015,
    SETDEVICEPROPVALUE = 0x1016,
    RESETDEVICEPROPVALUE = 0x1017,
    TERMINATECAPTURE = 0x1018,
    MOVEOBJECT = 0x1019,
    COPYOBJECT = 0x101A,
    GETPARTIALOBJECT = 0x101B,
    INITIATEOPENCAPTURE = 0x101C,
    RESERVED_FIRST = 0x1026,
    RESERVED_LAST = 0x1FFF,
    VENDOREXTENSION_FIRST = 0x9000,
    GETSERVICEIDS = 0x9301,
    GETSERVICEINFO = 0x9302,
    GETSERVICECAPABILITIES = 0x9303,
    GETSERVICEPROPERTIES = 0x9304,
    GETSERVICEPROPERTYLIST = 0x9305,
    SETSERVICEPROPERTYLIST = 0x9306,
    UPDATEOBJECTPROPLIST = 0x9307,
    DELETEOBJECTPROPLIST = 0x9308,
    DELETESERVICEPROPLIST = 0x9309,
    GETFORMATCAPABILITIES = 0x930A,
    VENDOREXTENSION_LAST = 0x97FF,
    GETOBJECTPROPSSUPPORTED = 0x9801,
    GETOBJECTPROPDESC = 0x9802,
    GETOBJECTPROPVALUE = 0x9803,
    SETOBJECTPROPVALUE = 0x9804,
    GETOBJECTPROPLIST = 0x9805,
    SETOBJECTPROPLIST = 0x9806,
    GETINTERDEPENDENTPROPDESC = 0x9807,
    SENDOBJECTPROPLIST = 0x9808,
    GETFORMATCAPABILITIES = 0x9809,
    UPDATEOBJECTPROPLIST = 0x980A,
    DELETEOBJECTPROPLIST = 0x980B,
    GETOBJECTREFERENCES = 0x9810,
    SETOBJECTREFERENCES = 0x9811,
    UPDATEDEVICEFIRMWARE = 0x9812,
    RESETOBJECTPROPVALUE = 0x9813,
    GETSERVICEIDS = 0x9900,
    GETSERVICEINFO = 0x9901,
    GETSERVICECAPABILITIES = 0x9902,
    GETSERVICEPROPERTIES = 0x9903,
    GETSERVICEPROPERTYLIST = 0x9904,
    SETSERVICEPROPERTYLIST = 0x9905,
    DELETESERVICEPROPLIST = 0x9906,
    OPENOBJECTSTREAM = 0x9910,
    READOBJECTSTREAM = 0x9911,
    WRITEOBJECTSTREAM = 0x9912,
    SEEKOBJECTSTREAM = 0x9913,
    CLOSEOBJECTSTREAM = 0x9914,
}

local MTP_OPERATION_DESCRIPTIONS = {
    [MTP_OPERATIONS.UNDEFINED] = 'Undefined',
    [MTP_OPERATIONS.GETDEVICEINFO] = 'GetDeviceInfo',
    [MTP_OPERATIONS.OPENSESSION] = 'OpenSession',
    [MTP_OPERATIONS.CLOSESESSION] = 'CloseSession',
    [MTP_OPERATIONS.GETSTORAGEIDS] = 'GetStorageIDs',
    [MTP_OPERATIONS.GETSTORAGEINFO] = 'GetStorageInfo',
    [MTP_OPERATIONS.GETNUMOBJECTS] = 'GetNumObjects',
    [MTP_OPERATIONS.GETOBJECTHANDLES] = 'GetObjectHandles',
    [MTP_OPERATIONS.GETOBJECTINFO] = 'GetObjectInfo',
    [MTP_OPERATIONS.GETOBJECT] = 'GetObject',
    [MTP_OPERATIONS.GETTHUMB] = 'GetThumb',
    [MTP_OPERATIONS.DELETEOBJECT] = 'DeleteObject',
    [MTP_OPERATIONS.SENDOBJECTINFO] = 'SendObjectInfo',
    [MTP_OPERATIONS.SENDOBJECT] = 'SendObject',
    [MTP_OPERATIONS.INITIATECAPTURE] = 'InitiateCapture',
    [MTP_OPERATIONS.FORMATSTORE] = 'FormatStore',
    [MTP_OPERATIONS.RESETDEVICE] = 'ResetDevice',
    [MTP_OPERATIONS.SELFTEST] = 'SelfTest',
    [MTP_OPERATIONS.SETOBJECTPROTECTION] = 'SetObjectProtection',
    [MTP_OPERATIONS.POWERDOWN] = 'PowerDown',
    [MTP_OPERATIONS.GETDEVICEPROPDESC] = 'GetDevicePropDesc',
    [MTP_OPERATIONS.GETDEVICEPROPVALUE] = 'GetDevicePropValue',
    [MTP_OPERATIONS.SETDEVICEPROPVALUE] = 'SetDevicePropValue',
    [MTP_OPERATIONS.RESETDEVICEPROPVALUE] = 'ResetDevicePropValue',
    [MTP_OPERATIONS.TERMINATECAPTURE] = 'TerminateOpenCapture',
    [MTP_OPERATIONS.MOVEOBJECT] = 'MoveObject',
    [MTP_OPERATIONS.COPYOBJECT] = 'CopyObject',
    [MTP_OPERATIONS.GETPARTIALOBJECT] = 'GetPartialObject',
    [MTP_OPERATIONS.INITIATEOPENCAPTURE] = 'InitiateOpenCapture',
    [MTP_OPERATIONS.RESERVED_FIRST] = 'RESERVED_FIRST',
    [MTP_OPERATIONS.RESERVED_LAST] = 'RESERVED_LAST',
    [MTP_OPERATIONS.VENDOREXTENSION_FIRST] = 'VENDOREXTENSION_FIRST',
    [MTP_OPERATIONS.GETSERVICEIDS] = 'GETSERVICEIDS',
    [MTP_OPERATIONS.GETSERVICEINFO] = 'GETSERVICEINFO',
    [MTP_OPERATIONS.GETSERVICECAPABILITIES] = 'GETSERVICECAPABILITIES',
    [MTP_OPERATIONS.GETSERVICEPROPERTIES] = 'GETSERVICEPROPERTIES',
    [MTP_OPERATIONS.GETSERVICEPROPERTYLIST] = 'GETSERVICEPROPERTYLIST',
    [MTP_OPERATIONS.SETSERVICEPROPERTYLIST] = 'SETSERVICEPROPERTYLIST',
    [MTP_OPERATIONS.UPDATEOBJECTPROPLIST] = 'UPDATEOBJECTPROPLIST',
    [MTP_OPERATIONS.DELETEOBJECTPROPLIST] = 'DELETEOBJECTPROPLIST',
    [MTP_OPERATIONS.DELETESERVICEPROPLIST] = 'DELETESERVICEPROPLIST',
    [MTP_OPERATIONS.GETFORMATCAPABILITIES] = 'GETFORMATCAPABILITIES',
    [MTP_OPERATIONS.VENDOREXTENSION_LAST] = 'VENDOREXTENSION_LAST',
    [MTP_OPERATIONS.GETOBJECTPROPSSUPPORTED] = 'GetObjectPropsSupported',
    [MTP_OPERATIONS.GETOBJECTPROPDESC] = 'GetObjectPropDesc',
    [MTP_OPERATIONS.GETOBJECTPROPVALUE] = 'GetObjectPropValue',
    [MTP_OPERATIONS.SETOBJECTPROPVALUE] = 'SetObjectPropValue',
    [MTP_OPERATIONS.GETOBJECTPROPLIST] = 'GetObjectPropList',
    [MTP_OPERATIONS.SETOBJECTPROPLIST] = 'SetObjectPropList',
    [MTP_OPERATIONS.GETINTERDEPENDENTPROPDESC] = 'GetInterDependentPropDesc',
    [MTP_OPERATIONS.SENDOBJECTPROPLIST] = 'SendObjectPropList',
    [MTP_OPERATIONS.GETFORMATCAPABILITIES] = 'GetFormatCapabilities',
    [MTP_OPERATIONS.UPDATEOBJECTPROPLIST] = 'UpdateObjectPropList',
    [MTP_OPERATIONS.DELETEOBJECTPROPLIST] = 'DeleteObjectPropList',
    [MTP_OPERATIONS.GETOBJECTREFERENCES] = 'GetObjectReferences',
    [MTP_OPERATIONS.SETOBJECTREFERENCES] = 'SetObjectReferences',
    [MTP_OPERATIONS.UPDATEDEVICEFIRMWARE] = 'UpdateDeviceFirmWare',
    [MTP_OPERATIONS.RESETOBJECTPROPVALUE] = 'ResetObjectPropValue',
    [MTP_OPERATIONS.GETSERVICEIDS] = 'GetServiceIDs',
    [MTP_OPERATIONS.GETSERVICEINFO] = 'GetServiceInfo',
    [MTP_OPERATIONS.GETSERVICECAPABILITIES] = 'GetServiceCapabilities',
    [MTP_OPERATIONS.GETSERVICEPROPERTIES] = 'GetServiceProperties',
    [MTP_OPERATIONS.GETSERVICEPROPERTYLIST] = 'GetServicePropList',
    [MTP_OPERATIONS.SETSERVICEPROPERTYLIST] = 'SetServicePropList',
    [MTP_OPERATIONS.DELETESERVICEPROPLIST] = 'DeleteServicePropList',
    [MTP_OPERATIONS.OPENOBJECTSTREAM] = 'OpenObjectStream',
    [MTP_OPERATIONS.READOBJECTSTREAM] = 'ReadObjectStream',
    [MTP_OPERATIONS.WRITEOBJECTSTREAM] = 'WriteObjectStream',
    [MTP_OPERATIONS.SEEKOBJECTSTREAM] = 'SeekObjectStream',
    [MTP_OPERATIONS.CLOSEOBJECTSTREAM] = 'CloseObjectStream',

}


-- Request parameter descriptions
    -- the table below has inconsistent string quotations to accomodate an ' occuring within the descriptions
local MTP_REQUEST_PARAMETERS = {
    [MTP_OPERATIONS.OPENSESSION] = {"SessionID"},
    [MTP_OPERATIONS.GETSTORAGEINFO] = {"StorageID"},
    [MTP_OPERATIONS.GETNUMOBJECTS] = {"StorageID","[ObjectFormatCode]","[ObjectHandle of Association for which number of children is needed]"},
    [MTP_OPERATIONS.GETOBJECTHANDLES] = {"StorageID","[ObjectFormatCode]","[ObjectHandle of Association or Hierarchical folder for which a list of children is needed]"},
    [MTP_OPERATIONS.GETOBJECTINFO] = {"ObjectHandle"},
    [MTP_OPERATIONS.GETOBJECT] = {"ObjectHandle"},
    [MTP_OPERATIONS.GETTHUMB] = {"ObjectHandle"},
    [MTP_OPERATIONS.DELETEOBJECT] = {"ObjectHandle","[ObjectFormatCode]"},
    [MTP_OPERATIONS.SENDOBJECTINFO] = {"[Destination StorageID on responder]","[Parent ObjectHandle on responder where object shall be placed]"},
    [MTP_OPERATIONS.INITIATECAPTURE] = {"[StorageID]","[ObjectFormatCode]"},
    [MTP_OPERATIONS.FORMATSTORE] = {"StorageID","[FileSystem Format]"},
    [MTP_OPERATIONS.SELFTEST] = {"[SelfTest Type]"},
    [MTP_OPERATIONS.SETOBJECTPROTECTION] = {"ObjectHandle","ProtectionStatus"},
    [MTP_OPERATIONS.GETDEVICEPROPDESC] = {"DevicePropCode"},
    [MTP_OPERATIONS.GETDEVICEPROPVALUE] = {"DevicePropCode"},
    [MTP_OPERATIONS.SETDEVICEPROPVALUE] = {"DevicePropCode"},
    [MTP_OPERATIONS.RESETDEVICEPROPVALUE] = {"DevicePropCode"},
    [MTP_OPERATIONS.TERMINATECAPTURE] = {"TransactionID"},
    [MTP_OPERATIONS.MOVEOBJECT] = {"ObjectHandle","StorageID of store to move object to","ObjectHandle of the new ParentObject"},
    [MTP_OPERATIONS.COPYOBJECT] = {"ObjectHandle","StorageID that the newly copied object shall be placed into","ObjectHandle of the newly copied object's parent"},
    [MTP_OPERATIONS.GETPARTIALOBJECT] = {"ObjectHandle","Offset in bytes","Maximum number of bytes to obtain"},
    [MTP_OPERATIONS.INITIATEOPENCAPTURE] = {"[StorageID]","[ObjectFormatCode]"},
    [MTP_OPERATIONS.GETSERVICEINFO] = {"ServiceID"},
    [MTP_OPERATIONS.GETSERVICECAPABILITIES] = {"ServiceID","[FormatCode]"},
    [MTP_OPERATIONS.GETSERVICEPROPERTIES] = {"ServiceID","[ServicePropCode]"},
    [MTP_OPERATIONS.GETSERVICEPROPERTYLIST] = {"ServiceID","[ServicePropCode]"},
    [MTP_OPERATIONS.SETSERVICEPROPERTYLIST] = {"ServiceID"},
    [MTP_OPERATIONS.UPDATEOBJECTPROPLIST] = {"ObjectHandle"},
    [MTP_OPERATIONS.DELETEOBJECTPROPLIST] = {"ObjectHandle"},
    [MTP_OPERATIONS.DELETESERVICEPROPLIST] = {"ServiceID"},
    [MTP_OPERATIONS.GETFORMATCAPABILITIES] = {"[FormatCode]"},
    [MTP_OPERATIONS.GETOBJECTPROPSSUPPORTED] = {"ObjectFormatCode"},
    [MTP_OPERATIONS.GETOBJECTPROPDESC] = {"ObjectPropCode","ObjectFormatCode"},
    [MTP_OPERATIONS.GETOBJECTPROPVALUE] = {"ObjectHandle","ObjectPropCode"},
    [MTP_OPERATIONS.SETOBJECTPROPVALUE] = {"ObjectHandle","ObjectPropCode"},
    [MTP_OPERATIONS.GETOBJECTPROPLIST] = {"ObjectHandle","[ObjectFormatCode]","ObjectPropCode","[ObjectPropGroupCode]","[Depth]"},
    [MTP_OPERATIONS.GETINTERDEPENDENTPROPDESC] = {"ObjectFormatCode"},
    [MTP_OPERATIONS.SENDOBJECTPROPLIST] = {"[Destination StorageID on responder]","[Parent ObjectHandle on responder where object shall be placed]","ObjectFormatCode","ObjectSize(most  significant 4 bytes)","ObjectSize(least significant 4 bytes)"},
    [MTP_OPERATIONS.GETFORMATCAPABILITIES] = {"[FormatCode]"},
    [MTP_OPERATIONS.UPDATEOBJECTPROPLIST] = {"ObjectHandle"},
    [MTP_OPERATIONS.DELETEOBJECTPROPLIST] = {"ObjectHandle"},
    [MTP_OPERATIONS.GETOBJECTREFERENCES] = {"ObjectHandle"},
    [MTP_OPERATIONS.SETOBJECTREFERENCES] = {"ObjectHandle"},
    [MTP_OPERATIONS.GETSERVICEINFO] = {"ServiceID"},
    [MTP_OPERATIONS.GETSERVICECAPABILITIES] = {"ServiceID","[FormatCode]"},
    [MTP_OPERATIONS.GETSERVICEPROPERTIES] = {"ServiceID","[ServicePropCode]"},
    [MTP_OPERATIONS.GETSERVICEPROPERTYLIST] = {"ServiceID","[ServicePropCode]"},
    [MTP_OPERATIONS.SETSERVICEPROPERTYLIST] = {"ServiceID"},
    [MTP_OPERATIONS.DELETESERVICEPROPLIST] = {"ServiceID"},
    [MTP_OPERATIONS.OPENOBJECTSTREAM] = {"ObjectHandle","Access","[MTU (Initiator)]","[Alternative Stream]"},
    [MTP_OPERATIONS.READOBJECTSTREAM] = {"StreamHandle","Units","Units to read"},
    [MTP_OPERATIONS.WRITEOBJECTSTREAM] = {"StreamHandle"},
    [MTP_OPERATIONS.SEEKOBJECTSTREAM] = {"StreamHandle","Offset type","Units of offset","Offset (most  significant 4 bytes)","Offset (least significant 4 bytes)"},
    [MTP_OPERATIONS.CLOSEOBJECTSTREAM] = {"StreamHandle"},
}


-- Response codes
    local MTP_RESPONSE_DESCRIPTIONS = {
    [0x2000] = 'UNDEFINED',
    [0x2001] = 'OK',
    [0x2002] = 'GENERALERROR',
    [0x2003] = 'SESSIONNOTOPEN',
    [0x2004] = 'INVALIDTRANSACTIONID',
    [0x2005] = 'OPERATIONNOTSUPPORTED',
    [0x2006] = 'PARAMETERNOTSUPPORTED',
    [0x2007] = 'INCOMPLETETRANSFER',
    [0x2008] = 'INVALIDSTORAGEID',
    [0x2009] = 'INVALIDOBJECTHANDLE',
    [0x200A] = 'DEVICEPROPERTYNOTSUPPORTED',
    [0x200B] = 'INVALIDOBJECTFORMATCODE',
    [0x200C] = 'STOREFULL',
    [0x200D] = 'OBJECTWRITEPROTECTED',
    [0x200E] = 'STOREWRITEPROTECTED',
    [0x200F] = 'ACCESSDENIED',
    [0x2010] = 'NOTHUMBNAILPRESENT',
    [0x2011] = 'SELFTESTFAILED',
    [0x2012] = 'PARTIALDELETION',
    [0x2013] = 'STORENOTAVAILABLE',
    [0x2014] = 'NOSPECIFICATIONBYFORMAT',
    [0x2015] = 'NOVALIDOBJECTINFO',
    [0x2016] = 'INVALIDCODEFORMAT',
    [0x2017] = 'UNKNOWNVENDORCODE',
    [0x2018] = 'CAPTUREALREADYTERMINATED',
    [0x2019] = 'DEVICEBUSY',
    [0x201A] = 'INVALIDPARENT',
    [0x201B] = 'INVALIDPROPFORMAT',
    [0x201C] = 'INVALIDPROPVALUE',
    [0x201D] = 'INVALIDPARAMETER',
    [0x201E] = 'SESSIONALREADYOPENED',
    [0x201F] = 'TRANSACTIONCANCELLED',
    [0x2020] = 'SPECIFICATIONOFDESTINATIONUNSUPPORTED',
    [0x2024] = 'RESERVED_FIRST',
    [0x2FFF] = 'RESERVED_LAST',
    [0xA000] = 'VENDOREXTENSION_FIRST',
    [0xA301] = 'INVALID_SERVICEID',
    [0xA302] = 'INVALID_SERVICEPROPCODE',
    [0xA7FF] = 'VENDOREXTENSION_LAST',
    [0xA801] = 'INVALIDOBJECTPROPCODE',
    [0xA802] = 'INVALIDOBJECTPROPFORMAT',
    [0xA803] = 'INVALIDOBJECTPROPVALUE',
    [0xA804] = 'INVALIDOBJECTREFERENCE',
    [0xA805] = 'INVALIDOBJECTGROUPCODE',
    [0xA806] = 'INVALIDDATASET',
    [0xA807] = 'SPECIFICATIONBYGROUPUNSUPPORTED',
    [0xA809] = 'OBJECTTOOLARGE',
    [0xA80A] = 'OBJECTPROPERTYNOTSUPPORTED',
    [0xA80B] = 'INVALID_SERVICEID',
    [0xA80C] = 'INVALID_SERVICEPROPCODE',
    [0xA80D] = 'Max_Streams_Reached',
    [0xA80E] = 'Max_Streams_Per_Object_Reached',
    [0xA80F] = 'SESSIONLIMITREACHED',
}


-- Response parameter descriptions
    -- the table below has inconsistent string quotations to accomodate an ' occuring within the descriptions
local MTP_RESPONSE_PARAMETERS = {
    [MTP_OPERATIONS.GETNUMOBJECTS] = {"NumObjects"},
    [MTP_OPERATIONS.SENDOBJECTINFO] = {"Responder StorageID in which the object will be stored","Responder parent ObjectHandle in which the object will be stored","Responder's reserved ObjectHandle for the incoming object"},
    [MTP_OPERATIONS.COPYOBJECT] = {"ObjectHandle of new copy of object"},
    [MTP_OPERATIONS.GETPARTIALOBJECT] = {"Actual number of bytes sent"},
    [MTP_OPERATIONS.UPDATEOBJECTPROPLIST] = {"[Index of failed property]"},
    [MTP_OPERATIONS.DELETEOBJECTPROPLIST] = {"[Index of failed property]"},
    [MTP_OPERATIONS.DELETESERVICEPROPLIST] = {"[Index of failed property]"},
    [MTP_OPERATIONS.SENDOBJECTPROPLIST] = {"Responder StorageID in which the object will be stored","Responder parent ObjectHandle in which the object will be stored","Responder's reserved ObjectHandle for the incoming object","[Index of failed property]"},
    [MTP_OPERATIONS.UPDATEOBJECTPROPLIST] = {"[Index of failed property]"},
    [MTP_OPERATIONS.DELETEOBJECTPROPLIST] = {"[Index of failed property]"},
    [MTP_OPERATIONS.DELETESERVICEPROPLIST] = {"[Index of failed property]"},
    [MTP_OPERATIONS.OPENOBJECTSTREAM] = {"StreamHandle","[MTU (Responder)]"},
    [MTP_OPERATIONS.READOBJECTSTREAM] = {"Units read"},
    [MTP_OPERATIONS.WRITEOBJECTSTREAM] = {"Bytes written"},
    [MTP_OPERATIONS.SEEKOBJECTSTREAM] = {"Absolute position (most  significant 4 bytes)","Absolute position (least significant 4 bytes)"},
}


-- Event names and types
    local MTP_EVENT_DESCRIPTIONS = {
    [0x4000] = 'UNDEFINED',
    [0x4001] = 'CANCELTRANSACTION',
    [0x4002] = 'OBJECTADDED',
    [0x4003] = 'OBJECTREMOVED',
    [0x4004] = 'STOREADDED',
    [0x4005] = 'STOREREMOVED',
    [0x4006] = 'DEVICEPROPCHANGED',
    [0x4007] = 'OBJECTINFOCHANGED',
    [0x4008] = 'DEVICEINFOCHANGED',
    [0x4009] = 'REQUESTOBJECTTRANSFER',
    [0x400A] = 'STOREFULL',
    [0x400B] = 'DEVICERESET',
    [0x400C] = 'STORAGEINFOCHANGED',
    [0x400D] = 'CAPTURECOMPLETE',
    [0x400E] = 'UNREPORTEDSTATUS',
    [0x400F] = 'RESERVED_FIRST',
    [0x4FFF] = 'RESERVED_LAST',
    [0xC000] = 'VENDOREXTENSION_FIRST',
    [0xC301] = 'SERVICEADDED',
    [0xC302] = 'SERVICEREMOVED',
    [0xC303] = 'SERVICEPROPCHANGED',
    [0xC304] = 'METHODCOMPLETE',
    [0xC7FF] = 'VENDOREXTENSION_LAST',
    [0xC801] = 'OBJECTPROPCHANGED',
    [0xC802] = 'OBJECTPROPDESCCHANGED',
    [0xC804] = 'SERVICEADDED',
    [0xC805] = 'SERVICEREMOVED',
    [0xC806] = 'SERVICEPROPCHANGED',
    [0xC807] = 'METHODCOMPLETE',
}


-- Device Properties
    local MTP_DEVICE_PROP_DESCRIPTIONS = {
    [0x0] = 'NOTUSED',
    [0x5000] = 'UNDEFINED',
    [0x5001] = 'BATTERYLEVEL',
    [0x5002] = 'FUNCTIONMODE',
    [0x5003] = 'IMAGESIZE',
    [0x5004] = 'COMPRESSIONSETTING',
    [0x5005] = 'WHITEBALANCE',
    [0x5006] = 'RGBGAIN',
    [0x5007] = 'FNUMBER',
    [0x5008] = 'FOCALLENGTH',
    [0x5009] = 'FOCUSDISTANCE',
    [0x500A] = 'FOCUSMODE',
    [0x500B] = 'EXPOSUREMETERINGMODE',
    [0x500C] = 'FLASHMODE',
    [0x500D] = 'EXPOSURETIME',
    [0x500E] = 'EXPOSUREPROGRAMMODE',
    [0x500F] = 'EXPOSUREINDEX',
    [0x5010] = 'EXPOSURECOMPENSATION',
    [0x5011] = 'DATETIME',
    [0x5012] = 'CAPTUREDELAY',
    [0x5013] = 'STILLCAPTUREMODE',
    [0x5014] = 'CONTRAST',
    [0x5015] = 'SHARPNESS',
    [0x5016] = 'DIGITALZOOM',
    [0x5017] = 'EFFECTMODE',
    [0x5018] = 'BURSTNUMBER',
    [0x5019] = 'BURSTINTERVAL',
    [0x501A] = 'TIMELAPSENUMBER',
    [0x501B] = 'TIMELAPSEINTERVAL',
    [0x501C] = 'FOCUSMETERINGMODE',
    [0x501D] = 'UPLOADURL',
    [0x501E] = 'ARTIST',
    [0x501F] = 'COPYRIGHTINFO',
    [0x5020] = 'RESERVED_FIRST',
    [0x5FFF] = 'RESERVED_LAST',
    [0xD000] = 'VENDOREXTENSION_FIRST',
    [0xD301] = 'FUNCTIONALID',
    [0xD302] = 'MODELUNIQUEID',
    [0xD303] = 'USEDEVICESTAGE',
    [0xD3FF] = 'VENDOREXTENSION_LAST',
    [0xD401] = 'SYNCHRONIZATIONPARTNER',
    [0xD402] = 'DEVICEFRIENDLYNAME',
    [0xD403] = 'VOLUME',
    [0xD404] = 'CONSUMPTIONFORMATPREFERENCES',
    [0xD405] = 'DEVICEICON',
    [0xD406] = 'SESSIONINITIATORVERSIONINFO',
    [0xD407] = 'PERCEIVEDDEVICETYPE',
    [0xD408] = 'FUNCTIONALID',
    [0xD410] = 'PLAYBACKRATE',
    [0xD411] = 'PLAYBACKOBJECT',
    [0xD412] = 'PLAYBACKCONTAINER',
    [0xD413] = 'PLAYBACKPOSITION',
    [0xFFFFFFFF] = 'ALL',
}


-- Object Properties
    local MTP_OBJECT_PROP_DESCRIPTIONS = {
    [0x0] = 'NOTUSED',
    [0xD000] = 'UNDEFINED',
    [0xD800] = 'VENDOREXTENSION_FIRST',
    [0xDBFF] = 'VENDOREXTENSION_LAST',
    [0xDC01] = 'STORAGEID',
    [0xDC02] = 'OBJECTFORMAT',
    [0xDC03] = 'PROTECTIONSTATUS',
    [0xDC04] = 'OBJECTSIZE',
    [0xDC05] = 'ASSOCIATIONTYPE',
    [0xDC06] = 'ASSOCIATIONDESC',
    [0xDC07] = 'OBJECTFILENAME',
    [0xDC08] = 'DATECREATED',
    [0xDC09] = 'DATEMODIFIED',
    [0xDC0A] = 'KEYWORDS',
    [0xDC0B] = 'PARENT',
    [0xDC0C] = 'ALLOWEDFOLDERCONTENTS',
    [0xDC0D] = 'HIDDEN',
    [0xDC0E] = 'SYSTEMOBJECT',
    [0xDC41] = 'PERSISTENTUNIQUEOBJECTIDENTIFIER',
    [0xDC42] = 'SYNCID',
    [0xDC43] = 'PROPERTYBAG',
    [0xDC44] = 'NAME',
    [0xDC45] = 'CREATEDBY',
    [0xDC46] = 'ARTIST',
    [0xDC47] = 'DATEAUTHORED',
    [0xDC48] = 'DESCRIPTION',
    [0xDC49] = 'URLREFERENCE',
    [0xDC4A] = 'LANGUAGELOCALE',
    [0xDC4B] = 'COPYRIGHTINFORMATION',
    [0xDC4C] = 'SOURCE',
    [0xDC4D] = 'ORIGINLOCATION',
    [0xDC4E] = 'DATEADDED',
    [0xDC4F] = 'NONCONSUMABLE',
    [0xDC50] = 'CORRUPTUNPLAYABLE',
    [0xDC51] = 'PRODUCERSERIALNUMBER',
    [0xDC81] = 'REPRESENTATIVESAMPLEFORMAT',
    [0xDC82] = 'REPRESENTATIVESAMPLESIZE',
    [0xDC83] = 'REPRESENTATIVESAMPLEHEIGHT',
    [0xDC84] = 'REPRESENTATIVESAMPLEWIDTH',
    [0xDC85] = 'REPRESENTATIVESAMPLEDURATION',
    [0xDC86] = 'REPRESENTATIVESAMPLEDATA',
    [0xDC87] = 'WIDTH',
    [0xDC88] = 'HEIGHT',
    [0xDC89] = 'DURATION',
    [0xDC8A] = 'USERRATING',
    [0xDC8B] = 'TRACK',
    [0xDC8C] = 'GENRE',
    [0xDC8D] = 'CREDITS',
    [0xDC8E] = 'LYRICS',
    [0xDC8F] = 'SUBSCRIPTIONCONTENTID',
    [0xDC90] = 'PRODUCEDBY',
    [0xDC91] = 'USECOUNT',
    [0xDC92] = 'SKIPCOUNT',
    [0xDC93] = 'LASTACCESSED',
    [0xDC94] = 'PARENTALRATING',
    [0xDC95] = 'METAGENRE',
    [0xDC96] = 'COMPOSER',
    [0xDC97] = 'EFFECTIVERATING',
    [0xDC98] = 'SUBTITLE',
    [0xDC99] = 'ORIGINALRELEASEDATE',
    [0xDC9A] = 'ALBUMNAME',
    [0xDC9B] = 'ALBUMARTIST',
    [0xDC9C] = 'MOOD',
    [0xDC9D] = 'DRMPROTECTION',
    [0xDC9E] = 'SUBDESCRIPTION',
    [0xDCD1] = 'ISCROPPED',
    [0xDCD2] = 'ISCOLOURCORRECTED',
    [0xDCD3] = 'IMAGEBITDEPTH',
    [0xDCD4] = 'FNUMBER',
    [0xDCD5] = 'EXPOSURETIME',
    [0xDCD6] = 'EXPOSUREINDEX',
    [0xDCE0] = 'DISPLAYNAME',
    [0xDCE1] = 'BODYTEXT',
    [0xDCE2] = 'SUBJECT',
    [0xDCE3] = 'PRIORITY',
    [0xDD00] = 'GIVENNAME',
    [0xDD01] = 'MIDDLENAMES',
    [0xDD02] = 'FAMILYNAME',
    [0xDD03] = 'PREFIX',
    [0xDD04] = 'SUFFIX',
    [0xDD05] = 'PHONETICGIVENNAME',
    [0xDD06] = 'PHONETICFAMILYNAME',
    [0xDD07] = 'EMAILPRIMARY',
    [0xDD08] = 'EMAILPERSONAL1',
    [0xDD09] = 'EMAILPERSONAL2',
    [0xDD0A] = 'EMAILBUSINESS1',
    [0xDD0B] = 'EMAILBUSINESS2',
    [0xDD0C] = 'EMAILOTHERS',
    [0xDD0D] = 'PHONENUMBERPRIMARY',
    [0xDD0E] = 'PHONENUMBERPERSONAL',
    [0xDD0F] = 'PHONENUMBERPERSONAL2',
    [0xDD10] = 'PHONENUMBERBUSINESS',
    [0xDD11] = 'PHONENUMBERBUSINESS2',
    [0xDD12] = 'PHONENUMBERMOBIL',
    [0xDD13] = 'PHONENUMBERMOBIL2',
    [0xDD14] = 'FAXNUMBERPRIMARY',
    [0xDD15] = 'FAXNUMBERPERSONAL',
    [0xDD16] = 'FAXNUMBERBUSINESS',
    [0xDD17] = 'PAGERNUMBER',
    [0xDD18] = 'PHONENUMBEROTHERS',
    [0xDD19] = 'PRIMARYWEBADDRESS',
    [0xDD1A] = 'PERSONALWEBADDRESS',
    [0xDD1B] = 'BUSINESSWEBADDRESS',
    [0xDD1C] = 'INSTANCEMESSENGERADDRESS',
    [0xDD1D] = 'INSTANCEMESSENGERADDRESS2',
    [0xDD1E] = 'INSTANCEMESSENGERADDRESS3',
    [0xDD1F] = 'POSTALADDRESSPERSONALFULL',
    [0xDD20] = 'POSTALADDRESSPERSONALLINE1',
    [0xDD21] = 'POSTALADDRESSPERSONALLINE2',
    [0xDD22] = 'POSTALADDRESSPERSONALCITY',
    [0xDD23] = 'POSTALADDRESSPERSONALREGION',
    [0xDD24] = 'POSTALADDRESSPERSONALPOSTALCODE',
    [0xDD25] = 'POSTALADDRESSPERSONALCOUNTRY',
    [0xDD26] = 'POSTALADDRESSBUSINESSFULL',
    [0xDD27] = 'POSTALADDRESSBUSINESSLINE1',
    [0xDD28] = 'POSTALADDRESSBUSINESSLINE2',
    [0xDD29] = 'POSTALADDRESSBUSINESSCITY',
    [0xDD2A] = 'POSTALADDRESSBUSINESSREGION',
    [0xDD2B] = 'POSTALADDRESSBUSINESSPOSTALCODE',
    [0xDD2C] = 'POSTALADDRESSBUSINESSCOUNTRY',
    [0xDD2D] = 'POSTALADDRESSOTHERFULL',
    [0xDD2E] = 'POSTALADDRESSOTHERLINE1',
    [0xDD2F] = 'POSTALADDRESSOTHERLINE2',
    [0xDD30] = 'POSTALADDRESSOTHERCITY',
    [0xDD31] = 'POSTALADDRESSOTHERREGION',
    [0xDD32] = 'POSTALADDRESSOTHERPOSTALCODE',
    [0xDD33] = 'POSTALADDRESSOTHERCOUNTRY',
    [0xDD34] = 'ORGANIZATIONNAME',
    [0xDD35] = 'PHONETICORGANIZATIONNAME',
    [0xDD36] = 'ROLE',
    [0xDD37] = 'BIRTHDAY',
    [0xDD40] = 'MESSAGETO',
    [0xDD41] = 'MESSAGECC',
    [0xDD42] = 'MESSAGEBCC',
    [0xDD43] = 'MESSAGEREAD',
    [0xDD44] = 'MESSAGERECEIVETIME',
    [0xDD45] = 'MESSAGESENDER',
    [0xDD50] = 'ACTIVITYBEGINTIME',
    [0xDD51] = 'ACTIVITYENDTIME',
    [0xDD52] = 'ACTIVITYLOCATION',
    [0xDD54] = 'ACTIVITYREQUIREDATTENDEES',
    [0xDD55] = 'ACTIVITYOPTIONALATTENDEES',
    [0xDD56] = 'ACTIVITYRESOURCES',
    [0xDD57] = 'ACTIVITYACCEPTEDED',
    [0xDD58] = 'ACTIVITYTENTATIVE',
    [0xDD59] = 'ACTIVITYDECLINED',
    [0xDD5A] = 'ACTIVITYREMINDERTIME',
    [0xDD5B] = 'ACTIVITYOWNER',
    [0xDD5C] = 'ACTIVITYSTATUS',
    [0xDD5D] = 'OWNER',
    [0xDD5E] = 'EDITOR',
    [0xDD5F] = 'WEBMASTER',
    [0xDD60] = 'URLSOURCE',
    [0xDD61] = 'URLDESTINATION',
    [0xDD62] = 'TIME_BOOKMARK',
    [0xDD63] = 'OBJECT_BOOKMARK',
    [0xDD64] = 'BYTE_BOOKMARK',
    [0xDD65] = 'DATA_OFFSET',
    [0xDD66] = 'DATA_LENGTH',
    [0xDD67] = 'DATA_UNITS',
    [0xDD68] = 'DATA_REFERENCED_OBJECT_RESOURCE',
    [0xDD69] = 'BACK_REFERENCES',
    [0xDD70] = 'LASTBUILDDATE',
    [0xDD71] = 'TIMETOLIVE',
    [0xDD72] = 'MEDIAGUID',
    [0xDE91] = 'TOTALBITRATE',
    [0xDE92] = 'BITRATETYPE',
    [0xDE93] = 'SAMPLERATE',
    [0xDE94] = 'NUMBEROFCHANNELS',
    [0xDE95] = 'AUDIOBITDEPTH',
    [0xDE96] = 'BLOCKALIGNMENT',
    [0xDE97] = 'SCANTYPE',
    [0xDE98] = 'COLOURRANGE',
    [0xDE99] = 'AUDIOWAVECODEC',
    [0xDE9A] = 'AUDIOBITRATE',
    [0xDE9B] = 'VIDEOFOURCCCODEC',
    [0xDE9C] = 'VIDEOBITRATE',
    [0xDE9D] = 'FRAMESPERMILLISECOND',
    [0xDE9E] = 'KEYFRAMEDISTANCE',
    [0xDE9F] = 'BUFFERSIZE',
    [0xDEA0] = 'ENCODINGQUALITY',
    [0xDEA1] = 'ENCODINGPROFILE',
    [0xDEA2] = 'AUDIOENCODINGPROFILE',
    [0xFFFFFFFF] = 'ALL',
}


-- various descriptions
    local MTP_FORM_FLAG_DESCRIPTIONS = {
    [0] = 'NONE',
    [0x81] = 'SVCEXT_OBJECTPROP',
    [0x82] = 'SVCEXT_METHODPARAM',
    [0x83] = 'SVCEXT_OBJECTID',
    [0xff] = 'LONGSTRING',
    [1] = 'RANGE',
    [2] = 'ENUM',
    [3] = 'DATETIME',
    [4] = 'FIXEDARRAY',
    [5] = 'REGEX',
    [6] = 'BYTEARRAY',
    [7] = 'OBJECTPROP',
    [8] = 'METHODPARAM',
    [9] = 'OBJECTID',
}

    local MTP_FORMAT_DESCRIPTIONS = {
    [0x0000] = 'NOTUSED',
    [0x3000] = 'UNDEFINED',
    [0x3001] = 'ASSOCIATION',
    [0x3002] = 'SCRIPT',
    [0x3003] = 'EXECUTABLE',
    [0x3004] = 'TEXT',
    [0x3005] = 'HTML',
    [0x3006] = 'DPOF',
    [0x3007] = 'AIFF',
    [0x3008] = 'WAVE',
    [0x3009] = 'MP3',
    [0x300A] = 'AVI',
    [0x300B] = 'MPEG',
    [0x300C] = 'ASF',
    [0x300D] = 'RESERVED_FIRST',
    [0x37FF] = 'RESERVED_LAST',
    [0x3800] = 'UNDEFINEDIMAGE',
    [0x3801] = 'EXIF/JPEG',
    [0x3802] = 'TIFF/EP',
    [0x3803] = 'FLASHPIX',
    [0x3804] = 'BMP',
    [0x3805] = 'CIFF',
    [0x3807] = 'GIF',
    [0x3808] = 'JFIF',
    [0x3809] = 'PCD',
    [0x380A] = 'PICT',
    [0x380B] = 'PNG',
    [0x380D] = 'TIFF',
    [0x380E] = 'TIFF/IT',
    [0x380F] = 'JP2',
    [0x3810] = 'JPX',
    [0x3811] = 'IMAGE_RESERVED_FIRST',
    [0x3FFF] = 'IMAGE_RESERVED_LAST',
    [0xB000] = 'VENDOREXTENSION_FIRST',
    [0xB7FF] = 'VENDOREXTENSION_LAST',
    [0xB802] = 'UNDEFINEDFIRMWARE',
    [0xB803] = 'WBMP',
    [0xB804] = 'JPEGXR',
    [0xB881] = 'WINDOWSIMAGEFORMAT',
    [0xB900] = 'UNDEFINEDAUDIO',
    [0xB901] = 'WMA',
    [0xB902] = 'OGG',
    [0xB903] = 'AAC',
    [0xB904] = 'AUDIBLE',
    [0xB906] = 'FLAC',
    [0xB907] = 'QCELP',
    [0xB908] = 'AMR',
    [0xB980] = 'UNDEFINEDVIDEO',
    [0xB981] = 'WMV',
    [0xB982] = 'MP4',
    [0xB983] = 'MP2',
    [0xB984] = '3GP',
    [0xB985] = '3G2',
    [0xB986] = 'AVCHD',
    [0xB987] = 'ATSCTS',
    [0xB988] = 'DVBTS',
    [0xBA00] = 'UNDEFINEDCOLLECTION',
    [0xBA01] = 'ABSTRACTMULTIMEDIAALBUM',
    [0xBA02] = 'ABSTRACTIMAGEALBUM',
    [0xBA03] = 'ABSTRACTAUDIOALBUM',
    [0xBA04] = 'ABSTRACTVIDEOALBUM',
    [0xBA05] = 'ABSTRACTAUDIOVIDEOPLAYLIST',
    [0xBA06] = 'ABSTRACTCONTACTGROUP',
    [0xBA07] = 'ABSTRACTMESSAGEFOLDER',
    [0xBA08] = 'ABSTRACTCHAPTEREDPRODUCTION',
    [0xBA0B] = 'ABSTRACTMEDIACAST',
    [0xBA10] = 'WPLPLAYLIST',
    [0xBA11] = 'M3UPLAYLIST',
    [0xBA12] = 'MPLPLAYLIST',
    [0xBA13] = 'ASXPLAYLIST',
    [0xBA14] = 'PLSPLAYLIST',
    [0xBA80] = 'UNDEFINEDDOCUMENT',
    [0xBA81] = 'ABSTRACTDOCUMENT',
    [0xBA82] = 'XMLDOCUMENT',
    [0xBA83] = 'MICROSOFTWORDDOCUMENT',
    [0xBA84] = 'MHTCOMPILEDHTMLDOCUMENT',
    [0xBA85] = 'MICROSOFTEXCELSPREADSHEET',
    [0xBA86] = 'MICROSOFTPOWERPOINTDOCUMENT',
    [0xBB00] = 'UNDEFINEDMESSAGE',
    [0xBB01] = 'ABSTRACTMESSAGE',
    [0xBB80] = 'UNDEFINEDCONTACT',
    [0xBB81] = 'ABSTRACTCONTACT',
    [0xBB82] = 'VCARD2',
    [0xBB83] = 'VCARD3',
    [0xBE00] = 'UNDEFINEDCALENDARITEM',
    [0xBE01] = 'ABSTRACTCALENDARITEM',
    [0xBE02] = 'VCALENDAR1',
    [0xBE03] = 'VCALENDAR2',
    [0xBE80] = 'UNDEFINEDWINDOWSEXECUTABLE',
    [0xFFFFFFFF] = 'ALLIMAGES',
}


local mtp_lookup = {
    -- ['OperationCodes'] = MTP_OPERATIONS,
    ['OperationCodeDescriptions'] = MTP_OPERATION_DESCRIPTIONS,
    ['ResponseCodeDescriptions'] = MTP_RESPONSE_DESCRIPTIONS,
    ['RequestParameters'] = MTP_REQUEST_PARAMETERS,
    ['ResponseParameters'] = MTP_RESPONSE_PARAMETERS,
    ['EventCodeDescriptions'] = MTP_EVENT_DESCRIPTIONS,
    ['DevicePropCode'] = MTP_DEVICE_PROP_DESCRIPTIONS,
    ['ObjectPropCode'] = MTP_OBJECT_PROP_DESCRIPTIONS,
    ['FormFlag'] = MTP_FORM_FLAG_DESCRIPTIONS,
    ['ObjectFormatCode'] = MTP_FORMAT_DESCRIPTIONS,
}

--------------------------------------------------
-- vendor specific ptp extensions. They are potentially based on JSON extracted from [4]

        
local ANDROID_EXTENSIONS = {
    ['OperationCodeDescriptions'] = {
        [0x95C1] = 'GetPartialObject64',
        [0x95C2] = 'SendPartialObject',
        [0x95C3] = 'TruncateObject',
        [0x95C4] = 'BeginEditObject',
        [0x95C5] = 'EndEditObject',
    },

}

    
local CANON_EXTENSIONS = {
    ['DevicePropCode'] = {
        [0xD001] = 'BeepMode',
        [0xD002] = 'BatteryKind',
        [0xD003] = 'BatteryStatus',
        [0xD004] = 'UILockType',
        [0xD005] = 'CameraMode',
        [0xD006] = 'ImageQuality',
        [0xD007] = 'FullViewFileFormat',
        [0xD008] = 'ImageSize',
        [0xD009] = 'SelfTime',
        [0xD00A] = 'FlashMode',
        [0xD00B] = 'Beep',
        [0xD00C] = 'ShootingMode',
        [0xD00D] = 'ImageMode',
        [0xD00E] = 'DriveMode',
        [0xD00F] = 'EZoom',
        [0xD010] = 'MeteringMode',
        [0xD011] = 'AFDistance',
        [0xD012] = 'FocusingPoint',
        [0xD013] = 'WhiteBalance',
        [0xD014] = 'SlowShutterSetting',
        [0xD015] = 'AFMode',
        [0xD016] = 'ImageStabilization',
        [0xD017] = 'Contrast',
        [0xD018] = 'ColorGain',
        [0xD019] = 'Sharpness',
        [0xD01A] = 'Sensitivity',
        [0xD01B] = 'ParameterSet',
        [0xD01C] = 'ISOSpeed',
        [0xD01D] = 'Aperture',
        [0xD01E] = 'ShutterSpeed',
        [0xD01F] = 'ExpCompensation',
        [0xD020] = 'FlashCompensation',
        [0xD021] = 'AEBExposureCompensation',
        [0xD023] = 'AvOpen',
        [0xD024] = 'AvMax',
        [0xD025] = 'FocalLength',
        [0xD026] = 'FocalLengthTele',
        [0xD027] = 'FocalLengthWide',
        [0xD028] = 'FocalLengthDenominator',
        [0xD029] = 'CaptureTransferMode',
        [0xD02A] = 'Zoom',
        [0xD02B] = 'NamePrefix',
        [0xD02C] = 'SizeQualityMode',
        [0xD02D] = 'SupportedThumbSize',
        [0xD02E] = 'SizeOfOutputDataFromCamera',
        [0xD02F] = 'SizeOfInputDataToCamera',
        [0xD030] = 'RemoteAPIVersion',
        [0xD031] = 'FirmwareVersion',
        [0xD032] = 'CameraModel',
        [0xD033] = 'CameraOwner',
        [0xD034] = 'UnixTime',
        [0xD035] = 'CameraBodyID',
        [0xD036] = 'CameraOutput',
        [0xD037] = 'DispAv',
        [0xD038] = 'AvOpenApex',
        [0xD039] = 'DZoomMagnification',
        [0xD03A] = 'MlSpotPos',
        [0xD03B] = 'DispAvMax',
        [0xD03C] = 'AvMaxApex',
        [0xD03D] = 'EZoomStartPosition',
        [0xD03E] = 'FocalLengthOfTele',
        [0xD03F] = 'EZoomSizeOfTele',
        [0xD040] = 'PhotoEffect',
        [0xD041] = 'AssistLight',
        [0xD042] = 'FlashQuantityCount',
        [0xD043] = 'RotationAngle',
        [0xD044] = 'RotationScene',
        [0xD045] = 'EventEmulateMode',
        [0xD046] = 'DPOFVersion',
        [0xD047] = 'TypeOfSupportedSlideShow',
        [0xD048] = 'AverageFilesizes',
        [0xD049] = 'ModelID',
        [0xD101] = 'EOS_Aperture',
        [0xD102] = 'EOS_ShutterSpeed',
        [0xD103] = 'EOS_ISOSpeed',
        [0xD104] = 'EOS_ExpCompensation',
        [0xD105] = 'EOS_AutoExposureMode',
        [0xD106] = 'EOS_DriveMode',
        [0xD107] = 'EOS_MeteringMode',
        [0xD108] = 'EOS_FocusMode',
        [0xD109] = 'EOS_WhiteBalance',
        [0xD10A] = 'EOS_ColorTemperature',
        [0xD10B] = 'EOS_WhiteBalanceAdjustA',
        [0xD10C] = 'EOS_WhiteBalanceAdjustB',
        [0xD10D] = 'EOS_WhiteBalanceXA',
        [0xD10E] = 'EOS_WhiteBalanceXB',
        [0xD10F] = 'EOS_ColorSpace',
        [0xD110] = 'EOS_PictureStyle',
        [0xD111] = 'EOS_BatteryPower',
        [0xD112] = 'EOS_BatterySelect',
        [0xD113] = 'EOS_CameraTime',
        [0xD114] = 'EOS_AutoPowerOff',
        [0xD115] = 'EOS_Owner',
        [0xD116] = 'EOS_ModelID',
        [0xD119] = 'EOS_PTPExtensionVersion',
        [0xD11A] = 'EOS_DPOFVersion',
        [0xD11B] = 'EOS_AvailableShots',
        [0xD11C] = 'EOS_CaptureDestination',
        [0xD11D] = 'EOS_BracketMode',
        [0xD11E] = 'EOS_CurrentStorage',
        [0xD11F] = 'EOS_CurrentFolder',
        [0xD130] = 'EOS_CompressionS',
        [0xD131] = 'EOS_CompressionM1',
        [0xD132] = 'EOS_CompressionM2',
        [0xD133] = 'EOS_CompressionL',
        [0xD138] = 'EOS_AEModeDial',
        [0xD139] = 'EOS_AEModeCustom',
        [0xD13A] = 'EOS_MirrorUpSetting',
        [0xD13B] = 'EOS_HighlightTonePriority',
        [0xD13C] = 'EOS_AFSelectFocusArea',
        [0xD13D] = 'EOS_HDRSetting',
        [0xD140] = 'EOS_PCWhiteBalance1',
        [0xD141] = 'EOS_PCWhiteBalance2',
        [0xD142] = 'EOS_PCWhiteBalance3',
        [0xD143] = 'EOS_PCWhiteBalance4',
        [0xD144] = 'EOS_PCWhiteBalance5',
        [0xD145] = 'EOS_MWhiteBalance',
        [0xD146] = 'EOS_MWhiteBalanceEx',
        [0xD150] = 'EOS_PictureStyleStandard',
        [0xD151] = 'EOS_PictureStylePortrait',
        [0xD152] = 'EOS_PictureStyleLandscape',
        [0xD153] = 'EOS_PictureStyleNeutral',
        [0xD154] = 'EOS_PictureStyleFaithful',
        [0xD155] = 'EOS_PictureStyleBlackWhite',
        [0xD156] = 'EOS_PictureStyleAuto',
        [0xD160] = 'EOS_PictureStyleUserSet1',
        [0xD161] = 'EOS_PictureStyleUserSet2',
        [0xD162] = 'EOS_PictureStyleUserSet3',
        [0xD170] = 'EOS_PictureStyleParam1',
        [0xD171] = 'EOS_PictureStyleParam2',
        [0xD172] = 'EOS_PictureStyleParam3',
        [0xD178] = 'EOS_HighISOSettingNoiseReduction',
        [0xD179] = 'EOS_MovieServoAF',
        [0xD17A] = 'EOS_ContinuousAFValid',
        [0xD17B] = 'EOS_Attenuator',
        [0xD17C] = 'EOS_UTCTime',
        [0xD17D] = 'EOS_Timezone',
        [0xD17E] = 'EOS_Summertime',
        [0xD17F] = 'EOS_FlavorLUTParams',
        [0xD180] = 'EOS_CustomFunc1',
        [0xD181] = 'EOS_CustomFunc2',
        [0xD182] = 'EOS_CustomFunc3',
        [0xD183] = 'EOS_CustomFunc4',
        [0xD184] = 'EOS_CustomFunc5',
        [0xD185] = 'EOS_CustomFunc6',
        [0xD186] = 'EOS_CustomFunc7',
        [0xD187] = 'EOS_CustomFunc8',
        [0xD188] = 'EOS_CustomFunc9',
        [0xD189] = 'EOS_CustomFunc10',
        [0xD18a] = 'EOS_CustomFunc11',
        [0xD18b] = 'EOS_CustomFunc12',
        [0xD18c] = 'EOS_CustomFunc13',
        [0xD18d] = 'EOS_CustomFunc14',
        [0xD18e] = 'EOS_CustomFunc15',
        [0xD18f] = 'EOS_CustomFunc16',
        [0xD190] = 'EOS_CustomFunc17',
        [0xD191] = 'EOS_CustomFunc18',
        [0xD192] = 'EOS_CustomFunc19',
        [0xD193] = 'EOS_InnerDevelop',
        [0xD194] = 'EOS_MultiAspect',
        [0xD195] = 'EOS_MovieSoundRecord',
        [0xD196] = 'EOS_MovieRecordVolume',
        [0xD197] = 'EOS_WindCut',
        [0xD198] = 'EOS_ExtenderType',
        [0xD199] = 'EOS_OLCInfoVersion',
        [0xD1a0] = 'EOS_CustomFuncEx',
        [0xD1a1] = 'EOS_MyMenu',
        [0xD1a2] = 'EOS_MyMenuList',
        [0xD1a3] = 'EOS_WftStatus',
        [0xD1a4] = 'EOS_WftInputTransmission',
        [0xD1a5] = 'EOS_HDDirectoryStructure',
        [0xD1a6] = 'EOS_BatteryInfo',
        [0xD1a7] = 'EOS_AdapterInfo',
        [0xD1a8] = 'EOS_LensStatus',
        [0xD1a9] = 'EOS_QuickReviewTime',
        [0xD1aa] = 'EOS_CardExtension',
        [0xD1ab] = 'EOS_TempStatus',
        [0xD1ac] = 'EOS_ShutterCounter',
        [0xD1ad] = 'EOS_SpecialOption',
        [0xD1ae] = 'EOS_PhotoStudioMode',
        [0xD1af] = 'EOS_SerialNumber',
        [0xD1b0] = 'EOS_EVFOutputDevice',
        [0xD1b1] = 'EOS_EVFMode',
        [0xD1b2] = 'EOS_DepthOfFieldPreview',
        [0xD1b3] = 'EOS_EVFSharpness',
        [0xD1b4] = 'EOS_EVFWBMode',
        [0xD1b5] = 'EOS_EVFClickWBCoeffs',
        [0xD1b6] = 'EOS_EVFColorTemp',
        [0xD1b7] = 'EOS_ExposureSimMode',
        [0xD1b8] = 'EOS_EVFRecordStatus',
        [0xD1ba] = 'EOS_LvAfSystem',
        [0xD1bb] = 'EOS_MovSize',
        [0xD1bc] = 'EOS_LvViewTypeSelect',
        [0xD1bd] = 'EOS_MirrorDownStatus',
        [0xD1be] = 'EOS_MovieParam',
        [0xD1bf] = 'EOS_MirrorLockupState',
        [0xD1C0] = 'EOS_FlashChargingState',
        [0xD1C1] = 'EOS_AloMode',
        [0xD1C2] = 'EOS_FixedMovie',
        [0xD1C3] = 'EOS_OneShotRawOn',
        [0xD1C4] = 'EOS_ErrorForDisplay',
        [0xD1C5] = 'EOS_AEModeMovie',
        [0xD1C6] = 'EOS_BuiltinStroboMode',
        [0xD1C7] = 'EOS_StroboDispState',
        [0xD1C8] = 'EOS_StroboETTL2Metering',
        [0xD1C9] = 'EOS_ContinousAFMode',
        [0xD1CA] = 'EOS_MovieParam2',
        [0xD1CB] = 'EOS_StroboSettingExpComposition',
        [0xD1CC] = 'EOS_MovieParam3',
        [0xD1CF] = 'EOS_LVMedicalRotate',
        [0xD1d0] = 'EOS_Artist',
        [0xD1d1] = 'EOS_Copyright',
        [0xD1d2] = 'EOS_BracketValue',
        [0xD1d3] = 'EOS_FocusInfoEx',
        [0xD1d4] = 'EOS_DepthOfField',
        [0xD1d5] = 'EOS_Brightness',
        [0xD1d6] = 'EOS_LensAdjustParams',
        [0xD1d7] = 'EOS_EFComp',
        [0xD1d8] = 'EOS_LensName',
        [0xD1d9] = 'EOS_AEB',
        [0xD1da] = 'EOS_StroboSetting',
        [0xD1db] = 'EOS_StroboWirelessSetting',
        [0xD1dc] = 'EOS_StroboFiring',
        [0xD1dd] = 'EOS_LensID',
        [0xD1de] = 'EOS_LCDBrightness',
        [0xD1df] = 'EOS_CADarkBright',
    },

    ['EventCode'] = {
        [0xC008] = 'ObjectInfoChanged',
        [0xC009] = 'RequestObjectTransfer',
        [0xC00B] = 'ShutterButtonPressed0',
        [0xC00C] = 'CameraModeChanged',
        [0xC00E] = 'ShutterButtonPressed1',
        [0xC011] = 'StartDirectTransfer',
        [0xC013] = 'StopDirectTransfer',
        [0xc101] = 'EOS_RequestGetEvent',
        [0xc181] = 'EOS_ObjectAddedEx',
        [0xc182] = 'EOS_ObjectRemoved',
        [0xc183] = 'EOS_RequestGetObjectInfoEx',
        [0xc184] = 'EOS_StorageStatusChanged',
        [0xc185] = 'EOS_StorageInfoChanged',
        [0xc186] = 'EOS_RequestObjectTransfer',
        [0xc187] = 'EOS_ObjectInfoChangedEx',
        [0xc188] = 'EOS_ObjectContentChanged',
        [0xc189] = 'EOS_PropValueChanged',
        [0xc18a] = 'EOS_AvailListChanged',
        [0xc18b] = 'EOS_CameraStatusChanged',
        [0xc18d] = 'EOS_WillSoonShutdown',
        [0xc18e] = 'EOS_ShutdownTimerUpdated',
        [0xc18f] = 'EOS_RequestCancelTransfer',
        [0xc190] = 'EOS_RequestObjectTransferDT',
        [0xc191] = 'EOS_RequestCancelTransferDT',
        [0xc192] = 'EOS_StoreAdded',
        [0xc193] = 'EOS_StoreRemoved',
        [0xc194] = 'EOS_BulbExposureTime',
        [0xc195] = 'EOS_RecordingTime',
        [0xc1a2] = 'EOS_RequestObjectTransferTS',
        [0xc1a3] = 'EOS_AfResult',
        [0xc1a4] = 'EOS_CTGInfoCheckComplete',
        [0xc1a5] = 'EOS_OLCInfoChanged',
        [0xc1a9] = 'EOS_ObjectAddedNew',
        [0xc1f1] = 'EOS_RequestObjectTransferFTP',
    },

    ['FormatCode'] = {
        [0xb101] = 'CRW',
        [0xb103] = 'CRW3',
        [0xb104] = 'MOV',
        [0xb105] = 'MOV2',
        [0xb1ff] = 'CHDK_CRW',
    },

    ['OperationCodeDescriptions'] = {
        [0x9001] = 'GetPartialObjectInfo',
        [0x9002] = 'SetObjectArchive',
        [0x9003] = 'KeepDeviceOn',
        [0x9004] = 'LockDeviceUI',
        [0x9005] = 'UnlockDeviceUI',
        [0x9006] = 'GetObjectHandleByName',
        [0x9008] = 'InitiateReleaseControl',
        [0x9009] = 'TerminateReleaseControl',
        [0x900A] = 'TerminatePlaybackMode',
        [0x900B] = 'ViewfinderOn',
        [0x900C] = 'ViewfinderOff',
        [0x900D] = 'DoAeAfAwb',
        [0x900E] = 'GetCustomizeSpec',
        [0x900F] = 'GetCustomizeItemInfo',
        [0x9010] = 'GetCustomizeData',
        [0x9011] = 'SetCustomizeData',
        [0x9012] = 'GetCaptureStatus',
        [0x9013] = 'CheckEvent',
        [0x9014] = 'FocusLock',
        [0x9015] = 'FocusUnlock',
        [0x9016] = 'GetLocalReleaseParam',
        [0x9017] = 'SetLocalReleaseParam',
        [0x9018] = 'AskAboutPcEvf',
        [0x9019] = 'SendPartialObject',
        [0x901A] = 'InitiateCaptureInMemory',
        [0x901B] = 'GetPartialObjectEx',
        [0x901C] = 'SetObjectTime',
        [0x901D] = 'GetViewfinderImage',
        [0x901E] = 'GetObjectAttributes',
        [0x901F] = 'ChangeUSBProtocol',
        [0x9020] = 'GetChanges',
        [0x9021] = 'GetObjectInfoEx',
        [0x9022] = 'InitiateDirectTransfer',
        [0x9023] = 'TerminateDirectTransfer',
        [0x9024] = 'SendObjectInfoByPath',
        [0x9025] = 'SendObjectByPath',
        [0x9026] = 'InitiateDirectTansferEx',
        [0x9027] = 'GetAncillaryObjectHandles',
        [0x9028] = 'GetTreeInfo',
        [0x9029] = 'GetTreeSize',
        [0x902A] = 'NotifyProgress',
        [0x902B] = 'NotifyCancelAccepted',
        [0x902C] = '902C',
        [0x902D] = 'GetDirectory',
        [0x902E] = '902E',
        [0x9030] = 'SetPairingInfo',
        [0x9031] = 'GetPairingInfo',
        [0x9032] = 'DeletePairingInfo',
        [0x9033] = 'GetMACAddress',
        [0x9034] = 'SetDisplayMonitor',
        [0x9035] = 'PairingComplete',
        [0x9036] = 'GetWirelessMAXChannel',
        [0x9068] = 'GetWebServiceSpec',
        [0x9069] = 'GetWebServiceData',
        [0x906B] = 'SetWebServiceData',
        [0x906C] = 'GetRootCertificateSpec',
        [0x906D] = 'GetRootCertificateData',
        [0x906F] = 'SetRootCertificateData',
        [0x9101] = 'EOS_GetStorageIDs',
        [0x9102] = 'EOS_GetStorageInfo',
        [0x9103] = 'EOS_GetObjectInfo',
        [0x9104] = 'EOS_GetObject',
        [0x9105] = 'EOS_DeleteObject',
        [0x9106] = 'EOS_FormatStore',
        [0x9107] = 'EOS_GetPartialObject',
        [0x9108] = 'EOS_GetDeviceInfoEx',
        [0x9109] = 'EOS_GetObjectInfoEx',
        [0x910A] = 'EOS_GetThumbEx',
        [0x910B] = 'EOS_SendPartialObject',
        [0x910C] = 'EOS_SetObjectAttributes',
        [0x910D] = 'EOS_GetObjectTime',
        [0x910E] = 'EOS_SetObjectTime',
        [0x910F] = 'EOS_RemoteRelease',
        [0x9110] = 'EOS_SetDevicePropValueEx',
        [0x9113] = 'EOS_GetRemoteMode',
        [0x9114] = 'EOS_SetRemoteMode',
        [0x9115] = 'EOS_SetEventMode',
        [0x9116] = 'EOS_GetEvent',
        [0x9117] = 'EOS_TransferComplete',
        [0x9118] = 'EOS_CancelTransfer',
        [0x9119] = 'EOS_ResetTransfer',
        [0x911A] = 'EOS_PCHDDCapacity',
        [0x911B] = 'EOS_SetUILock',
        [0x911C] = 'EOS_ResetUILock',
        [0x911D] = 'EOS_KeepDeviceOn',
        [0x911E] = 'EOS_SetNullPacketMode',
        [0x911F] = 'EOS_UpdateFirmware',
        [0x9120] = 'EOS_TransferCompleteDT',
        [0x9121] = 'EOS_CancelTransferDT',
        [0x9122] = 'EOS_SetWftProfile',
        [0x9123] = 'EOS_GetWftProfile',
        [0x9124] = 'EOS_SetProfileToWft',
        [0x9125] = 'EOS_BulbStart',
        [0x9126] = 'EOS_BulbEnd',
        [0x9127] = 'EOS_RequestDevicePropValue',
        [0x9128] = 'EOS_RemoteReleaseOn',
        [0x9129] = 'EOS_RemoteReleaseOff',
        [0x912A] = 'EOS_RegistBackgroundImage',
        [0x912B] = 'EOS_ChangePhotoStudioMode',
        [0x912C] = 'EOS_GetPartialObjectEx',
        [0x9130] = 'EOS_ResetMirrorLockupState',
        [0x9131] = 'EOS_PopupBuiltinFlash',
        [0x9132] = 'EOS_EndGetPartialObjectEx',
        [0x9133] = 'EOS_MovieSelectSWOn',
        [0x9134] = 'EOS_MovieSelectSWOff',
        [0x9135] = 'EOS_GetCTGInfo',
        [0x9136] = 'EOS_GetLensAdjust',
        [0x9137] = 'EOS_SetLensAdjust',
        [0x9138] = 'EOS_GetMusicInfo',
        [0x9139] = 'EOS_CreateHandle',
        [0x913A] = 'EOS_SendPartialObjectEx',
        [0x913B] = 'EOS_EndSendPartialObjectEx',
        [0x913C] = 'EOS_SetCTGInfo',
        [0x913D] = 'EOS_SetRequestOLCInfoGroup',
        [0x913E] = 'EOS_SetRequestRollingPitchingLevel',
        [0x913F] = 'EOS_GetCameraSupport',
        [0x9142] = 'EOS_RequestInnerDevelopParamChange',
        [0x9143] = 'EOS_RequestInnerDevelopEnd',
        [0x9145] = 'EOS_GetGpsLogCurrentHandle',
        [0x9151] = 'EOS_InitiateViewfinder',
        [0x9152] = 'EOS_TerminateViewfinder',
        [0x9153] = 'EOS_GetViewFinderData',
        [0x9154] = 'EOS_DoAf',
        [0x9155] = 'EOS_DriveLens',
        [0x915A] = 'EOS_SetLiveAfFrame',
        [0x9160] = 'EOS_AfCancel',
        [0x91BE] = 'EOS_SetDefaultCameraSetting',
        [0x91BF] = 'EOS_GetAEData',
        [0x91E8] = 'EOS_NotifyNetworkError',
        [0x91E9] = 'EOS_AdapterTransferProgress',
        [0x91F0] = 'EOS_TransferComplete2',
        [0x91F1] = 'EOS_CancelTransfer2',
        [0x91FE] = 'EOS_FAPIMessageTX',
        [0x91FF] = 'EOS_FAPIMessageRX',
    },

    ['ResponseCodeDescriptions'] = {
        [0xA001] = 'UNKNOWN_COMMAND',
        [0xA005] = 'OPERATION_REFUSED',
        [0xA006] = 'LENS_COVER',
        [0xA009] = 'A009',
        [0xA101] = 'BATTERY_LOW',
        [0xA102] = 'NOT_READY',
    },

}

    
local CASIO_EXTENSIONS = {
    ['DevicePropCode'] = {
        [0xD004] = 'UNKNOWN_1',
        [0xD005] = 'UNKNOWN_2',
        [0xD007] = 'UNKNOWN_3',
        [0xD008] = 'RECORD_LIGHT',
        [0xD009] = 'UNKNOWN_4',
        [0xD00A] = 'UNKNOWN_5',
        [0xD00B] = 'MOVIE_MODE',
        [0xD00C] = 'HD_SETTING',
        [0xD00D] = 'HS_SETTING',
        [0xD00F] = 'CS_HIGH_SPEED',
        [0xD010] = 'CS_UPPER_LIMIT',
        [0xD011] = 'CS_SHOT',
        [0xD012] = 'UNKNOWN_6',
        [0xD013] = 'UNKNOWN_7',
        [0xD015] = 'UNKNOWN_8',
        [0xD017] = 'UNKNOWN_9',
        [0xD018] = 'UNKNOWN_10',
        [0xD019] = 'UNKNOWN_11',
        [0xD01A] = 'UNKNOWN_12',
        [0xD01B] = 'UNKNOWN_13',
        [0xD01C] = 'UNKNOWN_14',
        [0xD01D] = 'UNKNOWN_15',
        [0xD020] = 'UNKNOWN_16',
        [0xD030] = 'UNKNOWN_17',
        [0xD080] = 'UNKNOWN_18',
    },

    ['OperationCodeDescriptions'] = {
        [0x9001] = 'STILL_START',
        [0x9002] = 'STILL_STOP',
        [0x9007] = 'FOCUS',
        [0x9009] = 'CF_PRESS',
        [0x900A] = 'CF_RELEASE',
        [0x900C] = 'GET_OBJECT_INFO',
        [0x9024] = 'SHUTTER',
        [0x9025] = 'GET_OBJECT',
        [0x9026] = 'GET_THUMBNAIL',
        [0x9027] = 'GET_STILL_HANDLES',
        [0x9028] = 'STILL_RESET',
        [0x9029] = 'HALF_PRESS',
        [0x902A] = 'HALF_RELEASE',
        [0x902B] = 'CS_PRESS',
        [0x902C] = 'CS_RELEASE',
        [0x902D] = 'ZOOM',
        [0x902E] = 'CZ_PRESS',
        [0x902F] = 'CZ_RELEASE',
        [0x9041] = 'MOVIE_START',
        [0x9042] = 'MOVIE_STOP',
        [0x9043] = 'MOVIE_PRESS',
        [0x9044] = 'MOVIE_RELEASE',
        [0x9045] = 'GET_MOVIE_HANDLES',
        [0x9046] = 'MOVIE_RESET',
    },

}

    
local EK_EXTENSIONS = {
    ['DevicePropCode'] = {
        [0xD001] = 'ColorTemperature',
        [0xD002] = 'DateTimeStampFormat',
        [0xD003] = 'BeepMode',
        [0xD004] = 'VideoOut',
        [0xD005] = 'PowerSaving',
        [0xD006] = 'UI_Language',
    },

    ['FormatCode'] = {
        [0xb002] = 'M3U',
    },

    ['OperationCodeDescriptions'] = {
        [0x9003] = 'GetSerial',
        [0x9004] = 'SetSerial',
        [0x9005] = 'SendFileObjectInfo',
        [0x9006] = 'SendFileObject',
        [0x9008] = 'SetText',
    },

    ['ResponseCodeDescriptions'] = {
        [0xA001] = 'FilenameRequired',
        [0xA002] = 'FilenameConflicts',
        [0xA003] = 'FilenameInvalid',
    },

}

    
local FUJI_EXTENSIONS = {
    ['DevicePropCode'] = {
        [0xD017] = 'ColorTemperature',
        [0xD018] = 'Quality',
        [0xD201] = 'ReleaseMode',
        [0xD206] = 'FocusAreas',
        [0xD213] = 'AELock',
        [0xD218] = 'Aperture',
        [0xD219] = 'ShutterSpeed',
    },

}

    
local LEICA_EXTENSIONS = {
    ['OperationCodeDescriptions'] = {
        [0x9001] = 'SetCameraSettings',
        [0x9002] = 'GetCameraSettings',
        [0x9003] = 'GetLensParameter',
        [0x9004] = 'Release',
        [0x9005] = 'OpenLESession',
        [0x9006] = 'CloseLESession',
        [0x9007] = 'RequestObjectTransferReady',
    },

}

    
local MTP_EXTENSIONS = {
    ['DevicePropCode'] = {
        [0xD101] = 'SecureTime',
        [0xD102] = 'DeviceCertificate',
        [0xD103] = 'RevocationInfo',
        [0xD131] = 'PlaysForSureID',
        [0xD132] = 'ZUNE_UNKNOWN2',
        [0xD181] = 'ZUNE_UNKNOWN1',
        [0xD215] = 'ZUNE_UNKNOWN3',
        [0xD216] = 'ZUNE_UNKNOWN4',
        [0xD401] = 'SynchronizationPartner',
        [0xD402] = 'DeviceFriendlyName',
        [0xD403] = 'VolumeLevel',
        [0xD405] = 'DeviceIcon',
        [0xD406] = 'SessionInitiatorInfo',
        [0xD407] = 'PerceivedDeviceType',
        [0xD410] = 'PlaybackRate',
        [0xD411] = 'PlaybackObject',
        [0xD412] = 'PlaybackContainerIndex',
        [0xD413] = 'PlaybackPosition',
    },

    ['EventCode'] = {
        [0xC801] = 'ObjectPropChanged',
        [0xC802] = 'ObjectPropDescChanged',
        [0xC803] = 'ObjectReferencesChanged',
    },

    ['FormatCode'] = {
        [0xb211] = 'MediaCard',
        [0xb212] = 'MediaCardGroup',
        [0xb213] = 'Encounter',
        [0xb214] = 'EncounterBox',
        [0xb215] = 'M4A',
        [0xb802] = 'Firmware',
        [0xb881] = 'WindowsImageFormat',
        [0xb900] = 'UndefinedAudio',
        [0xb901] = 'WMA',
        [0xb902] = 'OGG',
        [0xb903] = 'AAC',
        [0xb904] = 'AudibleCodec',
        [0xb906] = 'FLAC',
        [0xb909] = 'SamsungPlaylist',
        [0xb980] = 'UndefinedVideo',
        [0xb981] = 'WMV',
        [0xb982] = 'MP4',
        [0xb983] = 'MP2',
        [0xb984] = '3GP',
        [0xba00] = 'UndefinedCollection',
        [0xba01] = 'AbstractMultimediaAlbum',
        [0xba02] = 'AbstractImageAlbum',
        [0xba03] = 'AbstractAudioAlbum',
        [0xba04] = 'AbstractVideoAlbum',
        [0xba05] = 'AbstractAudioVideoPlaylist',
        [0xba06] = 'AbstractContactGroup',
        [0xba07] = 'AbstractMessageFolder',
        [0xba08] = 'AbstractChapteredProduction',
        [0xba09] = 'AbstractAudioPlaylist',
        [0xba0a] = 'AbstractVideoPlaylist',
        [0xba0b] = 'AbstractMediacast',
        [0xba10] = 'WPLPlaylist',
        [0xba11] = 'M3UPlaylist',
        [0xba12] = 'MPLPlaylist',
        [0xba13] = 'ASXPlaylist',
        [0xba14] = 'PLSPlaylist',
        [0xba80] = 'UndefinedDocument',
        [0xba81] = 'AbstractDocument',
        [0xba82] = 'XMLDocument',
        [0xba83] = 'MSWordDocument',
        [0xba84] = 'MHTCompiledHTMLDocument',
        [0xba85] = 'MSExcelSpreadsheetXLS',
        [0xba86] = 'MSPowerpointPresentationPPT',
        [0xbb00] = 'UndefinedMessage',
        [0xbb01] = 'AbstractMessage',
        [0xbb80] = 'UndefinedContact',
        [0xbb81] = 'AbstractContact',
        [0xbb82] = 'vCard2',
        [0xbb83] = 'vCard3',
        [0xbe00] = 'UndefinedCalendarItem',
        [0xbe01] = 'AbstractCalendarItem',
        [0xbe02] = 'vCalendar1',
        [0xbe03] = 'vCalendar2',
        [0xbe80] = 'UndefinedWindowsExecutable',
        [0xbe81] = 'MediaCast',
        [0xbe82] = 'Section',
    },

    ['OperationCodeDescriptions'] = {
        [0x9101] = 'WMDRMPD_GetSecureTimeChallenge',
        [0x9102] = 'WMDRMPD_GetSecureTimeResponse',
        [0x9103] = 'WMDRMPD_SetLicenseResponse',
        [0x9104] = 'WMDRMPD_GetSyncList',
        [0x9105] = 'WMDRMPD_SendMeterChallengeQuery',
        [0x9106] = 'WMDRMPD_GetMeterChallenge',
        [0x9107] = 'WMDRMPD_SetMeterResponse',
        [0x9108] = 'WMDRMPD_CleanDataStore',
        [0x9109] = 'WMDRMPD_GetLicenseState',
        [0x910A] = 'WMDRMPD_SendWMDRMPDCommand',
        [0x910B] = 'WMDRMPD_SendWMDRMPDRequest',
        [0x9122] = 'WPDWCN_ProcessWFCObject',
        [0x9170] = 'AAVT_OpenMediaSession',
        [0x9171] = 'AAVT_CloseMediaSession',
        [0x9172] = 'AAVT_GetNextDataBlock',
        [0x9173] = 'AAVT_SetCurrentTimePosition',
        [0x9180] = 'WMDRMND_SendRegistrationRequest',
        [0x9181] = 'WMDRMND_GetRegistrationResponse',
        [0x9182] = 'WMDRMND_GetProximityChallenge',
        [0x9183] = 'WMDRMND_SendProximityResponse',
        [0x9184] = 'WMDRMND_SendWMDRMNDLicenseRequest',
        [0x9185] = 'WMDRMND_GetWMDRMNDLicenseResponse',
        [0x9201] = 'WMPPD_ReportAddedDeletedItems',
        [0x9202] = 'WMPPD_ReportAcquiredItems',
        [0x9203] = 'WMPPD_PlaylistObjectPref',
        [0x9204] = 'ZUNE_GETUNDEFINED001',
        [0x9212] = 'WMDRMPD_SendWMDRMPDAppRequest',
        [0x9213] = 'WMDRMPD_GetWMDRMPDAppResponse',
        [0x9214] = 'WMDRMPD_EnableTrustedFilesOperations',
        [0x9215] = 'WMDRMPD_DisableTrustedFilesOperations',
        [0x9216] = 'WMDRMPD_EndTrustedAppSession',
        [0x9801] = 'GetObjectPropsSupported',
        [0x9802] = 'GetObjectPropDesc',
        [0x9803] = 'GetObjectPropValue',
        [0x9804] = 'SetObjectPropValue',
        [0x9805] = 'GetObjPropList',
        [0x9806] = 'SetObjPropList',
        [0x9807] = 'GetInterdependendPropdesc',
        [0x9808] = 'SendObjectPropList',
        [0x9810] = 'GetObjectReferences',
        [0x9811] = 'SetObjectReferences',
        [0x9812] = 'UpdateDeviceFirmware',
        [0x9820] = 'Skip',
    },

    ['ResponseCodeDescriptions'] = {
        [0xA121] = 'Invalid_WFC_Syntax',
        [0xA122] = 'WFC_Version_Not_Supported',
        [0xA171] = 'Media_Session_Limit_Reached',
        [0xA172] = 'No_More_Data',
        [0xA800] = 'Undefined',
        [0xA801] = 'Invalid_ObjectPropCode',
        [0xA802] = 'Invalid_ObjectProp_Format',
        [0xA803] = 'Invalid_ObjectProp_Value',
        [0xA804] = 'Invalid_ObjectReference',
        [0xA806] = 'Invalid_Dataset',
        [0xA807] = 'Specification_By_Group_Unsupported',
        [0xA808] = 'Specification_By_Depth_Unsupported',
        [0xA809] = 'Object_Too_Large',
        [0xA80A] = 'ObjectProp_Not_Supported',
    },

}

    
local NIKON_EXTENSIONS = {
    ['DevicePropCode'] = {
        [0xD010] = 'ShootingBank',
        [0xD011] = 'ShootingBankNameA',
        [0xD012] = 'ShootingBankNameB',
        [0xD013] = 'ShootingBankNameC',
        [0xD014] = 'ShootingBankNameD',
        [0xD015] = 'ResetBank0',
        [0xD016] = 'RawCompression',
        [0xD017] = 'WhiteBalanceAutoBias',
        [0xD018] = 'WhiteBalanceTungstenBias',
        [0xD019] = 'WhiteBalanceFluorescentBias',
        [0xD01A] = 'WhiteBalanceDaylightBias',
        [0xD01B] = 'WhiteBalanceFlashBias',
        [0xD01C] = 'WhiteBalanceCloudyBias',
        [0xD01D] = 'WhiteBalanceShadeBias',
        [0xD01E] = 'WhiteBalanceColorTemperature',
        [0xD01F] = 'WhiteBalancePresetNo',
        [0xD020] = 'WhiteBalancePresetName0',
        [0xD021] = 'WhiteBalancePresetName1',
        [0xD022] = 'WhiteBalancePresetName2',
        [0xD023] = 'WhiteBalancePresetName3',
        [0xD024] = 'WhiteBalancePresetName4',
        [0xD025] = 'WhiteBalancePresetVal0',
        [0xD026] = 'WhiteBalancePresetVal1',
        [0xD027] = 'WhiteBalancePresetVal2',
        [0xD028] = 'WhiteBalancePresetVal3',
        [0xD029] = 'WhiteBalancePresetVal4',
        [0xD02A] = 'ImageSharpening',
        [0xD02B] = 'ToneCompensation',
        [0xD02C] = 'ColorModel',
        [0xD02D] = 'HueAdjustment',
        [0xD030] = 'ShootingMode',
        [0xD031] = 'JPEG_Compression_Policy',
        [0xD032] = 'ColorSpace',
        [0xD033] = 'AutoDXCrop',
        [0xD034] = 'FlickerReduction',
        [0xD035] = 'RemoteMode',
        [0xD036] = 'VideoMode',
        [0xD037] = 'EffectMode',
        [0xD038] = '1_Mode',
        [0xD040] = 'CSMMenuBankSelect',
        [0xD041] = 'MenuBankNameA',
        [0xD042] = 'MenuBankNameB',
        [0xD043] = 'MenuBankNameC',
        [0xD044] = 'MenuBankNameD',
        [0xD045] = 'ResetBank',
        [0xD048] = 'A1AFCModePriority',
        [0xD049] = 'A2AFSModePriority',
        [0xD04A] = 'A3GroupDynamicAF',
        [0xD04B] = 'A4AFActivation',
        [0xD04C] = 'FocusAreaIllumManualFocus',
        [0xD04D] = 'FocusAreaIllumContinuous',
        [0xD04E] = 'FocusAreaIllumWhenSelected',
        [0xD050] = 'VerticalAFON',
        [0xD051] = 'AFLockOn',
        [0xD052] = 'FocusAreaZone',
        [0xD053] = 'EnableCopyright',
        [0xD054] = 'ISOAuto',
        [0xD055] = 'EVISOStep',
        [0xD057] = 'EVStepExposureComp',
        [0xD058] = 'ExposureCompensation',
        [0xD059] = 'CenterWeightArea',
        [0xD05A] = 'ExposureBaseMatrix',
        [0xD05B] = 'ExposureBaseCenter',
        [0xD05C] = 'ExposureBaseSpot',
        [0xD05E] = 'AELockMode',
        [0xD05F] = 'AELAFLMode',
        [0xD061] = 'LiveViewAFFocus',
        [0xD062] = 'MeterOff',
        [0xD063] = 'SelfTimer',
        [0xD064] = 'MonitorOff',
        [0xD065] = 'ImgConfTime',
        [0xD066] = 'AutoOffTimers',
        [0xD067] = 'AngleLevel',
        [0xD069] = 'D2MaximumShots',
        [0xD06A] = 'ExposureDelayMode',
        [0xD06B] = 'LongExposureNoiseReduction',
        [0xD06C] = 'FileNumberSequence',
        [0xD06D] = 'ControlPanelFinderRearControl',
        [0xD06E] = 'ControlPanelFinderViewfinder',
        [0xD06F] = 'D7Illumination',
        [0xD070] = 'NrHighISO',
        [0xD071] = 'SHSET_CH_GUID_DISP',
        [0xD072] = 'ArtistName',
        [0xD073] = 'CopyrightInfo',
        [0xD074] = 'FlashSyncSpeed',
        [0xD076] = 'E3AAFlashMode',
        [0xD077] = 'E4ModelingFlash',
        [0xD07A] = 'BracketOrder',
        [0xD07C] = 'BracketingSet',
        [0xD080] = 'F1CenterButtonShootingMode',
        [0xD081] = 'CenterButtonPlaybackMode',
        [0xD082] = 'F2Multiselector',
        [0xD08B] = 'CenterButtonZoomRatio',
        [0xD08C] = 'FunctionButton2',
        [0xD08D] = 'AFAreaPoint',
        [0xD08E] = 'NormalAFOn',
        [0xD08F] = 'CleanImageSensor',
        [0xD090] = 'ImageCommentString',
        [0xD091] = 'ImageCommentEnable',
        [0xD092] = 'ImageRotation',
        [0xD093] = 'ManualSetLensNo',
        [0xD0A0] = 'MovScreenSize',
        [0xD0A1] = 'MovVoice',
        [0xD0A2] = 'MovMicrophone',
        [0xD0A3] = 'MovFileSlot',
        [0xD0A4] = 'MovRecProhibitCondition',
        [0xD0A6] = 'ManualMovieSetting',
        [0xD0A7] = 'MovQuality',
        [0xD0B2] = 'LiveViewScreenDisplaySetting',
        [0xD0B3] = 'MonitorOffDelay',
        [0xD0C0] = 'Bracketing',
        [0xD0C1] = 'AutoExposureBracketStep',
        [0xD0C2] = 'AutoExposureBracketProgram',
        [0xD0C3] = 'AutoExposureBracketCount',
        [0xD0C4] = 'WhiteBalanceBracketStep',
        [0xD0C5] = 'WhiteBalanceBracketProgram',
        [0xD0E0] = 'LensID',
        [0xD0E1] = 'LensSort',
        [0xD0E2] = 'LensType',
        [0xD0E3] = 'FocalLengthMin',
        [0xD0E4] = 'FocalLengthMax',
        [0xD0E5] = 'MaxApAtMinFocalLength',
        [0xD0E6] = 'MaxApAtMaxFocalLength',
        [0xD0F0] = 'FinderISODisp',
        [0xD0F2] = 'AutoOffPhoto',
        [0xD0F3] = 'AutoOffMenu',
        [0xD0F4] = 'AutoOffInfo',
        [0xD0F5] = 'SelfTimerShootNum',
        [0xD0F7] = 'VignetteCtrl',
        [0xD0F8] = 'AutoDistortionControl',
        [0xD0F9] = 'SceneMode',
        [0xD0FD] = 'SceneMode2',
        [0xD0FE] = 'SelfTimerInterval',
        [0xD101] = 'ACPower',
        [0xD102] = 'WarningStatus',
        [0xD104] = 'AFLockStatus',
        [0xD105] = 'AELockStatus',
        [0xD106] = 'FVLockStatus',
        [0xD107] = 'AutofocusLCDTopMode2',
        [0xD108] = 'AutofocusArea',
        [0xD109] = 'FlexibleProgram',
        [0xD10C] = 'USBSpeed',
        [0xD10D] = 'CCDNumber',
        [0xD10E] = 'CameraOrientation',
        [0xD10F] = 'GroupPtnType',
        [0xD110] = 'FNumberLock',
        [0xD112] = 'TVLockSetting',
        [0xD113] = 'AVLockSetting',
        [0xD114] = 'IllumSetting',
        [0xD115] = 'FocusPointBright',
        [0xD120] = 'ExternalFlashAttached',
        [0xD121] = 'ExternalFlashStatus',
        [0xD122] = 'ExternalFlashSort',
        [0xD123] = 'ExternalFlashMode',
        [0xD124] = 'ExternalFlashCompensation',
        [0xD125] = 'NewExternalFlashMode',
        [0xD126] = 'FlashExposureCompensation',
        [0xD130] = 'HDRMode',
        [0xD131] = 'HDRHighDynamic',
        [0xD132] = 'HDRSmoothing',
        [0xD140] = 'OptimizeImage',
        [0xD142] = 'Saturation',
        [0xD143] = 'BW_FillerEffect',
        [0xD144] = 'BW_Sharpness',
        [0xD145] = 'BW_Contrast',
        [0xD146] = 'BW_Setting_Type',
        [0xD148] = 'Slot2SaveMode',
        [0xD149] = 'RawBitMode',
        [0xD14F] = 'FlourescentType',
        [0xD150] = 'TuneColourTemperature',
        [0xD151] = 'TunePreset0',
        [0xD152] = 'TunePreset1',
        [0xD153] = 'TunePreset2',
        [0xD154] = 'TunePreset3',
        [0xD155] = 'TunePreset4',
        [0xD160] = 'BeepOff',
        [0xD161] = 'AutofocusMode',
        [0xD163] = 'AFAssist',
        [0xD165] = 'ImageReview',
        [0xD166] = 'AFAreaIllumination',
        [0xD167] = 'FlashMode',
        [0xD168] = 'FlashCommanderMode',
        [0xD169] = 'FlashSign',
        [0xD16A] = 'ISO_Auto',
        [0xD16B] = 'RemoteTimeout',
        [0xD16C] = 'GridDisplay',
        [0xD16D] = 'FlashModeManualPower',
        [0xD16E] = 'FlashModeCommanderPower',
        [0xD16F] = 'AutoFP',
        [0xD170] = 'DateImprintSetting',
        [0xD171] = 'DateCounterSelect',
        [0xD172] = 'DateCountData',
        [0xD173] = 'DateCountDisplaySetting',
        [0xD174] = 'RangeFinderSetting',
        [0xD180] = 'CSMMenu',
        [0xD181] = 'WarningDisplay',
        [0xD182] = 'BatteryCellKind',
        [0xD183] = 'ISOAutoHiLimit',
        [0xD184] = 'DynamicAFArea',
        [0xD186] = 'ContinuousSpeedHigh',
        [0xD187] = 'InfoDispSetting',
        [0xD189] = 'PreviewButton',
        [0xD18A] = 'PreviewButton2',
        [0xD18B] = 'AEAFLockButton2',
        [0xD18D] = 'IndicatorDisp',
        [0xD18E] = 'CellKindPriority',
        [0xD190] = 'BracketingFramesAndSteps',
        [0xD1A0] = 'LiveViewMode',
        [0xD1A1] = 'LiveViewDriveMode',
        [0xD1A2] = 'LiveViewStatus',
        [0xD1A3] = 'LiveViewImageZoomRatio',
        [0xD1A4] = 'LiveViewProhibitCondition',
        [0xD1A8] = 'MovieShutterSpeed',
        [0xD1A9] = 'MovieFNumber',
        [0xD1AA] = 'MovieISO',
        [0xD1B0] = 'ExposureDisplayStatus',
        [0xD1B1] = 'ExposureIndicateStatus',
        [0xD1B2] = 'InfoDispErrStatus',
        [0xD1B3] = 'ExposureIndicateLightup',
        [0xD1C0] = 'FlashOpen',
        [0xD1C1] = 'FlashCharged',
        [0xD1D0] = 'FlashMRepeatValue',
        [0xD1D1] = 'FlashMRepeatCount',
        [0xD1D2] = 'FlashMRepeatInterval',
        [0xD1D3] = 'FlashCommandChannel',
        [0xD1D4] = 'FlashCommandSelfMode',
        [0xD1D5] = 'FlashCommandSelfCompensation',
        [0xD1D6] = 'FlashCommandSelfValue',
        [0xD1D7] = 'FlashCommandAMode',
        [0xD1D8] = 'FlashCommandACompensation',
        [0xD1D9] = 'FlashCommandAValue',
        [0xD1DA] = 'FlashCommandBMode',
        [0xD1DB] = 'FlashCommandBCompensation',
        [0xD1DC] = 'FlashCommandBValue',
        [0xD1F0] = 'ApplicationMode',
        [0xD1F2] = 'ActiveSlot',
        [0xD200] = 'ActivePicCtrlItem',
        [0xD201] = 'ChangePicCtrlItem',
        [0xD236] = 'MovieNrHighISO',
        [0xD241] = 'D241',
        [0xD244] = 'D244',
        [0xD247] = 'D247',
        [0xD24F] = 'GUID',
        [0xD250] = 'D250',
        [0xD251] = 'D251',
        [0xF002] = '1_ISO',
        [0xF009] = '1_ImageCompression',
        [0xF00A] = '1_ImageSize',
        [0xF00C] = '1_WhiteBalance',
        [0xF00D] = '1_LongExposureNoiseReduction',
        [0xF00E] = '1_HiISONoiseReduction',
        [0xF00F] = '1_ActiveDLighting',
        [0xF01C] = '1_MovQuality',
    },

    ['OperationCodeDescriptions'] = {
        [0x9006] = 'GetProfileAllData',
        [0x9007] = 'SendProfileData',
        [0x9008] = 'DeleteProfile',
        [0x9009] = 'SetProfileData',
        [0x9010] = 'AdvancedTransfer',
        [0x9011] = 'GetFileInfoInBlock',
        [0x90C4] = 'GetLargeThumb',
        [0x90E0] = 'GetDevicePTPIPInfo',
        [0x9200] = 'GetPreviewImg',
    },

    ['ResponseCodeDescriptions'] = {
        [0xA001] = 'HardwareError',
        [0xA002] = 'OutOfFocus',
        [0xA003] = 'ChangeCameraModeFailed',
        [0xA004] = 'InvalidStatus',
        [0xA005] = 'SetPropertyNotSupported',
        [0xA006] = 'WbResetError',
        [0xA007] = 'DustReferenceError',
        [0xA008] = 'ShutterSpeedBulb',
        [0xA009] = 'MirrorUpSequence',
        [0xA00A] = 'CameraModeNotAdjustFNumber',
        [0xA00B] = 'NotLiveView',
        [0xA00C] = 'MfDriveStepEnd',
        [0xA00E] = 'MfDriveStepInsufficiency',
        [0xA022] = 'AdvancedTransferCancel',
    },

}

    
local OLYMPUS_EXTENSIONS = {
    ['DevicePropCode'] = {
        [0xD102] = 'ResolutionMode',
        [0xD103] = 'FocusPriority',
        [0xD104] = 'DriveMode',
        [0xD105] = 'DateTimeFormat',
        [0xD106] = 'ExposureBiasStep',
        [0xD107] = 'WBMode',
        [0xD108] = 'OneTouchWB',
        [0xD109] = 'ManualWB',
        [0xD10A] = 'ManualWBRBBias',
        [0xD10B] = 'CustomWB',
        [0xD10C] = 'CustomWBValue',
        [0xD10D] = 'ExposureTimeEx',
        [0xD10E] = 'BulbMode',
        [0xD10F] = 'AntiMirrorMode',
        [0xD110] = 'AEBracketingFrame',
        [0xD111] = 'AEBracketingStep',
        [0xD112] = 'WBBracketingFrame',
        [0xD113] = 'WBBracketingRBRange',
        [0xD114] = 'WBBracketingGMFrame',
        [0xD115] = 'WBBracketingGMRange',
        [0xD118] = 'FLBracketingFrame',
        [0xD119] = 'FLBracketingStep',
        [0xD11A] = 'FlashBiasCompensation',
        [0xD11B] = 'ManualFocusMode',
        [0xD11D] = 'RawSaveMode',
        [0xD11E] = 'AUXLightMode',
        [0xD11F] = 'LensSinkMode',
        [0xD120] = 'BeepStatus',
        [0xD122] = 'ColorSpace',
        [0xD123] = 'ColorMatching',
        [0xD124] = 'Saturation',
        [0xD126] = 'NoiseReductionPattern',
        [0xD127] = 'NoiseReductionRandom',
        [0xD129] = 'ShadingMode',
        [0xD12A] = 'ISOBoostMode',
        [0xD12B] = 'ExposureIndexBiasStep',
        [0xD12C] = 'FilterEffect',
        [0xD12D] = 'ColorTune',
        [0xD12E] = 'Language',
        [0xD12F] = 'LanguageCode',
        [0xD130] = 'RecviewMode',
        [0xD131] = 'SleepTime',
        [0xD132] = 'ManualWBGMBias',
        [0xD135] = 'AELAFLMode',
        [0xD136] = 'AELButtonStatus',
        [0xD137] = 'CompressionSettingEx',
        [0xD139] = 'ToneMode',
        [0xD13A] = 'GradationMode',
        [0xD13B] = 'DevelopMode',
        [0xD13C] = 'ExtendInnerFlashMode',
        [0xD13D] = 'OutputDeviceMode',
        [0xD13E] = 'LiveViewMode',
        [0xD140] = 'LCDBacklight',
        [0xD141] = 'CustomDevelop',
        [0xD142] = 'GradationAutoBias',
        [0xD143] = 'FlashRCMode',
        [0xD144] = 'FlashRCGroupValue',
        [0xD145] = 'FlashRCChannelValue',
        [0xD146] = 'FlashRCFPMode',
        [0xD147] = 'FlashRCPhotoChromicMode',
        [0xD148] = 'FlashRCPhotoChromicBias',
        [0xD149] = 'FlashRCPhotoChromicManualBias',
        [0xD14A] = 'FlashRCQuantityLightLevel',
        [0xD14B] = 'FocusMeteringValue',
        [0xD14C] = 'ISOBracketingFrame',
        [0xD14D] = 'ISOBracketingStep',
        [0xD14E] = 'BulbMFMode',
        [0xD14F] = 'BurstFPSValue',
        [0xD150] = 'ISOAutoBaseValue',
        [0xD151] = 'ISOAutoMaxValue',
        [0xD152] = 'BulbLimiterValue',
        [0xD153] = 'DPIMode',
        [0xD154] = 'DPICustomValue',
        [0xD155] = 'ResolutionValueSetting',
        [0xD157] = 'AFTargetSize',
        [0xD158] = 'LightSensorMode',
        [0xD159] = 'AEBracket',
        [0xD15A] = 'WBRBBracket',
        [0xD15B] = 'WBGMBracket',
        [0xD15C] = 'FlashBracket',
        [0xD15D] = 'ISOBracket',
        [0xD15E] = 'MyModeStatus',
    },

    ['OperationCodeDescriptions'] = {
        [0x9101] = 'Capture',
        [0x9103] = 'SelfCleaning',
        [0x9106] = 'SetRGBGain',
        [0x9107] = 'SetPresetMode',
        [0x9108] = 'SetWBBiasAll',
        [0x910a] = 'GetCameraControlMode',
        [0x910b] = 'SetCameraControlMode',
        [0x910c] = 'SetWBRGBGain',
        [0x9301] = 'GetDeviceInfo',
        [0x9302] = 'OpenSession',
        [0x9402] = 'SetDateTime',
        [0x9482] = 'GetDateTime',
        [0x9501] = 'SetCameraID',
        [0x9581] = 'GetCameraID',
    },

}

    
local RICOH_EXTENSIONS = {
    ['DevicePropCode'] = {
        [0xD00F] = 'ShutterSpeed',
    },

}

    
local SONY_EXTENSIONS = {
    ['DevicePropCode'] = {
        [0xD200] = 'DPCCompensation',
        [0xD201] = 'DRangeOptimize',
        [0xD203] = 'ImageSize',
        [0xD20D] = 'ShutterSpeed',
        [0xD20F] = 'ColorTemp',
        [0xD210] = 'CCFilter',
        [0xD211] = 'AspectRatio',
        [0xD216] = 'ExposeIndex',
        [0xD21B] = 'PictureEffect',
        [0xD21C] = 'ABFilter',
    },

    ['EventCode'] = {
        [0xC201] = 'ObjectAdded',
        [0xC202] = 'ObjectRemoved',
        [0xC203] = 'PropertyChanged',
    },

    ['FormatCode'] = {
        [0xb101] = 'RAW',
    },

    ['OperationCodeDescriptions'] = {
        [0x9201] = 'SDIOConnect',
        [0x9202] = 'GetSDIOGetExtDeviceInfo',
        [0x9203] = 'GetDevicePropdesc',
        [0x9204] = 'GetDevicePropertyValue',
        [0x9205] = 'SetControlDeviceA',
        [0x9206] = 'GetControlDeviceDesc',
        [0x9207] = 'SetControlDeviceB',
    },

}

local VENDORS = {
    UNKNOWN = 1,
    ANDROID = 2,    CANON = 3,    CASIO = 4,    EK = 5,    FUJI = 6,    LEICA = 7,    MTP = 8,    NIKON = 9,    OLYMPUS = 10,    RICOH = 11,    SONY = 12,}

local VENDOR_EXTENSIONS = {
    [VENDORS.ANDROID] = ANDROID_EXTENSIONS,    [VENDORS.CANON] = CANON_EXTENSIONS,    [VENDORS.CASIO] = CASIO_EXTENSIONS,    [VENDORS.EK] = EK_EXTENSIONS,    [VENDORS.FUJI] = FUJI_EXTENSIONS,    [VENDORS.LEICA] = LEICA_EXTENSIONS,    [VENDORS.MTP] = MTP_EXTENSIONS,    [VENDORS.NIKON] = NIKON_EXTENSIONS,    [VENDORS.OLYMPUS] = OLYMPUS_EXTENSIONS,    [VENDORS.RICOH] = RICOH_EXTENSIONS,    [VENDORS.SONY] = SONY_EXTENSIONS,}

local vendor_pref_enum = {
    { 1, "Generic MTP", 0 },
    {  2, 'Android', VENDORS.ANDROID },
    {  3, 'Canon', VENDORS.CANON },
    {  4, 'Casio', VENDORS.CASIO },
    {  5, 'Ek', VENDORS.EK },
    {  6, 'Fuji', VENDORS.FUJI },
    {  7, 'Leica', VENDORS.LEICA },
    {  8, 'Mtp', VENDORS.MTP },
    {  9, 'Nikon', VENDORS.NIKON },
    {  10, 'Olympus', VENDORS.OLYMPUS },
    {  11, 'Ricoh', VENDORS.RICOH },
    {  12, 'Sony', VENDORS.SONY },
}


-- Header fields used in the plugin:
-- Note: needs to be placed in front of the dissectors
local hdr_fields =
{
	-- length = ProtoField.uint32("ptp.length","Length"),
	-- packet_type = ProtoField.uint32("ptp.pktType","Packet Type",base.HEX),

	-- PTP layer
    -- Original comment: leaving names with "ptpip" to try and prevent namespace issues. probably changing later.
    transaction_id = ProtoField.uint32("ptp.transactionID","Transaction ID",base.HEX),
    opcode = ProtoField.uint16("ptp.opcode","Operation Code",base.HEX), --  BASE_HEX|BASE_EXT_STRING--
    response_code =  ProtoField.uint16("ptp.respcode","Response Code",base.HEX),
    vendor_response_code =  ProtoField.uint16("ptp.respcode","Response Code",base.HEX),
    event_code =  ProtoField.uint16("ptp.eventcode","Event Code",base.HEX),
    total_data_length = ProtoField.uint64("ptp.datalen", "Total Data Length",base.HEX),
    session_id = ProtoField.uint32("ptp.op.sessionid", "Session ID", base.HEX),
    param1 = ProtoField.uint32("ptp.param1", "Parameter 1", base.HEX),
    param2 = ProtoField.uint32("ptp.param2", "Parameter 2", base.HEX),
    param3 = ProtoField.uint32("ptp.param3", "Parameter 3", base.HEX),
    param4 = ProtoField.uint32("ptp.param4", "Parameter 4", base.HEX),
    param5 = ProtoField.uint32("ptp.param5", "Parameter 5", base.HEX),
    data = ProtoField.bytes("ptp.data", "Data", base.SPACE),
}

-- Wireshark parser implementation
ptp_proto = Proto("PTP-Payload","Picture Transfer Protocol (Payload only)")

-- add the field to the protocol
ptp_proto.fields = hdr_fields

-- import fields from the parent pre- dissector
parent_length_field = Field.new("ptp.length")
parent_packet_type_field = Field.new("ptp.pktType")
parent_header_offset_field = Field.new("ptp.headerTidOffset") -- protocol specific offset to the tid from the PTP/IP or PTP dissector
parent_code_field = Field.new("ptp.pktCode") -- op / event / response code

-- our dissector is added down

-- current lookup tables
mtp_lookup_tables = mtp_lookup  -- dynamic combination mtp + camera-vendor
mtp_transaction_opcodes = {}    -- lookup table transactionID -> request.opcode (allows response parameter parsing)
mtp_transaction_opdescriptions = {} -- lookup table transactionID -> request description ('GetDeviceProperty: Exposure')

----------------------------------------
-- do not modify this table
local debug_level = {
    DISABLED = 0,
    LEVEL_1  = 1,
    LEVEL_2  = 2
}

----------------------------------------
-- set this DEBUG to debug_level.LEVEL_1 to enable printing debug_level info
-- set it to debug_level.LEVEL_2 to enable really verbose printing
-- set it to debug_level.DISABLED to disable debug printing
-- note: this will be overridden by user's preference settings
local DEBUG = debug_level.LEVEL_1

-- a table of our default settings - these can be changed by changing
-- the preferences through the GUI or command-line; the Lua-side of that
-- preference handling is at the end of this script file
local default_settings =
{
    debug_level  = DEBUG,
    camera_vendor = VENDORS.UNKNOWN
}


local dprint = function() end
local dprint2 = function() end
local function resetDebugLevel()
    if default_settings.debug_level > debug_level.DISABLED then
        dprint = function(...)
            info(table.concat({"Lua: ", ...}," "))
        end

        if default_settings.debug_level > debug_level.LEVEL_1 then
            dprint2 = dprint
        end
    else
        dprint = function() end
        dprint2 = dprint
    end
end
-- call it now
resetDebugLevel()

----------------------------------------
-- individual packet parsers:

--[[
 * Dissects the Operation Request Packet specified in [1] Section 2.3.6

 	Format according to [5] in plain PTP would be

 	9.3.2 Operation Request Phase
	Field  Data Type
	OperationCode    UINT16
	SessionID        UINT32
	TransactionID    UINT32 <--- tvb starts here
	Parameter[n]     Any [4 bytes]

	with n = 0..5

--]]

function dissect_operation_request(tvb, pinfo, tree, length)
    -- local opcode = tvb(0,2):le_uint()

    local code = parent_code_field()()
	local proto_title = 'MTP Operation'
	local opcode_desc = mtp_lookup_tables['OperationCodeDescriptions'][code]

	if opcode_desc then proto_title = 'MTP: ' .. opcode_desc end

	local subtree = tree:add(ptp_proto,tvb(), proto_title )
	if not opcode_desc then
        dprint( 'Unknown opcode' .. code )
        opcode_desc = 'Unknown'
    end

    local label =  'OpCode: ' .. string.format( '0x%04X (', code ) .. opcode_desc .. ')'
    subtree:add( hdr_fields.opcode, code, label)

    -- insert the transaction type in the lookup table
    local transaction_id = tvb(0,4):le_uint()
    mtp_transaction_opcodes[transaction_id] = code
    mtp_transaction_opdescriptions[transaction_id] = opcode_desc
    add_transaction_description(tvb, subtree)

    -- parameters:
    local parameter_names = mtp_lookup_tables['RequestParameters'][code]
	local param_type_name = add_parameter_description(tvb, pinfo, subtree, length, parameter_names)
    if param_type_name then mtp_transaction_opdescriptions[transaction_id] = opcode_desc .. ' : ' .. param_type_name end
end

--[[
 * Dissects the Operation Response Packet specified in [1] Section 2.3.7

 	Format according to [5] in plain PTP would be

 	9.3.5 Operation Request Phase
	Field  Data Type
	ResponseCode     UINT16
	SessionID        UINT32
	TransactionID    UINT32 <-- here our tvb starts
	Parameter[n]     Any [4 bytes]
	with n = 0..5

--]]

function dissect_operation_response(tvb, pinfo, tree, length)
    -- local code_tvb = tvb(0,2)
    -- local code = code_tvb:le_uint()
    local code = parent_code_field()()
    local code_desc = mtp_lookup_tables['ResponseCodeDescriptions'][code]

	local response_header = code_desc
	if not response_header then response_header = string.format( '0x%04x', code ) end

	local subtree = tree:add( ptp_proto, tvb(), "MTP Response: " .. response_header )

	if code_desc then
		subtree:add( hdr_fields.response_code, code, "Code: " .. code_desc .. string.format( ' (0x%04x)', code ) )
	else
		dprint( 'Unknown opcode' .. code )
		subtree:add(hdr_fields.response_code, code)
	end

    add_transaction_description(tvb, subtree)

    local parameter_names = mtp_lookup_tables['RequestParameters'][code]
    add_parameter_description(tvb,pinfo,subtree, length, parameter_names)
end

function dissect_event(tvb, pinfo, tree, length)
    local code = parent_code_field()()
    local code_desc = mtp_lookup_tables['EventCodeDescriptions'][code]

	local response_header = code_desc
	if not response_header then response_header = string.format( '0x%04x', code ) end

	local subtree = tree:add( ptp_proto, tvb(), "MTP Event: " .. response_header )

	if code_desc then
		subtree:add( hdr_fields.event_code, code, "Code: " .. code_desc .. string.format( ' (0x%04x)', code ) )
	else
		dprint( 'Unknown opcode' .. code )
		subtree:add(hdr_fields.event_code, code)
	end

    local transaction_id = tvb(0,4):le_uint()
    mtp_transaction_opcodes[transaction_id] = code
    mtp_transaction_opdescriptions[transaction_id] = code_desc
    add_transaction_description(tvb, subtree)

    local parameter_names -- = mtp_lookup_tables['EventParameters'][code]
    add_parameter_description(tvb,pinfo,subtree, length, parameter_names)
end

function dissect_none(tvb, pinfo, tree, length)
    local subtree = tree:add( ptp_proto, tvb(), "MTP Packet: "  )
    add_transaction_description(tvb, subtree)
end

function dissect_start_data(tvb, pinfo, tree, length)
    dissect_none(tvb,pinfo, tree, length)
end

function dissect_end_data(tvb, pinfo, tree, length)

    local data_tvb = tvb( 4 )
    local data_description

    -- redundant description as convenience, it helps reading lengthy get/set property logs
    local transaction_id = tvb(0,4):le_uint()
    local data_description = mtp_transaction_opdescriptions[transaction_id] or ''
    if data_tvb:len() <= 8 then data_description = data_description .. '=> ' .. tostring( data_tvb ) end

    local subtree = tree:add( ptp_proto, tvb(), "Data Packet: " .. data_description  )
    add_transaction_description(tvb, subtree)

    subtree:add( hdr_fields.data, tvb( 4 ) )
end

-- Only the following packet types are part of MTP
local MTP_DISSECTORS = {
    [PTPIP_PACKETTYPE.CMD_REQUEST]=      dissect_operation_request,
    [PTPIP_PACKETTYPE.CMD_RESPONSE]=     dissect_operation_response,
    [PTPIP_PACKETTYPE.EVENT]=            dissect_event,
    [PTPIP_PACKETTYPE.START_DATA_PACKET]=dissect_start_data,
    [PTPIP_PACKETTYPE.DATA_PACKET]=     dissect_none,
    [PTPIP_PACKETTYPE.CANCEL_TRANSACTION]= dissect_none,
    [PTPIP_PACKETTYPE.END_DATA_PACKET]=  dissect_end_data,
}

----------------------------------------


-- create a function to dissect packets

function ptp_proto.dissector(tvb,pinfo,tree)

   -- pinfo.cols.protocol = "PTP"
   local packet_type = parent_packet_type_field()()
   local ptp_length = parent_length_field()()
   local offset_field = parent_header_offset_field()
   local offset
   if offset_field then offset = offset_field() end

   -- dprint2( "Found packet:" .. packet_type .. ' l:' .. ptp_length .. ' o:' .. offset .. ' r:' .. ptp_length - offset)
   if not offset or not ptp_length or not packet_type then
   		dprint( 'Missing field in dissection' )
   		return
   end

   if offset > ptp_length then
   		dprint( 'PTP data size problem' )
   		return
   end

   local ptp_tvb =  tvb(offset)
   local remaining_length = ptp_length - offset

   local handler = MTP_DISSECTORS[packet_type]
   if handler then handler(  ptp_tvb, pinfo, tree, remaining_length ) end
end


mtp_table = DissectorTable.new("ptp.data", "MTP Protocol", ftypes.STRING )
-- register our protocol
mtp_table:add('ptp',ptp_proto)

function add_transaction_description(tvb, subtree)
    local sub_tvb = tvb(0,4)
    local transaction_id = sub_tvb:le_uint()
    local transaction_desc = mtp_transaction_opdescriptions[transaction_id]
    if transaction_desc then
        subtree:add(hdr_fields.transaction_id, sub_tvb, transaction_id, string.format( 'TransactionID: 0x%04X (', transaction_id ) .. transaction_desc .. ')' )
    else
	    subtree:add_le(hdr_fields.transaction_id, sub_tvb)
    end
end

local param_fields = {
	[0] = hdr_fields.param1,
	[1] = hdr_fields.param2,
	[2] = hdr_fields.param3,
	[3] = hdr_fields.param4,
	[4] = hdr_fields.param5,
}

function add_parameter_description(tvb,pinfo,tree, length, parameter_names)
    local start_offset = 4
	local remainder = length - start_offset
	local parameter_count = remainder / 4
    local parameter_name
    local command_description -- return description of PropCode types
	-- dprint2( parameter_count .. " Parameters" )
	if parameter_count >  0 then
		parameter_count = math.min(parameter_count, 5)
		for i = 0, parameter_count - 1 do
            local param_tvb =  tvb(start_offset + (4 * i), 4)
            local param_value = param_tvb:le_uint()

            if parameter_names then parameter_name = parameter_names[i + 1] end
            if parameter_name then
                parameter_name = parameter_name:gsub('%[','')
                parameter_name = parameter_name:gsub('%]','')

                local param_description_table = mtp_lookup_tables[parameter_name]
                local param_description
                if param_description_table then
                    param_description = param_description_table[param_value]
                end
                local label =  parameter_name .. ': ' .. string.format( '0x%04X ', param_value )
                if param_description then
                    label = label .. '(' .. param_description ..')'
                end

                -- determine if any propcode is in the parameter name - we use these parameters also for describing the transactionID
                if parameter_name and string.find( parameter_name, 'PropCode') then
                    if param_description then
                        command_description = param_description
                    else
                        command_description = string.format( '0x%04X ', param_value )
                    end
                end

                tree:add( param_fields[i], param_tvb, param_value, label )
            else
			    tree:add_le( param_fields[i], param_tvb )
            end

		end
    end
    return command_description
end

--------------------------------------------------------------------------------
-- preferences handling stuff
--------------------------------------------------------------------------------

function combine_lookup(table1, table2)
    -- iterate all keys in
    local result = {}
    local key, value, k, v

    for key, value in pairs(table1) do
        if table2[key] then
            local combined = {}
            for k,v in pairs(table1[key]) do combined[k] = v end
            for k,v in pairs(table2[key]) do combined[k] = v end
            result[key] = combined
        else
            result[key] = table1[key]
        end
    end
    return result
end

local debug_pref_enum = {
    { 1,  "Disabled", debug_level.DISABLED },
    { 2,  "Level 1",  debug_level.LEVEL_1  },
    { 3,  "Level 2",  debug_level.LEVEL_2  },
}


----------------------------------------
-- register our preferences

ptp_proto.prefs.debug       = Pref.enum("Debug", default_settings.debug_level,
                                        "The debug printing level", debug_pref_enum)
ptp_proto.prefs.vendor      = Pref.enum("Camera vendor", default_settings.camera_vendor, "Properly translates vendor specific opcodes", vendor_pref_enum)

----------------------------------------
-- the function for handling preferences being changed
function ptp_proto.prefs_changed()
    dprint2("prefs_changed called")

	default_settings.camera_vendor = ptp_proto.prefs.vendor
    default_settings.debug_level = ptp_proto.prefs.debug
    resetDebugLevel()

    mtp_lookup_tables = combine_lookup( mtp_lookup, VENDOR_EXTENSIONS[default_settings.camera_vendor] )
end