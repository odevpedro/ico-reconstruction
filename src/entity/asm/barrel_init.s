	.text
	.p2align 3
	.globl	barrel_init
	.ent	barrel_init
barrel_init:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x60
	sw	$0,-0x6528($28)
	sd	$31,0x50($29)
	sd	$17,0x40($29)
	jal	0x0013eca8
	sd	$16,0x30($29)
	move	$4,$2
	.word	0x1080001f
	lui	$2,0x6b
	addiu	$17,$0,1
	addiu	$16,$2,-0x5380
	lw	$3,0x15c($4)
	beqz	$3,loc_001660b0
	nop	
	lw	$2,0x70($3)
	beqz	$2,loc_001660b0
	nop	
	lw	$2,0x16c($4)
	beqz	$2,loc_001660b0
	nop	
	lw	$2,4($4)
	bne	$2,$17,loc_001660b0
	nop	
	lw	$2,8($4)
	bltz	$2,loc_001660b0
	nop	
	lw	$2,0x74($3)
	beqz	$2,loc_001660b0
	lw	$2,-0x6528($28)
	sll	$3,$2,2
	addiu	$2,$2,1
	addu	$3,$3,$16
	sw	$2,-0x6528($28)
	sw	$4,0($3)
loc_001660b0:
	jal	0x0013ecf8
	.set	macro
	.set	reorder
	.end	barrel_init
