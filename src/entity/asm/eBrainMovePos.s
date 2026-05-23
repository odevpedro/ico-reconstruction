	.text
	.p2align 3
	.globl	eBrainMovePos
	.ent	eBrainMovePos
eBrainMovePos:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lw	$4,-0x6e0c($28)
	beqz	$4,loc_00191e0c
	addiu	$5,$0,0xf
	jal	0x0014b2f0
	move	$19,$0
	lw	$3,0x18($16)
	movn	$19,$18,$2
loc_00191e0c:
	bnez	$3,loc_00191e50
	lw	$2,0xd20($17)
	lw	$4,-0x6e08($28)
	beqz	$4,loc_00191e30
	sw	$0,-0x6168($28)
	jal	0x0014b2f0
	addiu	$5,$0,9
	bnez	$2,loc_00191e3c
	addiu	$2,$0,1
loc_00191e30:
	lw	$2,-0x5974($28)
	beqz	$2,loc_00191e4c
	addiu	$2,$0,1
loc_00191e3c:
	b	loc_00191e4c
	sw	$2,-0x6168($28)
	sw	$0,-0x6168($28)
	move	$19,$0
loc_00191e4c:
	lw	$2,0xd20($17)
loc_00191e50:
	bnez	$2,loc_00191eac
	addiu	$16,$17,0xd20
	bnez	$19,loc_00191e6c
	addiu	$2,$20,0x4ec0
	lw	$2,-0x6168($28)
	beqz	$2,loc_00191ea0
	addiu	$2,$20,0x4ec0
loc_00191e6c:
	lw	$3,0x18($2)
	bnez	$3,loc_00191ea4
	lw	$2,0xd20($17)
	addiu	$4,$16,8
	addiu	$5,$0,2
	addiu	$6,$0,1
	addiu	$7,$0,2
	jal	0x00141c28
	move	$8,$0
	lw	$2,0x14($16)
	beqz	$2,loc_00191ea0
	addiu	$2,$0,1
	sw	$2,-0x616c($28)
loc_00191ea0:
	lw	$2,0xd20($17)
loc_00191ea4:
	.word	0x10400023
	ld	$31,0x60($29)
loc_00191eac:
	lw	$2,-0x6170($28)
	addiu	$5,$0,0x60
	addiu	$3,$0,0x400
	xori	$2,$2,1
	bnez	$19,loc_00191eec
	movz	$5,$3,$2
	lw	$2,-0x6168($28)
	.word	0x14400009
	addiu	$4,$17,0xd20
	addiu	$3,$17,0xd20
	lw	$2,4($3)
	subu	$2,$2,$5
	.word	0x0441000b
	sw	$2,4($3)
	.word	0x10000009
	sw	$0,4($3)
loc_00191eec:
	addiu	$4,$17,0xd20
	.set	macro
	.set	reorder
	.end	eBrainMovePos
