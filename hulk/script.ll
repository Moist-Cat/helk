; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @pow(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind
declare i8* @malloc(i32)
declare void @free(i8*)
@.str.l0 = private unnamed_addr constant [5 x i8] c"blob\00", align 1
@l0 = alias i8, getelementptr inbounds ([5 x i8], [5 x i8]* @.str.l0, i64 0, i64 0)


define i8* @f(i8* %a) {
l1:
  ret i8* %a
}

define double @main() {
l1:
  %t1 = call i8* @f(i8* @l0)
  %t0 = call double @prints(i8* %t1)
  %t2 = fadd double 0.000000e+00, 0.000000
  ret double %t2
}
