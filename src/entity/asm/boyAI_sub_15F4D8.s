	.text
	.p2align 3
	.globl	boyAI_sub_15F4D8
	.ent	boyAI_sub_15F4D8
boyAI_sub_15F4D8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,0x20
	nop	
	addiu	$29,$29,-0x10
	sw	$4,0($29)
	jr	$31
	addiu	$29,$29,0x10
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	lw	$4,0($29)
	sd	$31,0x10($29)
	jal	0x00149ea8
	nop	
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	nop	
	addiu	$29,$29,-0x10
	addiu	$6,$0,-1
	sw	$4,0($29)
	lw	$4,0($29)
	lw	$5,0($29)
	lw	$2,0x164($4)
	lw	$3,0x670($2)
	sw	$6,0x2a0($3)
	lw	$2,0x164($5)
	lw	$3,0x670($2)
	sw	$0,0x2a4($3)
	jr	$31
	addiu	$29,$29,0x10
	nop	
	addiu	$29,$29,-0x20
	move	$5,$0
	sw	$4,0($29)
	lw	$4,0($29)
	sd	$31,0x10($29)
	jal	0x00158960
	nop	
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	lui	$4,0x63
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$4,$4,0x22f0
	lw	$3,0($29)
	lui	$4,0x28
	lw	$2,0x15c($3)
	addiu	$8,$4,0x2660
	ldl	$3,7($8)
	ldr	$3,0($8)
	lw	$6,8($8)
	sdl	$3,0x1c7($2)
	sdr	$3,0x1c0($2)
	sw	$6,0x1c8($2)
	lw	$4,0($29)
	jal	0x00158960
	move	$5,$0
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	addiu	$29,$29,-0x10
	.set	macro
	.set	reorder
	.end	boyAI_sub_15F4D8
