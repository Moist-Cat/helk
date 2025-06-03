; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @pow(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind
declare i8* @malloc(i32)
declare void @free(i8*)


define double @main() {
l0:
  %t0 = fadd double 0.000000e+00, 20.000000
  ; Variable assignment: a = %t0
  %t1 = fadd double 0.000000e+00, 42.000000
  ; Load variable a
  %t2 = call double @print(double %t1)
  %t3 = fadd double 0.000000e+00, 7.000000
  %t4 = fadd double 0.000000e+00, 7.000000
  %t5 = fadd double 0.000000e+00, 6.000000
  %t6 = fmul double %t4, %t5
  ; Load variable a
  %t7 = call double @print(double %t6)
  %t8 = fadd double 0.000000e+00, 7.000000
  %t9 = fadd double 0.000000e+00, 7.000000
  %t10 = fadd double 0.000000e+00, 6.000000
  %t11 = fmul double %t9, %t10
  ; Load variable a
  %t12 = call double @print(double %t11)
  %t13 = fadd double 0.000000e+00, 0.000000
  ret double %t13
}
