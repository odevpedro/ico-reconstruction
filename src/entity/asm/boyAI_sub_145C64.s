	.text
	.p2align 3
	.globl	boyAI_sub_145C64
	.ent	boyAI_sub_145C64
boyAI_sub_145C64:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$2,$0,1
	bnel	$3,$2,loc_00145c80
	ld	$5,0x20($17)
	lw	$2,0x164($16)
	sb	$0,0x1ca($2)
	sw	$0,0x1a0($2)
	ld	$5,0x20($17)
loc_00145c80:
	dsll	$2,$5,9
	dsra32	$2,$2,0
	andi	$2,$2,1
	bnez	$2,loc_00145cdc
	addiu	$2,$0,0x190
	lw	$6,0x15c($16)
	lui	$4,0x56
	lw	$3,0x4a0($6)
	addiu	$4,$4,0x5060
	mult	$3,$3,$2
	addu	$3,$3,$4
	lw	$2,0x188($3)
	srl	$2,$2,0x1c
	andi	$2,$2,1
	beql	$2,$0,loc_00145cd0
	addiu	$4,$17,0x100
	lw	$2,0x4f8($6)
	beqz	$2,loc_00145cdc
	nop	
	addiu	$4,$17,0x100
loc_00145cd0:
	jal	0x00104508
	move	$5,$16
	ld	$5,0x20($17)
loc_00145cdc:
	lui	$2,0xfeff
	ori	$2,$2,0xffff
	lui	$3,0xff7f
	ori	$3,$3,0xffff
	and	$2,$5,$2
	move	$4,$16
	and	$2,$2,$3
	jal	0x00145638
	sd	$2,0x20($17)
	jal	0x001457a8
	move	$4,$16
	lw	$2,0x164($16)
	addiu	$4,$17,0x46c
	.set	macro
	.set	reorder
	.end	boyAI_sub_145C64
