	.text
	.p2align 3
	.globl	boyAI_sub_157108
	.ent	boyAI_sub_157108
boyAI_sub_157108:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x46140034
	nop	
	bc1f	loc_00157128
	addiu	$5,$0,0x90
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
	lwc1	$f1,0x94($29)
loc_00157128:
	lwc1	$f0,0x84($29)
	sub.s	$f0,$f0,$f1
	.word	0x46140034
	nop	
	bc1f	loc_0015714c
	addiu	$5,$0,0x91
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
loc_0015714c:
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000ffc6
	nop	
	nop	
	lui	$2,0x27
	addiu	$7,$0,0xa
	lw	$3,0x4ec0($2)
	addiu	$29,$29,-0xe0
	sd	$22,0xb0($29)
	addiu	$2,$2,0x4ec0
	mult	$3,$3,$7
	sd	$21,0xa0($29)
	sd	$20,0x90($29)
	addiu	$6,$0,0x3c
	sd	$19,0x80($29)
	sd	$18,0x70($29)
	addiu	$19,$29,0x20
	sd	$17,0x60($29)
	subu	$6,$6,$3
	sd	$16,0x50($29)
	addiu	$22,$0,0x74
	swc1	$f20,0xd0($29)
	move	$16,$0
	sd	$31,0xc0($29)
	lui	$1,0x40a0
	mtc1	$1,$f20
	lw	$5,4($2)
	sw	$4,0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_157108
