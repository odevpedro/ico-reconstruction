	.text
	.p2align 3
	.globl	boyAI_sub_14C63C
	.ent	boyAI_sub_14C63C
boyAI_sub_14C63C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
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
	bc1f	loc_0014c708
	addiu	$5,$16,0x4a0
	lwc1	$f0,4($5)
	.word	0x46010032
	bc1f	loc_0014c708
	nop	
	lwc1	$f0,8($5)
	.word	0x46010032
	bc1f	loc_0014c708
	nop	
	lwc1	$f0,0xc($5)
	.word	0x46010032
	nop	
	bc1f	loc_0014c708
	lui	$4,0x56
	jal	0x001a6e28
	addiu	$4,$4,-0x7e28
	.word	0x1000001e
	move	$2,$0
loc_0014c708:
	lui	$1,0xbf80
	mtc1	$1,$f12
	jal	0x00243b18
	.set	macro
	.set	reorder
	.end	boyAI_sub_14C63C
