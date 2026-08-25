	.text
	.p2align 3
	.globl	boyAI_sub_14C630
	.ent	boyAI_sub_14C630
boyAI_sub_14C630:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$4,$0,2
	or	$2,$2,$3
	b	loc_0014c688
	or	$2,$2,$4
	addiu	$3,$0,0x1000
	lui	$4,0x10
	lui	$6,0x20
	or	$2,$2,$3
	lui	$5,0x40
	or	$2,$2,$4
	lw	$3,-0x6e08($28)
	or	$2,$2,$6
	or	$4,$2,$5
	bne	$18,$3,loc_0014c68c
	sd	$4,0x480($16)
	ld	$2,0x20($16)
	lui	$3,0x60
	and	$2,$2,$3
	bnel	$2,$3,loc_0014c690
	ld	$5,0x470($16)
	addiu	$2,$0,-0x1001
	and	$2,$4,$2
loc_0014c688:
	sd	$2,0x480($16)
loc_0014c68c:
	ld	$5,0x470($16)
loc_0014c690:
	dsll	$2,$5,0x13
	dsra32	$2,$2,0
	andi	$2,$2,1
	.word	0x50400041
	ld	$2,0x480($16)
	lwc1	$f0,0x4a0($16)
	mtc1	$0,$f1
	lui	$1,0x42a0
	mtc1	$1,$f20
	.word	0x46010032
	nop	
	.word	0x45000012
	addiu	$5,$16,0x4a0
	lwc1	$f0,4($5)
	.word	0x46010032
	.word	0x4500000e
	.set	macro
	.set	reorder
	.end	boyAI_sub_14C630
