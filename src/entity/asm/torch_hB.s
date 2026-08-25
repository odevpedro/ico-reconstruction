	.text
	.p2align 3
	.globl	torch_hB
	.ent	torch_hB
torch_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0xa0
	sd	$17,0x50($29)
	sd	$31,0x80($29)
	move	$17,$4
	sd	$19,0x70($29)
	sd	$18,0x60($29)
	sd	$16,0x40($29)
	swc1	$f20,0x90($29)
	lw	$3,0x15c($17)
	lw	$4,0($3)
	beqz	$4,loc_001f1d3c
	lw	$16,0x800($3)
	lw	$2,0x16c($4)
	bnel	$2,$0,loc_001f1d40
	lw	$3,0x800($3)
	jal	0x001f19f0
	move	$4,$17
	b	loc_001f2028
	ld	$31,0x80($29)
loc_001f1d3c:
	lw	$3,0x800($3)
loc_001f1d40:
	lw	$2,4($3)
	bnel	$2,$0,loc_001f1e78
	lw	$3,8($16)
	lw	$2,0x10($3)
	beqz	$2,loc_001f2028
	ld	$31,0x80($29)
	move	$4,$29
	jal	0x00104508
	move	$5,$17
	lui	$1,0x43c8
	mtc1	$1,$f20
	jal	0x0013eb50
	addiu	$4,$0,0xa
	move	$16,$2
	beqz	$16,loc_001f1dfc
	move	$4,$0
	addiu	$18,$29,0x10
	nop	
loc_001f1d88:
	beq	$16,$17,loc_001f1de8
	nop	
	lw	$2,0x15c($16)
	lw	$3,0x800($2)
	lw	$4,4($3)
	beqz	$4,loc_001f1de8
	nop	
	lw	$2,0x16c($16)
	beqz	$2,loc_001f1de8
	move	$5,$16
	jal	0x00104508
	addiu	$4,$29,0x10
	.word	0xda410000
	.word	0xdba20000
	.word	0x4be208ec
	.word	0x4bc318ea
	.word	0x4b0318c1
	.word	0x4b0318c2
	.word	0x48221800
	mtc1	$2,$f0
	.word	0x46140034
	nop	
	bc1t	loc_001f1dfc
	move	$4,$16
loc_001f1de8:
	jal	0x0013ebe0
	move	$4,$16
	move	$16,$2
	bnez	$16,loc_001f1d88
	move	$4,$0
loc_001f1dfc:
	beqz	$4,loc_001f2028
	ld	$31,0x80($29)
	lw	$2,0x15c($17)
	lw	$3,0x15c($4)
	lw	$19,0($2)
	beqz	$19,loc_001f1e5c
	lw	$18,0($3)
	beqz	$18,loc_001f1e5c
	lw	$4,-0x6e0c($28)
	jal	0x0014b048
	lw	$16,0x164($4)
	bnez	$2,loc_001f1e60
	move	$2,$0
	lw	$2,0x140($16)
	bne	$18,$2,loc_001f1e60
	move	$2,$0
	lw	$2,0x144($16)
	bne	$19,$2,loc_001f1e60
	move	$2,$0
	lui	$4,0x62
	jal	0x001a6e28
	addiu	$4,$4,-0x5908
	b	loc_001f1e60
	addiu	$2,$0,1
loc_001f1e5c:
	move	$2,$0
loc_001f1e60:
	bnez	$2,loc_001f2028
	ld	$31,0x80($29)
	jal	0x001f1868
	move	$4,$17
	b	loc_001f2028
	ld	$31,0x80($29)
loc_001f1e78:
	ori	$2,$0,0xfffe
	slt	$2,$2,$3
	bnez	$2,loc_001f1e8c
	addiu	$2,$3,1
	sw	$2,8($16)
loc_001f1e8c:
	beqz	$4,loc_001f1ed8
	move	$4,$29
	jal	0x00104508
	move	$5,$17
	addiu	$4,$29,0x10
	jal	0x00104508
	move	$5,$17
	lw	$4,0x15c($17)
	addiu	$5,$29,0x10
	addiu	$6,$4,0x1f0
	jal	0x00243ae8
	addiu	$4,$4,0x130
	lw	$4,0x15c($17)
	addiu	$5,$29,0x10
	jal	0x00105f00
	addiu	$4,$4,0x1f0
	move	$4,$17
	jal	0x001f1ad8
	move	$5,$29
loc_001f1ed8:
	lw	$3,8($16)
	lw	$2,0xc($16)
	slt	$2,$2,$3
	beqz	$2,loc_001f2028
	ld	$31,0x80($29)
	jal	0x001f19f0
	move	$4,$17
	lw	$2,0x15c($17)
	lw	$3,0x800($2)
	lw	$4,0x10($3)
	beqz	$4,loc_001f2028
	ld	$31,0x80($29)
	addiu	$4,$29,0x20
	jal	0x00104508
	move	$5,$17
	lui	$1,0x43c8
	mtc1	$1,$f20
	jal	0x0013eb50
	addiu	$4,$0,0xa
	move	$16,$2
	beqz	$16,loc_001f1fac
	addiu	$19,$29,0x30
	addiu	$18,$29,0x20
	nop	
loc_001f1f38:
	beq	$16,$17,loc_001f1f98
	nop	
	lw	$2,0x15c($16)
	lw	$3,0x800($2)
	lw	$4,4($3)
	beqz	$4,loc_001f1f98
	nop	
	lw	$2,0x16c($16)
	beqz	$2,loc_001f1f98
	move	$5,$16
	jal	0x00104508
	addiu	$4,$29,0x30
	.word	0xda610000
	.word	0xda420000
	.word	0x4be208ec
	.word	0x4bc318ea
	.word	0x4b0318c1
	.word	0x4b0318c2
	.word	0x48221800
	mtc1	$2,$f0
	.word	0x46140034
	nop	
	bc1t	loc_001f1fb0
	move	$4,$16
loc_001f1f98:
	jal	0x0013ebe0
	move	$4,$16
	move	$16,$2
	bnez	$16,loc_001f1f38
	nop	
loc_001f1fac:
	move	$4,$0
loc_001f1fb0:
	beqz	$4,loc_001f2028
	ld	$31,0x80($29)
	lw	$2,0x15c($17)
	lw	$3,0x15c($4)
	lw	$19,0($2)
	beqz	$19,loc_001f2010
	lw	$18,0($3)
	beqz	$18,loc_001f2010
	lw	$4,-0x6e0c($28)
	jal	0x0014b048
	lw	$16,0x164($4)
	bnez	$2,loc_001f2014
	move	$2,$0
	lw	$2,0x140($16)
	bne	$18,$2,loc_001f2014
	move	$2,$0
	lw	$2,0x144($16)
	bne	$19,$2,loc_001f2014
	move	$2,$0
	lui	$4,0x62
	jal	0x001a6e28
	addiu	$4,$4,-0x5908
	b	loc_001f2014
	addiu	$2,$0,1
loc_001f2010:
	move	$2,$0
loc_001f2014:
	bnez	$2,loc_001f2028
	ld	$31,0x80($29)
	jal	0x001f1868
	move	$4,$17
	ld	$31,0x80($29)
loc_001f2028:
	ld	$19,0x70($29)
	ld	$18,0x60($29)
	ld	$17,0x50($29)
	ld	$16,0x40($29)
	lwc1	$f20,0x90($29)
	jr	$31
	addiu	$29,$29,0xa0
	.set	macro
	.set	reorder
	.end	torch_hB
