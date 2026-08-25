	.text
	.p2align 3
	.globl	boyAI_sub_15E9A8
	.ent	boyAI_sub_15E9A8
boyAI_sub_15E9A8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sd	$31,0x30($29)
	addiu	$4,$0,0xb1
	sd	$16,0x20($29)
	lw	$2,0($29)
	lw	$3,0x164($2)
	lwc1	$f1,0x190($3)
	lw	$2,0xc8($3)
	swc1	$f1,0x10($29)
	lwc1	$f0,0x194($3)
	swc1	$f0,0x14($29)
	lwc1	$f1,0x198($3)
	bne	$2,$4,loc_0015e9fc
	swc1	$f1,0x18($29)
	addiu	$16,$29,0x10
	lui	$1,0xbf80
	mtc1	$1,$f12
	move	$4,$16
	jal	0x00243b18
	move	$5,$16
	b	loc_0015ea00
	nop	
loc_0015e9fc:
	addiu	$16,$29,0x10
loc_0015ea00:
	lw	$4,0($29)
	jal	0x00106ab8
	move	$5,$16
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015ea00
	nop	
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	lui	$4,0x56
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$4,$4,-0x7408
	jal	0x00203aa0
	move	$4,$0
	.set	macro
	.set	reorder
	.end	boyAI_sub_15E9A8
