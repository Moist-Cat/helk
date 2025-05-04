; ModuleID = 'memelang'
declare double @print(double)


define double @main() {
entry:
  %t3 = fadd double 0.000000e+00, 2.000000
  ; Variable assignment: x = %t3
  %t6 = fadd double %t3, 0.000000e+00 ; Load variable
  %t7 = fadd double 0.000000e+00, 5.000000
  %t5 = fadd double %t6, %t7
  ; Variable assignment: y = %t5
  %t10 = fadd double %t5, 0.000000e+00 ; Load variable
  %t9 = call double @print(double %t10)
  ret double %t9
}
