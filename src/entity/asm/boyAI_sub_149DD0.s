	.text
	.p2align 3
	.globl	boyAI_sub_149DD0
	.ent	boyAI_sub_149DD0
boyAI_sub_149DD0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,0x670($3)
	b	loc_00149de8
	addiu	$2,$2,0x314
	lw	$3,0x164($4)
	lw	$2,0x670($3)
	addiu	$2,$2,0x31c
loc_00149de8:
	sw	$2,0($29)
	lw	$5,8($29)
	slti	$2,$5,3
	beqz	$2,loc_00149e24
	lw	$3,0($29)
	blez	$5,loc_00149e24
	lw	$2,4($29)
	bnel	$2,$0,loc_00149e28
	lw	$4,4($3)
	lw	$3,0($29)
	lw	$2,4($3)
	bne	$2,$5,loc_00149e60
	ld	$31,0x30($29)
	b	loc_00149e28
	lw	$4,4($3)
loc_00149e24:
	lw	$4,4($3)
loc_00149e28:
	beqz	$4,loc_00149e3c
	nop	
	lw	$2,0($3)
	bnez	$2,loc_00149e4c
	slt	$2,$5,$4
loc_00149e3c:
	jal	0x00149c28
	move	$2,$29
	b	loc_00149e60
	ld	$31,0x30($29)
loc_00149e4c:
	bnez	$2,loc_00149e60
	ld	$31,0x30($29)
	jal	0x00149c28
	move	$2,$29
	ld	$31,0x30($29)
loc_00149e60:
	ld	$16,0x20($29)
	jr	$31
	addiu	$29,$29,0x40
	nop	
	beqz	$4,loc_00149ea0
	lw	$2,-0x6e0c($28)
	bne	$4,$2,loc_00149ea0
	nop	
	lw	$4,0x164($4)
	beqz	$4,loc_00149ea0
	nop	
	ld	$2,0x20($4)
	dsll	$2,$2,8
	dsra32	$2,$2,0
	jr	$31
	andi	$2,$2,1
loc_00149ea0:
	jr	$31
	move	$2,$0
	lw	$2,0x15c($4)
	addiu	$5,$0,1
	sw	$5,0x544($2)
	lw	$3,0x15c($4)
	sw	$5,0x54c($3)
	lw	$2,0x15c($4)
	sw	$5,0x548($2)
	lw	$3,0x15c($4)
	jr	$31
	sw	$5,0x7c($3)
	lw	$2,0x15c($4)
	.set	macro
	.set	reorder
	.end	boyAI_sub_149DD0
