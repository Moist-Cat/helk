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
  %t0 = fadd double 0.000000e+00, 5.000000
  ; Variable assignment: a = %t0
  %t1 = fadd double 0.000000e+00, 5.000000
  ; Variable assignment: b = %t1
  ; Load variable a
  ; Load variable b
  %t2 = fadd double 0.000000e+00, 1.000000
  %t3 = fadd double %t1, %t2
  %t4 = fsub double %t0, %t3
  %cond5 = fcmp one double %t4, 0.000000e+00
  br i1 %cond5, label %l1, label %l2

l1:
  ; Load variable a
  ; Load variable a
  %t6 = fsub double %t0, %t0
  %cond7 = fcmp one double %t6, 0.000000e+00
  br i1 %cond7, label %l4, label %l5

l4:
  ; Load variable b
  %t8 = fadd double 0.000000e+00, 2.000000
  %t9 = fmul double %t1, %t8
  br label %l6

l5:
  ; Load variable b
  %t10 = fadd double 0.000000e+00, 10.000000
  %t11 = fmul double %t1, %t10
  br label %l6

l6:
  %t12 = phi double [ %t9, %l4 ], [ %t11, %l5 ]
  ; Variable assignment: tmp = %t12
  ; Load variable tmp
  %t13 = fadd double 0.000000e+00, 5.000000
  %t14 = fadd double %t12, %t13
  br label %l3

l2:
  ; Load variable b
  %t15 = fadd double 0.000000e+00, 10.000000
  %t16 = fsub double %t1, %t15
  br label %l3

l3:
  %t17 = phi double [ %t14, %l6 ], [ %t16, %l2 ]
  ; Variable assignment: x = %t17
  ; Load variable x
  %t18 = call double @print(double %t17)
  %t19 = fadd double 0.000000e+00, 0.000000
  ret double %t19
}
