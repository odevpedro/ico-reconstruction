	.text
	.p2align 3
	.globl	boyAI_sub_153C64
	.ent	boyAI_sub_153C64
boyAI_sub_153C64:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sdl	$3,0x447($2)
	sdr	$3,0x440($2)
	sw	$4,0x448($2)
loc_00153c70:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x117
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00153c70
	nop	
	nop	
	addiu	$29,$29,-0xa0
	sw	$4,0($29)
	sd	$20,0x80($29)
	lw	$4,0($29)
	addiu	$20,$0,0x1000
	sd	$19,0x70($29)
	sd	$18,0x60($29)
	addiu	$19,$0,2
	sd	$17,0x50($29)
	addiu	$18,$29,0x10
	sd	$31,0x90($29)
	addiu	$17,$29,0x20
	sd	$16,0x40($29)
	lw	$3,0($29)
	lw	$16,0x164($4)
	lw	$2,0x164($3)
	lw	$4,0x2c($2)
	ld	$3,0($4)
	sd	$3,0x10($29)
	ld	$2,8($4)
	sd	$2,0x18($29)
	ld	$3,0x10($4)
	sd	$3,0x20($29)
	ld	$2,0x18($4)
	sd	$2,0x28($29)
	ld	$3,0x20($4)
	sd	$3,0x30($29)
	ld	$2,0x28($4)
	sd	$2,0x38($29)
	nop	
	lw	$4,0($29)
	jal	0x0015d358
	move	$5,$17
	lw	$2,0x38($29)
	.word	0x10400005
	addiu	$5,$0,0xfa
	ld	$2,0x480($16)
	or	$2,$2,$20
	.set	macro
	.set	reorder
	.end	boyAI_sub_153C64
