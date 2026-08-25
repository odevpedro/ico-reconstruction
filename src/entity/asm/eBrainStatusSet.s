	.text
	.p2align 3
	.globl	eBrainStatusSet
	.ent	eBrainStatusSet
eBrainStatusSet:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x10
	addiu	$2,$0,4
	beq	$5,$2,loc_00191908
	sd	$31,0($29)
	b	loc_00191990
	move	$2,$0
loc_00191908:
	lui	$3,0x6d
	addiu	$5,$3,0x710
	lw	$2,0x18($5)
	beqz	$2,loc_00191948
	move	$7,$0
	addiu	$6,$5,0x18
	addiu	$7,$7,1
	nop	
loc_00191928:
	slti	$5,$7,0x20
	beqz	$5,loc_00191964
	addiu	$6,$6,0x1c
	lw	$2,0($6)
	bnel	$2,$0,loc_00191928
	addiu	$7,$7,1
	b	loc_0019194c
	nop	
loc_00191948:
	addiu	$5,$0,1
loc_0019194c:
	beqz	$5,loc_00191964
	addiu	$2,$0,0x1c
	addiu	$3,$3,0x710
	mult	$2,$7,$2
	b	loc_00191968
	addu	$3,$2,$3
loc_00191964:
	move	$3,$0
loc_00191968:
	bnel	$3,$0,loc_00191984
	sw	$4,0x18($3)
	lui	$4,0x56
	jal	0x001a6e28
	addiu	$4,$4,-0x5220
	b	loc_00191990
	move	$2,$0
loc_00191984:
	move	$2,$3
	sh	$0,0($3)
	sw	$0,0x10($3)
loc_00191990:
	ld	$31,0($29)
	jr	$31
	addiu	$29,$29,0x10
	nop	
	.set	macro
	.set	reorder
	.end	eBrainStatusSet
