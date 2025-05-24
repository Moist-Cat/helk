; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind
@.str.l0 = private unnamed_addr constant [30 x i8] c"We made this many iterations:\00", align 1
@l0 = alias i8, getelementptr inbounds ([30 x i8], [30 x i8]* @.str.l0, i64 0, i64 0)
@.str.l1 = private unnamed_addr constant [18 x i8] c"Final value of b:\00", align 1
@l1 = alias i8, getelementptr inbounds ([18 x i8], [18 x i8]* @.str.l1, i64 0, i64 0)


define double @main() {
l2:
  %t0 = fadd double 0.000000e+00, 5.000000
  ; Variable assignment: a = %t0
  %t1 = fadd double 0.000000e+00, 50.000000
  ; Variable assignment: b = %t1
  %t2 = fadd double 0.000000e+00, 10.000000
  ; Variable assignment: x = %t2
  br label %l3
l3:
  ; Variable redefiniton: x
  %t3 = phi double [%t2, %l2], [%t4, %l3]
  ; Variable redefiniton: b
  %t5 = phi double [%t1, %l2], [%t6, %l3]
  ; Load variable x
  %t7 = fadd double 0.000000e+00, 1.000000
  %t8 = fsub double %t3, %t7
  %t4 = fadd double %t8, 0.000000e+00  ; Load variable
  ; Load variable b
  ; Load variable a
  %t9 = fsub double %t5, %t0
  %t6 = fadd double %t9, 0.000000e+00  ; Load variable
  ; Load variable b
  ; Load variable a
  %t12 = fsub double %t6, %t0
  %t13 = fadd double 0.000000e+00, 0.000000
  %t11 = call double @max(double %t12, double %t13)
  %t14 = fadd double 0.000000e+00, 1.000000
  %t10 = call double @min(double %t11, double %t14)
  %while_cond = fcmp one double %t10, 0.000000e+00
  br i1 %while_cond, label %l3, label %l4
l4:
  %t15 = fadd double 0.000000e+00, 0.000000e+00
  %t16 = call double @prints(i8* @l0)
  %t18 = fadd double 0.000000e+00, 10.000000
  ; Load variable x
  %t19 = fsub double %t18, %t4
  %t17 = call double @print(double %t19)
  %t20 = call double @prints(i8* @l1)
  ; Load variable b
  %t21 = call double @print(double %t6)
  ret double %t21
}
