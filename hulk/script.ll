; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind


define double @fib(double %n) {
l0:
  %t2 = fadd double 0.000000e+00, 2.000000
  %t3 = fsub double %n, %t2
  %t4 = fadd double 0.000000e+00, 0.000000
  %t1 = call double @max(double %t3, double %t4)
  %t5 = fadd double 0.000000e+00, 1.000000
  %t0 = call double @min(double %t1, double %t5)
  %cond6 = fcmp one double %t0, 0.000000e+00
  br i1 %cond6, label %l1, label %l2

l1:
  %t8 = fadd double 0.000000e+00, 1.000000
  %t9 = fsub double %n, %t8
  %t7 = call double @fib(double %t9)
  %t11 = fadd double 0.000000e+00, 2.000000
  %t12 = fsub double %n, %t11
  %t10 = call double @fib(double %t12)
  %t13 = fadd double %t7, %t10
  br label %l3

l2:
  %t14 = fadd double 0.000000e+00, 1.000000
  br label %l3

l3:
  %t15 = phi double [ %t13, %l1 ], [ %t14, %l2 ]
  ret double %t15
}

define double @main() {
l4:
  %t18 = fadd double 0.000000e+00, 10.000000
  %t17 = call double @fib(double %t18)
  %t16 = call double @print(double %t17)
  ret double %t16
}
