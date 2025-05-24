; ModuleID = 'memelang'
declare double @print(double)
declare double @prints(i8* nocapture) nounwind


define double @main() {
entry:
  %t2 = fadd double 0.000000e+00, 10.000000
  ; Variable assignment: a = %t2
  br label %l4
l4:
  %t7 = fadd double %t2, 0.000000e+00 ; Load variable
  %while_cond = fcmp one double %t7, 0.000000e+00
  br i1 %while_cond, label %l5, label %l6
l5:
  %t11 = fadd double 0.000000e+00, 1.000000
  %t12 = fadd double 0.000000e+00, 1.000000
  %t10 = fsub double %t11, %t12
  %t2 = fadd double %t10, 0.00000e+00 ; Variable redefiniton: a = %t10
  %t15 = fadd double %t2, 0.000000e+00 ; Load variable
  %t14 = call double @print(double %t15)
  br label %l4
l6:
  %t16 = fadd double 0.000000e+00, 0.000000e+00
  ret double %t16
}
