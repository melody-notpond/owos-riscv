#ifndef KERNEL_VIRTIO_H
#define KERNEL_VIRTIO_H

#include "../filesystems/generic_file.h"

#define VIRTIO_GENERIC_INIT(addr, device_features_do)                                                    \
    do {                                                                                                 \
        /* 1. Reset the device (set status register to zero) */                                          \
        addr->status = 0;                                                                                \
                                                                                                         \
        /* 2. Acknowledge the device */                                                                  \
        addr->status |= VIRTIO_DEVICE_STATUS_ACKNOWLEDGE;                                                \
                                                                                                         \
        /* 3. State the OS knows how to use the device */                                                \
        addr->status |= VIRTIO_DEVICE_STATUS_DRIVER;                                                     \
                                                                                                         \
        /* 4. Read device specific features */                                                           \
        unsigned int features = addr->device_features;                                                   \
        features &= ~(1 << 29);                                                                          \
        { device_features_do }                                                                           \
        addr->driver_features = features;                                                                \
                                                                                                         \
        /* 5. Set features ok bit (can no longer set features) */                                        \
        addr->status |= VIRTIO_DEVICE_STATUS_FEATURES_OK;                                                \
                                                                                                         \
        /* 6. Check that the features ok bit is still enabled */                                         \
        if ((addr->status & VIRTIO_DEVICE_STATUS_FEATURES_OK) == 0) {                                    \
            uart_puts("Virtio device does not support given features. Giving up on initialisation.\n");  \
            return -1;                                                                                   \
        }                                                                                                \
                                                                                                         \
        /* 7. Device specific initialisation */                                                          \
    } while (0)

#define VIRTIO_GENERIC_INIT_FINISH(addr)                                                                 \
    /* 8. Set driver ok bit, device is ready for use. */                                                 \
    addr->status |= VIRTIO_DEVICE_STATUS_DRIVER_OK                                                       \


#define VIRTIO_MMIO_BASE 0x10001000
#define VIRTIO_MMIO_INTERVAL 0x1000
#define VIRTIO_MMIO_TOP 0x010008000
#define VIRTIO_DEVICE_COUNT ((VIRTIO_MMIO_TOP - VIRTIO_MMIO_BASE) / VIRTIO_MMIO_INTERVAL + 1)

enum {
    VIRTIO_DEVICE_STATUS_ACKNOWLEDGE = 1,
    VIRTIO_DEVICE_STATUS_DRIVER = 2,
    VIRTIO_DEVICE_STATUS_FEATURES_OK = 8,
    VIRTIO_DEVICE_STATUS_DRIVER_OK = 4,
};

enum {
    VIRTIO_INTERRUPT_USED_RING_UPDATE = 1,
    VIRTIO_INTERRUPT_CONFIGURATION_CHANGE = 2,
};

typedef struct __attribute__((__packed__, aligned(4))) {
    volatile unsigned int magic_value;
	volatile unsigned int version;
	volatile unsigned int device_id;
	volatile unsigned int vendor_id;

	volatile unsigned int device_features;
	volatile unsigned int device_features_sel;
	volatile unsigned char rsv1[8];

	volatile unsigned int driver_features;
	volatile unsigned int driver_features_sel;
	volatile unsigned char rsv2[8];

	volatile unsigned int queue_sel;
	volatile unsigned int queue_num_max;
	volatile unsigned int queue_num;
	volatile unsigned char rsv3[4];

	volatile unsigned char rsv4[4];
	volatile unsigned int queue_ready;
	volatile unsigned char rsv5[8];

	volatile unsigned int queue_notify;
	volatile unsigned char rsv6[12];

	volatile unsigned int interrupt_status;
	volatile unsigned int interrupt_ack;
	volatile unsigned char rsv7[8];

	volatile unsigned int status;
	volatile unsigned char rsv8[12];

    volatile unsigned int queue_desc_low;
    volatile unsigned int queue_desc_high;
	volatile unsigned char rsv9[8];

    volatile unsigned int queue_avail_low;
    volatile unsigned int queue_avail_high;
	volatile unsigned char rsv10[8];

    volatile unsigned int queue_used_low;
    volatile unsigned int queue_used_high;
	volatile unsigned char rsv11[8];

	volatile unsigned char rsv12[0x4c];
    volatile unsigned int config_gen;

	volatile char config[];
} virtio_mmio_t;

void virtio_probe(generic_dir_t* dev);

#endif /* KERNEL_VIRTIO_H */

