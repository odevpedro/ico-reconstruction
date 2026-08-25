	.text
	.p2align 3
	.globl	boyAI_sub_14C080
	.ent	boyAI_sub_14C080
boyAI_sub_14C080:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x00194508
	addiu	$5,$29,0x10
	mtc1	$2,$f0
	cvt.s.w	$f0,$f0
	.word	0x46140034
	nop	
	bc1t	loc_0014c0a4
	addiu	$2,$0,1
	move	$2,$0
loc_0014c0a4:
	andi	$2,$2,0xff
	beql	$2,$0,loc_0014c0fc
	ld	$2,0x480($16)
	lw	$8,0x2d0($16)
	andi	$2,$8,8
	beql	$2,$0,loc_0014c0e0
	ld	$2,0x480($16)
	lw	$2,-0x6e08($28)
	beql	$2,$0,loc_0014c0e0
	ld	$2,0x480($16)
	lw	$2,0x164($2)
	lw	$3,0x678($2)
	lw	$4,0x3a0($3)
	bnez	$4,loc_0014c0fc
	ld	$2,0x480($16)
loc_0014c0e0:
	addiu	$3,$0,1
	b	loc_0014c0f4
	or	$2,$2,$3
	addiu	$2,$0,1
	or	$2,$5,$2
loc_0014c0f4:
	sd	$2,0x480($16)
	ld	$2,0x480($16)
loc_0014c0fc:
	addiu	$3,$0,0x10
	addiu	$4,$0,0x20
	addiu	$8,$0,0x40
	or	$2,$2,$3
	addiu	$5,$0,0x80
	or	$2,$2,$4
	addiu	$3,$0,0x100
	or	$2,$2,$8
	addiu	$4,$0,0x200
	or	$2,$2,$5
	addiu	$6,$0,0x400
	or	$2,$2,$3
	ori	$9,$0,0x8000
	dsll32	$9,$9,4
	or	$2,$2,$4
	ori	$3,$0,0x8000
	.set	macro
	.set	reorder
	.end	boyAI_sub_14C080
