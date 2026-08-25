	.text
	.p2align 3
	.globl	boyAI_sub_15197C
	.ent	boyAI_sub_15197C
boyAI_sub_15197C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	mov.s	$f12,$f1
	mov.s	$f12,$f2
	jal	0x00263fb0
	nop	
	lui	$1,0x56
	ld	$5,-0x79e0($1)
	jal	0x00262be8
	move	$4,$2
	lui	$1,0x56
	ld	$4,-0x79d8($1)
	jal	0x00262b80
	move	$5,$2
	jal	0x002633b8
	move	$4,$2
	lw	$4,0($29)
	mov.s	$f12,$f0
	jal	0x0014b330
	addiu	$5,$0,2
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000ffb6
	nop	
	nop	
	addiu	$29,$29,-0xa0
	sw	$4,0($29)
	sd	$21,0x80($29)
	sd	$19,0x60($29)
	sd	$18,0x50($29)
	lui	$19,0x27
	sd	$17,0x40($29)
	addiu	$21,$19,0x4ec0
	sd	$16,0x30($29)
	addiu	$18,$29,0x20
	sd	$31,0x90($29)
	addiu	$17,$29,0x10
	lw	$2,0($29)
	addiu	$16,$0,0x3c
	sd	$20,0x70($29)
	lw	$20,0x164($2)
	jal	0x0014a0d8
	.set	macro
	.set	reorder
	.end	boyAI_sub_15197C
