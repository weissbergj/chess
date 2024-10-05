.text
.global turn_on_LED
turn_on_LED:
  lui a0, 0x2000
  li a1, 0x0001
  sw a1, 0x30(a0)
  li a2, 0b00000001
  sw a2, 0x40(a0)
  ret
