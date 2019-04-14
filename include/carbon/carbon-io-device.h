#ifndef CARBON_IO_DEVICE_H
#define CARBON_IO_DEVICE_H

#include <carbon/carbon-common.h>
#include <carbon/carbon-memfile.h>

typedef enum {
    carbon_io_device_file,
    carbon_io_device_memfile
} carbon_io_device_type_e;

struct carbon_io_device;

/**
  * Carbon IO device abstraction
  */
typedef struct carbon_io_device {
    // What's the underlying IO device?
    carbon_io_device_type_e tag;

    // Pointer to internally used structures such as file pointers
    void *extra;

    /**
     * Write data to the IO device.
     *
     * @author okirmis
     *
     * @param in Pointer to the input data structure that should be written to the device
     * @param element_size Size of a single element of the input data structure
     * @param num_elements Number of the elements of size element_size in the input data buffer
     * @return Whether writing the data to IO device was successful
     */
    bool (*write)(struct carbon_io_device *self, void *data, size_t element_size, size_t num_elements);

    /**
     * Read data from the IO device.
     *
     * @author okirmis
     *
     * @param out Pointer to the output data structure the data should be read to
     * @param element_size Size of a single element of the output data structure
     * @param num_elements Number of the elements of size element_size in the output data buffer
     * @return Number of elements (of size element_size) read from the device
     */
    size_t (*read)(struct carbon_io_device *self, void *data, size_t element_size, size_t num_elements);

    /**
     * Get the file pointers current position in the file
     *
     * @author okirmis
     * @return The file pointers current position
     */
    size_t (*tell)(struct carbon_io_device *self);

    /**
     * Set the file pointers current position in the file
     *
     * @author okirmis
     * @param position The new position of the file pointer
     * @return Whether the operation was successful
     */
    bool (*seek)(struct carbon_io_device *self, size_t position);
} carbon_io_device_t;

CARBON_EXPORT(void)
carbon_io_device_from_file(carbon_io_device_t *device, FILE *file);

CARBON_EXPORT(void)
carbon_io_device_from_memfile(carbon_io_device_t *device, carbon_memfile_t *memfile);

CARBON_EXPORT(bool)
carbon_io_device_write(carbon_io_device_t *device, void *data, size_t element_size, size_t num_elements);

CARBON_EXPORT(size_t)
carbon_io_device_read(carbon_io_device_t *device, void *data, size_t element_size, size_t num_elements);

CARBON_EXPORT(size_t)
carbon_io_device_tell(carbon_io_device_t *device);

CARBON_EXPORT(bool)
carbon_io_device_seek(carbon_io_device_t *device, size_t position);

CARBON_EXPORT(void)
carbon_io_device_drop(carbon_io_device_t *device);

#endif
