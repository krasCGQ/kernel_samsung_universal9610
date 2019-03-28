/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __CHUB_DEBUG_H
#define __CHUB_DEBUG_H

#include <linux/platform_device.h>
#include "chub.h"

enum dbg_dump_area {
	DBG_NANOHUB_DD_AREA,
	DBG_IPC_AREA,
	DBG_GPR_AREA,
	DBG_SRAM_AREA,
	DBG_AREA_MAX
};

#ifdef CONFIG_SENSORS_SSP
#define NUM_OF_GPR (17)
#define GPR_PC_INDEX (16)
#define AREA_NAME_MAX (8)
/* it's align ramdump side to prevent override */
#define SRAM_ALIGN (1024)
#define S_IRWUG (0660)

struct map_info {
	char name[AREA_NAME_MAX];
	u32 offset;
	u32 size;
};

struct dbg_dump {
	struct map_info info[DBG_AREA_MAX];
	long long time;
	int reason;
	struct contexthub_ipc_info chub;
	struct ipc_area ipc_addr[IPC_REG_MAX];
	u32 gpr[NUM_OF_GPR];
	int sram_start;
	char sram[];
};
#endif

int chub_dbg_init(struct contexthub_ipc_info *chub);
void *chub_dbg_get_memory(enum dbg_dump_area area);
void chub_dbg_dump_hw(struct contexthub_ipc_info *ipc, enum chub_err_type reason);
void chub_dbg_print_hw(struct contexthub_ipc_info *ipc);
int chub_dbg_check_and_download_image(struct contexthub_ipc_info *ipc);
void chub_dbg_dump_on_reset(struct contexthub_ipc_info *ipc);
#endif /* __CHUB_DEBUG_H */
