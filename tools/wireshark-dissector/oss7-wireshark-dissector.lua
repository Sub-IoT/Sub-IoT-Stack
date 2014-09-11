oss7_proto = Proto("oss7_proto", "DASH7 Alliance Protocol DRAFT 1.0")

local f = oss7_proto.fields



f.rssi = ProtoField.int16("oss7_proto.rssi", "RSSI")
f.lqi = ProtoField.uint8("oss7_proto.lqi", "Link Quality Indicator")

f.length = ProtoField.uint8("oss7_proto.dll.length", "Length")
f.subnet = ProtoField.uint8("oss7_proto.dll.subnet", "Subnet", base.HEX)
f.control = ProtoField.uint8("oss7_proto.dll.control", "Control", base.HEX)
f.control_tgt = ProtoField.bool("oss7_proto.dll.control.tgt", "Target Address Present")
f.control_vid = ProtoField.bool("oss7_proto.dll.control.vid", "Virtual ID")
f.tx_eirp = ProtoField.uint8("oss7_proto.dll.control.tx_eirp", "TX EIRP")
f.tadr = ProtoField.bytes("oss7_proto.dll.tadr", "Target Address")
f.payload = ProtoField.bytes("oss7_proto.dll.payload", "Payload")
f.crc = ProtoField.uint16("oss7_proto.dll.crc", "CRC", base.HEX)


f.nwl_control = ProtoField.uint8("oss7_proto.nwl.control", "Control", base.HEX)
f.nwl_security_header = ProtoField.bytes("oss7_proto.nwl.security_hdr", "Security Header")
f.nwl_source_access_template  = ProtoField.string("oss7_proto.nwl.access_tmpl", "Access Template")
f.nwl_payload = ProtoField.bytes("oss7_proto.nwl.payload", "Payload")
f.nwl_auth_data = ProtoField.bytes("oss7_proto.nwl.auth_data", "Authentication Data")

f.nwl_ctrl_nls = ProtoField.bool("oss7_proto.nwl.control.nls", "Network Layer Security Present")
f.nwl_ctrl_ext = ProtoField.bool("oss7_proto.nwl.control.ext", "Extension Bit")
f.nwl_ctrl_cfg = ProtoField.uint8("oss7_proto.nwl.control.cfg", "Routing Configuration")
f.nwl_ctrl_src = ProtoField.uint8("oss7_proto.nwl.control.src", "Source Information")
f.nwl_access_device_id = ProtoField.bytes("oss7_proto.nwl.auth_data", "Access Device ID")

f.trans_command_code = ProtoField.uint8("oss7_proto.trans.command_code", "Command Code", base.HEX)
f.trans_tran_id = ProtoField.uint8("oss7_proto.trans.tran_id", "Transaction ID", base.HEX)
f.trans_query_templ = ProtoField.bytes("oss7_proto.trans.query_templ", "Query Template")
f.trans_ack_templ = ProtoField.bytes("oss7_proto.trans.ack_templ", "Acknowledge Template")
f.trans_alp_data = ProtoField.bytes("oss7_proto.trans.alp_data", "ALP Data")

f.trans_cc_dialog = ProtoField.uint8("oss7_proto.trans.cc.dialog", "Dialog")
f.trans_cc_ordered = ProtoField.bool("oss7_proto.trans.cc.ordered", "Ordered")
f.trans_cc_ack_req = ProtoField.bool("oss7_proto.trans.cc.ack_req", "ACK Request")
f.trans_cc_nack_only = ProtoField.bool("oss7_proto.trans.cc.nack_only", "NACK Only")
f.trans_cc_query = ProtoField.bool("oss7_proto.trans.cc.query", "Query Template")
f.trans_cc_ack_tpl = ProtoField.bool("oss7_proto.trans.cc.ack_tpl", "Acknowledge Return Template")

local query_logical_codes = {
	[0] = "OR",
	[1] = "AND",
	[2] = "XOR"
}

f.trans_query_nr_of_queries =  ProtoField.uint8("oss7_proto.trans.query.nr", "Number of Unitary Queries")
f.trans_query_logical_code =  ProtoField.uint8("oss7_proto.trans.query.logical_code", "Logical Code")
f.trans_query_file_id = ProtoField.uint8("oss7_proto.trans.query.file_id", "File ID")
f.trans_query_file_offset = ProtoField.uint8("oss7_proto.trans.query.file_offset", "File Offset")
f.trans_query_file_compare_length = ProtoField.uint8("oss7_proto.trans.query.compare_length", "Compare Length")
f.trans_query_file_compare_code = ProtoField.uint8("oss7_proto.trans.query.compare_code", "Compare Code")
f.trans_query_file_compare_mask = ProtoField.uint8("oss7_proto.trans.query.compare_mask", "Compare Mask")
f.trans_query_file_compare_value = ProtoField.uint8("oss7_proto.trans.query.compare_value", "Compare Value")

local query_cc_types = {
	[0] = "Non-null check",
	[1] = "Arithmetic comparison",
	[2] = "String token search",
	[3] = "Proprietary/Custom"
}

f.trans_query_file_compare_code_masked = ProtoField.bool("oss7_proto.trans.query.cc.masked", "Masked")
f.trans_query_file_compare_code_type = ProtoField.uint8("oss7_proto.trans.query.cc.type", "Type", base.HEX, query_cc_types)
f.trans_query_file_compare_code_params = ProtoField.bool("oss7_proto.trans.query.cc.params", "Params, base.HEX")

f.alp_record_flags = ProtoField.uint8("oss7_proto.alp.record_flags", "Record Flags", base.HEX)
f.alp_record_length = ProtoField.uint8("oss7_proto.alp.record_lenght", "Record Length")
f.alp_id = ProtoField.uint8("oss7_proto.alp.id", "ALP ID")
f.alp_templates = ProtoField.bytes("oss7_proto.alp.templates", "ALP Templates")

f.alp_type = ProtoField.string("oss7_proto.alp.type", "ALP Template Type")

local alp_flag_chunk_ctrls = {
    [0] = "Intermediary packet of a chunked message",
    [1] = "First packet of a chunked message",
    [2] = "Last packet of a chunked message",
    [3] = "ALP packet is not chunked"
}

local alp_flag_types = {
	[0] = "Response message",
	[1] = "Unsolicited response type message",
	[2] = "Command with no response",
	[3] = "Command with required response"
}

f.alp_flag_chunk_ctrl = ProtoField.uint8("oss7_proto.alp.flag.chunk_ctrl", "Chunk Control", base.DEC, alp_flag_chunk_ctrls)
f.alp_flag_type = ProtoField.uint8("oss7_proto.alp.flag.type", "Type", base.DEC, alp_flag_types)
f.alp_flag_grouped = ProtoField.uint8("oss7_proto.alp.flag.grouped", "Grouped", base.DEC)

local alp_ops = {
	[0] = "Read File Data",
	[1] = "Read File Header",
	[2] = "Read File Header + Data",
	[4] = "Write File Data",
	[5] = "Write File Data Flush",
	[6] = "Write File Properties",
	[16] = "Exist File",
	[17] = "Create New File",
	[18] = "Delete File",
	[19] = "Restore File",
	[20] = "Flush File",
	[21] = "Open File",
	[22] = "Close File",
	[32] = "Return File Data",
	[33] = "Return File Header",
	[34] = "Return File Header + Data",
	[255] = "Return Error"
}

f.alp_op = ProtoField.uint8("oss7_proto.alp.template.op", "ALP OP", base.DEC, alp_ops)
f.alp_record_data = ProtoField.bytes("oss7_proto.alp.template.record_data", "ALP Record Data")

f.file_data_tmpl_id = ProtoField.uint8("oss7_proto.alp.file_data_template_id", "File ID")
f.file_data_start_byte_offset = ProtoField.uint16("oss7_proto.alp.file_data_template_start_byte_offset", "Start Byte Offset")
f.file_data_bytes_accessing = ProtoField.uint16("oss7_proto.alp.file_data_template_bytes_accessing", "Bytes Accessing")
f.file_data = ProtoField.bytes("oss7_proto.alp.file_data", "File Data")
f.file_header = ProtoField.bytes("oss7_proto.alp.file_header", "File Header Data")

local file_error_codes = {
	[0x00] = "No Error, OK Response",
	[0x01] = "Cannot access file: File ID does not exist",
	[0x02] = "Cannot create file: File ID already exists",
	[0x03] = "Cannot restore file: File is not restorable",
	[0x04] = "Cannot access file: Insufficient permissions",
	[0x05] = "Cannot create file: Supplied length (in header) is beyond file limits",
	[0x06] = "Cannot create file: Supplied allocation (in header) is beyond file limits",
	[0x07] = "Cannot write: Supplied start offset is out of bounds of file allocation",
	[0x08] = "Cannot complete write: Supplied data goes beyond file allocation",
	[0xFF] = "Unkown Error"
}

f.file_error_templ_id = ProtoField.uint8("oss7_proto.alp.file_error_template_id", "Erroneous File ID")
f.file_error_templ_error = ProtoField.uint8("oss7_proto.alp.file_error_template_error_code", "File Error Code", base.HEX, file_error_codes)

function parse_file_data_template(buffer,pointer,tree)	
	tree:add(f.alp_type, "File Data Template")
	tree:add(f.file_data_tmpl_id, buffer(pointer, 1))
	tree:add(f.file_data_start_byte_offset, buffer(pointer + 1, 2))
	local bytes_accessing = buffer(pointer + 3, 2):uint()
	tree:add(f.file_data_bytes_accessing, bytes_accessing)
	tree:add(f.file_data, buffer(pointer + 5, bytes_accessing))
	pointer = pointer + 5 + bytes_accessing
	return pointer
end

function parse_file_header_template(buffer,pointer,tree)
	tree:add(f.alp_type, "File Header Template")
	tree:add(f.file_data_tmpl_id, buffer(pointer, 1))
	tree:add(f.file_data_start_byte_offset, buffer(pointer + 1, 1))
	local bytes_accessing = buffer(pointer + 2, 1):uint()
	tree:add(f.file_data_bytes_accessing, bytes_accessing)
	tree:add(f.file_header, buffer(pointer + 3, bytes_accessing))
	pointer = pointer + 3 + bytes_accessing
	return pointer
end

function parse_file_header_data_template(buffer,pointer,tree)	
	tree:add(f.alp_type, "File Header + Data Template")
	tree:add(f.file_data_tmpl_id, buffer(pointer, 1))
	local start_byte = buffer(pointer + 1, 2)
	tree:add(f.file_data_start_byte_offset, start_byte)
	local bytes_accessing = buffer(pointer + 3, 2):uint()
	tree:add(f.file_data_bytes_accessing, bytes_accessing)
	tree:add(f.file_header, buffer(pointer + 5, start_byte))
	tree:add(f.file_data, buffer(pointer + 5 + start_byte, bytes_accessing - start_byte))
	pointer = pointer + 5 + bytes_accessing
	return pointer
end

function parse_file_id_template(buffer,pointer,tree)
	tree:add(f.alp_type, "File ID Template")
	tree:add(f.file_data_tmpl_id, buffer(pointer, 1))
	pointer = pointer + 1
	return pointer
end

function parse_file_error_template(buffer,pointer,tree)
	tree:add(f.alp_type, "File Error Template")
	tree:add(f.file_data_tmpl_id, buffer(pointer, 1))
	pointer = pointer + 1
	return pointer
end


function oss7_proto.dissector(buffer,pinfo,tree)
	pinfo.cols.protocol = "DASH7 Alliance Protocol DRAFT 1.0"

	-- Parse extra bytes from phy_rx -> (rssi and lqi)
	local subtree = tree:add(oss7_proto, buffer())
	local offset = 0 
	subtree:add(f.rssi, buffer(offset, 1)):append_text(" dBm")
	offset = offset + 1
	subtree:add(f.lqi, buffer(offset, 1))
	offset = offset + 1
	
	-- skip lenght in header
	offset = offset + 1
	
	
	-- DLL subtree
	local dll_subtree = subtree:add("Data Link Layer", nil)
	-- Length
	dll_subtree:add(f.length, buffer(offset, 1):int())
	local payload_length = buffer(offset, 1):int() - 2 -- headers will be removed later
	offset = offset + 1
	-- Subnet
	dll_subtree:add(f.subnet, buffer(offset, 1))	
	offset = offset + 1	
	-- Control
	dll_subtree:add(f.control, buffer(offset, 1))
	
		local dll_control_subtree = dll_subtree:add("DLL Control", nil)
		dll_control_subtree:add(f.control_tgt, buffer(offset, 1):uint() / 128  )	
		dll_control_subtree:add(f.control_vid, buffer(offset, 1):uint() % 128 / 64 )
		dll_control_subtree:add(f.tx_eirp, (buffer(offset, 1):uint() % 64) - 32 ):append_text(" dBm")	
	
	offset = offset + 1
	
	if f.control_tgt  == 1 then
		if f.vid  == 1 then
			dll_subtree:add(f.tadr, buffer(offset, 2))
			offset = offset + 8
		else			
			dll_subtree:add(f.tadr, buffer(offset, 8))
			offset = offset + 2
		end		
		pinfo.cols.dst = f.tadr
	else
		pinfo.cols.dst = "BROADCAST"
	end
	
	
	-- length of payload is the length of the packet - 2 (CRC) - (the already parsed bytes (offset) - the RX header (3))
	payload_length = payload_length - (offset - 3)
	local payload = buffer(offset, payload_length)
	dll_subtree:add(f.payload, payload)
	offset = offset + payload_length
	
	dll_subtree:add(f.crc, buffer(offset, 2))
	 
	-- todo check CRC
	
		-- Network Layer
		local nwl_subtree = subtree:add("Network Layer", nil)
		local nwl_offset = 0
		
		nwl_subtree:add(f.nwl_control, payload(nwl_offset, 1))
	
			local nwl_control_subtree = nwl_subtree:add("NWL Control", nil)
			nwl_control_subtree:add(f.nwl_ctrl_nls, payload(nwl_offset, 1):uint() / 128  )	
			nwl_control_subtree:add(f.nwl_ctrl_ext, payload(nwl_offset, 1):uint() % 128 / 64 )			
			nwl_control_subtree:add(f.nwl_ctrl_cfg, payload(nwl_offset, 1):uint() % 64 / 4 )

			local ctrl_src = payload(nwl_offset, 1):uint() % 4
			nwl_control_subtree:add(f.nwl_ctrl_src, payload(nwl_offset, 1):uint() % 4 )
			
		nwl_offset = nwl_offset + 1
		
		if ctrl_src == 1 then	
			nwl_subtree:add(f.nwl_source_access_template, "Short Access Template with UID Address")
			local nwl_access_temp_subtree = nwl_subtree:add("Access Template", nil);
			nwl_access_temp_subtree:add(f.nwl_access_device_id, payload(nwl_offset, 8));
			pinfo.cols.src = tostring(string.format("%s", payload(nwl_offset, 8):bytes()))
			
			nwl_offset = nwl_offset + 8
		elseif ctrl_src == 2 then	
			nwl_subtree:add(f.nwl_source_access_template, "Short Access Template with VID Address")			
			local nwl_access_temp_subtree = nwl_subtree:add("Access Template", nil);
			nwl_access_temp_subtree:add(f.nwl_access_device_id, payload(nwl_offset, 2));
			pinfo.cols.src = tostring(string.format("%s", payload(nwl_offset, 2):bytes()))
			nwl_offset = nwl_offset + 2
		elseif ctrl_src == 3 then	
			nwl_subtree:add(f.nwl_source_access_template, "Full Access Template")			
			
			-- todo
		end
		
		
		payload_length = payload_length - nwl_offset
		local nwl_payload = payload(nwl_offset, payload_length)
		nwl_subtree:add(f.nwl_payload, nwl_payload)
	
		-- Transport Layer
	
		local trans_subtree = subtree:add("Transport Layer", nil)
		local trans_offset = 0
		
		trans_subtree:add(f.trans_command_code, nwl_payload(trans_offset, 1))		
		
			local trans_command_code_subtree = trans_subtree:add("Command Code", nil)
			trans_command_code_subtree:add(f.trans_cc_dialog, nwl_payload(trans_offset, 1):uint() / 64)	
			trans_command_code_subtree:add(f.trans_cc_ordered, nwl_payload(trans_offset, 1):uint() % 64 / 32 )			
			trans_command_code_subtree:add(f.trans_cc_ack_req, nwl_payload(trans_offset, 1):uint() % 32 / 16 )			
			trans_command_code_subtree:add(f.trans_cc_nack_only, nwl_payload(trans_offset, 1):uint() % 16 / 8 )
			
			local trans_query_present = nwl_payload(trans_offset, 1):uint() % 4 / 2
			trans_command_code_subtree:add(f.trans_cc_query, trans_query_present)			
			trans_command_code_subtree:add(f.trans_cc_ack_tpl, nwl_payload(trans_offset, 1):uint() % 2)

		trans_offset = trans_offset + 1
		
		trans_subtree:add(f.trans_tran_id, nwl_payload(trans_offset, 1))
		trans_offset = trans_offset + 1
		
		if trans_query_present == 1 then 
			local nr_of_queries = nwl_payload(trans_offset, 1)
			trans_subtree:add(f.trans_query_nr_of_queries, nr_of_queries)
			trans_subtree:add(f.trans_query_logical_code, nwl_payload(trans_offset + 1, 1))
			trans_offset = trans_offset + 2
			
			for i=0,nr_of_queries,1 do
				trans_subtree:add(f.trans_query_file_id, nwl_payload(trans_offset, 1))
				trans_subtree:add(f.trans_query_file_offset, nwl_payload(trans_offset + 1, 1))
				local query_compare_length = nwl_payload(trans_offset + 2, 1)
				trans_subtree:add(f.trans_query_file_compare_length, query_compare_length)
				local query_compare_code =  nwl_payload(trans_offset + 3, 1):uint()
				trans_subtree:add(f.trans_query_file_compare_code, query_compare_code)
				
				local query_compare_code_subtree = trans_subtree:add("Compare Code", nil)
				local trans_query_file_compare_code_masked = query_compare_code / 128
				query_compare_code_subtree:add(f.trans_query_file_compare_code_masked, trans_query_file_compare_code_masked)	
				query_compare_code_subtree:add(f.trans_query_file_compare_code_type, query_compare_code % 128 / 32 )			
				query_compare_code_subtree:add(f.trans_query_file_compare_code_params, query_compare_code % 32)
				
				trans_offset = trans_offset + 4
				if trans_query_file_compare_code_masked then
					trans_subtree:add(f.trans_query_file_compare_mask, nwl_payload(trans_offset, query_compare_length))
					trans_offset = trans_offset + query_compare_length
				end
				
				trans_subtree:add(f.trans_query_file_compare_mask, nwl_payload(trans_offset, query_compare_length))
				trans_offset = trans_offset + query_compare_length		
				
			end
			
		end
		
		-- todo: ack template
		
		payload_length = payload_length - trans_offset
		local alp_data = nwl_payload(trans_offset, payload_length)
		trans_subtree:add(f.trans_alp_data, alp_data)
		
		-- Application Layer
		
		local alp_subtree = subtree:add("Application Layer Programming Interface", nil)
		local alp_offset = 0
		
		alp_subtree:add(f.alp_record_flags, alp_data(alp_offset, 1))
		
			local alp_flags_subtree = alp_subtree:add("Record Flags", nil)
			alp_flags_subtree:add(f.alp_flag_chunk_ctrl, alp_data(alp_offset, 1):uint() / 64)
			alp_flags_subtree:add(f.alp_flag_type, alp_data(alp_offset, 1):uint() % 64 / 16 )			
			alp_flags_subtree:add(f.alp_flag_grouped, alp_data(alp_offset, 1):uint() % 8)
			
		alp_offset = alp_offset + 1
		
		local alp_record_length = alp_data(alp_offset, 1):uint()
		alp_subtree:add(f.alp_record_length, alp_record_length)
		alp_offset = alp_offset + 1
		alp_subtree:add(f.alp_id, alp_data(alp_offset, 1))
		alp_offset = alp_offset + 1

		local alp_template = alp_data(alp_offset, alp_record_length - 3)
		alp_subtree:add(f.alp_templates, alp_template)
		
		while alp_offset < alp_record_length do
			local alp_template_subtree = alp_subtree:add("ALP Template", nil)
			local alp_op =  alp_data(alp_offset, 1):uint()
			alp_template_subtree:add(f.alp_op, alp_op)
			alp_offset = alp_offset + 1
			
			
			if alp_op <= 5 or alp_op == 32 then -- File Data Template
				alp_offset = parse_file_data_template(alp_data, alp_offset, alp_template_subtree)
			elseif alp_op == 6 or alp_op == 17 or alp_op == 33  then -- File Header Template			
				alp_offset = parse_file_header_template(alp_data, alp_offset, alp_template_subtree)
			elseif alp_op == 16 or (alp_op >= 18  and alp_op <= 22) then -- File ID Template			
				alp_offset = parse_file_id_template(alp_data, alp_offset, alp_template_subtree)
			elseif alp_op == 34 then -- File Header + Data Template				
				alp_offset = parse_file_header_data_template(alp_data, alp_offset, alp_template_subtree)
			elseif alp_op == 255 then -- File Error Template				
				alp_offset = parse_file_error_template(alp_data, alp_offset, alp_template_subtree)
			end
			
			
		
		end
		
		
		
	
	-- TODO set columns
	-- pinfo.cols.info

end

wtap_encap = DissectorTable.get("wtap_encap")
wtap_encap:add(wtap.USER0, oss7_proto)