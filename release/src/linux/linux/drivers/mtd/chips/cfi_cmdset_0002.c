/*
 * Common Flash Interface support:
 *   AMD & Fujitsu Standard Vendor Command Set (ID 0x0002)
 *
 * Copyright (C) 2000 Crossnet Co. <info@crossnet.co.jp>
 *
 * 2_by_8 routines added by Simon Munton
 *
 * This code is GPL
 *
 * $Id: cfi_cmdset_0002.c,v 1.1.1.4 2003/10/14 08:08:17 sparq Exp $
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/byteorder.h>

#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/mtd/map.h>
#include <linux/mtd/cfi.h>

#define AMD_BOOTLOC_BUG

static int cfi_amdstd_read (struct mtd_info *, loff_t, size_t, size_t *, u_char *);
static int cfi_amdstd_write(struct mtd_info *, loff_t, size_t, size_t *, const u_char *);
static int cfi_amdstd_erase_onesize(struct mtd_info *, struct erase_info *);
static int cfi_amdstd_erase_varsize(struct mtd_info *, struct erase_info *);
static void cfi_amdstd_sync (struct mtd_info *);
static int cfi_amdstd_suspend (struct mtd_info *);
static void cfi_amdstd_resume (struct mtd_info *);

static void cfi_amdstd_destroy(struct mtd_info *);

struct mtd_info *cfi_cmdset_0002(struct map_info *, int);
static struct mtd_info *cfi_amdstd_setup (struct map_info *);


static struct mtd_chip_driver cfi_amdstd_chipdrv = {
	probe: NULL, /* Not usable directly */
	destroy: cfi_amdstd_destroy,
	name: "cfi_cmdset_0002",
	module: THIS_MODULE
};

struct mtd_info *cfi_cmdset_0002(struct map_info *map, int primary)
{
	struct cfi_private *cfi = map->fldrv_priv;
	unsigned char bootloc;
	int ofs_factor = cfi->interleave * cfi->device_type;
	int i;
	__u8 major, minor;
	__u32 base = cfi->chips[0].start;

	if (cfi->cfi_mode==1){
		__u16 adr = primary?cfi->cfiq->P_ADR:cfi->cfiq->A_ADR;

		cfi_send_gen_cmd(0x98, 0x55, base, map, cfi, cfi->device_type, NULL);
		
		major = cfi_read_query(map, base + (adr+3)*ofs_factor);
		minor = cfi_read_query(map, base + (adr+4)*ofs_factor);
		
		printk(KERN_NOTICE " Amd/Fujitsu Extended Query Table v%c.%c at 0x%4.4X\n",
		       major, minor, adr);
				cfi_send_gen_cmd(0xf0, 0x55, base, map, cfi, cfi->device_type, NULL);
		
		cfi_send_gen_cmd(0xaa, 0x555, base, map, cfi, cfi->device_type, NULL);
		cfi_send_gen_cmd(0x55, 0x2aa, base, map, cfi, cfi->device_type, NULL);
		cfi_send_gen_cmd(0x90, 0x555, base, map, cfi, cfi->device_type, NULL);
		cfi->mfr = cfi_read_query(map, base);
		cfi->id = cfi_read_query(map, base + ofs_factor);

		printk(KERN_NOTICE " Flash Id: Vendor: 0x%04x Device: 0x%04x\n", cfi->mfr, cfi->id);

		/* Wheee. Bring me the head of someone at AMD. */
#ifdef AMD_BOOTLOC_BUG
		if (((major << 8) | minor) < 0x3131) {
			/* CFI version 1.0 => don't trust bootloc */
			if (cfi->id & 0x80) {
				printk(KERN_WARNING "%s: JEDEC Device ID is 0x%02X. Assuming broken CFI table.\n", map->name, cfi->id);
				bootloc = 3;	/* top boot */
			} else {
				bootloc = 2;	/* bottom boot */
			}
		} else
#endif
			{
				cfi_send_gen_cmd(0x98, 0x55, base, map, cfi, cfi->device_type, NULL);
				bootloc = cfi_read_query(map, base + (adr+15)*ofs_factor);
			}
		if (bootloc == 3 && cfi->cfiq->NumEraseRegions > 1) {
			printk(KERN_WARNING "%s: Swapping erase regions for broken CFI table.\n", map->name);
			
			for (i=0; i<cfi->cfiq->NumEraseRegions / 2; i++) {
				int j = (cfi->cfiq->NumEraseRegions-1)-i;
				__u32 swap;
				
				swap = cfi->cfiq->EraseRegionInfo[i];
				cfi->cfiq->EraseRegionInfo[i] = cfi->cfiq->EraseRegionInfo[j];
				cfi->cfiq->EraseRegionInfo[j] = swap;
			}
		}
		switch (cfi->device_type) {
		case CFI_DEVICETYPE_X8:
			cfi->addr_unlock1 = 0x555; 
			cfi->addr_unlock2 = 0x2aa; 
			break;
		case CFI_DEVICETYPE_X16:
			cfi->addr_unlock1 = 0xaaa;
			if (map->buswidth == cfi->interleave) {
				/* X16 chip(s) in X8 mode */
				cfi->addr_unlock2 = 0x555;
			} else {
				cfi->addr_unlock2 = 0x554;
			}
			break;
		case CFI_DEVICETYPE_X32:
			cfi->addr_unlock1 = 0x1555; 
			cfi->addr_unlock2 = 0xaaa; 
			break;
		default:
			printk(KERN_NOTICE "Eep. Unknown cfi_cmdset_0002 device type %d\n", cfi->device_type);
			return NULL;
		}
	} /* CFI mode */

	for (i=0; i< cfi->numchips; i++) {
		cfi->chips[i].word_write_time = 1<<cfi->cfiq->WordWriteTimeoutTyp;
		cfi->chips[i].buffer_write_time = 1<<cfi->cfiq->BufWriteTimeoutTyp;
		cfi->chips[i].erase_time = 1<<cfi->cfiq->BlockEraseTimeoutTyp;
	}		
	
	map->fldrv = &cfi_amdstd_chipdrv;
	MOD_INC_USE_COUNT;

	cfi_send_gen_cmd(0xf0, 0x55, base, map, cfi, cfi->device_type, NULL);
	return cfi_amdstd_setup(map);
}

static struct mtd_info *cfi_amdstd_setup(struct map_info *map)
{
	struct cfi_private *cfi = map->fldrv_priv;
	struct mtd_info *mtd;
	unsigned long devsize = (1<<cfi->cfiq->DevSize) * cfi->interleave;

	mtd = kmalloc(sizeof(*mtd), GFP_KERNEL);
	printk(KERN_NOTICE "number of %s chips: %d\n", (cfi->cfi_mode)?"CFI":"JEDEC",cfi->numchips);

	if (!mtd) {
	  printk(KERN_WARNING "Failed to allocate memory for MTD device\n");
	  kfree(cfi->cmdset_priv);
	  return NULL;
	}

	memset(mtd, 0, sizeof(*mtd));
	mtd->priv = map;
	mtd->type = MTD_NORFLASH;
	/* Also select the correct geometry setup too */ 
	mtd->size = devsize * cfi->numchips;
	
	if (cfi->cfiq->NumEraseRegions == 1) {
		/* No need to muck about with multiple erase sizes */
		mtd->erasesize = ((cfi->cfiq->EraseRegionInfo[0] >> 8) & ~0xff) * cfi->interleave;
	} else {
		unsigned long offset = 0;
		int i,j;

		mtd->numeraseregions = cfi->cfiq->NumEraseRegions * cfi->numchips;
		mtd->eraseregions = kmalloc(sizeof(struct mtd_erase_region_info) * mtd->numeraseregions, GFP_KERNEL);
		if (!mtd->eraseregions) { 
			printk(KERN_WARNING "Failed to allocate memory for MTD erase region info\n");
			kfree(cfi->cmdset_priv);
			return NULL;
		}
			
		for (i=0; i<cfi->cfiq->NumEraseRegions; i++) {
			unsigned long ernum, ersize;
			ersize = ((cfi->cfiq->EraseRegionInfo[i] >> 8) & ~0xff) * cfi->interleave;
			ernum = (cfi->cfiq->EraseRegionInfo[i] & 0xffff) + 1;
			
			if (mtd->erasesize < ersize) {
				mtd->erasesize = ersize;
			}
			for (j=0; j<cfi->numchips; j++) {
				mtd->eraseregions[(j*cfi->cfiq->NumEraseRegions)+i].offset = (j*devsize)+offset;
				mtd->eraseregions[(j*cfi->cfiq->NumEraseRegions)+i].erasesize = ersize;
				mtd->eraseregions[(j*cfi->cfiq->NumEraseRegions)+i].numblocks = ernum;
			}
			offset += (ersize * ernum);
		}
		if (offset != devsize) {
			/* Argh */
			printk(KERN_WARNING "Sum of regions (%lx) != total size of set of interleaved chips (%lx)\n", offset, devsize);
			kfree(mtd->eraseregions);
			kfree(cfi->cmdset_priv);
			return NULL;
		}
	}

	switch (CFIDEV_BUSWIDTH)
	{
	case 1:
	case 2:
	case 4:
		if (mtd->numeraseregions > 1)
			mtd->erase = cfi_amdstd_erase_varsize;
		else
			mtd->erase = cfi_amdstd_erase_onesize;
		mtd->read = cfi_amdstd_read;
		mtd->write = cfi_amdstd_write;
		break;

	default:
	        printk(KERN_WARNING "Unsupported buswidth\n");
		kfree(mtd);
		kfree(cfi->cmdset_priv);
		return NULL;
		break;
	}
	mtd->sync = cfi_amdstd_sync;
	mtd->suspend = cfi_amdstd_suspend;
	mtd->resume = cfi_amdstd_resume;
	mtd->flags = MTD_CAP_NORFLASH;
	map->fldrv = &cfi_amdstd_chipdrv;
	mtd->name = map->name;
	MOD_INC_USE_COUNT;
	return mtd;
}

static inline int do_read_onechip(struct map_info *map, struct flchip *chip, loff_t adr, size_t len, u_char *buf)
{
	DECLARE_WAITQUEUE(wait, current);
	unsigned long timeo = jiffies + HZ;

 retry:
	cfi_spin_lock(chip->mutex);

	if (chip->state != FL_READY){
		set_current_state(TASK_UNINTERRUPTIBLE);
		add_wait_queue(&chip->wq, &wait);
                
		cfi_spin_unlock(chip->mutex);

		schedule();
		remove_wait_queue(&chip->wq, &wait);
		timeo = jiffies + HZ;

		goto retry;
	}	

	adr += chip->start;

	chip->state = FL_READY;

	map->copy_from(map, buf, adr, len);

	wake_up(&chip->wq);
	cfi_spin_unlock(chip->mutex);

	return 0;
}

static int cfi_amdstd_read (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	struct map_info *map = mtd->priv;
	struct cfi_private *cfi = map->fldrv_priv;
	unsigned long ofs;
	int chipnum;
	int ret = 0;

	/* ofs: offset within the first chip that the first read should start */

	chipnum = (from >> cfi->chipshift);
	ofs = from - (chipnum <<  cfi->chipshift);


	*retlen = 0;

	while (len) {
		unsigned long thislen;

		if (chipnum >= cfi->numchips)
			break;

		if ((len + ofs -1) >> cfi->chipshift)
			thislen = (1<<cfi->chipshift) - ofs;
		else
			thislen = len;

		ret = do_read_onechip(map, &cfi->chips[chipnum], ofs, thislen, buf);
		if (ret)
			break;

		*retlen += thislen;
		len -= thislen;
		buf += thislen;

		ofs = 0;
		chipnum++;
	}
	return ret;
}

static int do_write_oneword(struct map_info *map, struct flchip *chip, unsigned long adr, __u32 datum, int fast)
{
	unsigned long timeo = jiffies + HZ;
	unsigned int Last[4];
	unsigned long Count = 0;
	struct cfi_private *cfi = map->fldrv_priv;
	DECLARE_WAITQUEUE(wait, current);
	int ret = 0;

 retry:
	cfi_spin_lock(chip->mutex);

	if (chip->state != FL_READY){
		set_current_state(TASK_UNINTERRUPTIBLE);
		add_wait_queue(&chip->wq, &wait);
                
		cfi_spin_unlock(chip->mutex);

		schedule();
		remove_wait_queue(&chip->wq, &wait);
		timeo = jiffies + HZ;

		goto retry;
	}	

	chip->state = FL_WRITING;

	adr += chip->start;
	ENABLE_VPP(map);
	if (fast) { /* Unlock bypass */
		cfi_send_gen_cmd(0xA0, 0, chip->start, map, cfi, cfi->device_type, NULL);
	}
	else {
	        cfi_send_gen_cmd(0xAA, cfi->addr_unlock1, chip->start, map, cfi, CFI_DEVICETYPE_X8, NULL);
	        cfi_send_gen_cmd(0x55, cfi->addr_unlock2, chip->start, map, cfi, CFI_DEVICETYPE_X8, NULL);
	        cfi_send_gen_cmd(0xA0, cfi->addr_unlock1, chip->start, map, cfi, CFI_DEVICETYPE_X8, NULL);
	}

	cfi_write(map, datum, adr);

	cfi_spin_unlock(chip->mutex);
	cfi_udelay(chip->word_write_time);
	cfi_spin_lock(chip->mutex);

	Last[0] = cfi_read(map, adr);
	//	printk("Last[0] is %x\n", Last[0]);
	Last[1] = cfi_read(map, adr);
	//	printk("Last[1] is %x\n", Last[1]);
	Last[2] = cfi_read(map, adr);
	//	printk("Last[2] is %x\n", Last[2]);

	for (Count = 3; Last[(Count - 1) % 4] != Last[(Count - 2) % 4] && Count < 10000; Count++){
		cfi_spin_unlock(chip->mutex);
		cfi_udelay(10);
		cfi_spin_lock(chip->mutex);
		
	        Last[Count % 4] = cfi_read(map, adr);
		//		printk("Last[%d%%4] is %x\n", Count, Last[Count%4]);
	}
	
	if (Last[(Count - 1) % 4] != datum){
		printk(KERN_WARNING "Last[%ld] is %x, datum is %x\n",(Count - 1) % 4,Last[(Count - 1) % 4],datum);
	        cfi_send_gen_cmd(0xF0, 0, chip->start, map, cfi, cfi->device_type, NULL);
		DISABLE_VPP(map);
		ret = -EIO;
	}       
	DISABLE_VPP(map);
	chip->state = FL_READY;
	wake_up(&chip->wq);
	cfi_spin_unlock(chip->mutex);
	
	return ret;
}

static int cfi_amdstd_write (struct mtd_info *mtd, loff_t to , size_t len, size_t *retlen, const u_char *buf)
{
	struct map_info *map = mtd->priv;
	struct cfi_private *cfi = map->fldrv_priv;
	int ret = 0;
	int chipnum;
	unsigned long ofs, chipstart;

	*retlen = 0;
	if (!len)
		return 0;

	chipnum = to >> cfi->chipshift;
	ofs = to  - (chipnum << cfi->chipshift);
	chipstart = cfi->chips[chipnum].start;

	/* If it's not bus-aligned, do the first byte write */
	if (ofs & (CFIDEV_BUSWIDTH-1)) {
		unsigned long bus_ofs = ofs & ~(CFIDEV_BUSWIDTH-1);
		int i = ofs - bus_ofs;
		int n = 0;
		u_char tmp_buf[4];
		__u32 datum;

		map->copy_from(map, tmp_buf, bus_ofs + cfi->chips[chipnum].start, CFIDEV_BUSWIDTH);
		while (len && i < CFIDEV_BUSWIDTH)
			tmp_buf[i++] = buf[n++], len--;

		if (cfi_buswidth_is_2()) {
			datum = *(__u16*)tmp_buf;
		} else if (cfi_buswidth_is_4()) {
			datum = *(__u32*)tmp_buf;
		} else {
			return -EINVAL;  /* should never happen, but be safe */
		}

		ret = do_write_oneword(map, &cfi->chips[chipnum], 
				bus_ofs, datum, 0);
		if (ret) 
			return ret;
		
		ofs += n;
		buf += n;
		(*retlen) += n;

		if (ofs >> cfi->chipshift) {
			chipnum ++; 
			ofs = 0;
			if (chipnum == cfi->numchips)
				return 0;
		}
	}
	
	/* Go into unlock bypass mode */
	cfi_send_gen_cmd(0xAA, cfi->addr_unlock1, chipstart, map, cfi, CFI_DEVICETYPE_X8, NULL);
	cfi_send_gen_cmd(0x55, cfi->addr_unlock2, chipstart, map, cfi, CFI_DEVICETYPE_X8, NULL);
	cfi_send_gen_cmd(0x20, cfi->addr_unlock1, chipstart, map, cfi, CFI_DEVICETYPE_X8, NULL);

	/* We are now aligned, write as much as possible */
	while(len >= CFIDEV_BUSWIDTH) {
		__u32 datum;

		if (cfi_buswidth_is_1()) {
			datum = *(__u8*)buf;
		} else if (cfi_buswidth_is_2()) {
			datum = *(__u16*)buf;
		} else if (cfi_buswidth_is_4()) {
			datum = *(__u32*)buf;
		} else {
			return -EINVAL;
		}
		ret = do_write_oneword(map, &cfi->chips[chipnum],
				       ofs, datum, cfi->fast_prog);
		if (ret) {
			if (cfi->fast_prog){
				/* Get out of unlock bypass mode */
				cfi_send_gen_cmd(0x90, 0, chipstart, map, cfi, cfi->device_type, NULL);
				cfi_send_gen_cmd(0x00, 0, chipstart, map, cfi, cfi->device_type, NULL);
			}
			return ret;
		}

		ofs += CFIDEV_BUSWIDTH;
		buf += CFIDEV_BUSWIDTH;
		(*retlen) += CFIDEV_BUSWIDTH;
		len -= CFIDEV_BUSWIDTH;

		if (ofs >> cfi->chipshift) {
			if (cfi->fast_prog){
				/* Get out of unlock bypass mode */
				cfi_send_gen_cmd(0x90, 0, chipstart, map, cfi, cfi->device_type, NULL);
				cfi_send_gen_cmd(0x00, 0, chipstart, map, cfi, cfi->device_type, NULL);
			}

			chipnum ++; 
			ofs = 0;
			if (chipnum == cfi->numchips)
				return 0;
			chipstart = cfi->chips[chipnum].start;
			if (cfi->fast_prog){
				/* Go into unlock bypass mode for next set of chips */
				cfi_send_gen_cmd(0xAA, cfi->addr_unlock1, chipstart, map, cfi, CFI_DEVICETYPE_X8, NULL);
				cfi_send_gen_cmd(0x55, cfi->addr_unlock2, chipstart, map, cfi, CFI_DEVICETYPE_X8, NULL);
				cfi_send_gen_cmd(0x20, cfi->addr_unlock1, chipstart, map, cfi, CFI_DEVICETYPE_X8, NULL);
			}
		}
	}

	if (cfi->fast_prog){
		/* Get out of unlock bypass mode */
		cfi_send_gen_cmd(0x90, 0, chipstart, map, cfi, cfi->device_type, NULL);
		cfi_send_gen_cmd(0x00, 0, chipstart, map, cfi, cfi->device_type, NULL);
	}

	if (len & (CFIDEV_BUSWIDTH-1)) {
		int i = 0, n = 0;
		u_char tmp_buf[4];
		__u32 datum;

		map->copy_from(map, tmp_buf, ofs + cfi->chips[chipnum].start, CFIDEV_BUSWIDTH);
		while (len--)
			tmp_buf[i++] = buf[n++];

		if (cfi_buswidth_is_2()) {
			datum = *(__u16*)tmp_buf;
		} else if (cfi_buswidth_is_4()) {
			datum = *(__u32*)tmp_buf;
		} else {
			return -EINVAL;  /* should never happen, but be safe */
		}

		ret = do_write_oneword(map, &cfi->chips[chipnum], 
				ofs, datum, 0);
		if (ret) 
			return ret;
		
		(*retlen) += n;
	}

	return 0;
}

static inline int do_erase_oneblock(struct map_info *map, struct flchip *chip, unsigned long adr)
{
	unsigned int status;
	unsigned long timeo = jiffies + HZ;
	struct cfi_private *cfi = map->fldrv_priv;
	unsigned int rdy_mask;
	DECLARE_WAITQUEUE(wait, current);

 retry:
	cfi_spin_lock(chip->mutex);

	if (chip->state != FL_READY){
		set_current_state(TASK_UNINTERRUPTIBLE);
		add_wait_queue(&chip->wq, &wait);
                
		cfi_spin_unlock(chip->mutex);

		schedule();
		remove_wait_queue(&chip->wq, &wait);
		timeo = jiffies + HZ;

		goto retry;
	}	

	chip->state = FL_ERASING;

	adr += chip->start;
	ENABLE_VPP(map);
	cfi_send_gen_cmd(0xAA, cfi->addr_unlock1, chip->start, map, cfi, CFI_DEVICETYPE_X8, NULL);
	cfi_send_gen_cmd(0x55, cfi->addr_unlock2, chip->start, map, cfi, CFI_DEVICETYPE_X8, NULL);
	cfi_send_gen_cmd(0x80, cfi->addr_unlock1, chip->start, map, cfi, CFI_DEVICETYPE_X8, NULL);
	cfi_send_gen_cmd(0xAA, cfi->addr_unlock1, chip->start, map, cfi, CFI_DEVICETYPE_X8, NULL);
	cfi_send_gen_cmd(0x55, cfi->addr_unlock2, chip->start, map, cfi, CFI_DEVICETYPE_X8, NULL);
	cfi_write(map, CMD(0x30), adr);
	
	timeo = jiffies + (HZ*20);

	cfi_spin_unlock(chip->mutex);
	schedule_timeout(HZ);
	cfi_spin_lock(chip->mutex);
	
	rdy_mask = CMD(0x80);

	/* Once the state machine's known to be working I'll do that */

	while ( ( (status = cfi_read(map,adr)) & rdy_mask ) != rdy_mask ) {
		static int z=0;

		if (chip->state != FL_ERASING) {
			/* Someone's suspended the erase. Sleep */
			set_current_state(TASK_UNINTERRUPTIBLE);
			add_wait_queue(&chip->wq, &wait);
			
			cfi_spin_unlock(chip->mutex);
			printk(KERN_DEBUG "erase suspended. Sleeping\n");
			
			schedule();
			remove_wait_queue(&chip->wq, &wait);
			timeo = jiffies + (HZ*2); 
			cfi_spin_lock(chip->mutex);
			continue;
		}

		/* OK Still waiting */
		if (time_after(jiffies, timeo)) {
			chip->state = FL_READY;
			cfi_spin_unlock(chip->mutex);
			printk(KERN_WARNING "waiting for erase to complete timed out.");
			DISABLE_VPP(map);
			return -EIO;
		}
		
		/* Latency issues. Drop the lock, wait a while and retry */
		cfi_spin_unlock(chip->mutex);

		z++;
		if ( 0 && !(z % 100 )) 
			printk(KERN_WARNING "chip not ready yet after erase. looping\n");

		cfi_udelay(1);
		
		cfi_spin_lock(chip->mutex);
		continue;
	}
	
	/* Done and happy. */
	DISABLE_VPP(map);
	chip->state = FL_READY;
	wake_up(&chip->wq);
	cfi_spin_unlock(chip->mutex);
	return 0;
}

static int cfi_amdstd_erase_varsize(struct mtd_info *mtd, struct erase_info *instr)
{
	struct map_info *map = mtd->priv;
	struct cfi_private *cfi = map->fldrv_priv;
	unsigned long adr, len;
	int chipnum, ret = 0;
	int i, first;
	struct mtd_erase_region_info *regions = mtd->eraseregions;

	if (instr->addr > mtd->size)
		return -EINVAL;

	if ((instr->len + instr->addr) > mtd->size)
		return -EINVAL;

	/* Check that both start and end of the requested erase are
	 * aligned with the erasesize at the appropriate addresses.
	 */

	i = 0;

	/* Skip all erase regions which are ended before the start of 
	   the requested erase. Actually, to save on the calculations,
	   we skip to the first erase region which starts after the
	   start of the requested erase, and then go back one.
	*/
	
	while (i < mtd->numeraseregions && instr->addr >= regions[i].offset)
	       i++;
	i--;

	/* OK, now i is pointing at the erase region in which this 
	   erase request starts. Check the start of the requested
	   erase range is aligned with the erase size which is in
	   effect here.
	*/

	if (instr->addr & (regions[i].erasesize-1))
		return -EINVAL;

	/* Remember the erase region we start on */
	first = i;

	/* Next, check that the end of the requested erase is aligned
	 * with the erase region at that address.
	 */

	while (i<mtd->numeraseregions && (instr->addr + instr->len) >= regions[i].offset)
		i++;

	/* As before, drop back one to point at the region in which
	   the address actually falls
	*/
	i--;
	
	if ((instr->addr + instr->len) & (regions[i].erasesize-1))
		return -EINVAL;
	
	chipnum = instr->addr >> cfi->chipshift;
	adr = instr->addr - (chipnum << cfi->chipshift);
	len = instr->len;

	i=first;

	while(len) {
		ret = do_erase_oneblock(map, &cfi->chips[chipnum], adr);

		if (ret)
			return ret;

		adr += regions[i].erasesize;
		len -= regions[i].erasesize;

		if (adr % (1<< cfi->chipshift) == ((regions[i].offset + (regions[i].erasesize * regions[i].numblocks)) %( 1<< cfi->chipshift)))
			i++;

		if (adr >> cfi->chipshift) {
			adr = 0;
			chipnum++;
			
			if (chipnum >= cfi->numchips)
			break;
		}
	}

	instr->state = MTD_ERASE_DONE;
	if (instr->callback)
		instr->callback(instr);
	
	return 0;
}

static int cfi_amdstd_erase_onesize(struct mtd_info *mtd, struct erase_info *instr)
{
	struct map_info *map = mtd->priv;
	struct cfi_private *cfi = map->fldrv_priv;
	unsigned long adr, len;
	int chipnum, ret = 0;

	if (instr->addr & (mtd->erasesize - 1))
		return -EINVAL;

	if (instr->len & (mtd->erasesize -1))
		return -EINVAL;

	if ((instr->len + instr->addr) > mtd->size)
		return -EINVAL;

	chipnum = instr->addr >> cfi->chipshift;
	adr = instr->addr - (chipnum << cfi->chipshift);
	len = instr->len;

	while(len) {
		ret = do_erase_oneblock(map, &cfi->chips[chipnum], adr);

		if (ret)
			return ret;

		adr += mtd->erasesize;
		len -= mtd->erasesize;

		if (adr >> cfi->chipshift) {
			adr = 0;
			chipnum++;
			
			if (chipnum >= cfi->numchips)
			break;
		}
	}
		
	instr->state = MTD_ERASE_DONE;
	if (instr->callback)
		instr->callback(instr);
	
	return 0;
}

static void cfi_amdstd_sync (struct mtd_info *mtd)
{
	struct map_info *map = mtd->priv;
	struct cfi_private *cfi = map->fldrv_priv;
	int i;
	struct flchip *chip;
	int ret = 0;
	DECLARE_WAITQUEUE(wait, current);

	for (i=0; !ret && i<cfi->numchips; i++) {
		chip = &cfi->chips[i];

	retry:
		cfi_spin_lock(chip->mutex);

		switch(chip->state) {
		case FL_READY:
		case FL_STATUS:
		case FL_CFI_QUERY:
		case FL_JEDEC_QUERY:
			chip->oldstate = chip->state;
			chip->state = FL_SYNCING;
			/* No need to wake_up() on this state change - 
			 * as the whole point is that nobody can do anything
			 * with the chip now anyway.
			 */
		case FL_SYNCING:
			cfi_spin_unlock(chip->mutex);
			break;

		default:
			/* Not an idle state */
			add_wait_queue(&chip->wq, &wait);
			
			cfi_spin_unlock(chip->mutex);

			schedule();

		        remove_wait_queue(&chip->wq, &wait);
			
			goto retry;
		}
	}

	/* Unlock the chips again */

	for (i--; i >=0; i--) {
		chip = &cfi->chips[i];

		cfi_spin_lock(chip->mutex);
		
		if (chip->state == FL_SYNCING) {
			chip->state = chip->oldstate;
			wake_up(&chip->wq);
		}
		cfi_spin_unlock(chip->mutex);
	}
}


static int cfi_amdstd_suspend(struct mtd_info *mtd)
{
	struct map_info *map = mtd->priv;
	struct cfi_private *cfi = map->fldrv_priv;
	int i;
	struct flchip *chip;
	int ret = 0;
//printk("suspend\n");

	for (i=0; !ret && i<cfi->numchips; i++) {
		chip = &cfi->chips[i];

		cfi_spin_lock(chip->mutex);

		switch(chip->state) {
		case FL_READY:
		case FL_STATUS:
		case FL_CFI_QUERY:
		case FL_JEDEC_QUERY:
			chip->oldstate = chip->state;
			chip->state = FL_PM_SUSPENDED;
			/* No need to wake_up() on this state change - 
			 * as the whole point is that nobody can do anything
			 * with the chip now anyway.
			 */
		case FL_PM_SUSPENDED:
			break;

		default:
			ret = -EAGAIN;
			break;
		}
		cfi_spin_unlock(chip->mutex);
	}

	/* Unlock the chips again */

	if (ret) {
    		for (i--; i >=0; i--) {
			chip = &cfi->chips[i];

			cfi_spin_lock(chip->mutex);
		
			if (chip->state == FL_PM_SUSPENDED) {
				chip->state = chip->oldstate;
				wake_up(&chip->wq);
			}
			cfi_spin_unlock(chip->mutex);
		}
	}
	
	return ret;
}

static void cfi_amdstd_resume(struct mtd_info *mtd)
{
	struct map_info *map = mtd->priv;
	struct cfi_private *cfi = map->fldrv_priv;
	int i;
	struct flchip *chip;
//printk("resume\n");

	for (i=0; i<cfi->numchips; i++) {
	
		chip = &cfi->chips[i];

		cfi_spin_lock(chip->mutex);
		
		if (chip->state == FL_PM_SUSPENDED) {
			chip->state = FL_READY;
			cfi_write(map, CMD(0xF0), chip->start);
			wake_up(&chip->wq);
		}
		else
			printk(KERN_ERR "Argh. Chip not in PM_SUSPENDED state upon resume()\n");

		cfi_spin_unlock(chip->mutex);
	}
}

static void cfi_amdstd_destroy(struct mtd_info *mtd)
{
	struct map_info *map = mtd->priv;
	struct cfi_private *cfi = map->fldrv_priv;
	kfree(cfi->cmdset_priv);
	kfree(cfi);
}

static char im_name[]="cfi_cmdset_0002";

int __init cfi_amdstd_init(void)
{
	inter_module_register(im_name, THIS_MODULE, &cfi_cmdset_0002);
	return 0;
}

static void __exit cfi_amdstd_exit(void)
{
	inter_module_unregister(im_name);
}

module_init(cfi_amdstd_init);
module_exit(cfi_amdstd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Crossnet Co. <info@crossnet.co.jp> et al.");
MODULE_DESCRIPTION("MTD chip driver for AMD/Fujitsu flash chips");
