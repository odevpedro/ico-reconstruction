#!/usr/bin/env python3
"""Generate byte-exact .s for verified named functions.

Strategy: emit integer instructions as real mnemonics (numeric regs),
and EVERY COP1 / COP2 / branch / jmp / jal as a raw .word, because the
ee-as 2.9 remaps float-register pairs and lacks COP1 compares / R5900
extras. Guarantees byte-exact assembly with zero toolchain surprises.
"""
import struct, sys
from capstone import Cs, CS_ARCH_MIPS, CS_MODE_MIPS32, CS_MODE_LITTLE_ENDIAN, CS_OP_REG, CS_OP_IMM, CS_OP_MEM

ELF = '.local/extracted/SCUS_971.13.elf'
BASE_VA = 0x00100000
BASE_OFF = 0x1000

data = open(ELF, 'rb').read()
md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS32 + CS_MODE_LITTLE_ENDIAN)
md.detail = True

def word(va):
    off = BASE_OFF + va - BASE_VA
    return struct.unpack('<I', data[off:off+4])[0]

def reg_names():
    return {
        0:'$0',1:'$1',2:'$2',3:'$3',4:'$4',5:'$5',6:'$6',7:'$7',8:'$8',
        9:'$9',10:'$10',11:'$11',12:'$12',13:'$13',14:'$14',15:'$15',
        16:'$16',17:'$17',18:'$18',19:'$19',20:'$20',21:'$21',22:'$22',
        23:'$23',24:'$24',25:'$25',26:'$26',27:'$27',28:'$28',29:'$29',
        30:'$30',31:'$31',
    }

def inst_str(va):
    w = word(va)
    ins = list(md.disasm(struct.pack('<I', w), va))
    if not ins:
        return None
    i = ins[0]
    op = i.mnemonic
    # classify: keep only pure integer ops as mnemonics
    cop1_ops = ('c.', 'mtc1','mfc1','ctc1','cfc1','lwc1','swc1','ldc1','sdc1',
                'add.s','sub.s','mul.s','div.s','sqrt.s','abs.s','mov.s','neg.s',
                'movf','movt','movn','movz','cvt.s.w','cvt.w.s','cvt.d.s','cvt.s.d',
                'trunc.w.s','floor.w.s','ceil.w.s','round.w.s','fadd','fsub','fmul',
                'fdiv','fsqrt','fabs','fmov','fneg','fcvt','ftrunc','ffloor',
                'fceil','fround','fmin','fmax')
    branch_ops = ('b','beq','bne','beqz','bnez','blez','bgtz','bltz','bgez','bgtzal',
                  'bltzal','bc1f','bc1t','bc1fl','bc1tl','beql','bnel','beqzl','bnezl',
                  'blezl','bgtzl','bltzl','bgezl','j','jal','jr','jalr','jals',
                  'bbit032','bbit132')
    acc_ops = ('mult','madd','maddu','msub','msubu','div')
    if op.startswith(cop1_ops):
        return None
    if op in branch_ops or op in acc_ops:
        return None
    reg = {
        'zero':0,'at':1,'v0':2,'v1':3,'a0':4,'a1':5,'a2':6,'a3':7,
        't0':8,'t1':9,'t2':10,'t3':11,'t4':12,'t5':13,'t6':14,'t7':15,
        's0':16,'s1':17,'s2':18,'s3':19,'s4':20,'s5':21,'s6':22,'s7':23,
        't8':24,'t9':25,'k0':26,'k1':27,'gp':28,'sp':29,'fp':30,'ra':31,
    }
    def rname(id):
        name = i.reg_name(id)
        return f'${reg.get(name, "?")}'
    parts = []
    for o in i.operands:
        if o.type == CS_OP_REG:
            parts.append(rname(o.reg))
        elif o.type == CS_OP_IMM:
            v = o.imm
            parts.append(hex(v) if v >= 0 else f'-0x{-v:x}')
        elif o.type == CS_OP_MEM:
            base = rname(o.mem.base) if o.mem.base else ''
            if o.mem.disp:
                disp = o.mem.disp
                d = hex(disp) if disp >= 0 else f'-0x{-disp:x}'
                parts.append(f'{d}({base})')
            else:
                parts.append(f'({base})')
    return f"{op:<9s} {', '.join(parts)}".rstrip()

def op_str_num(op_str):
    return op_str

def gen(name, va, size, module):
    r = reg_names()
    lines = []
    lines.append('\t.text')
    lines.append('\t.p2align 3')
    lines.append(f'\t.globl\t{name}')
    lines.append(f'\t.ent\t{name}')
    lines.append(f'{name}:')
    # frame size from first addiu $sp,$sp,-N ; otherwise 0
    off = BASE_OFF + va - BASE_VA
    w0 = struct.unpack('<I', data[off:off+4])[0]
    frame = 0
    if (w0 >> 26) == 0x09 and ((w0 >> 21) & 0x1f) == 29 and ((w0 >> 16) & 0x1f) == 29:
        imm = w0 & 0xffff
        imm = imm - 0x10000 if imm & 0x8000 else imm
        frame = -imm
    lines.append(f'\t.frame\t$sp,{frame},$31')
    lines.append('\t.mask\t0x00000000,0')
    lines.append('\t.fmask\t0x00000000,0')
    lines.append('\t.set\tnoreorder')
    lines.append('\t.set\tnomacro')
    lines.append('\t.set\tnoat')
    for k in range(0, size, 4):
        a = va + k
        s = inst_str(a)
        if s is None:
            w = word(a)
            s = f'.word\t0x{w:08X}'
        lines.append('\t' + s)
    lines.append('\t.set\tmacro')
    lines.append('\t.set\treorder')
    lines.append('\t.set\tat')
    lines.append(f'\t.end\t{name}')
    return '\n'.join(lines) + '\n'

if __name__ == '__main__':
    targets = [
        # Rev.164 decompilation targets (entity)
        ('subEnemyCollision',       0x15E2C8, 0xB8,  'entity'),
        ('HoldRope',                0x1E59A0, 0x154, 'entity'),
        ('SetGirlClothDispSwitch',  0x1C3C38, 0x1D0, 'entity'),
        ('GirlForceFieldGeo',       0x1C3C90, 0x178, 'entity'),
        # Rev.166 subEnemyCollision delegables (core)
        ('fn_14A100',               0x14A100, 0x74,  'core'),
        ('fn_15BCC8',               0x15BCC8, 0x7C,  'core'),
        ('fn_203AA0',               0x203AA0, 0xA0,  'core'),
    ]
    for name, va, size, module in targets:
        txt = gen(name, va, size, module)
        path = f'src/{module}/asm/{name}.s'
        open(path, 'w').write(txt)
        print(f"wrote {path} ({size} B)")