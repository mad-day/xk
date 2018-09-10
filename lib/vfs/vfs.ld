SECTIONS {
    .vfs_impls : {
        __vfs_impl_start = .;
        KEEP (*(.vfs_impl))
        __vfs_impl_end = .;
    }
}
INSERT AFTER .rodata;
