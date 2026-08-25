	.text
	.p2align 3
	.globl	boss_init
	.ent	boss_init
boss_init:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x50
	sd	$18,0x30($29)
	sd	$16,0x10($29)
	move	$18,$0
	sd	$31,0x40($29)
	jal	0x00202208
	sd	$17,0x20($29)
	jal	0x00203aa0
	addiu	$4,$0,1
	sw	$0,-0x4b28($28)
	addiu	$4,$0,0x21
	jal	0x0013eb50
	sw	$0,-0x4b2c($28)
	move	$16,$2
	beqz	$16,loc_001981cc
	lui	$17,0x56
loc_00198180:
	lui	$1,0x4f80
	mtc1	$1,$f0
	move	$4,$16
	sw	$0,0xc($29)
	move	$5,$29
	swc1	$f0,8($29)
	addiu	$18,$18,1
	swc1	$f0,0($29)
	jal	0x00104478
	swc1	$f0,4($29)
	lw	$2,0x15c($16)
	move	$4,$16
	jal	0x0013ebe0
	sw	$0,0x74($2)
	move	$16,$2
	bnez	$16,loc_00198180
	addiu	$4,$17,-0x3e98
	b	loc_001981d0
	nop	
loc_001981cc:
	addiu	$4,$17,-0x3e98
loc_001981d0:
	jal	0x001a6e28
	move	$5,$18
	blez	$18,loc_00198200
	move	$16,$0
	lui	$17,0x19
	addiu	$4,$17,0x7b30
loc_001981e8:
	jal	0x00203b78
	addiu	$5,$0,0x15
	addiu	$16,$16,1
	slt	$2,$16,$18
	bnez	$2,loc_001981e8
	addiu	$4,$17,0x7b30
loc_00198200:
	ld	$31,0x40($29)
	ld	$18,0x30($29)
	ld	$17,0x20($29)
	ld	$16,0x10($29)
	jr	$31
	addiu	$29,$29,0x50
	.set	macro
	.set	reorder
	.end	boss_init
