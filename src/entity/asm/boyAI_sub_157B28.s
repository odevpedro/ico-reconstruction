	.text
	.p2align 3
	.globl	boyAI_sub_157B28
	.ent	boyAI_sub_157B28
boyAI_sub_157B28:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$6,0x670($4)
	lw	$3,0x164($2)
	lwc1	$f0,0x28($29)
	lw	$4,0x670($3)
	swc1	$f0,0x348($6)
	lui	$1,0x42c8
	mtc1	$1,$f1
	lw	$3,0($29)
	lwc1	$f0,0x344($4)
	lw	$2,0x164($3)
	sub.s	$f0,$f0,$f1
	lw	$3,0x670($2)
	swc1	$f0,0x344($4)
	ldl	$2,0x17($29)
	ldr	$2,0x10($29)
	lw	$4,0x18($29)
	sdl	$2,0x357($3)
	sdr	$2,0x350($3)
	sw	$4,0x358($3)
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$3,0x164($2)
	lw	$6,0($29)
	lw	$7,0x670($3)
	jal	0x00194ff8
	addiu	$7,$7,0x330
	lw	$5,0($29)
	jal	0x001572f0
	move	$4,$17
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000ff8c
	nop	
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_157B28
