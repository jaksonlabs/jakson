#include <carbon/carbon-io-device.h>

bool   this_file_write(carbon_io_device_t *self, void *data, size_t element_size, size_t num_elements);
size_t this_file_read(carbon_io_device_t *self, void *data, size_t element_size, size_t num_elements);
size_t this_file_tell(carbon_io_device_t *self);
bool   this_file_seek(carbon_io_device_t *self, size_t position);

bool   this_memfile_write(carbon_io_device_t *self, void *data, size_t element_size, size_t num_elements);
size_t this_memfile_read(carbon_io_device_t *self, void *data, size_t element_size, size_t num_elements);
size_t this_memfile_tell(carbon_io_device_t *self);
bool   this_memfile_seek(carbon_io_device_t *self, size_t position);


void carbon_io_device_from_file(carbon_io_device_t *device, FILE *file) {
    device->tag   = carbon_io_device_file;
    device->extra = file;
    device->write = this_file_write;
    device->read  = this_file_read;
    device->tell  = this_file_tell;
    device->seek  = this_file_seek;
}

void carbon_io_device_from_memfile(carbon_io_device_t *device, carbon_memfile_t *memfile) {
    device->tag   = carbon_io_device_memfile;
    device->extra = memfile;
    device->write = this_memfile_write;
    device->read  = this_memfile_read;
    device->tell  = this_memfile_tell;
    device->seek  = this_memfile_seek;
}

bool carbon_io_device_write(carbon_io_device_t *device, void *data, size_t element_size, size_t num_elements) {
    return device->write(device, data, element_size, num_elements);
}

size_t carbon_io_device_read(carbon_io_device_t *device, void *data, size_t element_size, size_t num_elements) {
    return device->read(device, data, element_size, num_elements);
}

size_t carbon_io_device_tell(carbon_io_device_t *device) {
    return device->tell(device);
}

bool carbon_io_device_seek(carbon_io_device_t *device, size_t position) {
    return device->seek(device, position);
}

void carbon_io_device_drop(carbon_io_device_t *device)
{
    CARBON_UNUSED(device);
}



//////////////// Internals ///////////////////////

bool this_file_write(carbon_io_device_t *self, void *data, size_t element_size, size_t num_elements) {
    return fwrite(data, element_size, num_elements, (FILE *)self->extra) == num_elements;
}

size_t this_file_read(carbon_io_device_t *self, void *data, size_t element_size, size_t num_elements) {
    return fread(data, element_size, num_elements, (FILE *)self->extra);
}

size_t this_file_tell(carbon_io_device_t *self) {
    return (size_t)ftell((FILE *)self->extra);
}

bool this_file_seek(carbon_io_device_t *self, size_t position) {
    return fseek((FILE *)self->extra, (long long)position, SEEK_SET);
}



bool this_memfile_write(carbon_io_device_t *self, void *data, size_t element_size, size_t num_elements) {
    return carbon_memfile_write((carbon_memfile_t *)self->extra, data, element_size * num_elements);
}

size_t this_memfile_read(carbon_io_device_t *self, void *data, size_t element_size, size_t num_elements) {
    void const * ptr = carbon_memfile_read((carbon_memfile_t *)self->extra, element_size * num_elements);

    memcpy(data, ptr, element_size * num_elements);
    return element_size * num_elements;
}

size_t this_memfile_tell(carbon_io_device_t *self) {
    return CARBON_MEMFILE_TELL((carbon_memfile_t *)self->extra);
}

bool this_memfile_seek(carbon_io_device_t *self, size_t position) {
    return carbon_memfile_seek((carbon_memfile_t *)self->extra, position);
}
