int vfs_dedupe_file_range(struct file *file, struct file_dedupe_range *same)
{
	struct file_dedupe_range_info *info;
	struct inode *src = file_inode(file);
	u64 off;
	u64 len;
	int i;
	int ret;
	bool is_admin = capable(CAP_SYS_ADMIN);
	u16 count = same->dest_count;
	struct file *dst_file;
	loff_t dst_off;
	ssize_t deduped;

	if (!(file->f_mode & FMODE_READ))
		return -EINVAL;

	if (same->reserved1 || same->reserved2)
		return -EINVAL;

	off = same->src_offset;
	len = same->src_length;

	ret = -EISDIR;
	if (S_ISDIR(src->i_mode))
		goto out;

	ret = -EINVAL;
	if (!S_ISREG(src->i_mode))
		goto out;

	ret = clone_verify_area(file, off, len, false);
	if (ret < 0)
		goto out;
	ret = 0;

	if (off + len > i_size_read(src))
		return -EINVAL;

	/* pre-format output fields to sane values */
	for (i = 0; i < count; i++)
	{
		same->info[i].bytes_deduped = 0ULL;
		same->info[i].status = FILE_DEDUPE_RANGE_SAME;
	}

	for (i = 0, info = same->info; i < count; i++, info++)
	{
		struct inode *dst;
		struct fd dst_fd = fdget(info->dest_fd);

		dst_file = dst_fd.file;
		if (!dst_file)
		{
			info->status = -EBADF;
			goto next_loop;
		}
		dst = file_inode(dst_file);

		ret = mnt_want_write_file(dst_file);
		if (ret)
		{
			info->status = ret;
			goto next_fdput;
		}

		dst_off = info->dest_offset;
		ret = clone_verify_area(dst_file, dst_off, len, true);
		if (ret < 0)
		{
			info->status = ret;
			goto next_file;
		}
		ret = 0;

		if (info->reserved)
		{
			info->status = -EINVAL;
		}
		else if (!(is_admin || (dst_file->f_mode & FMODE_WRITE)))
		{
			info->status = -EINVAL;
		}
		else if (file->f_path.mnt != dst_file->f_path.mnt)
		{
			info->status = -EXDEV;
		}
		else if (S_ISDIR(dst->i_mode))
		{
			info->status = -EISDIR;
		}
		else if (dst_file->f_op->dedupe_file_range == NULL)
		{
			info->status = -EINVAL;
		}
		else
		{
			deduped = dst_file->f_op->dedupe_file_range(file, off,
														len, dst_file,
														info->dest_offset);
			if (deduped == -EBADE)
				info->status = FILE_DEDUPE_RANGE_DIFFERS;
			else if (deduped < 0)
				info->status = deduped;
			else
				info->bytes_deduped += deduped;
		}

	next_file:
		mnt_drop_write_file(dst_file);
	next_fdput:
		fdput(dst_fd);
	next_loop:
		if (fatal_signal_pending(current))
			goto out;
	}

out:
	return ret;
}
EXPORT_SYMBOL(vfs_dedupe_file_range);
