/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Red Hat
 *
 * based in parts on udlfb.c:
 * Copyright (C) 2009 Roberto De Ioris <roberto@unbit.it>
 * Copyright (C) 2009 Jaya Kumar <jayakumar.lkml@gmail.com>
 * Copyright (C) 2009 Bernie Thompson <bernie@plugable.com>
 */

#ifndef UDL_DRV_H
#define UDL_DRV_H

#include <linux/mm_types.h>
#include <linux/usb.h>

#include <drm/drm_device.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem.h>

struct drm_encoder;
struct drm_mode_create_dumb;

#define DRIVER_NAME		"udl"
#define DRIVER_DESC		"DisplayLink"
#define DRIVER_DATE		"20120220"

#define DRIVER_MAJOR		0
#define DRIVER_MINOR		0
#define DRIVER_PATCHLEVEL	1

struct udl_device;

struct urb_node {
	struct list_head entry;
	struct udl_device *dev;
	struct delayed_work release_urb_work;
	struct urb *urb;
};

struct urb_list {
	struct list_head list;
	spinlock_t lock;
	struct semaphore limit_sem;
	int available;
	int count;
	size_t size;
};

struct udl_device {
	struct drm_device drm;
	struct device *dev;
	struct usb_device *udev;
	struct drm_crtc *crtc;

	/* active framebuffer on the 16-bit channel */
	const struct drm_framebuffer *active_fb_16;
	spinlock_t active_fb_16_lock;

	struct mutex gem_lock;

	int sku_pixel_limit;

	struct urb_list urbs;
	atomic_t lost_pixels; /* 1 = a render op failed. Need screen refresh */

	char mode_buf[1024];
	uint32_t mode_buf_len;
	atomic_t bytes_rendered; /* raw pixel-bytes driver asked to render */
	atomic_t bytes_identical; /* saved effort with backbuffer comparison */
	atomic_t bytes_sent; /* to usb, after compression including overhead */
	atomic_t cpu_kcycles_used; /* transpired during pixel processing */
};

#define to_udl(x) container_of(x, struct udl_device, drm)

/* modeset */
int udl_modeset_init(struct drm_device *dev);
void udl_modeset_restore(struct drm_device *dev);
void udl_modeset_cleanup(struct drm_device *dev);
int udl_connector_init(struct drm_device *dev, struct drm_encoder *encoder);

struct drm_encoder *udl_encoder_init(struct drm_device *dev);

struct urb *udl_get_urb(struct drm_device *dev);

int udl_submit_urb(struct drm_device *dev, struct urb *urb, size_t len);
void udl_urb_completion(struct urb *urb);

int udl_init(struct udl_device *udl);
void udl_fini(struct drm_device *dev);

struct drm_framebuffer *
udl_fb_user_fb_create(struct drm_device *dev,
		      struct drm_file *file,
		      const struct drm_mode_fb_cmd2 *mode_cmd);

int udl_render_hline(struct drm_device *dev, int log_bpp, struct urb **urb_ptr,
		     const char *front, char **urb_buf_ptr,
		     u32 byte_offset, u32 device_byte_offset, u32 byte_width,
		     int *ident_ptr, int *sent_ptr);

struct drm_gem_object *udl_driver_gem_create_object(struct drm_device *dev,
						    size_t size);

int udl_handle_damage(struct drm_framebuffer *fb, int x, int y,
		      int width, int height);

int udl_drop_usb(struct drm_device *dev);

#define CMD_WRITE_RAW8   "\xAF\x60" /**< 8 bit raw write command. */
#define CMD_WRITE_RL8    "\xAF\x61" /**< 8 bit run length command. */
#define CMD_WRITE_COPY8  "\xAF\x62" /**< 8 bit copy command. */
#define CMD_WRITE_RLX8   "\xAF\x63" /**< 8 bit extended run length command. */

#define CMD_WRITE_RAW16  "\xAF\x68" /**< 16 bit raw write command. */
#define CMD_WRITE_RL16   "\xAF\x69" /**< 16 bit run length command. */
#define CMD_WRITE_COPY16 "\xAF\x6A" /**< 16 bit copy command. */
#define CMD_WRITE_RLX16  "\xAF\x6B" /**< 16 bit extended run length command. */

#endif
