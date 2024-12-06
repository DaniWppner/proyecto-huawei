int vfs_dedupe_file_range(struct file *file, struct file_dedupe_range *same)
{
	struct file_dedupe_range_info *info;
	struct inode *src = file_inode(file);
	u64 off;
	u64 len;
	int i;
	int ret;
	bool is_admin = capable(CAP_SYS_ADMIN); // not present in newer version
	u16 count = same->dest_count;
	struct file *dst_file; // not present in newer version
	loff_t dst_off;		   // not present in newer version
	ssize_t deduped;	   // type is loff_t in newer version. ssize_t = long; loff_t = long long
						   // As I understand it ssize_t is meant to represent "signed size"
						   // I think loff_t is supposed to represent "long offset?"
	if (!(file->f_mode & FMODE_READ))
		return -EINVAL; // invalid argument

	if (same->reserved1 || same->reserved2)
		return -EINVAL; // invalid argument

	off = same->src_offset;
	len = same->src_length;

	ret = -EISDIR; // is a directory
	if (S_ISDIR(src->i_mode))
		goto out; // new version does "return -EISDIR" instead of the goto hacks

	ret = -EINVAL; // invalid argument
	if (!S_ISREG(src->i_mode))
		goto out; // same about the goto

	ret = clone_verify_area(file, off, len, false); // new version calls remap_verify_area instead
	if (ret < 0)
		goto out;
	ret = 0;

	if (off + len > i_size_read(src))
		return -EINVAL; // invalid argument

	// new version sets len to min(len, 1<<30). 1<<30 is 1G.

	/* pre-format output fields to sane values */
	for (i = 0; i < count; i++)
	{
		same->info[i].bytes_deduped = 0ULL;
		same->info[i].status = FILE_DEDUPE_RANGE_SAME;
	}

	for (i = 0, info = same->info; i < count; i++, info++)
	{
		struct inode *dst; // variable dst does not exist in new version
		struct fd dst_fd = fdget(info->dest_fd);

		dst_file = dst_fd.file;
		if (!dst_file) // this is refactored to if(fd_emtpy(dst_fd)) in new version
		{
			info->status = -EBADF; // bad file number
			goto next_loop;
		}
		dst = file_inode(dst_file);

		// starting from here this is refactored into dedupe_file_range_one in new version
		// which is too different to reasonably learn anything from comparison

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
			// ->dedupe_file_range is the privileged function
			// It has been replaced with ->remap_file_range in new version
			// I do not know why it is privileged since this is the only place of calling it
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
