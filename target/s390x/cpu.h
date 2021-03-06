/*
 * S/390 virtual CPU header
 *
 *  Copyright (c) 2009 Ulrich Hecht
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Contributions after 2012-10-29 are licensed under the terms of the
 * GNU GPL, version 2 or (at your option) any later version.
 *
 * You should have received a copy of the GNU (Lesser) General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef S390X_CPU_H
#define S390X_CPU_H

#include "qemu-common.h"
#include "cpu-qom.h"

#define TARGET_LONG_BITS 64

#define ELF_MACHINE_UNAME "S390X"

#define CPUArchState struct CPUS390XState

#include "exec/cpu-defs.h"
#define TARGET_PAGE_BITS 12

#define TARGET_PHYS_ADDR_SPACE_BITS 64
#define TARGET_VIRT_ADDR_SPACE_BITS 64

#include "exec/cpu-all.h"

#include "fpu/softfloat.h"

#define NB_MMU_MODES 3
#define TARGET_INSN_START_EXTRA_WORDS 1

#define MMU_MODE0_SUFFIX _primary
#define MMU_MODE1_SUFFIX _secondary
#define MMU_MODE2_SUFFIX _home

#define MMU_USER_IDX 0

#define MAX_EXT_QUEUE 16
#define MAX_IO_QUEUE 16
#define MAX_MCHK_QUEUE 16

#define PSW_MCHK_MASK 0x0004000000000000
#define PSW_IO_MASK 0x0200000000000000

typedef struct PSW {
    uint64_t mask;
    uint64_t addr;
} PSW;

typedef struct ExtQueue {
    uint32_t code;
    uint32_t param;
    uint32_t param64;
} ExtQueue;

typedef struct IOIntQueue {
    uint16_t id;
    uint16_t nr;
    uint32_t parm;
    uint32_t word;
} IOIntQueue;

typedef struct MchkQueue {
    uint16_t type;
} MchkQueue;

typedef struct CPUS390XState {
    uint64_t regs[16];     /* GP registers */
    /*
     * The floating point registers are part of the vector registers.
     * vregs[0][0] -> vregs[15][0] are 16 floating point registers
     */
    CPU_DoubleU vregs[32][2];  /* vector registers */
    uint32_t aregs[16];    /* access registers */
    uint8_t riccb[64];     /* runtime instrumentation control */
    uint64_t gscb[4];      /* guarded storage control */

    /* Fields up to this point are not cleared by initial CPU reset */
    struct {} start_initial_reset_fields;

    uint32_t fpc;          /* floating-point control register */
    uint32_t cc_op;

    float_status fpu_status; /* passed to softfloat lib */

    /* The low part of a 128-bit return, or remainder of a divide.  */
    uint64_t retxl;

    PSW psw;

    uint64_t cc_src;
    uint64_t cc_dst;
    uint64_t cc_vr;

    uint64_t ex_value;

    uint64_t __excp_addr;
    uint64_t psa;

    uint32_t int_pgm_code;
    uint32_t int_pgm_ilen;

    uint32_t int_svc_code;
    uint32_t int_svc_ilen;

    uint64_t per_address;
    uint16_t per_perc_atmid;

    uint64_t cregs[16]; /* control registers */

    ExtQueue ext_queue[MAX_EXT_QUEUE];
    IOIntQueue io_queue[MAX_IO_QUEUE][8];
    MchkQueue mchk_queue[MAX_MCHK_QUEUE];

    int pending_int;
    int ext_index;
    int io_index[8];
    int mchk_index;

    uint64_t ckc;
    uint64_t cputm;
    uint32_t todpr;

    uint64_t pfault_token;
    uint64_t pfault_compare;
    uint64_t pfault_select;

    uint64_t gbea;
    uint64_t pp;

    /* Fields up to this point are cleared by a CPU reset */
    struct {} end_reset_fields;

    CPU_COMMON

    uint32_t cpu_num;
    uint64_t cpuid;

    uint64_t tod_offset;
    uint64_t tod_basetime;
    QEMUTimer *tod_timer;

    QEMUTimer *cpu_timer;

    /*
     * The cpu state represents the logical state of a cpu. In contrast to other
     * architectures, there is a difference between a halt and a stop on s390.
     * If all cpus are either stopped (including check stop) or in the disabled
     * wait state, the vm can be shut down.
     */
#define CPU_STATE_UNINITIALIZED        0x00
#define CPU_STATE_STOPPED              0x01
#define CPU_STATE_CHECK_STOP           0x02
#define CPU_STATE_OPERATING            0x03
#define CPU_STATE_LOAD                 0x04
    uint8_t cpu_state;

    /* currently processed sigp order */
    uint8_t sigp_order;

} CPUS390XState;

static inline CPU_DoubleU *get_freg(CPUS390XState *cs, int nr)
{
    return &cs->vregs[nr][0];
}

/**
 * S390CPU:
 * @env: #CPUS390XState.
 *
 * An S/390 CPU.
 */
struct S390CPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPUS390XState env;
    int64_t id;
    S390CPUModel *model;
    /* needed for live migration */
    void *irqstate;
    uint32_t irqstate_saved_size;
};

static inline S390CPU *s390_env_get_cpu(CPUS390XState *env)
{
    return container_of(env, S390CPU, env);
}

#define ENV_GET_CPU(e) CPU(s390_env_get_cpu(e))

#define ENV_OFFSET offsetof(S390CPU, env)

#ifndef CONFIG_USER_ONLY
extern const struct VMStateDescription vmstate_s390_cpu;
#endif

/* distinguish between 24 bit and 31 bit addressing */
#define HIGH_ORDER_BIT 0x80000000

/* Interrupt Codes */
/* Program Interrupts */
#define PGM_OPERATION                   0x0001
#define PGM_PRIVILEGED                  0x0002
#define PGM_EXECUTE                     0x0003
#define PGM_PROTECTION                  0x0004
#define PGM_ADDRESSING                  0x0005
#define PGM_SPECIFICATION               0x0006
#define PGM_DATA                        0x0007
#define PGM_FIXPT_OVERFLOW              0x0008
#define PGM_FIXPT_DIVIDE                0x0009
#define PGM_DEC_OVERFLOW                0x000a
#define PGM_DEC_DIVIDE                  0x000b
#define PGM_HFP_EXP_OVERFLOW            0x000c
#define PGM_HFP_EXP_UNDERFLOW           0x000d
#define PGM_HFP_SIGNIFICANCE            0x000e
#define PGM_HFP_DIVIDE                  0x000f
#define PGM_SEGMENT_TRANS               0x0010
#define PGM_PAGE_TRANS                  0x0011
#define PGM_TRANS_SPEC                  0x0012
#define PGM_SPECIAL_OP                  0x0013
#define PGM_OPERAND                     0x0015
#define PGM_TRACE_TABLE                 0x0016
#define PGM_SPACE_SWITCH                0x001c
#define PGM_HFP_SQRT                    0x001d
#define PGM_PC_TRANS_SPEC               0x001f
#define PGM_AFX_TRANS                   0x0020
#define PGM_ASX_TRANS                   0x0021
#define PGM_LX_TRANS                    0x0022
#define PGM_EX_TRANS                    0x0023
#define PGM_PRIM_AUTH                   0x0024
#define PGM_SEC_AUTH                    0x0025
#define PGM_ALET_SPEC                   0x0028
#define PGM_ALEN_SPEC                   0x0029
#define PGM_ALE_SEQ                     0x002a
#define PGM_ASTE_VALID                  0x002b
#define PGM_ASTE_SEQ                    0x002c
#define PGM_EXT_AUTH                    0x002d
#define PGM_STACK_FULL                  0x0030
#define PGM_STACK_EMPTY                 0x0031
#define PGM_STACK_SPEC                  0x0032
#define PGM_STACK_TYPE                  0x0033
#define PGM_STACK_OP                    0x0034
#define PGM_ASCE_TYPE                   0x0038
#define PGM_REG_FIRST_TRANS             0x0039
#define PGM_REG_SEC_TRANS               0x003a
#define PGM_REG_THIRD_TRANS             0x003b
#define PGM_MONITOR                     0x0040
#define PGM_PER                         0x0080
#define PGM_CRYPTO                      0x0119

/* External Interrupts */
#define EXT_INTERRUPT_KEY               0x0040
#define EXT_CLOCK_COMP                  0x1004
#define EXT_CPU_TIMER                   0x1005
#define EXT_MALFUNCTION                 0x1200
#define EXT_EMERGENCY                   0x1201
#define EXT_EXTERNAL_CALL               0x1202
#define EXT_ETR                         0x1406
#define EXT_SERVICE                     0x2401
#define EXT_VIRTIO                      0x2603

/* PSW defines */
#undef PSW_MASK_PER
#undef PSW_MASK_DAT
#undef PSW_MASK_IO
#undef PSW_MASK_EXT
#undef PSW_MASK_KEY
#undef PSW_SHIFT_KEY
#undef PSW_MASK_MCHECK
#undef PSW_MASK_WAIT
#undef PSW_MASK_PSTATE
#undef PSW_MASK_ASC
#undef PSW_SHIFT_ASC
#undef PSW_MASK_CC
#undef PSW_MASK_PM
#undef PSW_MASK_64
#undef PSW_MASK_32
#undef PSW_MASK_ESA_ADDR

#define PSW_MASK_PER            0x4000000000000000ULL
#define PSW_MASK_DAT            0x0400000000000000ULL
#define PSW_MASK_IO             0x0200000000000000ULL
#define PSW_MASK_EXT            0x0100000000000000ULL
#define PSW_MASK_KEY            0x00F0000000000000ULL
#define PSW_SHIFT_KEY           52
#define PSW_MASK_MCHECK         0x0004000000000000ULL
#define PSW_MASK_WAIT           0x0002000000000000ULL
#define PSW_MASK_PSTATE         0x0001000000000000ULL
#define PSW_MASK_ASC            0x0000C00000000000ULL
#define PSW_SHIFT_ASC           46
#define PSW_MASK_CC             0x0000300000000000ULL
#define PSW_MASK_PM             0x00000F0000000000ULL
#define PSW_MASK_64             0x0000000100000000ULL
#define PSW_MASK_32             0x0000000080000000ULL
#define PSW_MASK_ESA_ADDR       0x000000007fffffffULL

#undef PSW_ASC_PRIMARY
#undef PSW_ASC_ACCREG
#undef PSW_ASC_SECONDARY
#undef PSW_ASC_HOME

#define PSW_ASC_PRIMARY         0x0000000000000000ULL
#define PSW_ASC_ACCREG          0x0000400000000000ULL
#define PSW_ASC_SECONDARY       0x0000800000000000ULL
#define PSW_ASC_HOME            0x0000C00000000000ULL

/* the address space values shifted */
#define AS_PRIMARY              0
#define AS_ACCREG               1
#define AS_SECONDARY            2
#define AS_HOME                 3

/* tb flags */

#define FLAG_MASK_PSW_SHIFT     31
#define FLAG_MASK_PER           (PSW_MASK_PER    >> FLAG_MASK_PSW_SHIFT)
#define FLAG_MASK_PSTATE        (PSW_MASK_PSTATE >> FLAG_MASK_PSW_SHIFT)
#define FLAG_MASK_ASC           (PSW_MASK_ASC    >> FLAG_MASK_PSW_SHIFT)
#define FLAG_MASK_64            (PSW_MASK_64     >> FLAG_MASK_PSW_SHIFT)
#define FLAG_MASK_32            (PSW_MASK_32     >> FLAG_MASK_PSW_SHIFT)
#define FLAG_MASK_PSW		(FLAG_MASK_PER | FLAG_MASK_PSTATE \
                                | FLAG_MASK_ASC | FLAG_MASK_64 | FLAG_MASK_32)

/* Control register 0 bits */
#define CR0_LOWPROT             0x0000000010000000ULL
#define CR0_SECONDARY           0x0000000004000000ULL
#define CR0_EDAT                0x0000000000800000ULL

/* MMU */
#define MMU_PRIMARY_IDX         0
#define MMU_SECONDARY_IDX       1
#define MMU_HOME_IDX            2

static inline int cpu_mmu_index(CPUS390XState *env, bool ifetch)
{
    switch (env->psw.mask & PSW_MASK_ASC) {
    case PSW_ASC_PRIMARY:
        return MMU_PRIMARY_IDX;
    case PSW_ASC_SECONDARY:
        return MMU_SECONDARY_IDX;
    case PSW_ASC_HOME:
        return MMU_HOME_IDX;
    case PSW_ASC_ACCREG:
        /* Fallthrough: access register mode is not yet supported */
    default:
        abort();
    }
}

static inline void cpu_get_tb_cpu_state(CPUS390XState* env, target_ulong *pc,
                                        target_ulong *cs_base, uint32_t *flags)
{
    *pc = env->psw.addr;
    *cs_base = env->ex_value;
    *flags = (env->psw.mask >> FLAG_MASK_PSW_SHIFT) & FLAG_MASK_PSW;
}

/* PER bits from control register 9 */
#define PER_CR9_EVENT_BRANCH           0x80000000
#define PER_CR9_EVENT_IFETCH           0x40000000
#define PER_CR9_EVENT_STORE            0x20000000
#define PER_CR9_EVENT_STORE_REAL       0x08000000
#define PER_CR9_EVENT_NULLIFICATION    0x01000000
#define PER_CR9_CONTROL_BRANCH_ADDRESS 0x00800000
#define PER_CR9_CONTROL_ALTERATION     0x00200000

/* PER bits from the PER CODE/ATMID/AI in lowcore */
#define PER_CODE_EVENT_BRANCH          0x8000
#define PER_CODE_EVENT_IFETCH          0x4000
#define PER_CODE_EVENT_STORE           0x2000
#define PER_CODE_EVENT_STORE_REAL      0x0800
#define PER_CODE_EVENT_NULLIFICATION   0x0100

#define EXCP_EXT 1 /* external interrupt */
#define EXCP_SVC 2 /* supervisor call (syscall) */
#define EXCP_PGM 3 /* program interruption */
#define EXCP_IO  7 /* I/O interrupt */
#define EXCP_MCHK 8 /* machine check */

#define INTERRUPT_EXT        (1 << 0)
#define INTERRUPT_TOD        (1 << 1)
#define INTERRUPT_CPUTIMER   (1 << 2)
#define INTERRUPT_IO         (1 << 3)
#define INTERRUPT_MCHK       (1 << 4)

/* Program Status Word.  */
#define S390_PSWM_REGNUM 0
#define S390_PSWA_REGNUM 1
/* General Purpose Registers.  */
#define S390_R0_REGNUM 2
#define S390_R1_REGNUM 3
#define S390_R2_REGNUM 4
#define S390_R3_REGNUM 5
#define S390_R4_REGNUM 6
#define S390_R5_REGNUM 7
#define S390_R6_REGNUM 8
#define S390_R7_REGNUM 9
#define S390_R8_REGNUM 10
#define S390_R9_REGNUM 11
#define S390_R10_REGNUM 12
#define S390_R11_REGNUM 13
#define S390_R12_REGNUM 14
#define S390_R13_REGNUM 15
#define S390_R14_REGNUM 16
#define S390_R15_REGNUM 17
/* Total Core Registers. */
#define S390_NUM_CORE_REGS 18

static inline void setcc(S390CPU *cpu, uint64_t cc)
{
    CPUS390XState *env = &cpu->env;

    env->psw.mask &= ~(3ull << 44);
    env->psw.mask |= (cc & 3) << 44;
    env->cc_op = cc;
}

/* STSI */
#define STSI_LEVEL_MASK         0x00000000f0000000ULL
#define STSI_LEVEL_CURRENT      0x0000000000000000ULL
#define STSI_LEVEL_1            0x0000000010000000ULL
#define STSI_LEVEL_2            0x0000000020000000ULL
#define STSI_LEVEL_3            0x0000000030000000ULL
#define STSI_R0_RESERVED_MASK   0x000000000fffff00ULL
#define STSI_R0_SEL1_MASK       0x00000000000000ffULL
#define STSI_R1_RESERVED_MASK   0x00000000ffff0000ULL
#define STSI_R1_SEL2_MASK       0x000000000000ffffULL

/* Basic Machine Configuration */
struct sysib_111 {
    uint32_t res1[8];
    uint8_t  manuf[16];
    uint8_t  type[4];
    uint8_t  res2[12];
    uint8_t  model[16];
    uint8_t  sequence[16];
    uint8_t  plant[4];
    uint8_t  res3[156];
};

/* Basic Machine CPU */
struct sysib_121 {
    uint32_t res1[80];
    uint8_t  sequence[16];
    uint8_t  plant[4];
    uint8_t  res2[2];
    uint16_t cpu_addr;
    uint8_t  res3[152];
};

/* Basic Machine CPUs */
struct sysib_122 {
    uint8_t res1[32];
    uint32_t capability;
    uint16_t total_cpus;
    uint16_t active_cpus;
    uint16_t standby_cpus;
    uint16_t reserved_cpus;
    uint16_t adjustments[2026];
};

/* LPAR CPU */
struct sysib_221 {
    uint32_t res1[80];
    uint8_t  sequence[16];
    uint8_t  plant[4];
    uint16_t cpu_id;
    uint16_t cpu_addr;
    uint8_t  res3[152];
};

/* LPAR CPUs */
struct sysib_222 {
    uint32_t res1[32];
    uint16_t lpar_num;
    uint8_t  res2;
    uint8_t  lcpuc;
    uint16_t total_cpus;
    uint16_t conf_cpus;
    uint16_t standby_cpus;
    uint16_t reserved_cpus;
    uint8_t  name[8];
    uint32_t caf;
    uint8_t  res3[16];
    uint16_t dedicated_cpus;
    uint16_t shared_cpus;
    uint8_t  res4[180];
};

/* VM CPUs */
struct sysib_322 {
    uint8_t  res1[31];
    uint8_t  count;
    struct {
        uint8_t  res2[4];
        uint16_t total_cpus;
        uint16_t conf_cpus;
        uint16_t standby_cpus;
        uint16_t reserved_cpus;
        uint8_t  name[8];
        uint32_t caf;
        uint8_t  cpi[16];
        uint8_t res5[3];
        uint8_t ext_name_encoding;
        uint32_t res3;
        uint8_t uuid[16];
    } vm[8];
    uint8_t res4[1504];
    uint8_t ext_names[8][256];
};

/* MMU defines */
#define _ASCE_ORIGIN            ~0xfffULL /* segment table origin             */
#define _ASCE_SUBSPACE          0x200     /* subspace group control           */
#define _ASCE_PRIVATE_SPACE     0x100     /* private space control            */
#define _ASCE_ALT_EVENT         0x80      /* storage alteration event control */
#define _ASCE_SPACE_SWITCH      0x40      /* space switch event               */
#define _ASCE_REAL_SPACE        0x20      /* real space control               */
#define _ASCE_TYPE_MASK         0x0c      /* asce table type mask             */
#define _ASCE_TYPE_REGION1      0x0c      /* region first table type          */
#define _ASCE_TYPE_REGION2      0x08      /* region second table type         */
#define _ASCE_TYPE_REGION3      0x04      /* region third table type          */
#define _ASCE_TYPE_SEGMENT      0x00      /* segment table type               */
#define _ASCE_TABLE_LENGTH      0x03      /* region table length              */

#define _REGION_ENTRY_ORIGIN    ~0xfffULL /* region/segment table origin      */
#define _REGION_ENTRY_RO        0x200     /* region/segment protection bit    */
#define _REGION_ENTRY_TF        0xc0      /* region/segment table offset      */
#define _REGION_ENTRY_INV       0x20      /* invalid region table entry       */
#define _REGION_ENTRY_TYPE_MASK 0x0c      /* region/segment table type mask   */
#define _REGION_ENTRY_TYPE_R1   0x0c      /* region first table type          */
#define _REGION_ENTRY_TYPE_R2   0x08      /* region second table type         */
#define _REGION_ENTRY_TYPE_R3   0x04      /* region third table type          */
#define _REGION_ENTRY_LENGTH    0x03      /* region third length              */

#define _SEGMENT_ENTRY_ORIGIN   ~0x7ffULL /* segment table origin             */
#define _SEGMENT_ENTRY_FC       0x400     /* format control                   */
#define _SEGMENT_ENTRY_RO       0x200     /* page protection bit              */
#define _SEGMENT_ENTRY_INV      0x20      /* invalid segment table entry      */

#define VADDR_PX                0xff000   /* page index bits                  */

#define _PAGE_RO        0x200            /* HW read-only bit  */
#define _PAGE_INVALID   0x400            /* HW invalid bit    */
#define _PAGE_RES0      0x800            /* bit must be zero  */

#define SK_C                    (0x1 << 1)
#define SK_R                    (0x1 << 2)
#define SK_F                    (0x1 << 3)
#define SK_ACC_MASK             (0xf << 4)

/* SIGP order codes */
#define SIGP_SENSE             0x01
#define SIGP_EXTERNAL_CALL     0x02
#define SIGP_EMERGENCY         0x03
#define SIGP_START             0x04
#define SIGP_STOP              0x05
#define SIGP_RESTART           0x06
#define SIGP_STOP_STORE_STATUS 0x09
#define SIGP_INITIAL_CPU_RESET 0x0b
#define SIGP_CPU_RESET         0x0c
#define SIGP_SET_PREFIX        0x0d
#define SIGP_STORE_STATUS_ADDR 0x0e
#define SIGP_SET_ARCH          0x12
#define SIGP_STORE_ADTL_STATUS 0x17

/* SIGP condition codes */
#define SIGP_CC_ORDER_CODE_ACCEPTED 0
#define SIGP_CC_STATUS_STORED       1
#define SIGP_CC_BUSY                2
#define SIGP_CC_NOT_OPERATIONAL     3

/* SIGP status bits */
#define SIGP_STAT_EQUIPMENT_CHECK   0x80000000UL
#define SIGP_STAT_INCORRECT_STATE   0x00000200UL
#define SIGP_STAT_INVALID_PARAMETER 0x00000100UL
#define SIGP_STAT_EXT_CALL_PENDING  0x00000080UL
#define SIGP_STAT_STOPPED           0x00000040UL
#define SIGP_STAT_OPERATOR_INTERV   0x00000020UL
#define SIGP_STAT_CHECK_STOP        0x00000010UL
#define SIGP_STAT_INOPERATIVE       0x00000004UL
#define SIGP_STAT_INVALID_ORDER     0x00000002UL
#define SIGP_STAT_RECEIVER_CHECK    0x00000001UL

/* SIGP SET ARCHITECTURE modes */
#define SIGP_MODE_ESA_S390 0
#define SIGP_MODE_Z_ARCH_TRANS_ALL_PSW 1
#define SIGP_MODE_Z_ARCH_TRANS_CUR_PSW 2

/* SIGP order code mask corresponding to bit positions 56-63 */
#define SIGP_ORDER_MASK 0x000000ff

/* from s390-virtio-ccw */
#define MEM_SECTION_SIZE             0x10000000UL
#define MAX_AVAIL_SLOTS              32

/* machine check interruption code */

/* subclasses */
#define MCIC_SC_SD 0x8000000000000000ULL
#define MCIC_SC_PD 0x4000000000000000ULL
#define MCIC_SC_SR 0x2000000000000000ULL
#define MCIC_SC_CD 0x0800000000000000ULL
#define MCIC_SC_ED 0x0400000000000000ULL
#define MCIC_SC_DG 0x0100000000000000ULL
#define MCIC_SC_W  0x0080000000000000ULL
#define MCIC_SC_CP 0x0040000000000000ULL
#define MCIC_SC_SP 0x0020000000000000ULL
#define MCIC_SC_CK 0x0010000000000000ULL

/* subclass modifiers */
#define MCIC_SCM_B  0x0002000000000000ULL
#define MCIC_SCM_DA 0x0000000020000000ULL
#define MCIC_SCM_AP 0x0000000000080000ULL

/* storage errors */
#define MCIC_SE_SE 0x0000800000000000ULL
#define MCIC_SE_SC 0x0000400000000000ULL
#define MCIC_SE_KE 0x0000200000000000ULL
#define MCIC_SE_DS 0x0000100000000000ULL
#define MCIC_SE_IE 0x0000000080000000ULL

/* validity bits */
#define MCIC_VB_WP 0x0000080000000000ULL
#define MCIC_VB_MS 0x0000040000000000ULL
#define MCIC_VB_PM 0x0000020000000000ULL
#define MCIC_VB_IA 0x0000010000000000ULL
#define MCIC_VB_FA 0x0000008000000000ULL
#define MCIC_VB_VR 0x0000004000000000ULL
#define MCIC_VB_EC 0x0000002000000000ULL
#define MCIC_VB_FP 0x0000001000000000ULL
#define MCIC_VB_GR 0x0000000800000000ULL
#define MCIC_VB_CR 0x0000000400000000ULL
#define MCIC_VB_ST 0x0000000100000000ULL
#define MCIC_VB_AR 0x0000000040000000ULL
#define MCIC_VB_GS 0x0000000008000000ULL
#define MCIC_VB_PR 0x0000000000200000ULL
#define MCIC_VB_FC 0x0000000000100000ULL
#define MCIC_VB_CT 0x0000000000020000ULL
#define MCIC_VB_CC 0x0000000000010000ULL


/* cpu.c */
int s390_get_clock(uint8_t *tod_high, uint64_t *tod_low);
int s390_set_clock(uint8_t *tod_high, uint64_t *tod_low);
void s390_crypto_reset(void);
bool s390_get_squash_mcss(void);
int s390_get_memslot_count(void);
int s390_set_memory_limit(uint64_t new_limit, uint64_t *hw_limit);
void s390_cmma_reset(void);
int s390_cpu_restart(S390CPU *cpu);
void s390_enable_css_support(S390CPU *cpu);
int s390_assign_subch_ioeventfd(EventNotifier *notifier, uint32_t sch_id,
                                int vq, bool assign);
#ifndef CONFIG_USER_ONLY
unsigned int s390_cpu_set_state(uint8_t cpu_state, S390CPU *cpu);
#else
static inline unsigned int s390_cpu_set_state(uint8_t cpu_state, S390CPU *cpu)
{
    return 0;
}
#endif /* CONFIG_USER_ONLY */


/* cpu_models.c */
void s390_cpu_list(FILE *f, fprintf_function cpu_fprintf);
#define cpu_list s390_cpu_list
const char *s390_default_cpu_model_name(void);


/* helper.c */
S390CPU *cpu_s390x_init(const char *cpu_model);
#define cpu_init(model) CPU(cpu_s390x_init(model))
S390CPU *s390x_new_cpu(const char *cpu_model, int64_t id, Error **errp);
/* you can call this signal handler from your SIGBUS and SIGSEGV
   signal handlers to inform the virtual CPU of exceptions. non zero
   is returned if the signal was handled by the virtual CPU.  */
int cpu_s390x_signal_handler(int host_signum, void *pinfo, void *puc);
#define cpu_signal_handler cpu_s390x_signal_handler


/* interrupt.c */
void s390_crw_mchk(void);
void s390_io_interrupt(uint16_t subchannel_id, uint16_t subchannel_nr,
                       uint32_t io_int_parm, uint32_t io_int_word);
/* automatically detect the instruction length */
#define ILEN_AUTO                   0xff
void program_interrupt(CPUS390XState *env, uint32_t code, int ilen);
/* service interrupts are floating therefore we must not pass an cpustate */
void s390_sclp_extint(uint32_t parm);


/* mmu_helper.c */
int s390_cpu_virt_mem_rw(S390CPU *cpu, vaddr laddr, uint8_t ar, void *hostbuf,
                         int len, bool is_write);
#define s390_cpu_virt_mem_read(cpu, laddr, ar, dest, len)    \
        s390_cpu_virt_mem_rw(cpu, laddr, ar, dest, len, false)
#define s390_cpu_virt_mem_write(cpu, laddr, ar, dest, len)       \
        s390_cpu_virt_mem_rw(cpu, laddr, ar, dest, len, true)
#define s390_cpu_virt_mem_check_write(cpu, laddr, ar, len)   \
        s390_cpu_virt_mem_rw(cpu, laddr, ar, NULL, len, true)


/* outside of target/s390x/ */
S390CPU *s390_cpu_addr2state(uint16_t cpu_addr);
extern void subsystem_reset(void);
int sclp_service_call(CPUS390XState *env, uint64_t sccb, uint32_t code);
int s390_virtio_hypercall(CPUS390XState *env);

#endif
