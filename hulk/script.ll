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
  %t14 = fadd double 0.000000e+00, 1.000000
  %t15 = fadd double 0.000000e+00, 1.000000
  %t13 = fsub double %t14, %t15
  %t11 = fadd double %t12, %t13
  %t9 = fsub double %t10, %t11
  %cond16 = fcmp one double %t9, 0.000000e+00
  br i1 %cond16, label %l0, label %l1

l0:
  %t21 = load double, double* %t3
  %t22 = load double, double* %t3
  %t20 = fsub double %t21, %t22
  %cond23 = fcmp one double %t20, 0.000000e+00
  br i1 %cond23, label %l3, label %l4

l3:
  %t26 = load double, double* %t3
  %t27 = fadd double 0.000000e+00, 2.000000
  %t25 = fmul double %t26, %t27
  br label %l5

l4:
  %t30 = load double, double* %t3
  %t31 = fadd double 0.000000e+00, 10.000000
  %t29 = fmul double %t30, %t31
  br label %l5

l5:
  %t32 = phi double [ %t25, %l3 ], [ %t29, %l4 ]
  ; Variable assignment: tmp = %t32
  %t33 = alloca double
  store double %t32, double* %t33
  %t35 = load double, double* %t33
  %t36 = fadd double 0.000000e+00, 5.000000
  %t34 = fadd double %t35, %t36
  br label %l2

l1:
  %t40 = fadd double 0.000000e+00, 0.000000
  %cond41 = fcmp one double %t40, 0.000000e+00
  br i1 %cond41, label %l6, label %l7

l6:
  %t44 = fadd double 0.000000e+00, 1.000000
  %cond45 = fcmp one double %t44, 0.000000e+00
  br i1 %cond45, label %l9, label %l10

l9:
  %t47 = fadd double 0.000000e+00, 5.000000
  br label %l11

l10:
  %t49 = fadd double 0.000000e+00, 2.000000
  br label %l11

l11:
  %t50 = phi double [ %t47, %l9 ], [ %t49, %l10 ]
  br label %l8

l7:
  %t52 = fadd double 0.000000e+00, 7.000000
  br label %l8

l8:
  %t53 = phi double [ %t50, %l11 ], [ %t52, %l7 ]
  ; Variable assignment: y = %t53
  %t54 = alloca double
  store double %t53, double* %t54
  %t56 = load double, double* %t54
  %t57 = fadd double 0.000000e+00, 10.000000
  %t55 = fsub double %t56, %t57
  br label %l2

l2:
  %t58 = phi double [ %t34, %l5 ], [ %t55, %l8 ]
  ; Variable assignment: x = %t58
  %t59 = alloca double
  store double %t58, double* %t59
  %t62 = load double, double* %t59
  %t61 = call double @print(double %t62)
  ret double %t61
}
