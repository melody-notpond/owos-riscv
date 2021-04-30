#ifndef KERNEL_VIRTIO_H
#define KERNEL_VIRTIO_H

#define VIRTIO_GENERIC_INIT(addr, device_features, device_specific)                                      \
    do {                                                                                                 \
        /* 1. Reset the device (set status register to zero) */                                          \
        addr->status = 0;                                                                                \
                                                                                                         \
        /* 2. Acknowledge the device */                                                                  \
        addr->status |= VIRTIO_DEVICE_STATUS_ACKNOWLEDGE;                                                \
                                                                                                         \
        /* 3. State the OS knows how to use the device */                                                \
        addr->status |= VIRTIO_DEVICE_STATUS_DRIVER;                                                        \
                                                                                                         \
        /* 4. Read device specific features */                                                           \
        unsigned int features = addr->host_features;                                                     \
        { device_features }                                                                              \
        addr->guest_features = features;                                                                 \
                                                                                                         \
        /* 5. Set features ok bit (can no longer set features) */                                        \
        addr->status |= VIRTIO_DEVICE_STATUS_FEATURES_OK;                                                \
                                                                                                         \
        /* 6. Check that the features ok bit is still enabled */                                         \
        if ((addr->status & VIRTIO_DEVICE_STATUS_FEATURES_OK) == 0) {                                    \
            uart_puts("Virtio device does not support given features. Giving up on initialisation.\n");  \
            return 0;                                                                                    \
        }                                                                                                \
                                                                                                         \
        /* 7. Device specific initialisation */                                                          \
        { device_specific };                                                                             \
                                                                                                         \
        /* 8. Set driver ok bit, device is ready for use. */                                             \
        addr->status |= VIRTIO_DEVICE_STATUS_DRIVER_OK;                                                  \
    } while (0)

#define VIRTIO_MMIO_BASE 0x10001000
#define VIRTIO_RING_SIZE (1 << 7)
#define PAGE_SIZE 4096
#define VIRTIO_DEVICE_COUNT 8

enum {
    VIRTIO_DEVICE_STATUS_ACKNOWLEDGE = 1,
    VIRTIO_DEVICE_STATUS_DRIVER = 2,
    VIRTIO_DEVICE_STATUS_FEATURES_OK = 8,
    VIRTIO_DEVICE_STATUS_DRIVER_OK = 4,
};

typedef struct __attribute__((__packed__, aligned(4))) {
    unsigned long long addr;
    unsigned int length;
    unsigned short flags;
    unsigned short next;
} virtio_descriptor_t;

typedef struct __attribute__((__packed__, aligned(4))) {
    unsigned short flags;
    unsigned short idx;
    unsigned short ring[VIRTIO_RING_SIZE];
    unsigned short event;
} virtio_available_t;

typedef struct __attribute__((__packed__, aligned(4))) {
    unsigned short flags;
    unsigned short idx;
    struct __attribute__((__packed__, aligned(4))) {
        unsigned int id;
        unsigned int length;
    } ring[VIRTIO_RING_SIZE];
    unsigned short event;
} virtio_used_t;

typedef struct __attribute__((__packed__, aligned(4))) {
    virtio_descriptor_t desc[VIRTIO_RING_SIZE];
    virtio_available_t available;
    unsigned char _padding[PAGE_SIZE - sizeof(virtio_descriptor_t) * VIRTIO_RING_SIZE - sizeof(virtio_available_t)];
    virtio_used_t used;
} virtio_queue_t;

typedef struct  __attribute__((__packed__, aligned(4))) {
    unsigned int magic_value;
	unsigned int version;
	unsigned int device_id;
	unsigned int vendor_id;
	unsigned int host_features;
	unsigned int host_features_sel;
	unsigned char rsv1[8];
	unsigned int guest_features;
	unsigned int guest_features_sel;
	unsigned int guest_page_size;
	unsigned char rsv2[4];
	unsigned int queue_sel;
	unsigned int queue_num_max;
	unsigned int queue_num;
	unsigned int queue_align;
	unsigned long long queue_pfn;
	unsigned char rsv3[8];
	unsigned int queue_notify;
	unsigned char rsv4[12];
	unsigned int interrupt_status;
	unsigned int interrupt_ack;
	unsigned char rsv5[8];
	unsigned int status;
} virtio_mmio_t;

void virtio_probe();

#endif /* KERNEL_VIRTIO_H */

