/*
 * Samsung Exynos SoC series VIPx driver
 *
 * Copyright (c) 2018 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/slab.h>

#include "vipx-log.h"
#include "vipx-core.h"
#include "vipx-context.h"
#include "vipx-ioctl.h"

static int __vipx_ioctl_get_graph(struct vs4l_graph *karg,
		struct vs4l_graph __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_from_user(karg, uarg, sizeof(*karg));
	if (ret) {
		vipx_err("Copy failed [S_GRAPH] (%d)\n", ret);
		goto p_err;
	}

	if (karg->size || karg->addr) {
		ret = -EINVAL;
		vipx_err("invalid parameters (%u/%#lx) [S_GRAPH]\n",
				karg->size, karg->addr);
		goto p_err;
	} else {
		karg->addr = 0;
	}

	vipx_leave();
	return 0;
p_err:
	return ret;
}

static void __vipx_ioctl_put_graph(struct vs4l_graph *karg,
		struct vs4l_graph __user *uarg)
{
	vipx_enter();
	vipx_leave();
}

static int __vipx_ioctl_get_format(struct vs4l_format_list *karg,
		struct vs4l_format_list __user *uarg)
{
	int ret;
	size_t size;
	struct vs4l_format __user *uformat;
	struct vs4l_format *kformat;

	vipx_enter();
	ret = copy_from_user(karg, uarg, sizeof(*karg));
	if (ret) {
		vipx_err("Copy failed [S_FORMAT] (%d)\n", ret);
		goto p_err;
	}

	uformat = karg->formats;

	size = karg->count * sizeof(*kformat);
	kformat = kzalloc(size, GFP_KERNEL);
	if (!kformat) {
		ret = -ENOMEM;
		vipx_err("Failed to alloc kformat (%zu)\n", size);
		goto p_err;
	}

	ret = copy_from_user(kformat, uformat, size);
	if (ret) {
		vipx_err("Copy failed [S_FORMAT] (%d)\n", ret);
		goto p_err_free;
	}

	karg->formats = kformat;

	vipx_leave();
	return 0;
p_err_free:
	kfree(kformat);
p_err:
	return ret;
}

static void __vipx_ioctl_put_format(struct vs4l_format_list *karg,
		struct vs4l_format_list __user *uarg)
{
	vipx_enter();
	kfree(karg->formats);
	vipx_leave();
}

static int __vipx_ioctl_get_param(struct vs4l_param_list *karg,
		struct vs4l_param_list __user *uarg)
{
	int ret;
	unsigned int idx;
	size_t size;
	struct vs4l_param __user *uparam;
	struct vs4l_param *kparam;

	vipx_enter();
	ret = copy_from_user(karg, uarg, sizeof(*karg));
	if (ret) {
		vipx_err("Copy failed [S_PARAM] (%d)\n", ret);
		goto p_err;
	}

	uparam = karg->params;

	size = karg->count * sizeof(*kparam);
	kparam = kzalloc(size, GFP_KERNEL);
	if (!kparam) {
		ret = -ENOMEM;
		vipx_err("Failed to alloc kparam (%zu)\n", size);
		goto p_err;
	}

	ret = copy_from_user(kparam, uparam, size);
	if (ret) {
		vipx_err("Copy failed [S_PARAM] (%d)\n", ret);
		goto p_err_free;
	}

	for (idx = 0; idx < karg->count; ++idx) {
		if (kparam[idx].size || kparam[idx].addr) {
			vipx_err("invalid parameters ([%d]%u/%#lx) [S_PARAM]\n",
					idx, kparam[idx].size,
					kparam[idx].addr);
			goto p_err_free;
		}
	}

	karg->params = kparam;
	vipx_leave();
	return 0;
p_err_free:
	kfree(kparam);
p_err:
	return ret;
}

static void __vipx_ioctl_put_param(struct vs4l_param_list *karg,
		struct vs4l_param_list __user *uarg)
{
	vipx_enter();
	kfree(karg->params);
	vipx_leave();
}

static int __vipx_ioctl_get_ctrl(struct vs4l_ctrl *karg,
		struct vs4l_ctrl __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_from_user(karg, uarg, sizeof(*karg));
	if (ret) {
		vipx_err("Copy failed [S_CTRL] (%d)\n", ret);
		goto p_err;
	}

	vipx_leave();
	return 0;
p_err:
	return ret;
}

static void __vipx_ioctl_put_ctrl(struct vs4l_ctrl *karg,
		struct vs4l_ctrl __user *uarg)
{
	vipx_enter();
	vipx_leave();
}

static int __vipx_ioctl_get_container(struct vs4l_container_list *karg,
		struct vs4l_container_list __user *uarg)
{
	int ret;
	size_t size;
	unsigned int idx;
	struct vs4l_container *ucon;
	struct vs4l_container *kcon;
	struct vs4l_buffer *ubuf;
	struct vs4l_buffer *kbuf;

	vipx_enter();
	ret = copy_from_user(karg, uarg, sizeof(*karg));
	if (ret) {
		vipx_err("Copy failed [CONTAINER] (%d)\n", ret);
		goto p_err;
	}

	ucon = karg->containers;

	size = karg->count * sizeof(*kcon);
	kcon = kzalloc(size, GFP_KERNEL);
	if (!kcon) {
		ret = -ENOMEM;
		vipx_err("Failed to alloc kcon (%zu)\n", size);
		goto p_err;
	}

	karg->containers = kcon;

	ret = copy_from_user(kcon, ucon, size);
	if (ret) {
		vipx_err("Copy failed [CONTAINER] (%d)\n", ret);
		goto p_err_free;
	}

	for (idx = 0; idx < karg->count; ++idx) {
		ubuf = kcon[idx].buffers;

		size = kcon[idx].count * sizeof(*kbuf);
		kbuf = kzalloc(size, GFP_KERNEL);
		if (!kbuf) {
			ret = -ENOMEM;
			vipx_err("Failed to alloc kbuf (%zu)\n", size);
			goto p_err_free;
		}

		kcon[idx].buffers = kbuf;

		ret = copy_from_user(kbuf, ubuf, size);
		if (ret) {
			vipx_err("Copy failed [CONTAINER] (%d)\n", ret);
			goto p_err_free;
		}
	}

	vipx_leave();
	return 0;
p_err_free:
	for (idx = 0; idx < karg->count; ++idx)
		kfree(kcon[idx].buffers);

	kfree(kcon);
p_err:
	return ret;
}

static void __vipx_ioctl_put_container(struct vs4l_container_list *karg,
		struct vs4l_container_list __user *uarg)
{
	int ret;
	unsigned int idx;

	vipx_enter();
	ret = copy_to_user(uarg, karg, sizeof(*karg));
	if (ret)
		vipx_err("Copy failed to user [CONTAINER]\n");

	for (idx = 0; idx < karg->count; ++idx)
		kfree(karg->containers[idx].buffers);

	kfree(karg->containers);
	vipx_leave();
}

static int __vipx_ioctl_get_load_kernel_binary(
		struct vipx_ioc_load_kernel_binary *karg,
		struct vipx_ioc_load_kernel_binary __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_from_user(karg, uarg, sizeof(*uarg));
	if (ret) {
		vipx_err("Copy failed [Load kernel binary] (%d)\n", ret);
		goto p_err;
	}

	memset(karg->timestamp, 0, sizeof(karg->timestamp));
	memset(karg->reserved, 0, sizeof(karg->reserved));

	vipx_leave();
	return 0;
p_err:
	return ret;
}

static void __vipx_ioctl_put_load_kernel_binary(
		struct vipx_ioc_load_kernel_binary *karg,
		struct vipx_ioc_load_kernel_binary __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_to_user(uarg, karg, sizeof(*karg));
	if (ret)
		vipx_err("Copy failed to user [Load kernel binary]\n");

	vipx_leave();
}

static int __vipx_ioctl_get_unload_kernel_binary(
		struct vipx_ioc_unload_kernel_binary *karg,
		struct vipx_ioc_unload_kernel_binary __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_from_user(karg, uarg, sizeof(*uarg));
	if (ret) {
		vipx_err("Copy failed [Unload kernel binary] (%d)\n", ret);
		goto p_err;
	}

	memset(karg->timestamp, 0, sizeof(karg->timestamp));
	memset(karg->reserved, 0, sizeof(karg->reserved));

	vipx_leave();
	return 0;
p_err:
	return ret;
}

static void __vipx_ioctl_put_unload_kernel_binary(
		struct vipx_ioc_unload_kernel_binary *karg,
		struct vipx_ioc_unload_kernel_binary __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_to_user(uarg, karg, sizeof(*karg));
	if (ret)
		vipx_err("Copy failed to user [Unload kernel binary]\n");

	vipx_leave();
}

static int __vipx_ioctl_get_load_graph_info(
		struct vipx_ioc_load_graph_info *karg,
		struct vipx_ioc_load_graph_info __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_from_user(karg, uarg, sizeof(*uarg));
	if (ret) {
		vipx_err("Copy failed [Load graph info] (%d)\n", ret);
		goto p_err;
	}

	memset(karg->timestamp, 0, sizeof(karg->timestamp));
	memset(karg->reserved, 0, sizeof(karg->reserved));

	vipx_leave();
	return 0;
p_err:
	return ret;
}

static void __vipx_ioctl_put_load_graph_info(
		struct vipx_ioc_load_graph_info *karg,
		struct vipx_ioc_load_graph_info __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_to_user(uarg, karg, sizeof(*karg));
	if (ret)
		vipx_err("Copy failed to user [Load graph info]\n");

	vipx_leave();
}

static int __vipx_ioctl_get_unload_graph_info(
		struct vipx_ioc_unload_graph_info *karg,
		struct vipx_ioc_unload_graph_info __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_from_user(karg, uarg, sizeof(*uarg));
	if (ret) {
		vipx_err("Copy failed [Unload graph info] (%d)\n", ret);
		goto p_err;
	}

	memset(karg->timestamp, 0, sizeof(karg->timestamp));
	memset(karg->reserved, 0, sizeof(karg->reserved));

	vipx_leave();
	return 0;
p_err:
	return ret;
}

static void __vipx_ioctl_put_unload_graph_info(
		struct vipx_ioc_unload_graph_info *karg,
		struct vipx_ioc_unload_graph_info __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_to_user(uarg, karg, sizeof(*karg));
	if (ret)
		vipx_err("Copy failed to user [Unload graph info]\n");

	vipx_leave();
}

static int __vipx_ioctl_get_execute_submodel(
		struct vipx_ioc_execute_submodel *karg,
		struct vipx_ioc_execute_submodel __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_from_user(karg, uarg, sizeof(*uarg));
	if (ret) {
		vipx_err("Copy failed [Execute submodel] (%d)\n", ret);
		goto p_err;
	}

	memset(karg->timestamp, 0, sizeof(karg->timestamp));
	memset(karg->reserved, 0, sizeof(karg->reserved));

	vipx_leave();
	return 0;
p_err:
	return ret;
}

static void __vipx_ioctl_put_execute_submodel(
		struct vipx_ioc_execute_submodel *karg,
		struct vipx_ioc_execute_submodel __user *uarg)
{
	int ret;

	vipx_enter();
	ret = copy_to_user(uarg, karg, sizeof(*karg));
	if (ret)
		vipx_err("Copy failed to user [Execute submodel]\n");

	vipx_leave();
}

long vipx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;
	struct vipx_context *vctx;
	const struct vipx_ioctl_ops *ops;
	union vipx_ioc_arg karg;
	void __user *uarg;

	vipx_enter();
	vctx = file->private_data;
	ops = vctx->core->ioc_ops;
	uarg = (void __user *)arg;

	switch (cmd) {
	case VS4L_VERTEXIOC_S_GRAPH:
		ret = __vipx_ioctl_get_graph(&karg.graph, uarg);
		if (ret)
			goto p_err;

		ret = ops->set_graph(vctx, &karg.graph);
		__vipx_ioctl_put_graph(&karg.graph, uarg);
		break;
	case VS4L_VERTEXIOC_S_FORMAT:
		ret = __vipx_ioctl_get_format(&karg.flist, uarg);
		if (ret)
			goto p_err;

		ret = ops->set_format(vctx, &karg.flist);
		__vipx_ioctl_put_format(&karg.flist, uarg);
		break;
	case VS4L_VERTEXIOC_S_PARAM:
		ret = __vipx_ioctl_get_param(&karg.plist, uarg);
		if (ret)
			goto p_err;

		ret = ops->set_param(vctx, &karg.plist);
		__vipx_ioctl_put_param(&karg.plist, uarg);
		break;
	case VS4L_VERTEXIOC_S_CTRL:
		ret = __vipx_ioctl_get_ctrl(&karg.ctrl, uarg);
		if (ret)
			goto p_err;

		ret = ops->set_ctrl(vctx, &karg.ctrl);
		__vipx_ioctl_put_ctrl(&karg.ctrl, uarg);
		break;
	case VS4L_VERTEXIOC_STREAM_ON:
		ret = ops->streamon(vctx);
		break;
	case VS4L_VERTEXIOC_STREAM_OFF:
		ret = ops->streamoff(vctx);
		break;
	case VS4L_VERTEXIOC_QBUF:
		ret = __vipx_ioctl_get_container(&karg.clist, uarg);
		if (ret)
			goto p_err;

		ret = ops->qbuf(vctx, &karg.clist);
		__vipx_ioctl_put_container(&karg.clist, uarg);
		break;
	case VS4L_VERTEXIOC_DQBUF:
		ret = __vipx_ioctl_get_container(&karg.clist, uarg);
		if (ret)
			goto p_err;

		ret = ops->dqbuf(vctx, &karg.clist);
		__vipx_ioctl_put_container(&karg.clist, uarg);
		break;
	case VIPX_IOC_LOAD_KERNEL_BINARY:
		ret = __vipx_ioctl_get_load_kernel_binary(&karg.kernel_bin,
				uarg);
		if (ret)
			goto p_err;

		ret = ops->load_kernel_binary(vctx, &karg.kernel_bin);
		__vipx_ioctl_put_load_kernel_binary(&karg.kernel_bin, uarg);
		break;
	case VIPX_IOC_UNLOAD_KERNEL_BINARY:
		ret = __vipx_ioctl_get_unload_kernel_binary(&karg.unload_kbin,
				uarg);
		if (ret)
			goto p_err;

		ret = ops->unload_kernel_binary(vctx, &karg.unload_kbin);
		__vipx_ioctl_put_unload_kernel_binary(&karg.unload_kbin, uarg);
		break;
	case VIPX_IOC_LOAD_GRAPH_INFO:
		ret = __vipx_ioctl_get_load_graph_info(&karg.load_ginfo,
				uarg);
		if (ret)
			goto p_err;

		ret = ops->load_graph_info(vctx, &karg.load_ginfo);
		__vipx_ioctl_put_load_graph_info(&karg.load_ginfo, uarg);
		break;
	case VIPX_IOC_UNLOAD_GRAPH_INFO:
		ret = __vipx_ioctl_get_unload_graph_info(&karg.unload_ginfo,
				uarg);
		if (ret)
			goto p_err;

		ret = ops->unload_graph_info(vctx, &karg.unload_ginfo);
		__vipx_ioctl_put_unload_graph_info(&karg.unload_ginfo, uarg);
		break;
	case VIPX_IOC_EXECUTE_SUBMODEL:
		ret = __vipx_ioctl_get_execute_submodel(&karg.exec, uarg);
		if (ret)
			goto p_err;

		ret = ops->execute_submodel(vctx, &karg.exec);
		__vipx_ioctl_put_execute_submodel(&karg.exec, uarg);
		break;

	default:
		ret = -EINVAL;
		vipx_err("ioc command(%x) is not supported\n", cmd);
		goto p_err;
	}

	vipx_leave();
p_err:
	return ret;
}
