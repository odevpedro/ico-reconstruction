	.text
	.p2align 3
	.globl	boyAI_sub_14C854
	.ent	boyAI_sub_14C854
boyAI_sub_14C854:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	mtc1	$2,$f0
	cvt.s.w	$f0,$f0
	.word	0x46140034
	nop	
	bc1t	loc_0014c898
	addiu	$2,$0,1
	b	loc_0014c898
	move	$2,$0
	jal	0x00194508
	addiu	$5,$29,0x10
	mtc1	$2,$f0
	cvt.s.w	$f0,$f0
	.word	0x46140034
	nop	
	bc1t	loc_0014c898
	addiu	$2,$0,1
	move	$2,$0
loc_0014c898:
	andi	$2,$2,0xff
	beql	$2,$0,loc_0014c8bc
	lwc1	$f1,0x33c($16)
	ld	$2,0x480($16)
	ori	$3,$0,0x8000
	dsll32	$3,$3,0xc
	or	$2,$2,$3
	sd	$2,0x480($16)
	lwc1	$f1,0x33c($16)
loc_0014c8bc:
	lwc1	$f0,-0x7de8($28)
	.word	0x46010034
	nop	
	bc1tl	loc_0014c8e8
	ld	$2,0x478($16)
	ld	$2,0x18($16)
	dsrl32	$2,$2,0x1a
	andi	$2,$2,1
	beql	$2,$0,loc_0014c908
	lw	$9,0x2d4($16)
	ld	$2,0x478($16)
loc_0014c8e8:
	ori	$3,$0,0x8000
	dsll	$3,$3,0x12
	ori	$4,$0,0x8000
	dsll	$4,$4,0x13
	or	$2,$2,$3
	or	$2,$2,$4
	sd	$2,0x478($16)
	lw	$9,0x2d4($16)
loc_0014c908:
	andi	$2,$9,0x10
	.word	0x1040001d
	move	$5,$9
	lwc1	$f0,-0x7de4($28)
	ori	$3,$0,0x8000
	dsll	$3,$3,0x17
	ld	$2,0x478($16)
	.word	0x46010034
	or	$4,$2,$3
	.word	0x45000010
	sd	$4,0x478($16)
	ori	$2,$0,0x8000
	dsll	$2,$2,0x18
	ori	$3,$0,0x8000
	dsll	$3,$3,0x19
	or	$2,$4,$2
	or	$2,$2,$3
	sd	$2,0x478($16)
	lw	$3,0x164($18)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14C854
