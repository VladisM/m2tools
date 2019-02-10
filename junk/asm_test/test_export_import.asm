.SECTION test_section_A
.EXPORT label_A
label_A: 	OR R0 R0 R0
		AND R1 R2 R3
		INC R3 R3

.SECTION test_section_B
.IMPORT label_A
		BZ R0 label_A
