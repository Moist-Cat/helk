; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @pow(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind
declare i8* @malloc(i32)
declare void @free(i8*)

%struct.Point = type { double, double }
define %struct.Point* @Point_constructor(double %x, double %y) {
  %heap_ptr = call i8* @malloc(i32 16)
  %obj_ptr = bitcast i8* %heap_ptr to %struct.Point*
  %x_ptr = getelementptr %struct.Point, %struct.Point* %obj_ptr, i32 0, i32 0
  store double %x, double* %x_ptr
  %y_ptr = getelementptr %struct.Point, %struct.Point* %obj_ptr, i32 0, i32 1
  store double %y, double* %y_ptr
  ret %struct.Point* %obj_ptr
}

define double @Point_do(double %x) {
l0:
  %t0 = fadd double 0.000000e+00, 17.000000
  ; Load variable x
  %t1 = fadd double %t0, %x
  ret double %t1
}
%struct.Point2 = type { double, double, double }
define %struct.Point2* @Point2_constructor(double %x, double %y, double %z) {
  %heap_ptr = call i8* @malloc(i32 24)
  %obj_ptr = bitcast i8* %heap_ptr to %struct.Point2*
  %x_ptr = getelementptr %struct.Point2, %struct.Point2* %obj_ptr, i32 0, i32 0
  store double %x, double* %x_ptr
  %y_ptr = getelementptr %struct.Point2, %struct.Point2* %obj_ptr, i32 0, i32 1
  store double %y, double* %y_ptr
  %z_ptr = getelementptr %struct.Point2, %struct.Point2* %obj_ptr, i32 0, i32 2
  store double %z, double* %z_ptr
  ret %struct.Point2* %obj_ptr
}

define double @Point2_do(double %x) {
l0:
  ; Load variable x
  %t0 = fadd double 0.000000e+00, 1.000000
  %t1 = fadd double %x, %t0
  ret double %t1
}

define double @main() {
l0:
  %t1 = fadd double 0.000000e+00, 3.000000
  %t2 = fadd double 0.000000e+00, 4.000000
  %t0 = call %struct.Point* @Point_constructor(double %t1, double %t2)
  ; Variable assignment: a = %t0
  %t4_ptr = getelementptr %struct.Point, %struct.Point* %t0, i32 0, i32 0
  %t4 = load double, double* %t4_ptr
  %t3 = call double @print(double %t4)
  ; Variable assignment: c = %t3
  %t6_ptr = getelementptr %struct.Point, %struct.Point* %t0, i32 0, i32 1
  %t6 = load double, double* %t6_ptr
  %t5 = call double @print(double %t6)
  ; Variable assignment: d = %t5
  %t8_ptr = getelementptr %struct.Point, %struct.Point* %t0, i32 0, i32 1
  %t8 = load double, double* %t8_ptr
  %t7 = call double @print(double %t8)
  ; Variable assignment: e = %t7
  %t10_ptr = getelementptr %struct.Point, %struct.Point* %t0, i32 0, i32 0
  %t10 = load double, double* %t10_ptr
  %t9 = call double @print(double %t10)
  ; Variable assignment: f = %t9
  %t11 = fadd double 0.000000e+00, 0.000000
  %t13 = fadd double 0.000000e+00, 1.000000
  %t14 = fadd double 0.000000e+00, 1.000000
  %t12 = call %struct.Point* @Point_constructor(double %t13, double %t14)
  ; Variable assignment: b = %t12
  %t17 = fadd double 0.000000e+00, 9.000000
  %t16 = call double @Point_do(double %t17)
  %t15 = call double @print(double %t16)
  %t19 = fadd double 0.000000e+00, 1.000000
  %t20 = fadd double 0.000000e+00, 6.000000
  %t21 = fadd double 0.000000e+00, 9.000000
  %t18 = call %struct.Point2* @Point2_constructor(double %t19, double %t20, double %t21)
  ; Variable assignment: k = %t18
  %t24 = fadd double 0.000000e+00, 1.000000
  %t23 = call double @Point2_do(double %t24)
  %t22 = call double @print(double %t23)
  ; Variable assignment: lol = %t22
  %t26_ptr = getelementptr %struct.Point2, %struct.Point2* %t18, i32 0, i32 0
  %t26 = load double, double* %t26_ptr
  %t25 = call double @print(double %t26)
  ; Variable assignment: lmao = %t25
  %t28_ptr = getelementptr %struct.Point2, %struct.Point2* %t18, i32 0, i32 1
  %t28 = load double, double* %t28_ptr
  %t27 = call double @print(double %t28)
  ; Variable assignment: rofl = %t27
  %t30_ptr = getelementptr %struct.Point2, %struct.Point2* %t18, i32 0, i32 2
  %t30 = load double, double* %t30_ptr
  %t29 = call double @print(double %t30)
  ; Variable assignment: exdee = %t29
  %t31 = fadd double 0.000000e+00, 0.000000
  %t32 = fadd double 0.000000e+00, 0.000000
  ret double %t32
}
