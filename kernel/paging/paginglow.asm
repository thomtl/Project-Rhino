[bits 32]
[section .text]

[global read_cr0]
read_cr0:
  mov eax, cr0
  ret

[global write_cr0]
write_cr0:
  push ebp
  mov ebp, esp
  mov eax, [ebp+8]
  mov cr0, eax
  pop ebp
  ret

[global read_cr3]
read_cr3:
  mov eax, cr3
  ret

[global write_cr3]
  push ebp
  mov ebp, esp
  mov eax, [ebp+8]
  mov cr3, eax
  pop ebp
  ret

[global read_cr4]
read_cr4:
  mov eax, cr4
  ret

[global write_cr4]
write_cr4
  push ebp
  mov ebp, esp
  mov eax, [ebp+8]
  mov cr4, eax
  pop ebp
  ret
