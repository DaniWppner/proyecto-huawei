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
