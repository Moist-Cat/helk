; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind


define double @main() {
entry:
  %t2 = fadd double 0.000000e+00, 5.000000
  ; Variable assignment: a = %t2
  %t3 = alloca double
  store double %t2, double* %t3
  %t5 = fadd double 0.000000e+00, 5.000000
  ; Variable assignment: b = %t5
  %t6 = alloca double
  store double %t5, double* %t6
  %t10 = load double, double* %t3
  %t12 = load double, double* %t6
  %t13 = fadd double 0.000000e+00, 1.000000
  %t11 = fadd double %t12, %t13
  %t9 = fsub double %t10, %t11
  %cond14 = fcmp one double %t9, 0.000000e+00
  br i1 %cond14, label %l0, label %l1

l0:
  %t19 = load double, double* %t3
  %t20 = load double, double* %t3
  %t18 = fsub double %t19, %t20
  %cond21 = fcmp one double %t18, 0.000000e+00
  br i1 %cond21, label %l3, label %l4

l3:
  %t24 = load double, double* %t3
  %t25 = fadd double 0.000000e+00, 2.000000
  %t23 = fmul double %t24, %t25
  br label %l5

l4:
  %t28 = load double, double* %t3
  %t29 = fadd double 0.000000e+00, 10.000000
  %t27 = fmul double %t28, %t29
  br label %l5

l5:
  %t30 = phi double [ %t23, %l3 ], [ %t27, %l4 ]
  ; Variable assignment: tmp = %t30
  %t31 = alloca double
  store double %t30, double* %t31
  %t33 = load double, double* %t31
  %t34 = fadd double 0.000000e+00, 5.000000
  %t32 = fadd double %t33, %t34
  br label %l2

l1:
  %t37 = load double, double* %t6
  %t38 = fadd double 0.000000e+00, 10.000000
  %t36 = fsub double %t37, %t38
  br label %l2

l2:
  %t39 = phi double [ %t32, %l5 ], [ %t36, %l1 ]
  ; Variable assignment: x = %t39
  %t40 = alloca double
  store double %t39, double* %t40
  %t43 = load double, double* %t40
  %t42 = call double @print(double %t43)
  ret double %t42
}
