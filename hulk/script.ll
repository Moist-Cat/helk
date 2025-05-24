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
  %t10 = load double, double* %t3
  %t12 = load double, double* %t6
  %t13 = fadd double 0.000000e+00, 1.000000
  %t11 = fadd double %t12, %t13
  %t9 = fsub double %t10, %t11
  %cond17 = fcmp one double %t9, 0.000000e+00
  br i1 %cond17, label %l14, label %l15

l14:
  %t20 = fadd double 0.000000e+00, 5.000000
  ; Variable assignment: tmp = %t20
  %t21 = alloca double
  store double %t20, double* %t21
  %t23 = load double, double* %t21
  %t24 = fadd double 0.000000e+00, 5.000000
  %t22 = fadd double %t23, %t24
  br label %l16

l15:
  %t27 = load double, double* %t6
  %t28 = fadd double 0.000000e+00, 10.000000
  %t26 = fsub double %t27, %t28
  br label %l16

l16:
  %t29 = phi double [ %t22, %l14 ], [ %t26, %l15 ]
  ; Variable assignment: x = %t29
  %t30 = alloca double
  store double %t29, double* %t30
  %t33 = fadd double 0.000000e+00, 0.000000
  %cond37 = fcmp one double %t33, 0.000000e+00
  br i1 %cond37, label %l34, label %l35

l34:
  %t39 = fadd double 0.000000e+00, 5.000000
  br label %l36

l35:
  %t41 = fadd double 0.000000e+00, 2.000000
  br label %l36

l36:
  %t42 = phi double [ %t39, %l34 ], [ %t41, %l35 ]
  ; Variable assignment: y = %t42
  %t43 = alloca double
  store double %t42, double* %t43
  %t46 = load double, double* %t30
  %t45 = call double @print(double %t46)
  %t49 = load double, double* %t43
  %t48 = call double @print(double %t49)
  ret double %t48
}
