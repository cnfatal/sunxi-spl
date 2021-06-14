/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Constants and data structures used in Allwinner "eGON" images, as
 * parsed by the Boot-ROM.
 *
 * Shared between mkimage and the SPL.
 */
#include <sys/types.h>
#ifndef SUNXI_IMAGE_H
#define SUNXI_IMAGE_H

#define BOOT0_MAGIC "eGON.BT0"
#define BROM_STAMP_VALUE 0x5f0a6c39
#define SPL_SIGNATURE "SPL" /* marks "sunxi" SPL header */
#define SPL_MAJOR_BITS 3
#define SPL_MINOR_BITS 5
#define SPL_VERSION(maj, min)                                     \
	((((maj) & ((1U << SPL_MAJOR_BITS) - 1)) << SPL_MINOR_BITS) | \
	 ((min) & ((1U << SPL_MINOR_BITS) - 1)))

#define SPL_HEADER_VERSION SPL_VERSION(0, 2)

#define SPL_ENV_HEADER_VERSION SPL_VERSION(0, 1)
#define SPL_DT_HEADER_VERSION SPL_VERSION(0, 2)
#define SPL_DRAM_HEADER_VERSION SPL_VERSION(0, 3)

/* boot head definition from sun4i boot code */
struct boot_file_head
{
	uint32_t b_instruction; /* one intruction jumping to real code */
	uint8_t magic[8];		/* ="eGON.BT0" or "eGON.BT1", not C-style str */
	uint32_t check_sum;		/* generated by PC */
	uint32_t length;		/* generated by PC */
	/*
	 * We use a simplified header, only filling in what is needed
	 * by the boot ROM. To be compatible with Allwinner tools we
	 * would need to implement the proper fields here instead of
	 * padding.
	 *
	 * Actually we want the ability to recognize our "sunxi" variant
	 * of the SPL. To do so, let's place a special signature into the
	 * "pub_head_size" field. We can reasonably expect Allwinner's
	 * boot0 to always have the upper 16 bits of this set to 0 (after
	 * all the value shouldn't be larger than the limit imposed by
	 * SRAM size).
	 * If the signature is present (at 0x14), then we know it's safe
	 * to use the remaining 8 bytes (at 0x18) for our own purposes.
	 * (E.g. sunxi-tools "fel" utility can pass information there.)
	 */
	union
	{
		uint32_t pub_head_size;
		uint8_t spl_signature[4];
	};
	uint32_t fel_script_address; /* since v0.1, set by sunxi-fel */
	/*
	 * If the fel_uEnv_length member below is set to a non-zero value,
	 * it specifies the size (byte count) of data at fel_script_address.
	 * At the same time this indicates that the data is in uEnv.txt
	 * compatible format, ready to be imported via "env import -t".
	 */
	uint32_t fel_uEnv_length; /* since v0.1, set by sunxi-fel */
	/*
	 * Offset of an ASCIIZ string (relative to the SPL header), which
	 * contains the default device tree name (CONFIG_DEFAULT_DEVICE_TREE).
	 * This is optional and may be set to NULL. Is intended to be used
	 * by flash programming tools for providing nice informative messages
	 * to the users.
	 */
	uint32_t dt_name_offset; /* since v0.2, set by mksunxiboot */
	uint32_t dram_size;		 /* in MiB, since v0.3, set by SPL */
	uint32_t boot_media;	 /* written here by the boot ROM */
	/* A padding area (may be used for storing text strings) */
	uint32_t string_pool[13]; /* since v0.2, filled by mksunxiboot */
							  /* The header must be a multiple of 32 bytes (for VBAR alignment) */
};

/* Compile time check to assure proper alignment of structure */
typedef char boot_file_head_not_multiple_of_32[1 - 2 * (sizeof(struct boot_file_head) % 32)];

#endif
