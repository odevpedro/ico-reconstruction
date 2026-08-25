	.text
	.p2align 3
	.globl	boyAI_sub_1562D4
	.ent	boyAI_sub_1562D4
boyAI_sub_1562D4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x5200001b
	move	$18,$17
	lh	$2,0x10($16)
	beq	$2,$19,loc_00156340
	lhu	$3,0x10($16)
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
	.word	0x52000007
	move	$18,$17
	lh	$2,0x10($16)
	bne	$2,$19,loc_001562f0
	lhu	$3,0x10($16)
	.word	0x10000002
	move	$18,$17
loc_00156340:
	move	$18,$17
	.set	macro
	.set	reorder
	.end	boyAI_sub_1562D4
