; ModuleID = 'memelang'
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
  %t13 = load double, double* %t3
  %t15 = load double, double* %t6
  %t16 = fadd double 0.000000e+00, 1.000000
  %t14 = fadd double %t15, %t16
  %t12 = fsub double %t13, %t14
  %t24 = load double, double* %t3
  %t25 = load double, double* %t3
  %t23 = fsub double %t24, %t25
  %t28 = load double, double* %t3
  %t29 = fadd double 0.000000e+00, 2.000000
  %t27 = fmul double %t28, %t29
  %t32 = load double, double* %t3
  %t33 = fadd double 0.000000e+00, 10.000000
  %t31 = fmul double %t32, %t33
  %cond34 = fcmp one double %t23, 0.000000e+00
  br i1 %cond34, label %l20, label %l21

l20:
  br label %l22

l21:
  br label %l22

l22:
  %t35 = phi double [ %t27, %l20 ], [ %t31, %l21 ]
  ; Variable assignment: tmp = %t35
  %t36 = alloca double
  store double %t35, double* %t36
  %t38 = load double, double* %t36
  %t39 = fadd double 0.000000e+00, 5.000000
  %t37 = fadd double %t38, %t39
  %t42 = load double, double* %t6
  %t43 = fadd double 0.000000e+00, 10.000000
  %t41 = fsub double %t42, %t43
  %cond44 = fcmp one double %t12, 0.000000e+00
  br i1 %cond44, label %l9, label %l10

l9:
  br label %l11

l10:
  br label %l11

l11:
  %t45 = phi double [ %t37, %l9 ], [ %t41, %l10 ]
  ; Variable assignment: x = %t45
  %t46 = alloca double
  store double %t45, double* %t46
  %t49 = load double, double* %t46
  %t48 = call double @print(double %t49)
  ret double %t48
}
