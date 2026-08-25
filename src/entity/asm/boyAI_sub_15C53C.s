	.text
	.p2align 3
	.globl	boyAI_sub_15C53C
	.ent	boyAI_sub_15C53C
boyAI_sub_15C53C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	swc1	$f0,0x40($29)
	lwc1	$f0,4($2)
	move	$4,$18
	jal	0x0015ef88
	swc1	$f0,0x44($29)
	lwc1	$f0,8($2)
	lw	$4,0xa8($29)
	lui	$1,0x40a0
	mtc1	$1,$f12
	lw	$5,0xbc($29)
	jal	0x00243b18
	swc1	$f0,0x48($29)
	lw	$4,0xac($29)
	lw	$5,0xb8($29)
	jal	0x00243ad0
	lw	$6,0xa8($29)
	lw	$4,0($29)
	jal	0x00104360
	lw	$5,0xac($29)
	b	loc_0015c5e0
	nop	
	lw	$3,0($29)
	lui	$1,0x41a0
	mtc1	$1,$f1
	lw	$2,0x15c($3)
	lwc1	$f0,0x4ac($2)
	.word	0x46010034
	bc1f	loc_0015c5e0
	nop	
	jal	0x0015eff8
	lw	$4,-0x6e08($28)
	move	$16,$2
	jal	0x0015eff8
	move	$4,$18
	lw	$4,0xb0($29)
	move	$5,$16
	jal	0x001947d0
	move	$6,$2
	lw	$5,0xb0($29)
	jal	0x00106ab8
	move	$4,$18
loc_0015c5e0:
	jal	0x0015f248
	move	$4,$18
	.word	0x10400035
	lw	$4,-0x6e0c($28)
	lw	$4,0($29)
	jal	0x0015eff8
	nop	
	lwc1	$f0,0($2)
	lw	$4,0($29)
	jal	0x0015eff8
	.set	macro
	.set	reorder
	.end	boyAI_sub_15C53C
