use alloc::alloc::{GlobalAlloc, Layout};

// TODO: make a custom Mutex struct that does OS-y stuff
use spin::Mutex;

use super::syscalls::{mmap, munmap, PROT_READ, PROT_WRITE};

const PAGE_SIZE: usize = 4096;
const ALIGN: usize = 16;

struct Allocator {
    head: *mut u8,
}

struct MutexWrapper<T>(Mutex<T>);

#[repr(C)]
struct AllocHeader {
    size: usize,
    free: bool,
    next: *mut AllocHeader,
}

unsafe impl<T> Sync for MutexWrapper<T> { }

unsafe impl GlobalAlloc for MutexWrapper<Allocator> {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let mut allocator = self.0.lock();

        if layout.size() > PAGE_SIZE - core::mem::size_of::<AllocHeader>() {
            let head: *mut u8 = mmap(layout.size(), PROT_READ | PROT_WRITE);
            if head.is_null() {
                return core::ptr::null_mut();
            }

            let head = head as *mut AllocHeader;
            head.as_uninit_mut().unwrap().write(AllocHeader {
                size: layout.size(),
                free: false,
                next: core::ptr::null_mut(),
            });

            return head.add(1) as *mut u8;
        }

        if allocator.head.is_null() {
            allocator.head = mmap(PAGE_SIZE, PROT_READ | PROT_WRITE);
            if allocator.head.is_null() {
                return core::ptr::null_mut();
            }

            let head = allocator.head as *mut AllocHeader;
            head.as_uninit_mut().unwrap().write(AllocHeader {
                size: PAGE_SIZE - core::mem::size_of::<AllocHeader>(),
                free: true,
                next: core::ptr::null_mut(),
            });
        }

        let mut head = allocator.head as *mut AllocHeader;
        loop {
            let head_ref = head.as_mut().unwrap();
            if head_ref.size >= layout.size() && head_ref.free {
                if head_ref.size > layout.size() * 2 + core::mem::size_of::<AllocHeader>() + ALIGN {
                    head_ref.size = (layout.size() * 2 + ALIGN - 1) & !(ALIGN - 1);
                    let last_next = head_ref.next;
                    head_ref.next = (head.add(1) as *mut u8).add(head_ref.size) as *mut AllocHeader;
                    head_ref.next.as_mut().unwrap().next = last_next;
                }

                head_ref.free = false;

                return head.add(1) as *mut u8;
            }

            if head_ref.next.is_null() {
                head_ref.next = mmap(PAGE_SIZE, PROT_READ | PROT_WRITE);
                if head_ref.next.is_null() {
                    return core::ptr::null_mut();
                }

                let head = head_ref.next as *mut AllocHeader;
                head.as_uninit_mut().unwrap().write(AllocHeader {
                    size: PAGE_SIZE - core::mem::size_of::<AllocHeader>(),
                    free: true,
                    next: core::ptr::null_mut(),
                });
            }

            head = head_ref.next;
        }
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        let ptr = (ptr as *mut AllocHeader).sub(1);
        let ref_ = ptr.as_mut().unwrap();

        if ref_.size > PAGE_SIZE - core::mem::size_of::<AllocHeader>() {
            munmap(ptr, ref_.size);
        } else {
            ref_.free = true;

            // TODO: reclaim pages that are empty
        }
    }
}

#[global_allocator]
static GLOBAL: MutexWrapper<Allocator> = MutexWrapper(Mutex::new(Allocator {
    head: core::ptr::null_mut(),
}));

#[alloc_error_handler]
fn alloc_error(layout: Layout) -> ! {
    panic!("error allocating: {:?}", layout);
}
