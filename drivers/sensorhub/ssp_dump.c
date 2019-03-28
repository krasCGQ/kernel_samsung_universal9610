/*
 *  Copyright (C) 2018, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "ssp.h"
#include "ssp_platform.h"
#include "ssp_dump.h"

#define DUMP_DIR				"/data/log"
#define DUMP_COUNT_FILE			"SensorHubDumpCount"
static int create_dump_file(struct ssp_data *data, char *file_name)
{
	int ret = 0;
	struct file *filp;
	mm_segment_t old_fs;
	old_fs = get_fs();
	set_fs(get_ds());

	filp = filp_open(file_name, O_CREAT, 0666);
	if (IS_ERR(filp)) {
		ssp_errf("open '%s' file fail: %d\n", file_name, (int)PTR_ERR(filp));
		ret = (int)PTR_ERR(filp);
		goto out;
	}

	filp_close(filp, NULL);

out:
	set_fs(old_fs);
	return ret;
}

void init_ssp_dump_count_file(struct ssp_data *data)
{
	char file_path[70] = {0,};
	char buf[10] = {0,};
	struct file *filp;
	mm_segment_t old_fs;
	int ret;
	
	snprintf(file_path, sizeof(file_path), "%s/%s", DUMP_DIR, DUMP_COUNT_FILE);

	old_fs = get_fs();
	set_fs(get_ds());

	filp = filp_open(file_path, O_CREAT| O_RDWR, 0666);
	if (IS_ERR(filp)) {
		ssp_errf("open '%s' file fail: %d\n", file_path, (int)PTR_ERR(filp));
		goto out;
	}
	
	ret = vfs_read(filp, buf, sizeof(buf), &filp->f_pos);

	if (ret < 0) {
		data->cnt_dump = 0;
	} else {
		ret = kstrtos32(buf, 10, &data->cnt_dump);
		if(ret < 0) {
			data->cnt_dump = 0;
		}		
	}
	snprintf(buf, sizeof(buf), "%d", data->cnt_dump);

	ret = vfs_write(filp, buf, sizeof(buf), &filp->f_pos);		
	if (ret < 0) {
		ssp_errf("Can't write dump time to file (%d)", ret);
	}

	filp_close(filp, NULL);

out:
	set_fs(old_fs);
}

void initialize_ssp_dump(struct ssp_data *data)
{
	int i, j, ret;
	char file_path[70] = {0,};
	char file_name[30] = {0,};
	char file_type[5] = {0,};
	int num = sensorhub_dump_get_num_dumptype(data);
	
	for( j = 0 ; j < num ; j++) {
		sensorhub_dump_get_filename(data, j, file_name, file_type);
		for( i = 1 ; i <= NUM_DUMP_FILE ; i++) {
			snprintf(file_path, sizeof(file_path), "%s/%s_%d.%s",
				DUMP_DIR, file_name, i, file_type);
			ret = create_dump_file(data, file_path);
			if(ret != 0)
				goto err_create_fail;
		}
	}

	init_ssp_dump_count_file(data);
	
	ssp_infof("");
	return;
	
err_create_fail:
	ssp_errf("fail %d", ret);
	data->cnt_dump--;
	return;
}

void write_ssp_dump_count_file(struct ssp_data *data)
{
	char file_path[70] = {0,};
	char buf[10] = {0,};
	struct file *filp;
	mm_segment_t old_fs;
	int ret;
	
	snprintf(file_path, sizeof(file_path), "%s/%s", DUMP_DIR, DUMP_COUNT_FILE);
	snprintf(buf, sizeof(buf), "%d", data->cnt_dump);

	old_fs = get_fs();
	set_fs(get_ds());

	filp = filp_open(file_path, O_CREAT| O_WRONLY, 0666);
	if (IS_ERR(filp)) {
		ssp_errf("open '%s' file fail: %d\n", file_path, (int)PTR_ERR(filp));
		goto out;
	}

	ret = vfs_write(filp, buf, sizeof(buf), &filp->f_pos);
	
	if (ret < 0) {
		ssp_errf("Can't write dump time to file (%d)", ret);
	}
	
	filp_close(filp, NULL);
out:
	set_fs(old_fs);
}

void write_ssp_dump_file(struct ssp_data *data, char *file_name, char *file_type, void *buf, int size)
{
	char file_path[64] = {0,};
	struct file *filp;
	mm_segment_t old_fs;
	int ret;

	snprintf(file_path, sizeof(file_path), "%s/%s_%d.%s",
		DUMP_DIR, file_name, data->cnt_dump%NUM_DUMP_FILE, file_type);

	ssp_infof("%s", file_path);
	
	old_fs = get_fs();
	set_fs(get_ds());

	filp = filp_open(file_path, O_CREAT | O_WRONLY, 0666);
	if (IS_ERR(filp)) {
		ssp_errf("open '%s' file fail: %d\n", file_path, (int)PTR_ERR(filp));
		goto out;
	}

	ret = vfs_write(filp, buf, size, &filp->f_pos);
	
	if (ret < 0) {
		ssp_errf("Can't write dump time to file (%d)", ret);
	}
	
	filp_close(filp, NULL);
out:
	set_fs(old_fs);
}
