/*
 *  linux/arch/mips/kernel/proc.c
 *
 *  Copyright (C) 1995, 1996, 2001  Ralf Baechle
 *  Copyright (C) 2001  MIPS Technologies, Inc.
 */
#include <linux/config.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/mipsregs.h>
#include <asm/processor.h>
#include <asm/watch.h>

unsigned int vced_count, vcei_count;

#ifndef CONFIG_CPU_HAS_LLSC
unsigned long ll_ops, sc_ops;
#endif

static const char *cpu_name[] = {
	[CPU_UNKNOWN]	"unknown",
	[CPU_R2000]	"R2000",
	[CPU_R3000]	"R3000",
	[CPU_R3000A]	"R3000A",
	[CPU_R3041]	"R3041",
	[CPU_R3051]	"R3051",
	[CPU_R3052]	"R3052",
	[CPU_R3081]	"R3081",
	[CPU_R3081E]	"R3081E",
	[CPU_R4000PC]	"R4000PC",
	[CPU_R4000SC]	"R4000SC",
	[CPU_R4000MC]	"R4000MC",
        [CPU_R4200]	"R4200",
	[CPU_R4400PC]	"R4400PC",
	[CPU_R4400SC]	"R4400SC",
	[CPU_R4400MC]	"R4400MC",
	[CPU_R4600]	"R4600",
	[CPU_R6000]	"R6000",
        [CPU_R6000A]	"R6000A",
	[CPU_R8000]	"R8000",
	[CPU_R10000]	"R10000",
	[CPU_R4300]	"R4300",
	[CPU_R4650]	"R4650",
	[CPU_R4700]	"R4700",
	[CPU_R5000]	"R5000",
        [CPU_R5000A]	"R5000A",
	[CPU_R4640]	"R4640",
	[CPU_NEVADA]	"Nevada",
	[CPU_RM7000]	"RM7000",
	[CPU_R5432]	"R5432",
	[CPU_4KC]	"MIPS 4Kc",
        [CPU_5KC]	"MIPS 5Kc",
	[CPU_R4310]	"R4310",
	[CPU_SB1]	"SiByte SB1",
	[CPU_TX3912]	"TX3912",
	[CPU_TX3922]	"TX3922",
	[CPU_TX3927]	"TX3927",
	[CPU_AU1000]	"Au1000",
	[CPU_AU1500]	"Au1500",
	[CPU_4KEC]	"MIPS 4KEc",
	[CPU_4KSC]	"MIPS 4KSc",
	[CPU_VR41XX]	"NEC Vr41xx",
	[CPU_R5500]	"R5500",
	[CPU_TX49XX]	"TX49xx",
	[CPU_TX39XX]	"TX39xx",
	[CPU_20KC]	"MIPS 20Kc",
	[CPU_VR4111]	"NEC VR4111",
	[CPU_VR4121]	"NEC VR4121",
	[CPU_VR4122]	"NEC VR4122",
	[CPU_VR4131]	"NEC VR4131",
	[CPU_VR4181]	"NEC VR4181",
	[CPU_VR4181A]	"NEC VR4181A",
	[CPU_BCM4710]	"BCM4710",
	[CPU_BCM3302]	"BCM3302",
};

extern unsigned long unaligned_instructions;

static int show_cpuinfo(struct seq_file *m, void *v)
{
	unsigned int version = mips_cpu.processor_id;
	unsigned int fp_vers = mips_cpu.fpu_id;
	unsigned long n = (unsigned long) v - 1;
	char fmt [64];

#ifdef CONFIG_SMP
	if (!CPUMASK_TSTB(cpu_online_map, n))
		return 0;
#endif

	/*
	 * For the first processor also print the system type
	 */
	if (n == 0)
		seq_printf(m, "system type\t\t: %s\n", get_system_type());

	seq_printf(m, "processor\t\t: %ld\n", n);
	sprintf(fmt, "cpu model\t\t: %%s V%%d.%%d%s\n",
	        (mips_cpu.options & MIPS_CPU_FPU) ? "  FPU V%d.%d" : "");
	seq_printf(m, fmt, cpu_name[mips_cpu.cputype <= CPU_LAST ?
	                            mips_cpu.cputype : CPU_UNKNOWN],
	                           (version >> 4) & 0x0f, version & 0x0f,
	                           (fp_vers >> 4) & 0x0f, fp_vers & 0x0f);
	seq_printf(m, "BogoMIPS\t\t: %lu.%02lu\n",
	              loops_per_jiffy / (500000/HZ),
	              (loops_per_jiffy / (5000/HZ)) % 100);
	seq_printf(m, "wait instruction\t: %s\n", cpu_wait ? "yes" : "no");
	seq_printf(m, "microsecond timers\t: %s\n",
	              (mips_cpu.options & MIPS_CPU_COUNTER) ? "yes" : "no");
	seq_printf(m, "tlb_entries\t\t: %d\n", mips_cpu.tlbsize);
	seq_printf(m, "extra interrupt vector\t: %s\n",
	              (mips_cpu.options & MIPS_CPU_DIVEC) ? "yes" : "no");
	seq_printf(m, "hardware watchpoint\t: %s\n",
	              watch_available ? "yes" : "no");

	sprintf(fmt, "VCE%%c exceptions\t\t: %s\n",
	        (mips_cpu.options & MIPS_CPU_VCE) ? "%d" : "not available");
	seq_printf(m, fmt, 'D', vced_count);
	seq_printf(m, fmt, 'I', vcei_count);

#ifndef CONFIG_CPU_HAS_LLSC
	seq_printf(m, "ll emulations\t\t: %lu\n", ll_ops);
	seq_printf(m, "sc emulations\t\t: %lu\n", sc_ops);
#endif

	seq_printf(m, "unaligned_instructions\t: %u\n", unaligned_instructions);

#if defined(CONFIG_BCM4710) || defined(CONFIG_BCM4310) || defined(CONFIG_BCM4704)
	seq_printf(m, "dcache hits\t\t: %u\n",
		   read_perf_cntr(0));
	seq_printf(m, "dcache misses\t\t: %u\n",
		   read_perf_cntr(1));
	seq_printf(m, "icache hits\t\t: %u\n",
		   read_perf_cntr(2));
	seq_printf(m, "icache misses\t\t: %u\n",
		   read_perf_cntr(3));
	seq_printf(m, "instructions\t\t: %u\n",
		   read_perf_cntr(4));
#endif

	return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	unsigned long i = *pos;

	return i < NR_CPUS ? (void *) (i + 1) : NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return c_start(m, pos);
}

static void c_stop(struct seq_file *m, void *v)
{
}

struct seq_operations cpuinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= show_cpuinfo,
};
