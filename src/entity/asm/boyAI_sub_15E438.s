	.text
	.p2align 3
	.globl	boyAI_sub_15E438
	.ent	boyAI_sub_15E438
boyAI_sub_15E438:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$4,$0
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	addiu	$29,$29,-0x20
	sd	$31,0x10($29)
	sw	$4,0($29)
	nop	
loc_0015e458:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015e458
	nop	
	nop	
	addiu	$29,$29,-0x40
	sw	$4,0($29)
	sd	$16,0x20($29)
	lw	$4,0($29)
	sd	$31,0x30($29)
	jal	0x0015ef88
	addiu	$16,$29,0x10
	lui	$1,0xbf80
	mtc1	$1,$f12
	move	$5,$2
	jal	0x00243b18
	move	$4,$16
	lw	$4,0($29)
	jal	0x00106ab8
	move	$5,$16
	nop	
loc_0015e4b8:
	lw	$4,0($29)
	addiu	$5,$0,3
	lui	$1,0xbf80
	mtc1	$1,$f12
	jal	0x0014b270
	move	$6,$0
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015e4b8
	nop	
	addiu	$29,$29,-0x20
	.set	macro
	.set	reorder
	.end	boyAI_sub_15E438
