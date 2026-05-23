	.text
	.p2align 3
	.globl	boy_float_accum
	.ent	boy_float_accum
boy_float_accum:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x50
	lw	$4,-0x5650($28)
	swc1	$f22,0x40($29)
	swc1	$f21,0x38($29)
	swc1	$f20,0x30($29)
	sd	$17,0x10($29)
	sd	$31,0x20($29)
	move	$17,$0
	sd	$16,0($29)
	lui	$1,0x3f80
	mtc1	$1,$f22
	lwc1	$f21,-0x7718($28)
	lwc1	$f20,-0x7714($28)
	lw	$3,0($4)
loc_001c20e0:
	sll	$16,$17,2
	lw	$2,4($4)
	addiu	$17,$17,1
	addu	$3,$16,$3
	addu	$2,$16,$2
	lwc1	$f1,0($3)
	lwc1	$f0,0($2)
	add.s	$f1,$f1,$f0
	.word	0x4601b034
	nop	
	bc1f	loc_001c2140
	swc1	$f1,0($3)
	jal	0x00118a68
	nop	
	lw	$2,-0x5650($28)
	mul.s	$f0,$f0,$f21
	move	$4,$2
	add.s	$f0,$f0,$f20
	lw	$2,4($4)
	lw	$3,0($4)
	addu	$2,$16,$2
	swc1	$f0,0($2)
	addu	$3,$16,$3
	sw	$0,0($3)
loc_001c2140:
	slti	$2,$17,0x64
	bnel	$2,$0,loc_001c20e0
	lw	$3,0($4)
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	lwc1	$f22,0x40($29)
	lwc1	$f21,0x38($29)
	lwc1	$f20,0x30($29)
	jr	$31
	addiu	$29,$29,0x50
	nop	
	.set	macro
	.set	reorder
	.end	boy_float_accum
