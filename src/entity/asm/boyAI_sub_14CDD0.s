	.text
	.p2align 3
	.globl	boyAI_sub_14CDD0
	.ent	boyAI_sub_14CDD0
boyAI_sub_14CDD0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	ori	$3,$0,0x8000
	dsll32	$3,$3,0
	b	loc_0014ce48
	ld	$2,0x478($16)
	lwc1	$f0,-0x7dd4($28)
	.word	0x46010034
	nop	
	bc1fl	loc_0014ce54
	ld	$2,0x478($16)
	lw	$2,0x330($16)
	addiu	$2,$2,-0x2e
	sltiu	$2,$2,0x59
	beqz	$2,loc_0014ce1c
	nop	
	ori	$3,$0,0x8000
	dsll32	$3,$3,2
	b	loc_0014ce48
	ld	$2,0x478($16)
loc_0014ce1c:
	bc1fl	loc_0014ce54
	ld	$2,0x478($16)
	lw	$3,0x330($16)
	slti	$2,$3,-0x86
	bnel	$2,$0,loc_0014ce54
	ld	$2,0x478($16)
	slti	$2,$3,-0x2d
	beqz	$2,loc_0014ce54
	ld	$2,0x478($16)
	ori	$3,$0,0x8000
	dsll32	$3,$3,3
loc_0014ce48:
	or	$2,$2,$3
	sd	$2,0x478($16)
	ld	$2,0x478($16)
loc_0014ce54:
	ori	$3,$0,0x8000
	dsll32	$3,$3,1
	or	$2,$2,$3
	sd	$2,0x478($16)
	andi	$2,$9,0x20
	beqz	$2,loc_0014ce84
	lui	$3,0x1000
	ld	$2,0x480($16)
	lui	$4,0x400
	or	$2,$2,$3
	or	$2,$2,$4
	sd	$2,0x480($16)
loc_0014ce84:
	ld	$2,0x480($16)
	dsll	$2,$2,2
	dsra32	$2,$2,0
	andi	$2,$2,1
	beql	$2,$0,loc_0014cec4
	lw	$2,0x164($18)
	ld	$2,0x470($16)
	dsll	$2,$2,2
	dsra32	$2,$2,0
	andi	$2,$2,1
	beqz	$2,loc_0014cecc
	addiu	$4,$0,1
	lw	$2,0x164($18)
	lw	$3,0x670($2)
	b	loc_0014cecc
	sw	$4,0x2c0($3)
loc_0014cec4:
	lw	$3,0x670($2)
	sw	$0,0x2c0($3)
loc_0014cecc:
	ld	$31,0x70($29)
	ld	$18,0x60($29)
	ld	$17,0x50($29)
	ld	$16,0x40($29)
	lwc1	$f20,0x80($29)
	jr	$31
	addiu	$29,$29,0x90
	addiu	$29,$29,-0x1e0
	lui	$3,0x56
	sd	$30,0x1b0($29)
	addiu	$5,$0,0x190
	.set	macro
	.set	reorder
	.end	boyAI_sub_14CDD0
