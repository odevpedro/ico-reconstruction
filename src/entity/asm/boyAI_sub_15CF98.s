	.text
	.p2align 3
	.globl	boyAI_sub_15CF98
	.ent	boyAI_sub_15CF98
boyAI_sub_15CF98:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bc1t	loc_0015cfac
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xe0
loc_0015cfac:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xe1
	b	loc_0015cfe8
	lw	$3,0xc8($17)
	bgez	$16,loc_0015cfcc
	move	$2,$16
	negu	$2,$2
loc_0015cfcc:
	slti	$2,$2,0xf
	beql	$2,$0,loc_0015cfe8
	lw	$3,0xc8($17)
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xe1
	lw	$3,0xc8($17)
loc_0015cfe8:
	addiu	$2,$0,0xd8
	beq	$3,$2,loc_0015d064
	sltiu	$2,$3,0xd9
	beqz	$2,loc_0015d00c
	addiu	$2,$0,0xd7
	beq	$3,$2,loc_0015d02c
	lw	$2,0x4ec0($22)
	b	loc_0015d09c
	nop	
loc_0015d00c:
	addiu	$2,$0,0xd9
	beq	$3,$2,loc_0015d028
	addiu	$2,$0,0xda
	beq	$3,$2,loc_0015d068
	lw	$2,0x4ec0($22)
	b	loc_0015d09c
	nop	
loc_0015d028:
	lw	$2,0x4ec0($22)
loc_0015d02c:
	lui	$3,0x27
	addiu	$3,$3,0x4ec0
	mult	$2,$2,$30
	lw	$5,4($3)
	lw	$3,0($29)
	beql	$5,$0,loc_0015d048
	break	0,7
loc_0015d048:
	lw	$4,0x164($3)
	subu	$2,$23,$2
	div	$0,$2,$5
	lw	$3,0x678($4)
	mflo	$2
	b	loc_0015d09c
	sw	$2,0x384($3)
loc_0015d064:
	lw	$2,0x4ec0($22)
loc_0015d068:
	lui	$3,0x27
	addiu	$3,$3,0x4ec0
	mult	$2,$2,$30
	lw	$5,4($3)
	lw	$3,0($29)
	beql	$5,$0,loc_0015d084
	break	0,7
loc_0015d084:
	lw	$4,0x164($3)
	subu	$2,$23,$2
	div	$0,$2,$5
	lw	$3,0x678($4)
	mflo	$2
	sw	$2,0x388($3)
loc_0015d09c:
	jal	0x00203aa0
	addiu	$4,$0,1
	.set	macro
	.set	reorder
	.end	boyAI_sub_15CF98
