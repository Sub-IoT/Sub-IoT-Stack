oss7_proto = Proto("oss7_proto", "DASH7 Alliance Protocol DRAFT 0.2")

local f = oss7_proto.fields

f.length = ProtoField.int8("oss7_proto.length", "Length")
f.rssi = ProtoField.int8("oss7_proto.rssi", "RSSI")

function oss7_proto.dissector(buffer,pinfo,tree)
	pinfo.cols.protocol = "DASH7 Alliance Protocol DRAFT 0.2"

	local subtree = tree:add(oss7_proto, buffer())
	local offset = 2 -- TODO what are first 2 bytes?
	subtree:add(f.length, buffer(offset, 1))
	offset = offset + 1
	subtree:add(f.rssi, buffer(offset, 1))
	offset = offset + 1
end

wtap_encap = DissectorTable.get("wtap_encap")
wtap_encap:add(wtap.USER0, oss7_proto)