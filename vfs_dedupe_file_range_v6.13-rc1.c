int vfs_dedupe_file_range(struct file *file, struct file_dedupe_range *same)
{
    struct file_dedupe_range_info *info;
    struct inode *src = file_inode(file);
    u64 off;
    u64 len;
    int i;
    int ret;
    u16 count = same->dest_count;
    loff_t deduped;

    if (!(file->f_mode & FMODE_READ))
        return -EINVAL;

    if (same->reserved1 || same->reserved2)
        return -EINVAL;

    off = same->src_offset;
    len = same->src_length;

    if (S_ISDIR(src->i_mode))
        return -EISDIR;

    if (!S_ISREG(src->i_mode))
        return -EINVAL;

    if (!file->f_op->remap_file_range)
        return -EOPNOTSUPP;

    ret = remap_verify_area(file, off, len, false);
    if (ret < 0)
        return ret;
    ret = 0;

    if (off + len > i_size_read(src))
        return -EINVAL;

    /* Arbitrary 1G limit on a single dedupe request, can be raised. */
    len = min_t(u64, len, 1 << 30);

    /* pre-format output fields to sane values */
    for (i = 0; i < count; i++)
    {
        same->info[i].bytes_deduped = 0ULL;
        same->info[i].status = FILE_DEDUPE_RANGE_SAME;
    }

    for (i = 0, info = same->info; i < count; i++, info++)
    {
        CLASS(fd, dst_fd)
        (info->dest_fd);

        if (fd_empty(dst_fd))
        {
            info->status = -EBADF;
            goto next_loop;
        }

        if (info->reserved)
        {
            info->status = -EINVAL;
            goto next_loop;
        }

        deduped = vfs_dedupe_file_range_one(file, off, fd_file(dst_fd),
                                            info->dest_offset, len,
                                            REMAP_FILE_CAN_SHORTEN);
        if (deduped == -EBADE)
            info->status = FILE_DEDUPE_RANGE_DIFFERS;
        else if (deduped < 0)
            info->status = deduped;
        else
            info->bytes_deduped = len;

    next_loop:
        if (fatal_signal_pending(current))
            break;
    }
    return ret;
}
EXPORT_SYMBOL(vfs_dedupe_file_range);

loff_t vfs_dedupe_file_range_one(struct file *src_file, loff_t src_pos,
                                 struct file *dst_file, loff_t dst_pos,
                                 loff_t len, unsigned int remap_flags)
{
    loff_t ret;

    WARN_ON_ONCE(remap_flags & ~(REMAP_FILE_DEDUP |
                                 REMAP_FILE_CAN_SHORTEN));

    /*
     * This is redundant if called from vfs_dedupe_file_range(), but other
     * callers need it and it's not performance sesitive...
     */
    ret = remap_verify_area(src_file, src_pos, len, false);
    if (ret)
        return ret;

    ret = remap_verify_area(dst_file, dst_pos, len, true);
    if (ret)
        return ret;

    /*
     * This needs to be called after remap_verify_area() because of
     * sb_start_write() and before may_dedupe_file() because the mount's
     * MAY_WRITE need to be checked with mnt_get_write_access_file() held.
     */
    ret = mnt_want_write_file(dst_file);
    if (ret)
        return ret;

    ret = -EPERM;
    if (!may_dedupe_file(dst_file)) // This is a permission check
        goto out_drop_write;

    ret = -EXDEV;
    if (file_inode(src_file)->i_sb != file_inode(dst_file)->i_sb)
        goto out_drop_write;

    ret = -EISDIR;
    if (S_ISDIR(file_inode(dst_file)->i_mode))
        goto out_drop_write;

    ret = -EINVAL;
    if (!dst_file->f_op->remap_file_range) // This replaces the privileged function of v4.18
        goto out_drop_write;

    if (len == 0)
    {
        ret = 0;
        goto out_drop_write;
    }

    ret = dst_file->f_op->remap_file_range(src_file, src_pos, dst_file,
                                           dst_pos, len, remap_flags | REMAP_FILE_DEDUP);
out_drop_write:
    mnt_drop_write_file(dst_file);

    return ret;
}
EXPORT_SYMBOL(vfs_dedupe_file_range_one);