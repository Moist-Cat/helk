; ModuleID = 'memelang'
declare double @print(double)


define double @main() {
entry:
  %t2 = fadd double 0.000000e+00, 5.000000
  ; Variable assignment: a = %t2
  %t4 = fadd double 0.000000e+00, 5.000000
  ; Variable assignment: b = %t4
  %t8 = fadd double %t2, 0.000000e+00 ; Load variable
  %t10 = fadd double %t4, 0.000000e+00 ; Load variable
  %t11 = fadd double 0.000000e+00, 1.000000
  %t9 = fadd double %t10, %t11
  %t7 = fsub double %t8, %t9
  %cond15 = fcmp one double %t7, 0.000000e+00
  br i1 %cond15, label %l12, label %l13

l12:
  %t20 = fadd double %t2, 0.000000e+00 ; Load variable
  %t21 = fadd double %t2, 0.000000e+00 ; Load variable
  %t19 = fsub double %t20, %t21
  %cond25 = fcmp one double %t19, 0.000000e+00
  br i1 %cond25, label %l22, label %l23

l22:
  %t28 = fadd double %t2, 0.000000e+00 ; Load variable
  %t29 = fadd double 0.000000e+00, 2.000000
  %t27 = fmul double %t28, %t29
  br label %l24

l23:
  %t32 = fadd double %t2, 0.000000e+00 ; Load variable
  %t33 = fadd double 0.000000e+00, 10.000000
  %t31 = fmul double %t32, %t33
  br label %l24

l24:
  %t34 = phi double [ %t27, %l22 ], [ %t31, %l23 ]
  ; Variable assignment: tmp = %t34
  %t36 = fadd double %t34, 0.000000e+00 ; Load variable
  %t37 = fadd double 0.000000e+00, 5.000000
  %t35 = fadd double %t36, %t37
  br label %l14

l13:
  %t40 = fadd double %t4, 0.000000e+00 ; Load variable
  %t41 = fadd double 0.000000e+00, 10.000000
  %t39 = fsub double %t40, %t41
  br label %l14

l14:
  %t42 = phi double [ %t35, %l12 ], [ %t39, %l13 ]
  ; Variable assignment: x = %t42
  %t45 = fadd double %t42, 0.000000e+00 ; Load variable
  %t44 = call double @print(double %t45)
  ret double %t44
}
