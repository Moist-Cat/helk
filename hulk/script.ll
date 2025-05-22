; ModuleID = 'memelang'
declare double @print(double)


define double @main() {
entry:
  %t3 = fadd double 0.000000e+00, 2.000000
  ; Variable assignment: x = %t3
  %t4 = fadd double %t3, 0.000000e+00 ; Load variable
  %t8 = fadd double 0.000000e+00, 1.000000
  %t9 = fadd double 0.000000e+00, 1.000000
  %t7 = fadd double %t8, %t9
  %t6 = call double @print(double %t7)
  ret double %t6
}
