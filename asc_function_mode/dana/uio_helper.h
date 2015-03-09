/*
   uio_helper.h
   UIO helper functions - header file.

   Copyright (C) 2007 Hans J. Koch

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   MODIFIED BY: Jonathan Appavoo Boston University 2015
*/

#ifndef UIO_HELPER_H
#define UIO_HELPER_H

#define UIO_MAX_NAME_SIZE	64
#define UIO_MAX_NUM		255

#define UIO_INVALID_SIZE	-1
#define UIO_INVALID_ADDR	(~0)

#define UIO_MMAP_NOT_DONE	0
#define UIO_MMAP_OK		1
#define UIO_MMAP_FAILED		2

/* This should be identical to the define in include/linux/uio_driver.h */
#define MAX_UIO_MAPS 	5

struct uio_map_t {
        unsigned long addr;
	int size;
        int fd;
        void *map_addr;
	int mmap_status;
};

struct uio_dev_attr_t {
	char name[ UIO_MAX_NAME_SIZE ];
	char value[ UIO_MAX_NAME_SIZE ];
	struct uio_dev_attr_t *next;
};

struct uio_info_t {
	int uio_num;
	struct uio_map_t maps[ MAX_UIO_MAPS ];
	unsigned long event_count;
	char name[ UIO_MAX_NAME_SIZE ];
	char version[ UIO_MAX_NAME_SIZE ];
	struct uio_dev_attr_t *dev_attrs;
	struct uio_info_t* next;  /* for linked list */
};

/* function prototypes */

int uio_get_mem_size(struct uio_info_t* info, int map_num);
int uio_get_mem_addr(struct uio_info_t* info, int map_num);
int uio_get_event_count(struct uio_info_t* info);
int uio_get_name(struct uio_info_t* info);
int uio_get_version(struct uio_info_t* info);
int uio_get_all_info(struct uio_info_t* info);
int uio_get_device_attributes(struct uio_info_t* info);
void uio_free_dev_attrs(struct uio_info_t* info);
void uio_free_info(struct uio_info_t* info);
struct uio_info_t* uio_find_devices(int filter_num);
// JA: Moved into common routines
void uio_show_dev_attrs(struct uio_info_t *info, FILE *f);
int uio_show_single_map(struct uio_info_t *info, int map_num, FILE *f);
int uio_show_maps(struct uio_info_t *info, FILE *f);
void uio_show_device(struct uio_info_t *info, FILE *f);
void uio_single_release_dev(struct uio_info_t *info);
void uio_single_unmmap(struct uio_info_t* info, int map_num);
void uio_mmap_all(struct uio_info_t* info);
void uio_unmap_all(struct uio_info_t* info);
#endif
