	.text
	.p2align 3
	.globl	boyAI_sub_145A54
	.ent	boyAI_sub_145A54
boyAI_sub_145A54:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui	$1,0x4000
	mtc1	$1,$f0
	.word	0x46001034
	nop	
	bc1tl	loc_00145a6c
	addiu	$18,$0,1
loc_00145a6c:
	beql	$17,$0,loc_00145a80
	sw	$0,0x404($3)
	lw	$2,0x404($3)
	addiu	$2,$2,1
	sw	$2,0x404($3)
loc_00145a80:
	beqz	$18,loc_00145a9c
	lw	$2,0x164($16)
	lw	$4,0x678($2)
	lw	$3,0x408($4)
	addiu	$3,$3,1
	b	loc_00145aa4
	sw	$3,0x408($4)
loc_00145a9c:
	lw	$3,0x678($2)
	sw	$0,0x408($3)
loc_00145aa4:
	lw	$3,0x164($16)
	lw	$4,0x678($3)
	lw	$2,0x404($4)
	slti	$2,$2,4
	bnez	$2,loc_00145acc
	ld	$2,0x408($4)
	ori	$3,$0,0x8000
	dsll	$3,$3,0x11
	b	loc_00145ae4
	or	$2,$2,$3
loc_00145acc:
	addiu	$3,$0,-2
	dsll	$3,$3,0x10
	ori	$3,$3,0xffff
	dsll	$3,$3,0x10
	ori	$3,$3,0xffff
	and	$2,$2,$3
loc_00145ae4:
	sd	$2,0x408($4)
	lw	$3,0x164($16)
	lw	$4,0x678($3)
	lw	$2,0x408($4)
	slti	$2,$2,4
	bnez	$2,loc_00145b10
	ld	$2,0x408($4)
	ori	$3,$0,0x8000
	dsll	$3,$3,0x12
	b	loc_00145b28
	or	$2,$2,$3
loc_00145b10:
	addiu	$3,$0,-3
	dsll	$3,$3,0x10
	ori	$3,$3,0xffff
	dsll	$3,$3,0x10
	ori	$3,$3,0xffff
	and	$2,$2,$3
loc_00145b28:
	sd	$2,0x408($4)
	lw	$2,0x15c($16)
	addiu	$5,$0,0x190
	lui	$4,0x56
	lw	$3,0x4a0($2)
	.set	macro
	.set	reorder
	.end	boyAI_sub_145A54
