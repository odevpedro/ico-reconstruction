	.text
	.p2align 3
	.globl	boyAI_sub_15D978
	.ent	boyAI_sub_15D978
boyAI_sub_15D978:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$4,$0,1
	.word	0x1000fffa
	nop	
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	sd	$31,0x10($29)
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$3,0x164($2)
	lw	$5,0x678($3)
	jal	0x0015d348
	addiu	$5,$5,0x7e0
	nop	
loc_0015d9b0:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015d9b0
	nop	
	nop	
	addiu	$29,$29,-0x30
	sw	$4,0($29)
	sd	$31,0x20($29)
	lui	$4,0x56
	lw	$2,0($29)
	addiu	$4,$4,-0x7480
	sd	$16,0x10($29)
	jal	0x001a6e28
	lw	$16,0x164($2)
	lw	$4,0($29)
	jal	0x00106ab8
	addiu	$5,$16,0x4b0
loc_0015da00:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015da00
	nop	
	nop	
	addiu	$29,$29,-0xc0
	sd	$20,0xa0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15D978
