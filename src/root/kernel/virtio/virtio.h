#ifndef KERNEL_VIRTIO_H
#define KERNEL_VIRTIO_H

#include "../memory.h"

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
#define VIRTIO_RING_SIZE (1 << 7)
#define VIRTIO_DEVICE_COUNT 8

enum {
    VIRTIO_DEVICE_STATUS_ACKNOWLEDGE = 1,
    VIRTIO_DEVICE_STATUS_DRIVER = 2,
    VIRTIO_DEVICE_STATUS_FEATURES_OK = 8,
    VIRTIO_DEVICE_STATUS_DRIVER_OK = 4,
};

enum {
    VIRTIO_DESCRIPTOR_FLAG_NEXT = 1,
    VIRTIO_DESCRIPTOR_FLAG_WRITE_ONLY = 2,
    VIRTIO_DESCRIPTOR_FLAG_INDIRECT = 4,
};

typedef struct __attribute__((__packed__, aligned(1))) {
    void* addr;
    unsigned int length;
    unsigned short flags;
    unsigned short next;
} virtio_descriptor_t;

typedef struct __attribute__((__packed__, aligned(1))) {
    unsigned short flags;
    unsigned short idx;
    unsigned short ring[VIRTIO_RING_SIZE];
    unsigned short event;
} virtio_available_t;

typedef struct __attribute__((__packed__, aligned(1))) {
    unsigned short flags;
    unsigned short idx;
    struct __attribute__((__packed__, aligned(1))) {
        unsigned int id;
        unsigned int length;
    } ring[VIRTIO_RING_SIZE];
    unsigned short event;
} virtio_used_t;

typedef struct __attribute__((__packed__, aligned(1))) {
    volatile unsigned long long num;
    volatile virtio_descriptor_t* desc;
    volatile virtio_available_t* available;
    // unsigned char rsv1[PAGE_SIZE - sizeof(virtio_descriptor_t) * VIRTIO_RING_SIZE - sizeof(virtio_available_t)];
    volatile virtio_used_t* used;
} virtio_queue_t;

typedef struct __attribute__((__packed__, aligned(4))) {
    unsigned int magic_value;
	unsigned int version;
	unsigned int device_id;
	unsigned int vendor_id;

	unsigned int device_features;
	unsigned int device_features_sel;
	unsigned char rsv1[8];

	unsigned int driver_features;
	unsigned int driver_features_sel;
	unsigned char rsv2[8];

	unsigned int queue_sel;
	unsigned int queue_num_max;
	unsigned int queue_num;
	unsigned char rsv3[4];

	unsigned char rsv4[4];
	unsigned int queue_ready;
	unsigned char rsv5[8];

	unsigned int queue_notify;
	unsigned char rsv6[12];

	unsigned int interrupt_status;
	unsigned int interrupt_ack;
	unsigned char rsv7[8];

	unsigned int status;
	unsigned char rsv8[12];

    unsigned int queue_desc_low;
    unsigned int queue_desc_high;
	unsigned char rsv9[8];

    unsigned int queue_avail_low;
    unsigned int queue_avail_high;
	unsigned char rsv10[8];

    unsigned int queue_used_low;
    unsigned int queue_used_high;
	unsigned char rsv11[8];

	unsigned char rsv12[0x4c];
    unsigned int config_gen;

	volatile char config[];
} virtio_mmio_t;

void virtio_probe();

#endif /* KERNEL_VIRTIO_H */

