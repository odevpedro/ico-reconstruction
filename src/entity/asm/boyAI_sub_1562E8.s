	.text
	.p2align 3
	.globl	boyAI_sub_1562E8
	.ent	boyAI_sub_1562E8
boyAI_sub_1562E8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	b	loc_001562f4
	nop	
loc_001562f0:
	lhu	$4,0x10($18)
loc_001562f4:
	bnel	$3,$4,loc_00156324
	addiu	$16,$16,0x18
	lw	$2,0x14($16)
	addiu	$4,$21,0x22d8
	and	$2,$2,$20
	sw	$2,0x14($16)
	.word	0x7fa50010
	jal	0x001a6e28
	.word	0x7fa80020
	ld.b	$w0,-0x58($0)
	.word	0x7ba50010
	addiu	$16,$16,0x18
loc_00156324:
	beql	$16,$0,loc_00156344
	move	$18,$17
	lh	$2,0x10($16)
	bne	$2,$19,loc_001562f0
	lhu	$3,0x10($16)
	b	loc_00156344
	move	$18,$17
	move	$18,$17
loc_00156344:
	beql	$17,$0,loc_00156368
	lw	$3,0x15c($23)
	lhu	$3,0x10($17)
	move	$4,$3
	sll	$2,$4,0x10
	sra	$2,$2,0x10
	.word	0x5453ffda
	lh	$2,0x12($18)
	lw	$3,0x15c($23)
loc_00156368:
	addiu	$4,$0,0x190
	addiu	$5,$5,0x5060
	lw	$2,0x4a0($3)
	mult	$2,$2,$4
	addu	$2,$2,$5
	lw	$3,0x188($2)
	srl	$3,$3,0x12
	andi	$3,$3,1
	beqz	$3,loc_001563a4
	addiu	$4,$8,-0x31f0
	lui	$3,4
	lw	$2,0x104($4)
	or	$2,$2,$3
	b	loc_001563bc
	sw	$2,0x104($4)
loc_001563a4:
	addiu	$2,$8,-0x31f0
	lui	$3,0xfffb
	lw	$4,0x104($2)
	ori	$3,$3,0xffff
	and	$4,$4,$3
	sw	$4,0x104($2)
loc_001563bc:
	lw	$7,8($29)
	addiu	$2,$0,0x1a
	lw	$3,0x30($7)
	.word	0x14620021
	ld	$31,0xc0($29)
	lui	$3,0x27
	addiu	$7,$0,0xa
	lw	$6,0x4ec0($3)
	addiu	$2,$0,0x3c
	addiu	$3,$3,0x4ec0
	addiu	$4,$0,3
	.word	0x00c73018
	lw	$5,4($3)
	beql	$5,$0,loc_001563f8
	break	0,7
loc_001563f8:
	subu	$2,$2,$6
	lw	$7,8($29)
	div	$0,$2,$5
	lw	$3,0x48($7)
	mflo	$2
	.set	macro
	.set	reorder
	.end	boyAI_sub_1562E8
