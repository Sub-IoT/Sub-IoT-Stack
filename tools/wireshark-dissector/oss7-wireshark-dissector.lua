oss7_proto = Proto("oss7_proto", "DASH7 Alliance Protocol DRAFT 0.2")

local f = oss7_proto.fields

f.rssi = ProtoField.int16("oss7_proto.rssi", "RSSI")
f.lqi = ProtoField.uint8("oss7_proto.lqi", "Link Quality Indicator")
f.crc = ProtoField.uint16("oss7_proto.crc", "CRC")

f.length = ProtoField.uint8("oss7_proto.dll.length", "Length")
-- TODO only foreground frames for now
f.tx_eirp = ProtoField.uint8("oss7_proto.dll.frame_header.tx_eirp", "TX EIRP")
f.subnet = ProtoField.uint8("oss7_proto.dll.frame_header.subnet", "Subnet", base.HEX)
f.frame_control = ProtoField.uint8("oss7_proto.dll.frame_header.frame_control", "Frame Control", base.HEX)

function oss7_proto.dissector(buffer,pinfo,tree)
	pinfo.cols.protocol = "DASH7 Alliance Protocol DRAFT 0.2"

	local subtree = tree:add(oss7_proto, buffer())
	local offset = 0 
	subtree:add(f.rssi, buffer(offset, 1)):append_text(" dBm")
	offset = offset + 1
	subtree:add(f.lqi, buffer(offset, 1))
	offset = offset + 1
	-- TODO CRC
	-- DLL subtree
	local dll_subtree = subtree:add("Data Link Layer", nil)
	dll_subtree:add(f.length, buffer(offset, 1))
	offset = offset + 1
	offset = offset + 1 -- TODO length again? skip for now...
	dll_subtree:add(f.tx_eirp, buffer(offset, 1):int() * 0.5 - 40):append_text(" dBm")	
	offset = offset + 1
	dll_subtree:add(f.subnet, buffer(offset, 1))
	offset = offset + 1
	dll_subtree:add(f.frame_control, buffer(offset, 1)) -- TODO parse bitfield?
	offset = offset + 1
	-- TODO set columns
	--pinfo.cols.info
	--pinfo.cols.src
	--pinfo.cols.dst

end

wtap_encap = DissectorTable.get("wtap_encap")
wtap_encap:add(wtap.USER0, oss7_proto)