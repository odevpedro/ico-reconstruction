	.text
	.p2align 3
	.globl	boyAI_sub_14BF8C
	.ent	boyAI_sub_14BF8C
boyAI_sub_14BF8C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	or	$4,$4,$6
	or	$3,$3,$7
	sd	$4,0x488($16)
	sd	$3,0x478($16)
	.word	0x16420053
	sd	$5,0x480($16)
	lwc1	$f1,0x33c($16)
	lwc1	$f0,-0x7dec($28)
	.word	0x46010034
	nop	
	.word	0x45020051
	ld	$2,0x480($16)
	lw	$2,0x470($16)
	andi	$2,$2,1
	.word	0x5040004d
	ld	$2,0x480($16)
	lwc1	$f0,0x4a0($16)
	mtc1	$0,$f1
	lui	$1,0x42a0
	mtc1	$1,$f20
	.word	0x46010032
	nop	
	.word	0x45000012
	addiu	$5,$16,0x4a0
	lwc1	$f0,4($5)
	.word	0x46010032
	.word	0x4500000e
	nop	
	lwc1	$f0,8($5)
	.word	0x46010032
	.word	0x4500000a
	nop	
	lwc1	$f0,0xc($5)
	.word	0x46010032
	nop	
	.word	0x45000005
	lui	$4,0x56
	jal	0x001a6e28
	addiu	$4,$4,-0x7e28
	.set	macro
	.set	reorder
	.end	boyAI_sub_14BF8C
