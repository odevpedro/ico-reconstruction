	.text
	.p2align 3
	.globl	eBrainSendMes
	.ent	eBrainSendMes
eBrainSendMes:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$3,0x6d
	addiu	$6,$3,0x710
	lw	$2,0x18($6)
	beq	$2,$4,loc_001919d8
	move	$7,$0
	addiu	$6,$6,0x18
	addiu	$7,$7,1
	nop	
loc_001919c0:
	slti	$2,$7,0x20
	beqz	$2,loc_001919d8
	addiu	$6,$6,0x1c
	lw	$2,0($6)
	bnel	$2,$4,loc_001919c0
	addiu	$7,$7,1
loc_001919d8:
	addiu	$2,$0,0x20
	bne	$7,$2,loc_001919ec
	addiu	$2,$0,0x1c
	b	loc_001919f8
	move	$3,$0
loc_001919ec:
	addiu	$3,$3,0x710
	mult	$2,$7,$2
	addu	$3,$2,$3
loc_001919f8:
	addiu	$2,$0,9
	beq	$5,$2,loc_00191a44
	sw	$5,0x10($3)
	slti	$2,$5,0xa
	beqz	$2,loc_00191a30
	slti	$2,$5,6
	beqz	$2,loc_00191a3c
	slti	$2,$5,4
	bnez	$2,loc_00191a3c
	lw	$2,-0x4b44($28)
	.word	0x1444001d
	nop	
	jr	$31
	sw	$0,-0x4b44($28)
loc_00191a30:
	addiu	$2,$0,0xa
	beq	$5,$2,loc_00191a4c
	lw	$2,-0x4b44($28)
loc_00191a3c:
	jr	$31
	nop	
loc_00191a44:
	jr	$31
	sw	$4,-0x4b44($28)
loc_00191a4c:
	beql	$2,$4,loc_00191a54
	sw	$0,-0x4b44($28)
loc_00191a54:
	lhu	$4,0($3)
	addiu	$2,$0,1
	.word	0x10820005
	addiu	$2,$0,2
	.word	0x10820007
	addiu	$2,$0,7
	jr	$31
	.set	macro
	.set	reorder
	.end	eBrainSendMes
