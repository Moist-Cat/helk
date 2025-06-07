; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @pow(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind
declare i8* @malloc(i32)
declare void @free(i8*)
@.str.l0 = private unnamed_addr constant [1 x i8] c"\00", align 1
@l0 = alias i8, getelementptr inbounds ([1 x i8], [1 x i8]* @.str.l0, i64 0, i64 0)


define double @gcd(double %a, double %b) {
l1:
  br label %l2
l2:
  ; Variable redefiniton: b
  %t0 = phi double [%b, %l1], [%t1, %l2]
  ; Variable redefiniton: a
  %t2 = phi double [%a, %l1], [%t3, %l2]
  ; Load variable a
  ; Load variable b
  %t4 = call double @print(double %t0)
  %t5 = frem double %t2, %t4
  ; Variable assignment: m = %t5
  ; Load variable a
  %t1 = fadd double %t2, 0.000000e+00  ; Load variable
  ; Load variable m
  %t3 = fadd double %t5, 0.000000e+00  ; Load variable
  ; Load variable a
  ; Load variable a
  %while_cond = fcmp one double %t3, 0.000000e+00
  br i1 %while_cond, label %l2, label %l3
l3:
  %t6 = fadd double 0.000000e+00, 0.000000e+00
  ret double %t6
}

define double @f(double %a, i8* %b) {
l1:
  br label %l2
l2:
  ; Variable redefiniton: a
  %t0 = phi double [%a, %l1], [%t1, %l2]
  ; Load variable a
  %t2 = fadd double 0.000000e+00, 1.000000
  %t3 = fsub double %t0, %t2
  %t1 = fadd double %t3, 0.000000e+00  ; Load variable
  ; Load variable a
  %t4 = call double @print(double %t1)
  ; Load variable a
  %while_cond = fcmp one double %t1, 0.000000e+00
  br i1 %while_cond, label %l2, label %l3
l3:
  %t5 = fadd double 0.000000e+00, 0.000000e+00
  ret double %t5
}

define double @main() {
l1:
  %t1 = fadd double 0.000000e+00, 5.000000
  %t0 = call double @f(double %t1, i8* @l0)
  %t3 = fadd double 0.000000e+00, 14.000000
  %t4 = fadd double 0.000000e+00, 6.000000
  %t2 = call double @gcd(double %t3, double %t4)
  %t5 = fadd double 0.000000e+00, 0.000000
  ret double %t5
}
