	.text
	.p2align 3
	.globl	eBrainProcess
	.ent	eBrainProcess
eBrainProcess:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0xa0
	lw	$2,-0x6e0c($28)
	sd	$31,0x90($29)
	sd	$20,0x80($29)
	sd	$19,0x70($29)
	sd	$18,0x60($29)
	sd	$17,0x50($29)
	sd	$16,0x40($29)
	sw	$0,-0x4b48($28)
	bnez	$2,loc_00190b70
	sw	$0,-0x4b4c($28)
	lw	$5,-0x6e08($28)
	beqz	$5,loc_00190d54
	ld	$31,0x90($29)
	b	loc_00190b8c
	nop	
loc_00190b70:
	jal	0x00154520
	nop	
	lw	$5,-0x6e0c($28)
	move	$4,$29
	jal	0x00104508
	sw	$2,-0x4b40($28)
	lw	$5,-0x6e08($28)
loc_00190b8c:
	beqz	$5,loc_00190ba0
	move	$17,$0
	jal	0x00104508
	addiu	$4,$29,0x10
	move	$17,$0
loc_00190ba0:
	lui	$20,0x6d
	addiu	$19,$29,0x20
	addiu	$18,$29,0x30
	addiu	$2,$0,0x1c
loc_00190bb0:
	addiu	$3,$20,0x710
	mult	$2,$17,$2
	addu	$16,$2,$3
	lw	$5,0x18($16)
	beqz	$5,loc_00190d40
	addiu	$8,$17,1
	jal	0x00104508
	move	$4,$19
	move	$4,$18
	move	$5,$19
	jal	0x00243ae8
	move	$6,$29
	move	$4,$18
	jal	0x00243950
	move	$5,$18
	swc1	$f0,8($16)
	move	$4,$18
	move	$5,$19
	jal	0x00243ae8
	addiu	$6,$29,0x10
	move	$4,$18
	jal	0x00243950
	move	$5,$18
	lhu	$4,0($16)
	bnez	$4,loc_00190d28
	swc1	$f0,0xc($16)
	lw	$2,-0x6e0c($28)
	beqz	$2,loc_00190cac
	lw	$3,-0x4b4c($28)
	lui	$2,0x6d
	addiu	$9,$2,0xa90
	move	$7,$16
	move	$6,$3
	lwc1	$f1,8($16)
	slt	$2,$4,$6
	beqz	$2,loc_00190c8c
	move	$4,$0
	lw	$5,-0x6e08($28)
	addiu	$8,$17,1
	sll	$2,$4,2
loc_00190c50:
	addu	$2,$2,$9
	lw	$3,0($2)
	lwc1	$f0,8($3)
	.word	0x46000834
	nop	
	bc1f	loc_00190c78
	addiu	$4,$4,1
	sw	$7,0($2)
	mov.s	$f1,$f0
	move	$7,$3
loc_00190c78:
	slt	$2,$4,$6
	bnez	$2,loc_00190c50
	sll	$2,$4,2
	b	loc_00190c9c
	addiu	$3,$6,1
loc_00190c8c:
	lw	$5,-0x6e08($28)
	addiu	$8,$17,1
	sll	$2,$4,2
	addiu	$3,$6,1
loc_00190c9c:
	addu	$2,$2,$9
	sw	$3,-0x4b4c($28)
	b	loc_00190cb4
	sw	$7,0($2)
loc_00190cac:
	lw	$5,-0x6e08($28)
	addiu	$8,$17,1
loc_00190cb4:
	beqz	$5,loc_00190d40
	lui	$2,0x6d
	lw	$3,-0x4b48($28)
	move	$5,$16
	addiu	$7,$2,0xb10
	lwc1	$f1,0xc($5)
	move	$6,$3
	blez	$3,loc_00190d10
	move	$2,$0
	move	$2,$3
	move	$4,$7
loc_00190ce0:
	lw	$3,0($4)
	lwc1	$f0,0xc($3)
	.word	0x46000834
	nop	
	bc1f	loc_00190d04
	addiu	$2,$2,-1
	sw	$5,0($4)
	mov.s	$f1,$f0
	move	$5,$3
loc_00190d04:
	bnez	$2,loc_00190ce0
	addiu	$4,$4,4
	move	$2,$6
loc_00190d10:
	sll	$2,$2,2
	addiu	$3,$6,1
	addu	$2,$2,$7
	sw	$3,-0x4b48($28)
	b	loc_00190d40
	sw	$5,0($2)
loc_00190d28:
	addiu	$2,$0,1
	bne	$4,$2,loc_00190d40
	addiu	$8,$17,1
	lw	$2,0x14($16)
	addiu	$2,$2,1
	sw	$2,0x14($16)
loc_00190d40:
	move	$17,$8
	slti	$2,$17,0x20
	bnez	$2,loc_00190bb0
	addiu	$2,$0,0x1c
	ld	$31,0x90($29)
loc_00190d54:
	ld	$20,0x80($29)
	ld	$19,0x70($29)
	ld	$18,0x60($29)
	ld	$17,0x50($29)
	ld	$16,0x40($29)
	jr	$31
	addiu	$29,$29,0xa0
	.set	macro
	.set	reorder
	.end	eBrainProcess
