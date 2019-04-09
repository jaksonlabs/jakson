#include <carbon/compressor/compressor-utils.h>

size_t carbon_vlq_encode(size_t length, uint8_t *buffer) {
    size_t num_bytes = 0;
    do {
        if(length > 127L) {
            buffer[num_bytes] = (length & 0x7F) | 0x80;
        } else {
            buffer[num_bytes] = length & 0x7F;
        }

        length >>= 7;
        ++num_bytes;
    } while(num_bytes < 10 && length > 0);

    return num_bytes;
}

size_t carbon_vlq_decode(uint8_t *buffer, size_t *num_bytes) {
    *num_bytes = 0;
    size_t length = 0;
    size_t bit_pos = 0;
    while(*num_bytes < 10) {
        uint8_t byte = buffer[*num_bytes];

        length = length + (((size_t)(byte & 0x7Fu)) << bit_pos);
        bit_pos += 7;
        (*num_bytes)++;

        if((byte & 0x80) == 0)
            break;
    }

    return length;
}

void carbon_vlq_encode_to_file(size_t length, carbon_memfile_t *dst)
{
    uint8_t buf[10];
    size_t len = carbon_vlq_encode(length, buf);
    carbon_memfile_write(dst, buf, len);
}

size_t carbon_vlq_decode_from_file(FILE *src, bool *ok)
{
    uint8_t buf[10];
    size_t len = 0;
    size_t value = 0;

    if(fread(buf,1, 10, src) < 1) {
        *ok = false;
        return 0;
    }

    value = carbon_vlq_decode(buf, (size_t *)&len);
    fseek(src, (long)len - 10L, SEEK_CUR);

    *ok = true;
    return value;
}
