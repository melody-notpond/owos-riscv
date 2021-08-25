#include "uart.h"

#include "../../lib/string.h"
#include "../../lib/memory.h"
#include "../../interrupts.h"

#define UART_RX_REGISTER(mmio)                  ((volatile char*) ((mmio).base                          ))
#define UART_TX_REGISTER(mmio)                  ((volatile char*) ((mmio).base                          ))
#define UART_INTERRUPT_ENABLE_REGISTER(mmio)    ((volatile char*) ((mmio).base + (1 << (mmio).reg_shift)))
#define UART_INTERRUPT_STATUS_REGISTER(mmio)    ((volatile char*) ((mmio).base + (2 << (mmio).reg_shift)))
#define UART_FIFO_CONTROL_REGISTER(mmio)        ((volatile char*) ((mmio).base + (2 << (mmio).reg_shift)))
#define UART_LINE_CONTROL_REGISTER(mmio)        ((volatile char*) ((mmio).base + (3 << (mmio).reg_shift)))
#define UART_MODEM_CONTROL_REGISTER(mmio)       ((volatile char*) ((mmio).base + (4 << (mmio).reg_shift)))
#define UART_LINE_STATUS_REGISTER(mmio)         ((volatile char*) ((mmio).base + (5 << (mmio).reg_shift)))
#define UART_MODEM_STATUS_REGISTER(mmio)        ((volatile char*) ((mmio).base + (6 << (mmio).reg_shift)))
#define UART_SCRATCHPAD_REGISTER(mmio)          ((volatile char*) ((mmio).base + (7 << (mmio).reg_shift)))

#define UART_DIVISOR_LATCH_LSB_REGISTER(mmio)   ((volatile char*) ((mmio).base                          ))
#define UART_DIVISOR_LATCH_MSB_REGISTER(mmio)   ((volatile char*) ((mmio).base + (1 << (mmio).reg_shift)))
#define UART_PRESCALER_DIVISION_REGISTER(mmio)  ((volatile char*) ((mmio).base + (5 << (mmio).reg_shift)))

typedef enum {
    UART_TYPE_NS16550,
} uart_type_t;

typedef struct {
    unsigned long long length;
    char* data;
    unsigned long long mmu;
    void (*callback_fn)(void*);
    void* callback_data;
} uart_tx_queue_entry_t;

// TODO: make atomic
struct s_uart_mmio {
    volatile void* base;
    unsigned long long reg_shift;

    uart_type_t type;

    uart_tx_queue_entry_t* tx_queue;
    unsigned long long tx_queue_front;
    unsigned long long tx_queue_rear;
    unsigned long long tx_queue_length;
    unsigned long long tx_queue_capacity;

    char* rx_queue;
    unsigned long long rx_queue_front;
    unsigned long long rx_queue_rear;
    unsigned long long rx_queue_length;
    unsigned long long rx_queue_capacity;
};

uart_mmio_t mmio;

typedef enum {
    UART_INTERRUPT_STATUS_NONE                                  = 0b0001,
    UART_INTERRUPT_STATUS_RECEIVER_LINE_STATUS                  = 0b0110,
    UART_INTERRUPT_STATUS_RECEIVED_DATA_READY                   = 0b0100,
    UART_INTERRUPT_STATUS_RECEPTION_TIMEOUT                     = 0b1100,
    UART_INTERRUPT_STATUS_TRANSMITTER_HOLDING_REGISTER_EMPTY    = 0b0010,
    UART_INTERRUPT_STATUS_MODEM_STATUS                          = 0b0000,
    UART_INTERRUPT_STATUS_DMA_RECEPTION_EOT                     = 0b1110,
    UART_INTERRUPT_STATUS_DMA_TRANSMISSION_EOT                  = 0b1010,
} uart_interrupt_status_t;

void uart_enqueue_received_data(uart_mmio_t* mmio, char data) {
    if (mmio->rx_queue_length < mmio->rx_queue_capacity) {
        mmio->rx_queue[mmio->rx_queue_rear++] = data;
        mmio->rx_queue_rear %= mmio->rx_queue_capacity;
        mmio->rx_queue_length++;
    }

    // TODO: reallocate queue if too smol
}

void uart_put_next_character_into_tx(uart_mmio_t* mmio) {
    if (mmio->tx_queue_length != 0) {
        uart_tx_queue_entry_t* entry = &mmio->tx_queue[mmio->tx_queue_front];
        asm volatile("csrrw %0, satp, %0" : "=r" (entry->mmu) : "r" (entry->mmu));
        *UART_TX_REGISTER(*mmio) = entry->data[0];
        asm volatile("csrrw %0, satp, %0" : "=r" (entry->mmu) : "r" (entry->mmu));
        entry->length--;
        entry->data++;

        if (entry->length == 0) {
            asm volatile("csrrw %0, satp, %0" : "=r" (entry->mmu) : "r" (entry->mmu));
            entry->callback_fn(entry->callback_data);
            asm volatile("csrrw %0, satp, %0" : "=r" (entry->mmu) : "r" (entry->mmu));
            mmio->tx_queue_front++;
            mmio->tx_queue_front %= mmio->tx_queue_capacity;
            mmio->tx_queue_length--;
        }
    }
}

void uart_mei_handler(unsigned int _, void* callback_data) {
    uart_mmio_t* mmio = callback_data;

    uart_interrupt_status_t status = *UART_INTERRUPT_STATUS_REGISTER(*mmio);
    switch (status) {
        case UART_INTERRUPT_STATUS_NONE:
            break;

        case UART_INTERRUPT_STATUS_RECEIVER_LINE_STATUS:
            *UART_LINE_STATUS_REGISTER(*mmio);
            break;

        case UART_INTERRUPT_STATUS_RECEIVED_DATA_READY:
            uart_enqueue_received_data(mmio, *UART_RX_REGISTER(*mmio));
            break;

        case UART_INTERRUPT_STATUS_RECEPTION_TIMEOUT:
            uart_enqueue_received_data(mmio, *UART_RX_REGISTER(*mmio));
            break;

        case UART_INTERRUPT_STATUS_TRANSMITTER_HOLDING_REGISTER_EMPTY:
            uart_put_next_character_into_tx(mmio);
            break;

        case UART_INTERRUPT_STATUS_MODEM_STATUS:
            *UART_MODEM_STATUS_REGISTER(*mmio);
            break;

        case UART_INTERRUPT_STATUS_DMA_RECEPTION_EOT:
            break;

        case UART_INTERRUPT_STATUS_DMA_TRANSMISSION_EOT:
            break;
    }
}

uart_mmio_t* init_ns16550(fdt_t* fdt, void* node) {
    unsigned long long addr = fdt_get_node_addr(node);
    struct fdt_property interrupts_raw = fdt_get_property(fdt, node, "interrupts");
    struct fdt_property reg_shift_raw = fdt_get_property(fdt, node, "reg-shift");
    unsigned long long reg_shift = reg_shift_raw.data ? be_to_le(32, reg_shift_raw.data) : 0;

    unsigned long long mmu;
    mmu_level_1_t* top = (void*) 0;
    asm volatile("csrr %0, satp" : "=r" (mmu));
    top = (void*) ((mmu & 0x00000fffffffffff) << 12);
    mmu_map_range_identity(top, (void*) addr, (void*) (addr + (7 << reg_shift)), MMU_FLAG_READ | MMU_FLAG_WRITE);

    mmio = (uart_mmio_t) {
        .base = (void*) addr,
        .reg_shift = reg_shift,
        .type = UART_TYPE_NS16550,

        .tx_queue_front = 0,
        .tx_queue_rear = 0,
        .tx_queue_length = 0,
        .tx_queue_capacity = 4096,

        .rx_queue_front = 0,
        .rx_queue_rear = 0,
        .rx_queue_length = 0,
        .rx_queue_capacity = 4096,
    };
    mmio.tx_queue = malloc(sizeof(uart_tx_queue_entry_t) * mmio.tx_queue_capacity);
    mmio.rx_queue = malloc(sizeof(char) * mmio.rx_queue_capacity);

    while (interrupts_raw.len) {
        // TODO: figure out how to determine the format of interrupts
        if (register_mei_handler(be_to_le(32, interrupts_raw.data), 7, uart_mei_handler, &mmio)) {
            free(mmio.tx_queue);
            free(mmio.rx_queue);
            return (void*) 0;
        }
        interrupts_raw.len -= 4;
        interrupts_raw.data += 4;
    }

    // Disable FIFO stuff (we'll use our own queue)
    *UART_FIFO_CONTROL_REGISTER(mmio) = 0;

    // Enable interrupts for transmitter empty and receiver contains data
    *UART_INTERRUPT_ENABLE_REGISTER(mmio) = 0b00000011;
    return &mmio;
}

// init_uart(fdt_t*, char*) -> uart_mmio_t*
// Initialises a UART port. Returns 0 on success.
uart_mmio_t* init_uart(fdt_t* fdt, char* path) {
    void* node = fdt_path(fdt, path, (void*) 0);
    if (node == (void*) 0)
        return (void*) 0;

    struct fdt_property compatible = fdt_get_property(fdt, node, "compatible");
    while (compatible.len) {
        if (!strcmp(compatible.data, "ns16550a") || !strcmp(compatible.data, "ns16550"))
            return init_ns16550(fdt, node);

        while (*compatible.data) {
            compatible.data++;
            compatible.len--;
        }

        compatible.data++;
        compatible.len--;
    }

    return (void*) 0;
}

// uart_write_str(uart_mmio_t*, char*, unsigned long long, void (*)(void*), void*) -> int
// Writes a string to the uart. Returns 0 on success.
int uart_write_str(uart_mmio_t* mmio, char* data, unsigned long long length, void (*callback_fn)(void*), void* callback_data) {
    if (mmio->tx_queue_length >= mmio->tx_queue_capacity)
        return -1;

    unsigned long long mmu;
    asm volatile("csrr %0, satp" : "=r" (mmu));
    mmio->tx_queue[mmio->tx_queue_rear++] = (uart_tx_queue_entry_t) {
        .data = data,
        .length = length,
        .callback_fn = callback_fn,
        .callback_data = callback_data,
        .mmu = mmu
    };
    mmio->tx_queue_rear %= mmio->tx_queue_capacity;
    mmio->tx_queue_length++;
    return 0;
}

