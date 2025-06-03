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
  ; Load variable a
  %t1 = call double @print(double %t0)
  %t2 = fadd double 0.000000e+00, 0.000000
  ret double %t2
}
