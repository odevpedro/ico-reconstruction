	.text
	.p2align 3
	.globl	boyAI_sub_14FFD0
	.ent	boyAI_sub_14FFD0
boyAI_sub_14FFD0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	andi	$2,$2,8
	beql	$2,$0,loc_0014fffc
	lw	$2,0x48($18)
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x6d
	lui	$1,0x4120
	mtc1	$1,$f12
	jal	0x00181f38
	nop	
	lw	$2,0x48($18)
loc_0014fffc:
	.word	0x14400063
	nop	
	ld	$2,0x18($18)
	dsrl32	$2,$2,5
	andi	$2,$2,1
	.word	0x1040005e
	nop	
	lui	$1,0x447a
	mtc1	$1,$f12
	jal	0x00181f38
	nop	
	.word	0x10000058
	nop	
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d18($28)
	.word	0x46010034
	bc1f	loc_0015006c
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,0x2d
	sltiu	$2,$2,0x5b
	beqz	$2,loc_0015006c
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x136
	.word	0x10000049
	nop	
loc_0015006c:
	.word	0x4500000b
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,0x86
	sltiu	$2,$2,0x10d
	.word	0x14400006
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x137
	.word	0x1000003d
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_14FFD0
