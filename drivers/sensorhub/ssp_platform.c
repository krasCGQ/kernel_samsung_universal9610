
#include <linux/kernel.h>
#include <linux/rtc.h>
#include "ssp.h"
#include "ssp_dev.h"
#include "ssp_comm.h"
#include "ssp_dump.h"
#include "../staging/nanohub/chub.h"
#include "../staging/nanohub/chub_dbg.h"

int sensorhub_comms_read(void *ssp_data, u8 *buf, int length, int timeout)
{
	struct contexthub_ipc_info *ipc = ((struct ssp_data *)ssp_data)->platform_data;
	return contexthub_ipc_read(ipc, buf, length, timeout);
}

int sensorhub_comms_write(void *ssp_data, u8 *buf, int length, int timeout)
{
	struct contexthub_ipc_info *ipc = ((struct ssp_data *)ssp_data)->platform_data;
	int ret = contexthub_ipc_write(ipc, buf, length, timeout);
	if (!ret) {
		pr_err("%s : write fail\n", __func__);
	}
	return ret;
}

int sensorhub_reset(void *ssp_data)
{
	struct contexthub_ipc_info *ipc = ((struct ssp_data *)ssp_data)->platform_data;
	return contexthub_reset(ipc, 0, 0);
}

int sensorhub_firmware_download(void *ssp_data)
{
	struct contexthub_ipc_info *ipc = ((struct ssp_data *)ssp_data)->platform_data;
	return contexthub_reset(ipc, 1, 0);
}

int sensorhub_power_on(void *ssp_data)
{
	struct contexthub_ipc_info *ipc = ((struct ssp_data *)ssp_data)->platform_data;
	return contexthub_poweron(ipc);
}

int sensorhub_shutdown(void *ssp_data)
{
	struct contexthub_ipc_info *ipc = ((struct ssp_data *)ssp_data)->platform_data;
	int ret;
	ret = contexthub_ipc_write_event(ipc, MAILBOX_EVT_SHUTDOWN);
	if (ret)
	{
		ssp_errf("shutdonw fails, ret:%d\n", ret);
	}

	return ret;
}

void *ssp_device_probe(struct device *dev)
{
	return ssp_probe(dev);
}

void ssp_device_remove(void *ssp_data)
{
	struct ssp_data *data = ssp_data;
	ssp_remove(data);
}

void ssp_device_suspend(void *ssp_data)
{
	struct ssp_data *data = ssp_data;
	ssp_suspend(data);
}

void ssp_device_resume(void *ssp_data)
{
	struct ssp_data *data = ssp_data;
	ssp_resume(data);
}

void ssp_platform_init(void *ssp_data, void *platform_data)
{
	struct ssp_data *data = ssp_data;

	data->platform_data = platform_data;
}

void ssp_handle_recv_packet(void *ssp_data, char *packet, int packet_len)
{
	struct ssp_data *data = ssp_data;
	handle_packet(data, packet, packet_len);
}

void ssp_platform_start_refrsh_task(void *ssp_data)
{
	struct ssp_data *data = ssp_data;
	queue_refresh_task(data, 0);
}

enum
{
	DUMP_TYPE_INFO,
	DUMP_TYPE_DRAM,
	DUMP_TYPE_SRAM,
	DUMP_TYPE_MAX,
};

#define DUMP_FILES                                                       \
	{                                                                    \
		"SensorHubDump_Info", "SensorHubDump_Dram", "SensorHubDump_Sram" \
	}
#define DUMP_FILE_TYPES       \
	{                         \
		"txt", "dump", "dump" \
	}

int sensorhub_dump_get_num_dumptype(void *ssp_data)
{
	return DUMP_TYPE_MAX;
}

void sensorhub_dump_get_filename(void *ssp_data, int index, char *file_name, char *file_type)
{
	char ary_filename[DUMP_TYPE_MAX][30] = DUMP_FILES;
	char ary_filetype[DUMP_TYPE_MAX][5] = DUMP_FILE_TYPES;

	snprintf(file_name, sizeof(ary_filename[index]), ary_filename[index]);
	snprintf(file_type, sizeof(ary_filetype[index]), ary_filetype[index]);
}

void save_ram_dump(void *ssp_data, int reason)
{
	struct ssp_data *data = ssp_data;
	struct contexthub_ipc_info *ipc = data->platform_data;
	ssp_infof("");
	chub_dbg_dump_hw(ipc, CHUB_ERR_MAX + reason);
}

void ssp_dump_write_file(void *ssp_data, int reason, long long time, void *dram_buf, int dram_size, void *sram_buf, int sram_size, void *dbg_dump)
{
	struct ssp_data *data = ssp_data;
	struct dbg_dump *p_dbg = (struct dbg_dump *)dbg_dump;
	char ary_filename[DUMP_TYPE_MAX][30] = DUMP_FILES;
	char ary_filetype[DUMP_TYPE_MAX][5] = DUMP_FILE_TYPES;
	char dump_info[300] = {
		0,
	};
	u32 sec = time / NSEC_PER_SEC;
	struct timespec ts;
	struct rtc_time tm;
	unsigned int count;
	int i;
	if (data->cnt_dump < 0)
	{
		ssp_errf("do not prepare write dump file");
		return;
	}

	/*make dump info file content*/
	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);
	count = snprintf(dump_info, sizeof(dump_info), "%d-%02d-%02d %02d:%02d:%02d UTC %02u-%06u\n",
					 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, reason, sec);

	if (reason >= CHUB_ERR_MAX)
	{
		for (i = 0; i <= 15; i++)
			count += sprintf(dump_info + count, "R%02d : %08x\n", i, p_dbg->gpr[i]);

		count += sprintf(dump_info + count, "PC : %08x\n", p_dbg->gpr[GPR_PC_INDEX]);
	}

	data->cnt_dump++;
	ssp_infof("dump(%d) [%s]", data->cnt_dump, dump_info);

	write_ssp_dump_file(data, ary_filename[DUMP_TYPE_INFO], ary_filetype[DUMP_TYPE_INFO],
						dump_info, sizeof(dump_info));

	write_ssp_dump_file(data, ary_filename[DUMP_TYPE_DRAM], ary_filetype[DUMP_TYPE_DRAM],
						dram_buf, dram_size);

	write_ssp_dump_file(data, ary_filename[DUMP_TYPE_SRAM], ary_filetype[DUMP_TYPE_SRAM],
						sram_buf, sram_size);

	write_ssp_dump_count_file(data);

	if (reason < CHUB_ERR_MAX)
		sensorhub_reset(data);
}

bool is_sensorhub_working(void *ssp_data)
{
	struct contexthub_ipc_info *ipc = ((struct ssp_data *)ssp_data)->platform_data;
	if(atomic_read(&ipc->chub_status) == CHUB_ST_RUN)
		return true;
	else 
		return false;
}
