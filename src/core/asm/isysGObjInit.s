	.text
	.p2align 3
	.globl	isysGObjInit
	.ent	isysGObjInit
isysGObjInit:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x10
	lui	$3,0x28
	lui	$2,0x28
	sd	$31,0($29)
	addiu	$2,$2,0x1a90
	addiu	$3,$3,0x1a70
	addiu	$5,$0,7
	nop	
loc_0013ddc0:
	sw	$0,0($3)
	addiu	$5,$5,-1
	sw	$0,0($2)
	addiu	$3,$3,4
	addiu	$2,$2,4
	bgez	$5,loc_0013ddc0
	nop	
	jal	0x0013e4d0
	nop	
	ld	$31,0($29)
	sw	$0,-0x6724($28)
	sw	$0,-0x6720($28)
	j	0x13dd88
	addiu	$29,$29,0x10
	move	$5,$4
	bnel	$5,$0,loc_0013de10
	lw	$3,0x14($5)
	lui	$4,0x55
	j	0x1a6e28
	addiu	$4,$4,0x7a20
loc_0013de10:
	bnel	$3,$0,loc_0013de2c
	lw	$2,0x10($5)
	lw	$3,0x10($5)
	beql	$3,$0,loc_0013de48
	lbu	$6,0x18($5)
	b	loc_0013de34
	nop	
loc_0013de2c:
	sw	$2,0x10($3)
	lw	$3,0x10($5)
loc_0013de34:
	beql	$3,$0,loc_0013de48
	lbu	$6,0x18($5)
	lw	$2,0x14($5)
	sw	$2,0x14($3)
	lbu	$6,0x18($5)
loc_0013de48:
	lui	$2,0x28
	addiu	$2,$2,0x1a70
	sll	$3,$6,2
	addu	$3,$3,$2
	lw	$4,0($3)
	bne	$5,$4,loc_0013de74
	lui	$2,0x28
	lw	$2,0x10($5)
	sw	$2,0($3)
	lbu	$6,0x18($5)
	lui	$2,0x28
loc_0013de74:
	sll	$4,$6,2
	addiu	$2,$2,0x1a90
	addu	$4,$4,$2
	lw	$3,0($4)
	bne	$5,$3,loc_0013de94
	nop	
	lw	$2,0x14($5)
	sw	$2,0($4)
loc_0013de94:
	jr	$31
	nop	
	nop	
	.set	macro
	.set	reorder
	.end	isysGObjInit
