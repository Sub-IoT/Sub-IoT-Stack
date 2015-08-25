oss7_proto = Proto("oss7_proto", "DASH7 Alliance Protocol DRAFT 1.0 test")

local f = oss7_proto.fields

f.start = ProtoField.uint32("oss7_proto.start", "Start")
f.test = ProtoField.uint32("oss7_proto.debug_var", "Debug Variable")

f.packet_type = ProtoField.uint32("oss7_proto.packet_type", "Packet Type")
f.timestamp = ProtoField.uint32("oss7_proto.timestamp", "Timestamp")
f.channel_header_raw = ProtoField.uint8("oss7_proto.channel_header_raw", "Raw Channel Header")
f.center_freq_index = ProtoField.uint16("oss7_proto.center_freq_index", "Center Frequency Index")
f.syncword_class = ProtoField.uint8("oss7_proto.syncword_class", "Syncword Class", base.HEX)

f.crc = ProtoField.uint16("oss7_proto.crc", "CRC", base.HEX)

-- TX only
f.tx_eirp = ProtoField.uint8("oss7_proto.dll.control.tx_eirp", "TX EIRP")

-- RX only
f.rssi = ProtoField.int16("oss7_proto.rssi", "RSSI")
f.lqi = ProtoField.uint8("oss7_proto.lqi", "Link Quality Indicator")

f.dll_length = ProtoField.uint8("oss7_proto.dll.length", "Length")
f.dll_subnet = ProtoField.uint8("oss7_proto.dll.subnet", "Subnet", base.HEX)
f.dll_control = ProtoField.uint8("oss7_proto.dll.control", "Control", base.HEX)
f.dll_tadr = ProtoField.bytes("oss7_proto.dll.tadr", "Target Address")
f.dll_payload = ProtoField.bytes("oss7_proto.dll.payload", "Payload")
f.dll_crc = ProtoField.uint16("oss7_proto.dll.crc", "CRC", base.HEX)

f.dll_control_tgt = ProtoField.bool("oss7_proto.dll.control.tgt", "Target Address Present")
f.dll_control_vid = ProtoField.bool("oss7_proto.dll.control.vid", "Virtual ID")
f.dll_control_eirp = ProtoField.uint8("oss7_proto.dll.control.control_eirp", "CONTROL EIRP")

f.nwl_control = ProtoField.uint8("oss7_proto.nwl.control", "Control", base.HEX)
f.nwl_payload = ProtoField.bytes("oss7_proto.nwl.payload", "Payload")

--optional
f.nwl_hopping_control = ProtoField.bytes("oss7_proto.nwl.hopping_control", "Hopping Control")
f.nwl_intermediary_access_id = ProtoField.bytes("oss7_proto.nwl.intermediary_access_id", "Intermediary Access ID")
f.nwl_destination_access_id = ProtoField.bytes("oss7_proto.nwl.destination_access_id", "Destination Access ID")
f.nwl_origin_access_id = ProtoField.bytes("oss7_proto.nwl.origin_access_id", "Origin Access ID")
f.nwl_security_header = ProtoField.bytes("oss7_proto.nwl.security_hdr", "Security Header")
f.nwl_auth_data = ProtoField.bytes("oss7_proto.nwl.auth_data", "Authentication Data")

f.nwl_ctrl_nls = ProtoField.bool("oss7_proto.nwl.control.nls", "Network Layer Security Present")
f.nwl_ctrl_hop = ProtoField.bool("oss7_proto.nwl.control.hop", "Multi-hop enable")
f.nwl_ctrl_ori_id = ProtoField.bool("oss7_proto.nwl.control.ori_id", "Origin Access ID Present")
f.nwl_ctrl_ori_vid = ProtoField.bool("oss7_proto.nwl.control.ori_vid", "Origin Access ID Contains VID")
f.nwl_ctrl_ori_cl = ProtoField.uint8("oss7_proto.nwl.control.ori_cl", "Origin Access Class")

--optional
f.nwl_hctrl_rfu = ProtoField.uint8("oss7_proto.nwl.hoppingcontrol.rfu", "RFU")
f.nwl_hctrl_nrhop = ProtoField.bool("oss7_proto.nwl.hoppingcontrol.nrhop", "Hopping Counter (no-hop & one-hop)")
f.nwl_hctrl_int_id = ProtoField.bool("oss7_proto.nwl.hoppingcontrol.int_id", "Intermediary Access ID Present")
f.nwl_hctrl_int_vid = ProtoField.bool("oss7_proto.nwl.hoppingcontrol.int_vid", "Intermediary Access ID is VID")
f.nwl_hctrl_int_cl = ProtoField.uint8("oss7_proto.nwl.hoppingcontrol.int_cl", "Intermediary Access Class")
f.nwl_hctrl_dst_id = ProtoField.bool("oss7_proto.nwl.hoppingcontrol.dst_id", "Destination Access ID Present")
f.nwl_hctrl_dst_id = ProtoField.bool("oss7_proto.nwl.hoppingcontrol.dst_vid", "Destination Access ID is VID")
f.nwl_hctrl_dst_cl = ProtoField.uint8("oss7_proto.nwl.hoppingcontrol.dst_cl", "Destination Access Class")

--TODO: Security Header & Footer

--optional (initizalization vector)
f.nwl_auth_nlcf = ProtoField.uint8("oss7_proto.nwl.auth.nlcf", "Network Layer Control Field")
f.nwl_auth_origin_address = ProtoField.bytes("oss7_proto.nwl.auth.origin_address", "Origin Address")
f.nwl_auth_frame_counter = ProtoField.bytes("oss7_proto.nwl.auth.frame_counter", "Frame Counter")
f.nwl_auth_key_counter = ProtoField.uint8("oss7_proto.nwl.auth.key_counter", "Key Counter")
f.nwl_auth_payload_length = ProtoField.uint8("oss7_proto.nwl.auth.payload_length", "Payload Length")
f.nwl_auth_payload_destination_address = ProtoField.uint8("oss7_proto.nwl.auth.destination_address", "XOR'ed Destination Address or Subnet")

f.trans_ctrl = ProtoField.uint8("oss7_proto.trans.ctrl", "Control Field", base.HEX)
f.trans_dialog_id = ProtoField.uint8("oss7_proto.trans.dialog_id", "Dialog ID", base.HEX)
f.trans_tran_id = ProtoField.uint8("oss7_proto.trans.tran_id", "Transaction ID", base.HEX)
f.trans_timeout_templ = ProtoField.uint8("oss7_proto.trans.timeout_templ", "Timeout Template")
f.trans_ack_templ = ProtoField.bytes("oss7_proto.trans.ack_templ", "Acknowledge Template")
f.trans_payload= ProtoField.bytes("oss7_proto.trans.payload", "Payload")

f.trans_ctrl_start = ProtoField.bool("oss7_proto.trans.ctrl.start", "Request New Dialog")
f.trans_ctrl_stop = ProtoField.bool("oss7_proto.trans.ctrl.stop", "Last Transaction of Dialog")
f.trans_ctrl_timeout = ProtoField.bool("oss7_proto.trans.ctrl.timeout", "Timeout Template Present")
f.trans_ctrl_rfu = ProtoField.bool("oss7_proto.trans.ctrl.rfu", "RFU")
f.trans_ctrl_ack_req = ProtoField.bool("oss7_proto.trans.ctrl.ack_req", "Acknowledge Return Template Requested")
f.trans_ctrl_ack_not_void = ProtoField.bool("oss7_proto.trans.ctrl.ack_not_void", "ACK Bitmap Non-empty")
f.trans_ctrl_ack_record = ProtoField.bool("oss7_proto.trans.ctrl.ack_record", "Record ACK Bitmap")
f.trans_ctrl_ack_tpl = ProtoField.bool("oss7_proto.trans.ctrl.ack_tpl", "Acknowledge Return Template Present")

function oss7_proto.dissector(buffer,pinfo,tree)
	pinfo.cols.protocol = "DASH7 Alliance Protocol DRAFT 1.0"

	-- Parse extra bytes from phy_rx -> (rssi and lqi)
	local subtree = tree:add(oss7_proto, buffer())
	local offset = 0
	local type = 0
		
	-- DLL subtree
	local dll_subtree = subtree:add("Data Link Layer", nil)
	-- Length
	dll_subtree:add(f.dll_length, buffer(offset, 1):int())
	local payload_length = buffer(offset, 1):int() + 1 - 2 -- headers will be removed later
	
	offset = offset + 1
	-- Subnet
	dll_subtree:add(f.dll_subnet, buffer(offset, 1))	
	offset = offset + 1	
	-- Control
	dll_subtree:add(f.dll_control, buffer(offset, 1))
	
	local dll_control_subtree = dll_subtree:add("DLL Control", nil)
	dll_control_subtree:add(f.dll_control_tgt, buffer(offset, 1):uint() / 128  )	
	dll_control_subtree:add(f.dll_control_vid, buffer(offset, 1):uint() % 128 / 64 )
	dll_control_subtree:add(f.dll_control_eirp, buffer(offset, 1):uint() % 64 ):append_text(" dBm") -- % 64) - 32	
	
	offset = offset + 1
	
	if f.dll_control_tgt  == 1 then
		if f.dll_control_vid  == 1 then
			dll_subtree:add(f.dll_tadr, buffer(offset, 2)) -- 2 & 8 switched around?
			offset = offset + 2
		else			
			dll_subtree:add(f.dll_tadr, buffer(offset, 8))
			offset = offset + 8
		end		
		pinfo.cols.dst = f.dll_tadr
	else
		pinfo.cols.dst = "BROADCAST"
	end
	
	
	-- length of payload is the length of the packet - 2 (CRC) - (the already parsed bytes (offset) - the RX header (3))
	payload_length = payload_length - offset -- - 3) - 1 -- removed -1
	
	local payload = buffer(offset, payload_length)
	--dll_subtree:add(f.dll_payload, payload)
	offset = offset + payload_length
	
	local crc = buffer(offset, 2)
	 
	-- todo check CRC
	
		-- Network Layer
	local nwl_subtree = dll_subtree:add("Network Layer", nil)
	local nwl_offset = 0
		
	nwl_subtree:add(f.nwl_control, payload(nwl_offset, 1))
	nwl_offset = nwl_offset + 1
	
	local nwl_control_subtree = nwl_subtree:add("NWL Control", nil)
	nwl_control_subtree:add(f.nwl_ctrl_nls, payload(nwl_offset, 1):uint() / 128  )	
  nwl_control_subtree:add(f.nwl_ctrl_hop, payload(nwl_offset, 1):uint() % 128 / 64  )
    
  local nwl_ctrl_ori_id = payload(nwl_offset, 1):uint() % 64 / 32
  nwl_control_subtree:add(f.nwl_ctrl_ori_id, nwl_ctrl_ori_id  )
    
  local nwl_ctrl_ori_vid = payload(nwl_offset, 1):uint() % 32 / 16
  nwl_control_subtree:add(f.nwl_ctrl_ori_vid, nwl_ctrl_ori_vid  )
    
  nwl_control_subtree:add(f.nwl_ctrl_ori_cl, payload(nwl_offset, 1):uint() % 16  )
   
  -- for some reason, rounding is necessary here
  nwl_ctrl_ori_id = math.floor(nwl_ctrl_ori_id + 0.5)
  nwl_ctrl_ori_vid = math.floor(nwl_ctrl_ori_vid + 0.5)
           
  if nwl_ctrl_ori_id == 1 then
     nwl_subtree:add(f.test, 4)
    
    if nwl_ctrl_ori_vid == 1 then
       nwl_subtree:add(f.nwl_origin_access_id, payload(nwl_offset, 2))
       nwl_offset = nwl_offset + 2
    else
       nwl_subtree:add(f.nwl_origin_access_id, payload(nwl_offset, 8))
       nwl_offset = nwl_offset + 8
    end
      
  end
    
    --todo: optional nwl
		
	payload_length = payload_length - nwl_offset
		
  	local nwl_payload = payload(nwl_offset, payload_length)
--	nwl_subtree:add(f.nwl_payload, nwl_payload)
	
	-- Transport Layer
	
	local trans_subtree = nwl_subtree:add("Transport Layer", nil)
	local trans_offset = 0
		
	trans_subtree:add(f.trans_ctrl, nwl_payload(trans_offset, 1))		
		
	local trans_ctrl_subtree = trans_subtree:add("Control", nil)
	trans_ctrl_subtree:add(f.trans_ctrl_start, nwl_payload(trans_offset, 1):uint() / 128)	
  trans_ctrl_subtree:add(f.trans_ctrl_stop, nwl_payload(trans_offset, 1):uint() % 128 / 64) 
  trans_ctrl_subtree:add(f.trans_ctrl_timeout, nwl_payload(trans_offset, 1):uint() % 64 / 32) 
  trans_ctrl_subtree:add(f.trans_ctrl_rfu, nwl_payload(trans_offset, 1):uint() % 32 / 16) 
  trans_ctrl_subtree:add(f.trans_ctrl_ack_req, nwl_payload(trans_offset, 1):uint() % 16 / 8) 
  trans_ctrl_subtree:add(f.trans_ctrl_ack_not_void, nwl_payload(trans_offset, 1):uint() % 8 / 4) 
  trans_ctrl_subtree:add(f.trans_ctrl_ack_record, nwl_payload(trans_offset, 1):uint() % 4 / 2) 
  trans_ctrl_subtree:add(f.trans_ctrl_ack_tpl, nwl_payload(trans_offset, 1):uint() % 2) 

	trans_offset = trans_offset + 1
		
	trans_subtree:add(f.trans_dialog_id, nwl_payload(trans_offset, 1))
  trans_offset = trans_offset + 1
		
	trans_subtree:add(f.trans_tran_id, nwl_payload(trans_offset, 1))
	trans_offset = trans_offset + 1
		
	if f.trans_ctrl_timeout == 1 then
    trans_subtree:add(f.trans_timeout_templ, nwl_payload(trans_offset, 1))
    trans_offset = trans_offset + 1		  
	end

	payload_length = payload_length - trans_offset
	
	local trans_payload = nwl_payload(trans_offset, payload_length)
	
	trans_subtree:add(f.trans_payload,trans_payload)
	dll_subtree:add(f.crc, crc)
	

end

wtap_encap = DissectorTable.get("wtap_encap")
wtap_encap:add(wtap.USER0, oss7_proto)
