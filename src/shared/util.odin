package shared

util_hash :: proc(data : string) -> u16 {
	i, size := int(0), len(data)
	hash := u16(5381);

	for i < size {
		hash = ((hash << 5) + hash) + u16((u8(data[i])))
		i += 1
	}

	return hash;
}