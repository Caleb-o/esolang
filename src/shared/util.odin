package shared

util_hash :: proc(data : string) -> u32 {
	i, size := int(0), len(data)
	hash := u32(5381);

	for i < size {
		hash = ((hash << 5) + hash) + u32((u8(data[i])))
		i += 1
	}

	return hash;
}